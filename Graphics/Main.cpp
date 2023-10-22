#define AIE_SHOWCASE

#include "Application.h"
#include "LogUtils.h"
#include "DungeonHelpers.h"

int main(int argc, char * argv[])
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
	if (argc > 1)
		app->LaunchArgumentPreLoad(argv[1]);

	app->ConstructWindow();
	app->LoadResourceManagers();
	app->DoLoadingScreen();

	if (app->s_mode != Application::Mode::Art)
		app->InitialiseAdditionalGameAssets();

	if (argc > 1)
		app->LaunchArgumentPostLoad(argv[1]);
	// This is the default mode without any command line arguments. It launches the game proper for the standard user experieance.
	if (app->s_mode == Application::Mode::Game)
	{
		app->InitialiseMenu();
	}
	
	app->Run();
}

