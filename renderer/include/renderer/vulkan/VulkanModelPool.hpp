#pragma once

#include <renderer/IModelPool.hpp>
#include <renderer/vulkan/VulkanModel.hpp>
#include <renderer/vulkan/VulkanUniformBuffer.hpp>

#include <glm/glm.hpp>
#include <map>
#include <vector>

namespace Renderer
{

	namespace Vulkan
	{
		class VulkanDevice;
		class VulkanDescriptorSet;
		class VulkanPipeline;
		class VulkanModelPool : public IModelPool
		{
		public:
			VulkanModelPool(VulkanDevice* device, IVertexBuffer* vertex_buffer);
			VulkanModelPool(VulkanDevice* device, IVertexBuffer* vertex_buffer, IIndexBuffer* index_buffer);
			~VulkanModelPool();
			virtual IModel * CreateModel();
			virtual void Update();
			virtual void AttachBuffer(unsigned int index, IUniformBuffer * buffer);
			virtual void AttachDescriptorSet(unsigned int index, IDescriptorSet* descriptor_set);
			virtual void SetVertexDrawCount(unsigned int count);
			void AttachToCommandBuffer(VkCommandBuffer & command_buffer, VulkanPipeline* pipeline);
			bool HasChanged();
		private:
			void ResizeIndirectArray(unsigned int size);
			void Render(unsigned int index, bool should_render);
			unsigned int m_current_index;
			unsigned int m_vertex_draw_count;
			VulkanDevice * m_device;
			std::map<unsigned int, VulkanDescriptorSet*> m_descriptor_sets;
			std::map<unsigned int, VulkanUniformBuffer*> m_buffers;
			std::map<unsigned int, VulkanModel*> m_models;
			static const unsigned int m_indirect_array_padding;
			VulkanBuffer* m_indirect_draw_buffer = nullptr;

			union
			{
				void* m_indirect_command;
				VkDrawIndirectCommand* m_vertex_indirect_command;
				VkDrawIndexedIndirectCommand* m_indexed_indirect_command;
			};

			bool m_change;

			friend VulkanModel;
		};
	}
}