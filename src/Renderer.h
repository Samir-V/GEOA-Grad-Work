#pragma once
#include "structs.h"
#include "SDL.h"
#include "SDL_opengl.h"

class Camera;

class Renderer
{
public:
	explicit Renderer( const Window& window );
	Renderer( const Renderer& other ) = delete;
	Renderer& operator=( const Renderer& other ) = delete;
	Renderer(Renderer&& other) = delete;
	Renderer& operator=(Renderer&& other) = delete;

	~Renderer();

	void Run( );

	void Update(float elapsedSec);

	void Render();

	// Event handling
	void ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
	{

	}
	void ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
	{

	}
	void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e)
	{
		
	}
	void ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
	{
		
	}
	void ProcessMouseUpEvent(const SDL_MouseButtonEvent& e)
	{
		
	}

	const Rectf& GetViewPort() const
	{
		return m_Viewport;
	}

private:
	// DATA MEMBERS
	// The window properties
	const Window m_Window;
	const Rectf m_Viewport;
	// The window we render to
	SDL_Window* m_pWindow;
	// OpenGL context
	SDL_GLContext m_pContext;
	// Init info
	bool m_Initialized;
	// Prevent timing jumps when debugging
	const float m_MaxElapsedSeconds;


	// Camera info

	std::unique_ptr<Camera> m_CameraUPtr{};

	// Rendering info
	SDL_Surface* m_pBuffer{};
	uint32_t* m_pBufferPixels{};

	int m_Height;
	int m_Width;


	// FUNCTIONS
	void InitializeRenderer( );
	void CleanupRenderer( );
	void RenderPixel(uint32_t pixelIndex, float fov, float aspectRatio, const Camera* pCamera) const;
};
