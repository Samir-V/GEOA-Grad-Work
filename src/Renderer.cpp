#include <iostream>
#include <algorithm>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include "SDL_surface.h"
#include <chrono>
#include "Renderer.h"

#include <execution>

#include "FlyFish.h"
#include "utils.h"
#include "structs.h"
#include "Camera.h"

Renderer::Renderer(const Window& window)
	: m_Window{ window }
	, m_Viewport{ 0,0,window.width,window.height }
	, m_pWindow{ nullptr }
	, m_pContext{ nullptr }
	, m_Initialized{ false }
	, m_MaxElapsedSeconds{ 0.1f }
{
	InitializeRenderer();
}

Renderer::~Renderer()
{
	CleanupRenderer();
}

void Renderer::InitializeRenderer()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "Renderer::Initialize( ), error when calling SDL_Init: " << SDL_GetError() << std::endl;
		return;
	}

	// Use OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// Create window
	m_pWindow = SDL_CreateWindow(
		m_Window.title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		int(m_Window.width),
		int(m_Window.height),
		SDL_WINDOW_OPENGL);
	if (m_pWindow == nullptr)
	{
		std::cerr << "Renderer::Initialize( ), error when calling SDL_CreateWindow: " << SDL_GetError() << std::endl;
		return;
	}

	// Create OpenGL context 
	m_pContext = SDL_GL_CreateContext(m_pWindow);
	if (m_pContext == nullptr)
	{
		std::cerr << "Renderer::Initialize( ), error when calling SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
		return;
	}

	// Set the swap interval for the current OpenGL context,
	// synchronize it with the vertical retrace
	if (m_Window.isVSyncOn)
	{
		if (SDL_GL_SetSwapInterval(1) < 0)
		{
			std::cerr << "Renderer::Initialize( ), error when calling SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
			return;
		}
	}
	else
	{
		SDL_GL_SetSwapInterval(0);
	}

	// Set the Projection matrix to the identity matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up a two-dimensional orthographic viewing region.
	glOrtho(0, m_Window.width, 0, m_Window.height, -1, 1); // y from bottom to top

	// Set the viewport to the client window area
	// The viewport is the rectangular region of the window where the image is drawn.
	glViewport(0, 0, int(m_Window.width), int(m_Window.height));

	// Set the Modelview matrix to the identity matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Enable color blending and use alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		std::cerr << "Renderer::Initialize( ), error when calling TTF_Init: " << TTF_GetError() << std::endl;
		return;
	}


	m_pBuffer = SDL_GetWindowSurface(m_pWindow);

	SDL_GetWindowSize(m_pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_CameraUPtr = std::make_unique<Camera>(ThreeBlade(0.f, 0.f, 0.f), 60.f);

	m_Initialized = true;

	m_TestPlane = Plane(OneBlade{0, 0, 0, 1}, Color4f{0.4f, 0.1f, 0.8f, 1.0f});
}

void Renderer::Run()
{
	if (!m_Initialized)
	{
		std::cerr << "BaseGame::Run( ), BaseGame not correctly initialized, unable to run the BaseGame\n";
		std::cin.get();
		return;
	}

	// Main loop flag
	bool quit{ false };

	// Set start time
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point fpsTimer = t1;
	int frameCount = 0;

	//The event loop
	SDL_Event e{};
	while (!quit)
	{
		// Poll next event from queue
		while (SDL_PollEvent(&e) != 0)
		{
			// Handle the polled event
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				this->ProcessKeyDownEvent(e.key);
				break;
			case SDL_KEYUP:
				this->ProcessKeyUpEvent(e.key);
				break;
			case SDL_MOUSEMOTION:
				e.motion.y = int(m_Window.height) - e.motion.y;
				this->ProcessMouseMotionEvent(e.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
				e.button.y = int(m_Window.height) - e.button.y;
				this->ProcessMouseDownEvent(e.button);
				break;
			case SDL_MOUSEBUTTONUP:
				e.button.y = int(m_Window.height) - e.button.y;
				this->ProcessMouseUpEvent(e.button);
				break;
			}
		}

		if (!quit)
		{
			// Get current time
			std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

			// Calculate elapsed time
			float elapsedSeconds = std::chrono::duration<float>(t2 - t1).count();

			// Update current time
			t1 = t2;

			// Prevent jumps in time caused by break points
			elapsedSeconds = std::min(elapsedSeconds, m_MaxElapsedSeconds);

			// Call the object 's Update function, using time in seconds (!)
			this->Update(elapsedSeconds);

			// Draw in the back buffer
			this->Render();

			// Calculate fps
			++frameCount;
			auto fpsNow = std::chrono::steady_clock::now();
			float fpsElapsed = std::chrono::duration<float>(fpsNow - fpsTimer).count();
			if (fpsElapsed >= 1.0f)
			{
				std::cout << "FPS: " << frameCount / fpsElapsed << std::endl;
				frameCount = 0;
				fpsTimer = fpsNow;
			}
		}
	}
}

void Renderer::CleanupRenderer()
{
	SDL_GL_DeleteContext(m_pContext);

	SDL_DestroyWindow(m_pWindow);
	m_pWindow = nullptr;

	//Quit SDL subsystems
	TTF_Quit();
	SDL_Quit();

}

void Renderer::Update(float elapsedSec)
{
	//auto rot = Motor::Rotation(3.0f * elapsedSec, TwoBlade(0, 0, 0, 0, 0, 0));

	//m_TestPlane.PlaneGenerators = (rot * m_TestPlane.PlaneGenerators * ~rot).Grade1().Normalized();
}

void Renderer::Render()
{
	// Camera info

	const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

	const float FOVAngleRad{ m_CameraUPtr->GetFOVAngle() * utils::g_Pi / 180.0f };

	const float FOVScalar{ tanf(FOVAngleRad / 2.0f) };

	// Render info

	const uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };

	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);

	for (uint32_t index{}; index < amountOfPixels; ++index)
	{
		pixelIndices.emplace_back(index);
	}

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](const int index)
		{
			RenderPixel(index, FOVScalar, aspectRatio, m_CameraUPtr.get());
		});


	/*for (int index = 0; index < pixelIndices.size(); ++index)
	{
		RenderPixel(index, FOVScalar, aspectRatio, m_CameraUPtr.get());
	}*/

	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(uint32_t pixelIndex, float fov, float aspectRatio, const Camera* pCamera) const
{
	const uint32_t px{ pixelIndex % m_Width };
	const uint32_t py{ pixelIndex / m_Width };

	const float rx{ px + 0.5f };
	const float ry{ py + 0.5f };
	const float camX{ (2 * (rx / static_cast<float>(m_Width)) - 1) * aspectRatio * fov };
	const float camY{ (1 - 2 * (ry / static_cast<float>(m_Height))) * fov };

	ThreeBlade rayDirOrigin = ThreeBlade(camX, camY, 0);
	ThreeBlade rayDirLookAtPoint = ThreeBlade(camX, camY, 1.0f);
	TwoBlade rayDirNorm = (rayDirOrigin & rayDirLookAtPoint).Normalized();

	Color4f finalColor{0.3f, 0.3f, 0.3f, 1.0f};

	if (HitPlane(pCamera->CameraToWorldLine(rayDirNorm), m_TestPlane))
	{
		finalColor = m_TestPlane.Color;
	}

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}
