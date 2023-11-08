#include "Application.h"
#include "LogUtils.h"
#include "DungeonHelpers.h"

#ifndef RELEASE
int main(int argc, char * argv[])
#else
#include "Windows.h"
#include "StringUtils.h"
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif // !RELEASE
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
#ifndef RELEASE
	if (argc > 1)
		app->LaunchArgumentPreLoad(argv[1]);
#else
	std::string commands = lpCmdLine;
	if (commands.size() > 0)
	{
		std::string* split = StringUtils::Split(commands, " ");
		app->LaunchArgumentPreLoad(split[0].c_str());
		delete[] split;
	}
#endif // !RELEASE
	app->ConstructWindow();
	app->LoadResourceManagers();
	app->InitialiseInput();
	app->DoLoadingScreen();

	if (app->s_mode != Application::Mode::Art)
		app->InitialiseAdditionalGameAssets();

#ifndef RELEASE
	if (argc > 1)
		app->LaunchArgumentPostLoad(argv[1]);
#else
	if (commands.size() > 0)
	{
		std::string* split = StringUtils::Split(commands, " ");
		app->LaunchArgumentPostLoad(split[0].c_str());
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

