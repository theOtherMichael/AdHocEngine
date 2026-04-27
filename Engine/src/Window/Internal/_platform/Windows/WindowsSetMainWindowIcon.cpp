#include "WindowsSetMainWindowIcon.h"

#include "_platform/Windows/resource.h"
#include <asl/casts.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Window/WindowState.h>

#include <GLFW/glfw3.h>

#include <windows.h>

#include <algorithm>
#include <bit>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

static_assert(ADHOC_WINDOWS);

namespace Engine::Window::Internal::Platform
{

#pragma pack(push, 1)

struct ICONDIR
{
    WORD idReserved;
    WORD idType;
    WORD idCount;
};

struct ICONDIRENTRY
{
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
};

#pragma pack(pop)

struct OwnedGlfwImage
{
    GLFWimage image;
    std::unique_ptr<unsigned char[]> pixels;

    OwnedGlfwImage(int width, int height, std::unique_ptr<unsigned char[]> data) : pixels(std::move(data))
    {
        image.width  = width;
        image.height = height;
        image.pixels = pixels.get();
    }
};

static std::vector<OwnedGlfwImage> GetGlfwIconsFromEmbeddedResource(HINSTANCE hInstance, int resourceID)
{
    // Load resource

    auto iconsResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_GROUP_ICON);
    Assert_NotNull_Fmt(iconsResource, "Failed to find icons resource");

    auto iconsResourceHandle = LoadResource(hInstance, iconsResource);
    Assert_NotNull_Fmt(iconsResourceHandle, "Failed to load icons resource");

    auto* iconsDataBytes = static_cast<const BYTE*>(LockResource(iconsResourceHandle));
    Assert_NotNull_Fmt(iconsDataBytes, "Failed to lock icons resource");

    auto* iconsDataDirectory = std::bit_cast<const ICONDIR*>(iconsDataBytes);
    Assert_Eq_Fmt(iconsDataDirectory->idType, 1, "Icons resource is not icon type");

    auto ownedGlfwImages = std::vector<OwnedGlfwImage>{};

    for (int i = 0; i < iconsDataDirectory->idCount; ++i)
    {
        // Load icon

        auto iconDataResource = FindResource(hInstance, MAKEINTRESOURCE(i + 1), RT_ICON);
        if (!iconDataResource)
            continue;

        auto iconDataHandle = LoadResource(hInstance, iconDataResource);
        if (!iconDataHandle)
            continue;

        auto iconDataBytesSize = SizeofResource(hInstance, iconDataResource);
        auto* iconDataBytes    = static_cast<BYTE*>(LockResource(iconDataHandle));

        auto iconHandle =
            CreateIconFromResourceEx(iconDataBytes, iconDataBytesSize, TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
        if (!iconHandle)
            continue;

        // Convert to GLFWImage

        auto iconInfo = ICONINFO{};
        GetIconInfo(iconHandle, &iconInfo);

        auto bitmap = BITMAP{};
        GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bitmap);

        auto iconWidth  = bitmap.bmWidth;
        auto iconHeight = bitmap.bmHeight;

        auto screenDeviceContextHandle = GetDC(nullptr);
        auto screenMemoryDeviceContext = CreateCompatibleDC(screenDeviceContextHandle);

        auto bitmapInfo = BITMAPINFO{
            .bmiHeader = {.biSize        = sizeof(BITMAPINFOHEADER),
                          .biWidth       = iconWidth,
                          .biHeight      = -iconHeight, // top-down
                          .biPlanes      = 1,
                          .biBitCount    = 32,
                          .biCompression = BI_RGB}
        };

        auto pixels = std::vector<unsigned char>(iconWidth * iconHeight * 4);
        SelectObject(screenMemoryDeviceContext, iconInfo.hbmColor);
        GetDIBits(
            screenMemoryDeviceContext, iconInfo.hbmColor, 0, iconHeight, pixels.data(), &bitmapInfo, DIB_RGB_COLORS);

        DeleteDC(screenMemoryDeviceContext);
        ReleaseDC(nullptr, screenDeviceContextHandle);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);

        const auto numPixelsInIcon = asl::narrow<size_t>(iconWidth * iconHeight);

        // Convert BGRA to RGBA

        for (size_t i = 0; i < numPixelsInIcon; ++i)
        {
            std::swap(pixels[i * 4 + 0], pixels[i * 4 + 2]); // R <-> B
        }

        auto buffer = std::make_unique<unsigned char[]>(numPixelsInIcon * 4);
        std::ranges::copy(pixels, buffer.get());

        ownedGlfwImages.emplace_back(iconWidth, iconHeight, std::move(buffer));

        DestroyIcon(iconHandle);
    }

    return ownedGlfwImages;
}

void SetMainWindowIconImplementation()
{
#if ADHOC_DEBUG
    auto hInstance = GetModuleHandle(L"EngineD");
#elif ADHOC_DEV
    auto hInstance = GetModuleHandle(L"EngineDev");
#else // ADHOC_RELEASE
    auto hInstance = GetModuleHandle(L"Engine");
#endif

    auto glfwIcons = GetGlfwIconsFromEmbeddedResource(hInstance, IDI_ICON1);
    if (glfwIcons.empty())
        return;

    auto unwrappedGlfwImages = std::vector<GLFWimage>{};
    unwrappedGlfwImages.reserve(glfwIcons.size());
    for (const auto& icon : glfwIcons)
    {
        unwrappedGlfwImages.push_back(icon.image);
    }

    auto* const mainWindow = WindowState::Instance().mainWindowHandle;
    glfwSetWindowIcon(mainWindow, asl::narrow<int>(unwrappedGlfwImages.size()), unwrappedGlfwImages.data());
}

} // namespace Engine::Window::Internal::Platform
