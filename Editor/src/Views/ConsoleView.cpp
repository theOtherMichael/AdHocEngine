#include "ConsoleView.h"

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <string>
#include <utility>
#include <vector>

using Engine::Console::LogLevel;

namespace Editor::Views
{

struct ConsoleEntry
{
    std::string message;
    LogLevel logLevel;

    ConsoleEntry() = default;

    explicit ConsoleEntry(std::string&& message, const LogLevel logLevel) : message(message), logLevel(logLevel) {}
};

static std::vector<ConsoleEntry> consoleLogs;

void HandleConsoleViewLogs(const LogLevel logLevel, const std::string& message)
{
    auto formattedMessage = fmt::format("[{}] {}", logLevel, message);
    consoleLogs.emplace_back(std::move(formattedMessage), logLevel);
}

static ImVec4 ColorOfLogLevel(LogLevel logLevel)
{
    auto defaultTextColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

    switch (logLevel)
    {
    case Engine::Console::LogLevel::Fatal:
        return {1.0f, 0.4f, 0.4f, 1.0f};
        break;
    case Engine::Console::LogLevel::Error:
        return {1.0f, 0.4f, 0.4f, 1.0f};
        break;
    case Engine::Console::LogLevel::Warning:
        return {1.0f, 1.0f, 0.4f, 1.0f};
        break;
    case Engine::Console::LogLevel::Log:
        return defaultTextColor;
        break;
    case Engine::Console::LogLevel::Trace:
        Assert_NoEntry();
        return {};
        break;
    default:
        Assert_NoEntry();
        return {};
        break;
    }
}

void DrawConsoleView()
{
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Console##EditorView", nullptr))
    {
        ImGui::End();
        return;
    }

    static bool autoScroll = true;
    ImGui::Checkbox("Auto-scroll", &autoScroll);

    ImGui::SameLine();
    if (ImGui::SmallButton("Log"))
        Engine::Console::Log("A basic log!");
    ImGui::SameLine();
    if (ImGui::SmallButton("Warning"))
        Engine::Console::LogWarning("A warning!");
    ImGui::SameLine();
    if (ImGui::SmallButton("Error"))
        Engine::Console::LogError("An error!");

    ImGui::Separator();

    if (ImGui::BeginChild(
            "ScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

        for (auto& log : consoleLogs)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ColorOfLogLevel(log.logLevel));
            ImGui::TextUnformatted(log.message.data());
            ImGui::PopStyleColor();
        }

        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

    ImGui::End();
}

} // namespace Editor::Views
