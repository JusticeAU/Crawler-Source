#include "Application.h"
#include "LogUtils.h"

int main(int argc, char * argv[])
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
			app->LaunchArgument(argv[i]);
	}
	app->Run();
}

