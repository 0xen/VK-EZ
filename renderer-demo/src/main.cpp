

#define RENDERER_RAYTRACE

#include <assert.h>
#include <SDL.h>
#include <SDL_syswm.h>


#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <renderer\vulkan\VulkanSwapchain.hpp>
#include <renderer\vulkan\VulkanDescriptorSet.hpp>
#include <renderer\vulkan\VulkanModelPool.hpp>
#include <renderer\vulkan\VulkanAcceleration.hpp>
#include <renderer\vulkan\VulkanRaytracePipeline.hpp>
#include <renderer/vulkan/VulkanRenderer.hpp>
#include <renderer/vulkan/VulkanFlags.hpp>
#include <renderer\VertexBase.hpp>

#include "obj_loader.h"
#include <lodepng.h>

#include <vector>
#include <iostream>

using namespace Renderer;
using namespace Renderer::Vulkan;

static const int POSITION_BUFFER = 0;

struct Camera
{
	glm::mat4 view;
	glm::mat4 projection;
};



struct RayCamera
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 modelIT;
	// #VKRay
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
};


SDL_Window* window;
NativeWindowHandle* window_handle;
VulkanRenderer* renderer;
Camera camera;
RayCamera rayCamera;

class MeshVertex
{
public:
	MeshVertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal, glm::vec3 color) : position(position), uv(uv), normal(normal), color(color) {}
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 color;
};

class PositionVertex
{
public:
	PositionVertex(glm::mat4 pos) : pos(pos) {}
	glm::mat4 pos;
};

void WindowSetup(const char* title, int width, int height)
{
	window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
	);
	SDL_ShowWindow(window);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	bool sucsess = SDL_GetWindowWMInfo(window, &info);
	assert(sucsess && "Error, unable to get window info");

	window_handle = new NativeWindowHandle(info.info.win.window, width, height);
	window_handle->clear_color = { 0.2f,0.2f,0.2f,1.0f };
}

void PollWindow()
{

	// Poll Window
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			renderer->Stop();
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
void DestroyWindow()
{
	SDL_DestroyWindow(window);

	delete window_handle;
}


struct Vertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	glm::vec3 color;
	glm::vec2 texCoord;
	int       matID = 0;

	static auto getBindingDescription();
	static auto getAttributeDescriptions();
};


int main(int argc, char **argv)
{


	renderer = new VulkanRenderer();

	WindowSetup("Renderer", 1080, 720);

	// If the rendering was not fully created, error out
	assert(renderer != nullptr && "Error, renderer instance could not be created");
	
	renderer->Start(window_handle, VulkanFlags::Raytrace/* | VulkanFlags::ActiveCMDRebuild*/);



	/*std::vector<MeshVertex> vertexData = {
		MeshVertex(glm::vec3(1.0f,1.0f,0.0f), glm::vec2(0.0f,0.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(1.0f,1.0f,0.0f)),
		MeshVertex(glm::vec3(1.0f,-1.0f,0.0f), glm::vec2(0.0f,1.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(0.0f,1.0f,0.0f)),
		MeshVertex(glm::vec3(-1.0f,-1.0f,0.0f), glm::vec2(1.0f,1.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(.0f,1.0f,1.0f)),
		MeshVertex(glm::vec3(-1.0f,1.0f,0.0f), glm::vec2(1.0f,0.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(1.0f,0.0f,1.0f))
	};
	
	std::vector<uint16_t> indexData{
		0,1,2,
		0,2,3
	};*/


	// Camera setup
	camera.view = glm::mat4(1.0f);
	camera.view = glm::scale(camera.view, glm::vec3(1.0f, 1.0f, 1.0f));
	camera.view = glm::translate(camera.view, glm::vec3(0.0f, 0.0f, -15.0f));


	//camera.view = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	float aspectRatio = ((float)1080) / ((float)720);
	camera.projection = glm::perspective(glm::radians(65.0f), aspectRatio, 0.1f, 1000.0f);



	// Ray camera
	glm::mat4 mPos = glm::mat4(1.0f);
	mPos = glm::translate(mPos, glm::vec3(0, 0, 0));
	mPos = glm::scale(mPos, glm::vec3(1.0f, 1.0f, 1.0f));
	rayCamera.model = mPos;
	rayCamera.modelIT = glm::inverseTranspose(rayCamera.model);

	rayCamera.view = camera.view;
	rayCamera.proj = camera.projection;


	// Need to flip the projection as GLM was made for OpenGL
	rayCamera.proj[1][1] *= -1;

						   // #VKRay
	rayCamera.viewInverse = glm::inverse(rayCamera.view);
	rayCamera.projInverse = glm::inverse(rayCamera.proj);
	
	// Only flip the Y when the raytrace camera has out instance as it dose not flip
	camera.projection[1][1] *= -1;  // Inverting Y for Vulkan



	// Setup cameras descriptors and buffers
	IUniformBuffer* cameraBuffer = renderer->CreateUniformBuffer(&camera, BufferChain::Single, sizeof(Camera), 1, true);
	cameraBuffer->SetData(BufferSlot::Primary);

	IDescriptorPool* camera_pool = renderer->CreateDescriptorPool({
		renderer->CreateDescriptor(Renderer::DescriptorType::UNIFORM, Renderer::ShaderStage::VERTEX_SHADER, 0),
		});

	IDescriptorSet* camera_descriptor_set = camera_pool->CreateDescriptorSet();
	camera_descriptor_set->AttachBuffer(0, cameraBuffer);
	camera_descriptor_set->UpdateSet();





	ObjLoader<Vertex> loader;
	loader.loadModel("../../renderer-demo/media/scenes/Medieval_building.obj");


	uint32_t m_nbIndices = static_cast<uint32_t>(loader.m_indices.size());
	uint32_t m_nbVertices = static_cast<uint32_t>(loader.m_vertices.size());






	IVertexBuffer* vertexBuffer = renderer->CreateVertexBuffer(loader.m_vertices.data(), sizeof(Vertex), loader.m_vertices.size());
	IIndexBuffer* indexBuffer = renderer->CreateIndexBuffer(loader.m_indices.data(), sizeof(uint16_t), loader.m_indices.size());

	vertexBuffer->SetData(BufferSlot::Primary);
	indexBuffer->SetData(BufferSlot::Primary);

	IModelPool* model_pool1 = renderer->CreateModelPool(vertexBuffer, indexBuffer);

	glm::mat4* model_position_array1 = new glm::mat4[1000];
	IUniformBuffer* model_position_buffer1 = renderer->CreateUniformBuffer(model_position_array1, BufferChain::Double, sizeof(glm::mat4), 1000, true);


	model_pool1->AttachBuffer(POSITION_BUFFER, model_position_buffer1);

	IModel* model1 = model_pool1->CreateModel();

	glm::mat4 modelPosition = glm::mat4(1.0f);
	modelPosition = glm::translate(modelPosition, glm::vec3(2, 0, 0));
	float scale = 0.4f;
	modelPosition = glm::scale(modelPosition, glm::vec3(scale, scale, scale));

	model1->SetData(POSITION_BUFFER, modelPosition);

	{


		IModel* model2 = model_pool1->CreateModel();

		glm::mat4 modelPos = glm::mat4(1.0f);
		modelPos = glm::mat4(1.0f);
		modelPos = glm::translate(modelPos, glm::vec3(-2, 0, 0));
		modelPos = glm::scale(modelPos, glm::vec3(scale, scale, scale));

		model2->SetData(POSITION_BUFFER, modelPos);
	}
	{


		IModel* model2 = model_pool1->CreateModel();

		glm::mat4 modelPos = glm::mat4(1.0f);
		modelPos = glm::mat4(1.0f);
		modelPos = glm::translate(modelPos, glm::vec3(-2, 2, 0));
		modelPos = glm::scale(modelPos, glm::vec3(scale, scale, scale));

		model2->SetData(POSITION_BUFFER, modelPos);
	}
	{


		IModel* model2 = model_pool1->CreateModel();

		glm::mat4 modelPos = glm::mat4(1.0f);
		modelPos = glm::mat4(1.0f);
		modelPos = glm::translate(modelPos, glm::vec3(2, 2, 0));
		modelPos = glm::scale(modelPos, glm::vec3(scale, scale, scale));

		model2->SetData(POSITION_BUFFER, modelPos);
	}
	

	model_position_buffer1->SetData(BufferSlot::Secondery);
	model_position_buffer1->Transfer(BufferSlot::Primary, BufferSlot::Secondery);



	std::vector<unsigned char> image; //the raw pixels
	unsigned width;
	unsigned height;
	unsigned error = lodepng::decode(image, width, height, "../../renderer-demo/Images/cobble.png");
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	ITextureBuffer* texture = renderer->CreateTextureBuffer(image.data(), Renderer::DataFormat::R8G8B8A8_FLOAT, width, height);

	// Create texture pool
	IDescriptorPool* texture_pool = renderer->CreateDescriptorPool({
		renderer->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});

	IDescriptorSet* texture_descriptor_set1 = texture_pool->CreateDescriptorSet();
	texture_descriptor_set1->AttachBuffer(0, texture);
	texture_descriptor_set1->UpdateSet();

	model_pool1->AttachDescriptorSet(1, texture_descriptor_set1);





	/*bool raytrace = true;
	if (raytrace)
	{*/
		VulkanRaytracePipeline* ray_pipeline = renderer->CreateRaytracePipeline({
			{ ShaderStage::RAY_GEN,		"../../renderer-demo/Shaders/Raytrace/raygen.spv" },
			{ ShaderStage::MISS,		"../../renderer-demo/Shaders/Raytrace/miss.spv" },
			{ ShaderStage::MISS,		"../../renderer-demo/Shaders/Raytrace/shadowMiss.spv" }
		},
		{
			{ // Involved 
				{ ShaderStage::CLOSEST_HIT, "../../renderer-demo/Shaders/Raytrace/closesthit.spv" }
			},
			{} // For simple shadows, we do not need a hitgroup
		});

		ray_pipeline->AddRayGenerationProgram(0, {});
		ray_pipeline->AddMissProgram(1, {});
		ray_pipeline->AddMissProgram(2, {});
		ray_pipeline->AddHitGroup(3, {});
		ray_pipeline->AddHitGroup(4, {});


		ray_pipeline->SetMaxRecursionDepth(2);

		ray_pipeline->AttachVertexBinding({
			VertexInputRate::INPUT_RATE_VERTEX,
			{
				{ 0, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,position) },
				{ 1, DataFormat::R32G32_FLOAT,offsetof(MeshVertex,uv) },
				{ 2, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,normal) },
				{ 3, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,color) },
			},
			sizeof(MeshVertex),
			0
			});


		IDescriptorPool* raytracePool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 0),
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV, 1),
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_NV, 2),
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 3),
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 4),
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 5),
			renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 6),


		});




		ray_pipeline->AttachDescriptorPool(raytracePool);


		VulkanAcceleration* acceleration = renderer->CreateAcceleration();

		acceleration->AttachModelPool(static_cast<VulkanModelPool*>(model_pool1));

		acceleration->Build();


		VulkanDescriptorSet* raytracingSet = static_cast<VulkanDescriptorSet*>(raytracePool->CreateDescriptorSet());




		//RayCamera
		IUniformBuffer* cameraInfo = renderer->CreateUniformBuffer(&rayCamera, BufferChain::Single, sizeof(RayCamera), 1, true);
		cameraInfo->SetData(BufferSlot::Primary);


		IUniformBuffer* materialbuffer = renderer->CreateUniformBuffer(loader.m_materials.data(), BufferChain::Single, sizeof(MatrialObj), loader.m_materials.size(), true);
		materialbuffer->SetData(BufferSlot::Primary);


		raytracingSet->AttachBuffer(0, { acceleration->GetDescriptorAcceleration() });


		VulkanSwapchain* swapchain = renderer->GetSwapchain();
		
		

		//raytracingSet->AttachBuffer(1, { swapchain->GetBackBufferImageInfo(2) });

		raytracingSet->AttachBuffer(1, { swapchain->GetRayTraceStagingBuffer() });
		raytracingSet->AttachBuffer(2, cameraInfo);
		raytracingSet->AttachBuffer(3, model_pool1->GetVertexBuffer());
		raytracingSet->AttachBuffer(4, model_pool1->GetIndexBuffer());
		raytracingSet->AttachBuffer(5, materialbuffer);


		raytracingSet->AttachBuffer(6, texture);


		raytracingSet->UpdateSet();



		ray_pipeline->AttachDescriptorSet(0, raytracingSet);






		ray_pipeline->Build();

		/*

	}
	else
	{
		IGraphicsPipeline* pipeline = renderer->CreateGraphicsPipeline({
			{ ShaderStage::VERTEX_SHADER, "../../renderer-demo/Shaders/Raster/vert.spv" },
			{ ShaderStage::FRAGMENT_SHADER, "../../renderer-demo/Shaders/Raster/frag.spv" }
			});
		
		
		pipeline->AttachVertexBinding({
			VertexInputRate::INPUT_RATE_VERTEX,
			{
				{ 0, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,position) },
				{ 1, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,normal) },
				{ 2, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,color) },
				{ 3, DataFormat::R32G32_FLOAT,offsetof(MeshVertex,uv) },
			},
			sizeof(Vertex),
			0
			});
	
		pipeline->AttachVertexBinding({
			VertexInputRate::INPUT_RATE_INSTANCE,
			{ 
				{ 4, DataFormat::MAT4_FLOAT,0 } 
			},
			sizeof(PositionVertex),
			1 
			});


		
		pipeline->AttachDescriptorPool(camera_pool);
	
		pipeline->AttachDescriptorSet(0, camera_descriptor_set);
	
		pipeline->AttachDescriptorPool(texture_pool);
	
		pipeline->Build();

		pipeline->AttachModelPool(model_pool1);
	}
		
		*/



	while (renderer->IsRunning())
	{

		renderer->BeginFrame();




		modelPosition = glm::rotate(modelPosition, 0.001f, glm::vec3(0,1,0));

		model1->SetData(POSITION_BUFFER, modelPosition);

		model_position_buffer1->SetData(BufferSlot::Primary);

		acceleration->Update();

		renderer->EndFrame();
		PollWindow();
	}




	DestroyWindow();


	delete renderer;

	return 0;
}

