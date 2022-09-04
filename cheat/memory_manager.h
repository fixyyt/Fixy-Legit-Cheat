#pragma once
#include <windows.h>
#include <vector>
#include <TlHelp32.h>
#include <string>

class CMemoryManager
{
private:
	HANDLE m_hProcess;
	DWORD m_dwProcessId;
	std::vector<MODULEENTRY32> m_Modules; // module names

public:
	bool Attach(const std::string& strProcessName);
	bool GrabModule(const std::string& strModuleName);
	CMemoryManager(); // doesn't attach to any process
	CMemoryManager(const std::string& strProcessName = "csgo.exe"); // attaches to specific process

	HANDLE GetHandle();
	DWORD GetProcId();
	std::vector<MODULEENTRY32> GetModules();

	template<class T>
	inline bool Read(DWORD dwAddress, T& value) {
		return ReadProcessMemory(m_hProcess, reinterpret_cast<LPVOID>(dwAddress), &value, sizeof(T), NULL) == TRUE;
	}

	template<class T>
	inline bool Write(DWORD dwAddress, const T& value) {
		return WriteProcessMemory(m_hProcess, reinterpret_cast<LPVOID>(dwAddress), &value, sizeof(T), NULL) == TRUE;
	}
};
