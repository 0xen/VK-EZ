#pragma once

#include <vector>
#include <renderer/APIs.hpp>
#include <renderer/NativeWindowHandle.hpp>
#include <renderer\DescriptorType.hpp>
#include <renderer\ShaderStage.hpp>
#include <renderer\IUniformBuffer.hpp>
#include <renderer\IVertexBuffer.hpp>
#include <renderer\IIndexBuffer.hpp>
#include <renderer\IGraphicsPipeline.hpp>
#include <renderer\IComputePipeline.hpp>
#include <renderer\IComputeProgram.hpp>
#include <renderer\IModel.hpp>
#include <renderer\IModelPool.hpp>
#include <renderer\VertexBase.hpp>
#include <renderer\ITextureBuffer.hpp>
#include <renderer\IDescriptor.hpp>
#include <renderer\IDescriptorPool.hpp>

namespace Renderer
{
	class IRenderer
	{
	public:
		IRenderer();
		// Starts the renderer, this class is inherited by the parent class and it will define the function body
		virtual bool Start(NativeWindowHandle* window_handle) = 0;

		// Update the renderer, this class is inherited by the parent class and it will define the function body
		virtual void Update() = 0;
		// Stop the renderer, this class is inherited by the parent class and it will define the function body
		virtual void Stop() = 0;
		// Rebuild the rendering platform when a event takes place that it is required, such as a screen resize
		virtual void Rebuild() = 0;

		// Creates a instance of the renderer based on the chosen API. this IRenderer can be cast into the parent class
		static IRenderer* CreateRenderer(const RenderingAPI api);
		// Call the update function on all of the renderer's that have been created
		static void UpdateAll();
		// Remove the renderer from the list of renderers to be updated when calling UpdateAll, the renderer will still work as a stand alone renderer
		static void UnregisterRenderer(IRenderer* renderer);

		virtual IUniformBuffer* CreateUniformBuffer(void* dataPtr, BufferChain level, unsigned int indexSize, unsigned int elementCount, bool modifiable = false) = 0;

		virtual IVertexBuffer* CreateVertexBuffer(void* dataPtr, unsigned int indexSize, unsigned int elementCount) = 0;

		virtual IIndexBuffer* CreateIndexBuffer(void* dataPtr, unsigned int indexSize, unsigned int elementCount) = 0;

		virtual IGraphicsPipeline* CreateGraphicsPipeline(std::map<ShaderStage, const char*> paths, bool priority = false) = 0;

		virtual IComputePipeline* CreateComputePipeline(const char* path, unsigned int x, unsigned int y, unsigned int z) = 0;

		virtual IComputeProgram* CreateComputeProgram() = 0;

		virtual IModelPool* CreateModelPool(IVertexBuffer* vertex_buffer, IIndexBuffer* index_buffer) = 0;

		virtual IModelPool* CreateModelPool(IVertexBuffer* vertex_buffer) = 0;

		virtual ITextureBuffer* CreateTextureBuffer(void* dataPtr, DataFormat format, unsigned int width, unsigned int height) = 0;

		virtual IDescriptor* CreateDescriptor(DescriptorType descriptor_type, ShaderStage shader_stage, unsigned int binding) = 0;

		virtual IDescriptorPool* CreateDescriptorPool(std::vector<IDescriptor*> descriptors) = 0;

		bool IsRunning();
	private:
		// Store all renderers generated by the CreateRenderer class
		static std::vector<IRenderer*> m_renderers;
	protected:
		bool m_running = false;
	};
}