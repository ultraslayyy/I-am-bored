#ifdef WIN32
#ifdef USE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan_renderer.h"
#include <vulkan/vulkan_win32.h>

bool VulkanRenderer::init(IWindow* window_p, int w, int h) {
    hwnd = (HWND)window_p->getNativeHandle();
    width = w;
    height = h;

    // Instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = 2;
    instanceInfo.ppEnabledExtensionNames = extensions;

    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) return false;

    // Surface
    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = hwnd;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);

    vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);

    // Physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    physicalDevice = devices[0]; // TODO: pick properly

    // Queue family
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    for (uint32_t i = 0; i < queueCount; ++i) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
            graphicsQueueFamily = i;
            break;
        }
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qInfo{};
    qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qInfo.queueFamilyIndex = graphicsQueueFamily;
    qInfo.queueCount = 1;
    qInfo.pQueuePriorities = &priority;

    const char* deviceExt[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &qInfo;
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExt;

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) return false;

    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

    // Swapchain
    VkSwapchainCreateInfoKHR swapInfo{};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = surface;
    swapInfo.minImageCount = 2;
    swapInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapInfo.imageExtent = {(uint32_t)width, (uint32_t)height};
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    vkCreateSwapchainKHR(device, &swapInfo, nullptr, &swapchain);

    uint32_t imgCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imgCount, nullptr);

    swapchainImages.resize(imgCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imgCount, swapchainImages.data());

    swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainExtent = swapInfo.imageExtent;

    // Image views
    swapchainImageViews.resize(imgCount);

    for (uint32_t i = 0; i < imgCount; ++i) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchainFormat;
        viewInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY};

        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(device, &viewInfo, nullptr, &swapchainImageViews[i]);
    }

    // Command pool + buffer
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueFamily;

    vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // Sync objects
    VkSemaphoreCreateInfo semInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailable);
    vkCreateSemaphore(device, &semInfo, nullptr, &renderFinished);
    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence);

    // Vertex buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * 6 * 1000;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memReq);

    VkMemoryAllocateInfo bufferAlloc{};
    bufferAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    bufferAlloc.allocationSize = memReq.size;

    bufferAlloc.memoryTypeIndex = 0; // TODO: Select properly later

    vkAllocateMemory(device, &bufferAlloc, nullptr, &vertexMemory);
    vkBindBufferMemory(device, vertexBuffer, vertexMemory, 0);

    vkMapMemory(device, vertexMemory, 0, bufferInfo.size, 0, (void**)&mappedVertices);

    return true;
}

void VulkanRenderer::clear(int r, int g, int b) {
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);

    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkResetCommandBuffer(commandBuffer, 0);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkClearColorValue clearColor = {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        1.0f
    };

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.levelCount = 1;
    range.layerCount = 1;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.image = swapchainImages[imageIndex];

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkCmdClearColorImage(commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
}

void VulkanRenderer::present() {
    // Finish recording
    vkEndCommandBuffer(commandBuffer);

    // Submit
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailable;
    submitInfo.pWaitDstStageMask = &waitStage;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinished;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);

    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinished;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(graphicsQueue, &presentInfo);
}

void VulkanRenderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    float nx = (float)x / width * 2.0f - 1.0f;
    float ny = 1.0f - (float)y / height * 2.0f;
    float nx2 = (float)(x + w) / width * 2.0f - 1.0f;
    float ny2 = 1.0f - (float)(y + h) / height * 2.0f;

    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    Vertex quad[6] = {
        {nx,  ny,  0, rf, gf, bf},
        {nx2, ny,  0, rf, gf, bf},
        {nx,  ny2, 0, rf, gf, bf},

        {nx2, ny,  0, rf, gf, bf},
        {nx2, ny2, 0, rf, gf, bf},
        {nx,  ny2, 0, rf, gf, bf},
    };

    memcpy(mappedVertices + currentVertexOffset, quad, sizeof(quad));
    currentVertexOffset += 6;
}
#endif
#endif