#include <Engine/Window/_platform/Windows/WindowsWindowIcon.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformHelpers.h>

#include <algorithm>
#include <bit>
#include <ranges>
#include <vector>

ASSERT_PLATFORM_WINDOWS;

namespace Engine::Window::Windows
{

// Windows helper types

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

// RAII helper types

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
    Assert_NotNull_Fmt(iconsResource, "Failed to find icons resource!");

    auto iconsResourceHandle = LoadResource(hInstance, iconsResource);
    Assert_NotNull_Fmt(iconsResourceHandle, "Failed to load icons resource!");

    auto* iconsDataBytes = static_cast<const BYTE*>(LockResource(iconsResourceHandle));
    Assert_NotNull_Fmt(iconsDataBytes, "Failed to lock icons resource!");

    auto* iconsDataDirectory = std::bit_cast<const ICONDIR*>(iconsDataBytes);
    Assert_Eq_Fmt(iconsDataDirectory->idType, 1, "Icons resource is not icon type!");

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

        auto* iconDataBytes    = static_cast<BYTE*>(LockResource(iconDataHandle));
        auto iconDataBytesSize = SizeofResource(hInstance, iconDataResource);

        auto iconHandle =
            CreateIconFromResourceEx(iconDataBytes, iconDataBytesSize, TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
        if (!iconHandle)
            continue;

        // Convert to GLFWImage

        auto iconInfo = ICONINFO{};
        GetIconInfo(iconHandle, &iconInfo);

        auto bitmap = BITMAP{};
        GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bitmap);

        size_t iconWidth  = bitmap.bmWidth;
        size_t iconHeight = bitmap.bmHeight;

        HDC screenDeviceContextHandle = GetDC(nullptr);
        HDC screenMemoryDeviceContext = CreateCompatibleDC(screenDeviceContextHandle);

        auto bitmapInfo                    = BITMAPINFO{};
        bitmapInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth       = iconWidth;
        bitmapInfo.bmiHeader.biHeight      = -iconHeight; // top-down
        bitmapInfo.bmiHeader.biPlanes      = 1;
        bitmapInfo.bmiHeader.biBitCount    = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        std::vector<unsigned char> pixels(iconWidth * iconHeight * 4);
        SelectObject(screenMemoryDeviceContext, iconInfo.hbmColor);
        GetDIBits(
            screenMemoryDeviceContext, iconInfo.hbmColor, 0, iconHeight, pixels.data(), &bitmapInfo, DIB_RGB_COLORS);

        DeleteDC(screenMemoryDeviceContext);
        ReleaseDC(nullptr, screenDeviceContextHandle);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);

        // Convert BGRA to RGBA
        for (size_t i = 0; i < iconWidth * iconHeight; ++i)
        {
            std::swap(pixels[i * 4 + 0], pixels[i * 4 + 2]); // R <-> B
        }

        auto buffer = std::make_unique<unsigned char[]>(iconWidth * iconHeight * 4);
        std::ranges::copy(pixels, buffer.get());

        ownedGlfwImages.emplace_back(iconWidth, iconHeight, std::move(buffer));

        DestroyIcon(iconHandle);
    }

    return ownedGlfwImages;
}

void SetWindowIconWithEmbeddedIconResource(GLFWwindow* window, int resourceId)
{
    auto hInstance = GetModuleHandle(nullptr);

    auto glfwIcons = GetGlfwIconsFromEmbeddedResource(hInstance, resourceId);
    if (glfwIcons.empty())
        return;

    auto unwrappedGlfwImages = std::vector<GLFWimage>{};
    unwrappedGlfwImages.reserve(glfwIcons.size());
    for (const auto& icon : glfwIcons)
    {
        unwrappedGlfwImages.push_back(icon.image);
    }

    glfwSetWindowIcon(window, static_cast<int>(unwrappedGlfwImages.size()), unwrappedGlfwImages.data());
}

} // namespace Engine::Window::Windows
