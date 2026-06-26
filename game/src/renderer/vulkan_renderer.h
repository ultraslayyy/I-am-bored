#pragma once
#ifdef WIN32
#include <vulkan/vulkan.h>
#include <windows.h>
#include "../core/renderer.h"

class VulkanRenderer : public IRenderer {
public:
    bool init(IWindow* window, int width, int height) override;
    void clear(int r, int g, int b) override;
    void present() override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;

    int getWidth() const override { return width; }
    int getHeight() const override { return height; }

private:
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily = 0;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;

    uint32_t imageIndex = 0;
    
    struct Vertex {
        float x, y, z;
        float r, g, b;
    };

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    Vertex* mappedVertices = nullptr;
    uint32_t currentVertexOffset = 0;
};
#endif