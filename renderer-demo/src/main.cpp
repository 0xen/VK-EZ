#include <assert.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include <renderer\IRenderer.hpp>


#include <iostream>

using namespace Renderer;

SDL_Window* window;
NativeWindowHandle* window_handle;

Uint32 GetWindowFlags(RenderingAPI api)
{
	switch (api)
	{
		case VulkanAPI:
			return SDL_WINDOW_VULKAN;
			break;
	}
	return 0;
}

void WindowSetup(RenderingAPI api, const char* title, int width, int height)
{
	window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		GetWindowFlags(api) | SDL_WINDOW_RESIZABLE
	);
	SDL_ShowWindow(window);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	assert(SDL_GetWindowWMInfo(window, &info) && "Error, unable to get window info");

	window_handle = new NativeWindowHandle(info.info.win.window, width, height);
	window_handle->clear_color = { 0.2f,0.2f,0.2f,1.0f };
}

void DestroyWindow()
{
	SDL_DestroyWindow(window);

	delete window_handle;
}




int main(int argc, char **argv)
{

	// Define what rendering api we are wanting to use
	RenderingAPI rendering_api = RenderingAPI::VulkanAPI;

	WindowSetup(rendering_api, "Renderer", 1080, 720);

	// Create a instance of the renderer
	IRenderer* renderer = IRenderer::CreateRenderer(rendering_api);

	// If the rendering was not fully created, error out
	assert(renderer != nullptr && "Error, renderer instance could not be created");

	renderer->Start(window_handle);

	float* data = new float[10];

	for (int i = 0; i < 10; i++)
	{
		data[i] = 2 + i;
	}

	IUniformBuffer* buffer = renderer->CreateUniformBuffer(data, sizeof(float), 10, DescriptorType::UNIFORM, ShaderStage::COMPUTE_SHADER, 0);
	buffer->SetData();


	IComputePipeline* pipeline = renderer->CreateComputePipeline("../../renderer-demo/Shaders/Compute/comp.spv", 10, 1, 1);
	pipeline->AttachBuffer(buffer);
	pipeline->Build();

	IComputeProgram* program = renderer->CreateComputeProgram();
	program->AttachPipeline(pipeline);
	program->Build();
	program->Run();

	buffer->GetData();


	for (int i = 0; i < 10; i++)
	{
		std::cout << data[i] << std::endl;
	}

	bool running = true;
	while (running)
	{

		// Update all renderer's via there Update function
		IRenderer::UpdateAll();

		// Poll Window
		SDL_Event event;
		while (SDL_PollEvent(&event) > 0)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					//Get new dimensions and repaint on window size change
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					window_handle->width = event.window.data1;
					window_handle->height = event.window.data2;
					renderer->Rebuild();
					break;
				}
				break;
			}
		}
	}

	delete program;
	//delete pipeline;
	delete buffer;

	renderer->Stop();

	delete renderer;

	DestroyWindow();

    return 0;
}
