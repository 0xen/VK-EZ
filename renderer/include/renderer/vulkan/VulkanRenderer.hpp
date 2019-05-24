#pragma once

#include <renderer/vulkan/VulkanStatus.hpp>
#include <renderer/IRenderer.hpp>
#include <renderer\DescriptorType.hpp>
#include <renderer\ShaderStage.hpp>


#include <glm\glm.hpp>

namespace Renderer
{
	namespace Vulkan
	{
		class VulkanInstance;
		enum VulkanFlags;
		class VulkanPhysicalDevice;
		class VulkanDevice;
		class VulkanSwapchain;
		class VulkanDescriptor;
		class VulkanGraphicsPipeline;
		class VulkanVertexBuffer;
		class VulkanIndexBuffer;
		class VulkanTextureBuffer;
		class VulkanRaytracePipeline;
		class VulkanAcceleration;

		class VulkanRenderer : public IRenderer, public VulkanStatus
		{
		public:
			VulkanRenderer();
			~VulkanRenderer();
			bool Start(Renderer::NativeWindowHandle* window_handle, unsigned int flags);
			virtual bool Start(Renderer::NativeWindowHandle* window_handle);
			virtual void Update();
			void BeginFrame();
			void EndFrame();

			virtual void Stop();
			virtual void Rebuild();
			virtual IUniformBuffer* CreateUniformBuffer(void* dataPtr, BufferChain level, unsigned int indexSize, unsigned int elementCount, bool modifiable);

			virtual IVertexBuffer* CreateVertexBuffer(void* dataPtr, unsigned int indexSize, unsigned int elementCount);

			virtual IIndexBuffer* CreateIndexBuffer(void* dataPtr, unsigned int indexSize, unsigned int elementCount);

			virtual IGraphicsPipeline* CreateGraphicsPipeline(std::vector<std::pair<Renderer::ShaderStage, const char*>> paths, bool priority = false);

			virtual void RemoveGraphicsPipeline(IGraphicsPipeline* pipeline);

			virtual IComputePipeline* CreateComputePipeline(const char* path, unsigned int x, unsigned int y, unsigned int z);

			virtual IComputeProgram* CreateComputeProgram();

			virtual IModelPool* CreateModelPool(IVertexBuffer* vertex_buffer, IIndexBuffer* index_buffer);

			virtual IModelPool* CreateModelPool(IVertexBuffer* vertex_buffer);

			virtual ITextureBuffer* CreateTextureBuffer(void* dataPtr, DataFormat format, unsigned int width, unsigned int height);

			virtual IDescriptor* CreateDescriptor(DescriptorType descriptor_type, ShaderStage shader_stage, unsigned int binding);

			virtual IDescriptor* CreateDescriptor(VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage, unsigned int binding);

			virtual IDescriptorPool* CreateDescriptorPool(std::vector<IDescriptor*> descriptors);

			VulkanRaytracePipeline* CreateRaytracePipeline(std::vector<std::pair<Renderer::ShaderStage, const char*>> paths, std::vector<std::map<ShaderStage, const char*>> hitgroups, bool priority = false);

			VulkanAcceleration* CreateAcceleration();

			VulkanSwapchain* GetSwapchain();

			static VkDescriptorType ToDescriptorType(DescriptorType descriptor_type);

			static VkShaderStageFlagBits ToVulkanShader(ShaderStage stage);
		private:
			void CreateSurface(Renderer::NativeWindowHandle* window_handle);
			Renderer::NativeWindowHandle* m_window_handle;
			VulkanInstance * m_instance;
			VkSurfaceKHR m_surface;
			VulkanPhysicalDevice* m_physical_device;
			VulkanDevice* m_device;
			VulkanSwapchain* m_swapchain;
		};
	}
}