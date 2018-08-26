#pragma once

namespace Renderer
{
	class IVertexBuffer;
	class IIndexBuffer;
	class IUniformBuffer;
	class IDescriptorSet;
	class IModel;
	class IModelPool
	{
	public:
		IModelPool(IVertexBuffer* vertex_buffer, IIndexBuffer* index_buffer);
		virtual IModel * CreateModel() = 0;
		virtual void AttachBuffer(unsigned int index, IUniformBuffer * buffer) = 0;
		virtual void AttachDescriptorSet(unsigned int index, IDescriptorSet* descriptor_set) = 0;

	protected:
		IVertexBuffer * m_vertex_buffer;
		IIndexBuffer * m_index_buffer;
	};
}