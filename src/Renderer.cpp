#include <iostream>
#include <algorithm>
#include <limits>
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
#include "GEOAUtils.h"

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

	m_CameraUPtr = std::make_unique<Camera>(TriVector(0.f, 0.f, 0.f), 60.f);

	m_Initialized = true;

	/*m_TestPlane = Plane();
	m_TestPlane.Color = Color4f{0.4f, 0.1f, 0.8f, 1.0f};
	m_TestPlane.PlaneGenerators = Vector{5.0f, 0, 0, 1}.Normalized();*/

	m_TestSphere = Sphere();
	m_TestSphere.Color = Color4f{ 0.0f, 0.0f, 0.0f, 1.0f };
	m_TestSphere.Origin = TriVector{ 0.0f, 0.0f, 10.0f }.Normalized();
	m_TestSphere.Radius = 3.0f;

	m_TestLightParticle = std::make_unique<LightParticle>(
		TriVector{-5.0f, 0.0f, 10.0f, 1.0f}.Normalized(),
		BiVector{1, 0, 0, 0, 0, 0}, 
		0.0 
	);
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
	// Process camera movement based on held keys
	ProcessCameraInput(elapsedSec);

	// Rotate the test plane
	auto rot = Motor::Rotation(40.0f * elapsedSec, BiVector{0, 0, 0, 0, 1, 0});

	auto newPlane = (rot * m_TestPlane.PlaneGenerators * ~rot).Grade1();
	m_TestPlane.PlaneGenerators = newPlane.Normalized();

	// Update light particle
	m_TestLightParticle->Update(elapsedSec, m_CameraUPtr->GetOrigin());
}

void Renderer::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{
	m_PressedKeys.insert(e.keysym.sym);

	// Toggle mouse capture with Escape key
	if (e.keysym.sym == SDLK_ESCAPE)
	{
		m_MouseCaptured = !m_MouseCaptured;
		SDL_SetRelativeMouseMode(m_MouseCaptured ? SDL_TRUE : SDL_FALSE);

		if (m_MouseCaptured)
		{
			std::cout << "Mouse captured - move mouse to look around, WASD to move. Press ESC to release." << std::endl;
		}
		else
		{
			std::cout << "Mouse released. Press ESC to capture." << std::endl;
		}
	}
}

void Renderer::ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
{
	m_PressedKeys.erase(e.keysym.sym);
}

void Renderer::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e)
{
	if (m_MouseCaptured)
	{
		float deltaYaw = static_cast<float>(e.xrel);
		float deltaPitch = static_cast<float>(e.yrel);

		m_CameraUPtr->Rotate(deltaYaw, deltaPitch);
	}
}

void Renderer::ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
{
}

void Renderer::ProcessMouseUpEvent(const SDL_MouseButtonEvent& e)
{
}

void Renderer::ProcessCameraInput(float elapsedSec)
{
	float forward = 0.0f;
	float right = 0.0f;
	float up = 0.0f;

	if (m_PressedKeys.count(SDLK_w)) forward += elapsedSec;
	if (m_PressedKeys.count(SDLK_s)) forward -= elapsedSec;
	if (m_PressedKeys.count(SDLK_d)) right += elapsedSec;
	if (m_PressedKeys.count(SDLK_a)) right -= elapsedSec;

	if (m_PressedKeys.count(SDLK_SPACE)) up += elapsedSec;
	if (m_PressedKeys.count(SDLK_LSHIFT)) up -= elapsedSec;

	if (forward != 0.0f || right != 0.0f || up != 0.0f)
	{
		m_CameraUPtr->Move(forward, right, up, elapsedSec);
	}
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

	auto rayDirNorm = BiVector(0, 0, 0, camX, camY, 1.0f).Normalize();
	BiVector worldRay = pCamera->CameraToWorldLine(rayDirNorm);

	Color4f finalColor{0.3f, 0.3f, 0.3f, 1.0f};
	float closestDistSq = std::numeric_limits<float>::max();

	TriVector camPos = pCamera->GetOrigin().Normalized();

	// Test sphere
	float sphereDistSq{};
	if (HitSphere(worldRay, m_TestSphere, m_CameraUPtr.get(), sphereDistSq))
	{
		if (sphereDistSq < closestDistSq)
		{
			closestDistSq = sphereDistSq;
			finalColor = Color4f{
				m_TestSphere.Color.r,
				m_TestSphere.Color.g,
				m_TestSphere.Color.b,
				1.0f
			};
		}
	}

	// Test light particle
	if (HitBounds(worldRay, m_TestLightParticle->GetBoundsCenter(),
	              m_TestLightParticle->GetBoundsRadius(), camPos))
	{
		const auto& path = m_TestLightParticle->GetPath();

		for (const auto& pos : path)
		{
			float pointDistSq{};
			if (HitPoint(worldRay, pos, 0.05f, camPos, pointDistSq))
			{
				if (pointDistSq < closestDistSq)
				{
					closestDistSq = pointDistSq;
					finalColor = Color4f{1.0f, 1.0f, 1.0f, 1.0f};
				}
			}
		}

		float posDistSq{};
		if (HitPoint(worldRay, m_TestLightParticle->GetPosition(), 0.10f, camPos, posDistSq))
		{
			if (posDistSq < closestDistSq)
			{
				closestDistSq = posDistSq;
				finalColor = Color4f{1.0f, 1.0f, 1.0f, 1.0f};
			}
		}
	}

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}
