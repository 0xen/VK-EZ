#pragma once

#include <renderer\vulkan\VulkanHeader.hpp>
#include <renderer\IPipeline.hpp>
#include <renderer\ShaderStage.hpp>
#include <renderer\ITextureBuffer.hpp>

#include <vector>
#include <map>

namespace Renderer
{
	namespace Vulkan
	{
		class VulkanDevice;
		class VulkanUniformBuffer;
		class VulkanDescriptorPool;
		class VulkanDescriptorSet;
		class VulkanPipeline : public virtual IPipeline
		{
		public:
			VulkanPipeline(VulkanDevice * device, std::map<ShaderStage, const char*> paths);
			~VulkanPipeline();
			virtual void AttachDescriptorPool(IDescriptorPool* buffer);
			virtual void AttachDescriptorSet(unsigned int setID, IDescriptorSet* descriptor_set);
			virtual bool Build();
			virtual bool CreatePipeline();
			virtual void DestroyPipeline();
			virtual void AttachToCommandBuffer(VkCommandBuffer & command_buffer);
			bool Rebuild();
			VkPipelineLayout& GetPipelineLayout();
		protected:
			VulkanDevice * m_device;

			VkPipelineLayout m_pipeline_layout;

			std::vector<VulkanDescriptorPool*> m_descriptor_pools;
			std::map<unsigned int,VulkanDescriptorSet*> m_descriptor_sets;
			VkPipeline m_pipeline = VK_NULL_HANDLE;
		private:
		};
	}
}