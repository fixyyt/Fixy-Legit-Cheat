#include "gui.h"

#include"cheats.h"
#include"globals.h"
#include<iostream>
#include<fstream>

#include <thread>

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{
	// create gui
	gui::CreateHWindow("Cheat Menu");
	gui::CreateDevice();
	gui::CreateImGui();
	Memory mem{ "csgo.exe" };

	globals::clientAdress = mem.GetModuleAddress("client.dll");
	globals::engineAdress = mem.GetModuleAddress("engine.dll");
	
	std::thread(cheats::VisualsThread, mem).detach();


	while (gui::isRunning)
	{
		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	// destroy gui
	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();
	return EXIT_SUCCESS;
}
