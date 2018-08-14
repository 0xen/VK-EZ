#include <renderer/vulkan/VulkanCommon.hpp>
#include <renderer/vulkan/VulkanDevice.hpp>
#include <renderer/vulkan/VulkanPhysicalDevice.hpp>
#include <renderer/vulkan/VulkanBufferData.hpp>

#include <fstream>

using namespace Renderer::Vulkan;

void VulkanCommon::CreateImageView(VulkanDevice* device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView & view)
{
	VkImageViewCreateInfo create_info = VulkanInitializers::ImageViewCreate(image, format, aspect_flags);
	vkCreateImageView(
		*device->GetVulkanDevice(),
		&create_info,
		nullptr,
		&view
	);
}

VkFormat VulkanCommon::GetDepthImageFormat(VulkanDevice * device)
{
	return FindSupportedFormat(
		device,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },	// Formats
		VK_IMAGE_TILING_OPTIMAL,															// Tiling
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT										// Features
	);
}

VkFormat VulkanCommon::FindSupportedFormat(VulkanDevice * device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(*device->GetVulkanPhysicalDevice()->GetPhysicalDevice(), format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	assert(0 && "All formats are not supported");
	return VK_FORMAT_UNDEFINED;
}

void VulkanCommon::CreateImage(VulkanDevice* device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & image_memory)
{
	VkImageCreateInfo create_info = VulkanInitializers::ImageCreateInfo(extent.width, extent.height, format, tiling, usage);
	vkCreateImage(
		*device->GetVulkanDevice(),
		&create_info,
		nullptr,
		&image
	);

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(
		*device->GetVulkanDevice(),
		image,
		&mem_requirements
	);

	VkMemoryAllocateInfo alloc_info = VulkanInitializers::MemoryAllocateInfo(mem_requirements.size, FindMemoryType(
		device->GetVulkanPhysicalDevice(),
		mem_requirements.memoryTypeBits,
		properties
	));

	vkAllocateMemory(
		*device->GetVulkanDevice(),
		&alloc_info,
		nullptr,
		&image_memory
	);

	vkBindImageMemory(
		*device->GetVulkanDevice(),
		image,
		image_memory,
		0
	);
}

uint32_t VulkanCommon::FindMemoryType(VulkanPhysicalDevice * device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < device->GetPhysicalDeviceMemoryProperties()->memoryTypeCount; i++)
	{
		if (type_filter & (1 << i) &&
			(device->GetPhysicalDeviceMemoryProperties()->memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	assert(0 && "No available memory properties");
	return -1;
}

void VulkanCommon::TransitionImageLayout(VulkanDevice* device, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
	VkCommandBuffer command_buffer = BeginSingleTimeCommands(device,*device->GetGraphicsCommandPool());

	VkImageMemoryBarrier barrier = VulkanInitializers::ImageMemoryBarrier(image, format, old_layout, new_layout);

	vkCmdPipelineBarrier(
		command_buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier
	);
	EndSingleTimeCommands(device,command_buffer, *device->GetGraphicsCommandPool());
}

VkCommandBuffer VulkanCommon::BeginSingleTimeCommands(VulkanDevice * device, VkCommandPool command_pool)
{
	VkCommandBufferAllocateInfo alloc_info = VulkanInitializers::CommandBufferAllocateInfo(
		command_pool,
		1
	);
	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(
		*device->GetVulkanDevice(),
		&alloc_info,
		&command_buffer
	);
	VkCommandBufferBeginInfo begin_info = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkBeginCommandBuffer(
		command_buffer,
		&begin_info
	);
	return command_buffer;
}

void VulkanCommon::EndSingleTimeCommands(VulkanDevice * device, VkCommandBuffer command_buffer, VkCommandPool command_pool)
{
	vkEndCommandBuffer(command_buffer);
	VkSubmitInfo submit_info = VulkanInitializers::SubmitInfo(command_buffer);
	vkQueueSubmit(
		*device->GetGraphicsQueue(),
		1,
		&submit_info,
		VK_NULL_HANDLE
	);
	vkQueueWaitIdle(
		*device->GetGraphicsQueue()
	);
	vkFreeCommandBuffers(
		*device->GetVulkanDevice(),
		command_pool,
		1,
		&command_buffer
	);
}

bool VulkanCommon::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Renderer::Vulkan::VulkanCommon::CreateBuffer(VulkanDevice * device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBufferData & buffer)
{
	VkBufferCreateInfo buffer_info = VulkanInitializers::BufferCreateInfo(size, usage);

	vkCreateBuffer(
		*device->GetVulkanDevice(),
		&buffer_info,
		nullptr,
		&buffer.buffer
	);

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(
		*device->GetVulkanDevice(),
		buffer.buffer,
		&mem_requirements
	);


	VkMemoryAllocateInfo alloc_info = VulkanInitializers::MemoryAllocateInfo(mem_requirements.size, FindMemoryType(
		device->GetVulkanPhysicalDevice(),
		mem_requirements.memoryTypeBits,
		properties
	));
	buffer.size = mem_requirements.size;
	buffer.alignment = mem_requirements.alignment;
	vkAllocateMemory(
		*device->GetVulkanDevice(),
		&alloc_info,
		nullptr,
		&buffer.buffer_memory
	);

	vkBindBufferMemory(
		*device->GetVulkanDevice(),
		buffer.buffer,
		buffer.buffer_memory,
		0
	);
}

void Renderer::Vulkan::VulkanCommon::MapBufferMemory(VulkanDevice* device, VulkanBufferData & buffer, VkDeviceSize size)
{
	vkMapMemory(*device->GetVulkanDevice(), buffer.buffer_memory, 0, size, 0, &buffer.mapped_memory);
}

void Renderer::Vulkan::VulkanCommon::UnMapBufferMemory(VulkanDevice * device, VulkanBufferData & buffer)
{
	vkUnmapMemory(*device->GetVulkanDevice(), buffer.buffer_memory);
	buffer.mapped_memory = nullptr;
}

void Renderer::Vulkan::VulkanCommon::DestroyBuffer(VulkanDevice * device, VulkanBufferData & buffer)
{
	UnMapBufferMemory(device, buffer);

	vkDestroyBuffer(
		*device->GetVulkanDevice(),
		buffer.buffer,
		nullptr
	);
	vkFreeMemory(
		*device->GetVulkanDevice(),
		buffer.buffer_memory,
		nullptr
	);
}

std::vector<char> Renderer::Vulkan::VulkanCommon::ReadFile(const std::string & filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	size_t file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);
	file.seekg(0);
	file.read(buffer.data(), file_size);
	file.close();
	return buffer;
}

VkShaderModule Renderer::Vulkan::VulkanCommon::CreateShaderModule(VulkanDevice * device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo create_info = VulkanInitializers::ShaderModuleCreateInfo(code);

	VkShaderModule shader_module;
	vkCreateShaderModule(
		*device->GetVulkanDevice(),
		&create_info,
		nullptr,
		&shader_module
	);

	return shader_module;
}
