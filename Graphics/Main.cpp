#include "Application.h"
#include "LogUtils.h"
#include "DungeonHelpers.h"

// We have two different entry points here depending on if we want to show the console window or not.
// Commandline arguments have to be handled differently depending on the entry point!
#ifndef RELEASE
int main(int argc, char * argv[])
#else
#include "Windows.h"
#include "StringUtils.h"
int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#endif // !RELEASE
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();

	// Run Command Line Arguments
#ifndef RELEASE
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			app->LaunchArgumentPreLoad(argv[i]);
		}
	}
#else
	std::string commands = lpCmdLine;
	if (commands.size() > 0)
	{
		int count = 1;
		std::string* split = StringUtils::Split(commands, " ", &count);
		for (int i = 0; i < count; i++)
		{
			app->LaunchArgumentPreLoad(split[i].c_str());
		}
		delete[] split;
	}
#endif // !RELEASE
	app->ConstructWindow();
	app->LoadResourceManagers();
	app->InitialiseInput();
	app->DoLoadingScreen(); // This will load every texture and render a progress bar whilst this is happening until all textures are loaded.

	if (app->s_mode != Application::Mode::Art)
		app->InitialiseAdditionalGameAssets();

#ifndef RELEASE
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			app->LaunchArgumentPostLoad(argv[i]);
		}
	}
#else
	if (commands.size() > 0)
	{
		int count = 1;
		std::string* split = StringUtils::Split(commands, " ", &count);
		for (int i = 0; i < count; i++)
		{
			app->LaunchArgumentPostLoad(split[i].c_str());
		}
		delete[] split;
	}

#endif // !RELEASE

	// This is the default mode without any command line arguments. It launches the game proper for the standard user experieance.
	if (app->s_mode == Application::Mode::Game)
	{
		app->InitialiseMenu();
	}
	
	app->Run();
}

