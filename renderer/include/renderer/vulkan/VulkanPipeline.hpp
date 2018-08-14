#pragma once

#include <renderer\vulkan\VulkanHeader.hpp>
#include <renderer\IPipeline.hpp>

#include <vector>

namespace Renderer
{
	namespace Vulkan
	{
		class VulkanDevice;
		class VulkanUniformBuffer;
		class VulkanPipeline : public virtual IPipeline
		{
		public:
			VulkanPipeline(VulkanDevice * device, const char* path);
			virtual void AttachBuffer(IUniformBuffer* buffer);
			virtual void Build();
			virtual void CreatePipeline();
			virtual void DestroyPipeline();
			virtual void AttachToCommandBuffer(VkCommandBuffer & command_buffer);
			void Rebuild();
		protected:
			VulkanDevice * m_device;
			std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
			std::vector<VkDescriptorPoolSize> m_descriptor_pool_sizes;
			std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
			std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;

			VkDescriptorPool m_descriptor_pool;
			VkDescriptorSetLayout m_descriptor_set_layout;
			VkDescriptorSet m_descriptor_set;
			VkPipelineLayout m_compute_pipeline_layout;

			std::vector<VulkanUniformBuffer*> m_buffers;
			VkPipeline m_pipeline = VK_NULL_HANDLE;
		private:
		};
	}
}