#include "SDL.h"

#include <ctime>
#include "Renderer.h"

int main(int argv, char** args)
{
	srand(static_cast<unsigned int>(time(nullptr)));

	Window window{ "GEOA GRAD WORK", 1280.f , 720.f };
	Renderer Renderer{ window };
	Renderer.Run();

	return 0;
}

