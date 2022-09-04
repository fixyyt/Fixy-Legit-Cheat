#include "memory_manager.h"

/*Default constructor, not attached to any process
*/
CMemoryManager::CMemoryManager() {
	m_hProcess = INVALID_HANDLE_VALUE;
	m_dwProcessId = 0;
	m_Modules.clear();
}

/*Constructor that attaches to a specific process
@param strProcessName the name of the process, default "csgo.exe"
*/
CMemoryManager::CMemoryManager(const std::string& strProcessName) {
	m_hProcess = INVALID_HANDLE_VALUE;
	m_dwProcessId = 0;
	m_Modules.clear();
	// attach and throw if the function failed so we can manage the fail
	if (!Attach(strProcessName)) {
		throw 20;
	}
}

HANDLE CMemoryManager::GetHandle() {
	return m_hProcess;
}

DWORD CMemoryManager::GetProcId() {
	return m_dwProcessId;
}

std::vector<MODULEENTRY32> CMemoryManager::GetModules() {
	return m_Modules;
}


/*Attach to a process
@param strProcessName the name of the process
@return true on success, false on failure
*/
bool CMemoryManager::Attach(const std::string& strProcessName) {
	// remember to close this later
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapshot == INVALID_HANDLE_VALUE) return false;
	PROCESSENTRY32 ProcEntry;
	ProcEntry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &ProcEntry)) {
		// make sure to enable multi-byte character set in project configuration
		if (!strcmp(ProcEntry.szExeFile, strProcessName.c_str())) {
			// process has been found, now open a handle to it and return it
			CloseHandle(hSnapshot);
			// PROCESS_ALL_ACCESS grants read/write privileges
			m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcEntry.th32ProcessID);
			m_dwProcessId = ProcEntry.th32ProcessID;
			return true;
		}
		else {
			// first process is not the correct one, now loop through the rest of the processes
			while (Process32Next(hSnapshot, &ProcEntry))
			{
				// We do the same check we did for Process32First
				if (!strcmp(ProcEntry.szExeFile, strProcessName.c_str()))
				{
					m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcEntry.th32ProcessID);
					m_dwProcessId = ProcEntry.th32ProcessID;
					return true;
				}
			}
		}
	}

	CloseHandle(hSnapshot);
	return false;
}

bool CMemoryManager::GrabModule(const std::string& strModuleName) {
	// basically the same structure as Attach()
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_dwProcessId);
	if (hSnapshot == INVALID_HANDLE_VALUE) return false;
	MODULEENTRY32 ModEntry;
	ModEntry.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &ModEntry)) {
		if (!strcmp(ModEntry.szModule, strModuleName.c_str())) {
			CloseHandle(hSnapshot);
			m_Modules.push_back(ModEntry);
			return true;
		}
		else {
			while (Module32Next(hSnapshot, &ModEntry)) {
				if (!strcmp(ModEntry.szModule, strModuleName.c_str())) {
					CloseHandle(hSnapshot);
					m_Modules.push_back(ModEntry);
					return true;
				}
			}
		}
	}
	else {
		CloseHandle(hSnapshot);
		return false;
	}
	CloseHandle(hSnapshot);
	return false;
}