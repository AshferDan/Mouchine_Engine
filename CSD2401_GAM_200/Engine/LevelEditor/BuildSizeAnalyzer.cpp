/* Start Header ************************************************************************/
/*!
\file       BuildSizeAnalyzer.cpp
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "BuildSizeAnalyzer.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

namespace fs = std::filesystem;

// Windows native folder-picker (IFileDialog, Vista+)
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <shobjidl.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Shell32.lib")
#endif

// =============================================================================
//  Public API
// =============================================================================

void BuildSizeAnalyzer::Open()
{
    m_isOpen = true;
    // Always do a fresh scan when the window is opened so the list reflects
    // the current state of the Assets folder.
    m_needsScan = true;
    m_statusMsg.clear();
}

void BuildSizeAnalyzer::Render()
{
    if (!m_isOpen) return;

    // Lazy scan: runs once per Open() call.
    if (m_needsScan)
    {
        ScanAssets();
        LoadSelections();   // restore previously deselected files
        m_needsScan = false;
        UpdateTotal();
    }

    ImGui::SetNextWindowSize(ImVec2(720.0f, 520.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(480.0f, 300.0f), ImVec2(FLT_MAX, FLT_MAX));

    bool wasOpen = m_isOpen;
    if (!ImGui::Begin("Build Size Analyzer##BSA", &m_isOpen, ImGuiWindowFlags_NoCollapse))
    {
        // Collapsed or just closed
        if (wasOpen && !m_isOpen) SaveSelections();
        ImGui::End();
        return;
    }

    // -------------------------------------------------------------------------
    //  Toolbar row
    // -------------------------------------------------------------------------
    if (ImGui::Button("Refresh"))
    {
        m_needsScan = true;
        m_statusMsg.clear();
    }
    ImGui::SameLine();

    if (ImGui::Button("Select All"))
    {
        for (auto& a : m_assets) a.selected = true;
        UpdateTotal();
        SaveSelections();
    }
    ImGui::SameLine();

    if (ImGui::Button("Deselect All"))
    {
        for (auto& a : m_assets) a.selected = false;
        UpdateTotal();
        SaveSelections();
    }
    ImGui::SameLine();

    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputTextWithHint("##bsa_filter", "Filter by path...", m_filter, sizeof(m_filter));

    ImGui::Separator();

    // -------------------------------------------------------------------------
    //  Asset table
    // -------------------------------------------------------------------------
    // Reserve space for the footer bar below the table.
    const float footerH = ImGui::GetFrameHeightWithSpacing() * 2.5f;
    const float tableH  = ImGui::GetContentRegionAvail().y - footerH;

    constexpr ImGuiTableFlags kTableFlags =
        ImGuiTableFlags_Borders       |
        ImGuiTableFlags_RowBg         |
        ImGuiTableFlags_ScrollY       |
        ImGuiTableFlags_Resizable     |
        ImGuiTableFlags_SortTristate  |
        ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("##bsa_table", 3, kTableFlags, ImVec2(0.0f, tableH)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);   // freeze header row
        ImGui::TableSetupColumn("##sel",  ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 26.0f);
        ImGui::TableSetupColumn("Path",   ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Size",   ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        // Build a lower-case copy of the filter string for case-insensitive matching.
        std::string filterLow(m_filter);
        for (char& c : filterLow) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        for (int i = 0; i < static_cast<int>(m_assets.size()); ++i)
        {
            AssetEntry& a = m_assets[i];

            // Apply filter
            if (!filterLow.empty())
            {
                std::string pathLow = a.relPath;
                for (char& c : pathLow) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                if (pathLow.find(filterLow) == std::string::npos)
                    continue;
            }

            ImGui::TableNextRow();

            // --- Checkbox column ---
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(i);
            if (ImGui::Checkbox("##cb", &a.selected))
            {
                UpdateTotal();
                SaveSelections();
            }
            ImGui::PopID();

            // --- Path column ---
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(a.relPath.c_str());

            // Tooltip with full path on hover
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s", a.fullPath.c_str());

            // --- Size column ---
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(FormatBytes(a.sizeBytes).c_str());
        }

        ImGui::EndTable();
    }

    // -------------------------------------------------------------------------
    //  Footer bar  (summary + Export button)
    // -------------------------------------------------------------------------
    ImGui::Separator();

    int totalFiles    = static_cast<int>(m_assets.size());
    int selectedCount = 0;
    for (const auto& a : m_assets)
        if (a.selected) ++selectedCount;

    ImGui::Text("Selected: %d / %d files   Total: %s",
                selectedCount, totalFiles,
                FormatBytes(m_totalSelectedBytes).c_str());

    // Right-align the Export button on the same line.
    const float btnW      = 110.0f;
    const float rightEdge = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
    ImGui::SameLine(rightEdge - btnW);

    const bool hasSelection = (selectedCount > 0);
    if (!hasSelection) ImGui::BeginDisabled();

    if (ImGui::Button("Export...", ImVec2(btnW, 0.0f)))
    {
        std::string dest = BrowseForFolder();
        if (!dest.empty())
        {
            if (ExportSelected(dest))
            {
                m_statusMsg     = "Export complete -> " + dest;
                m_statusIsError = false;
                std::cout << "[BuildSizeAnalyzer] " << m_statusMsg << "\n";
            }
            else
            {
                m_statusMsg     = "Export finished with errors (check console).";
                m_statusIsError = true;
                std::cerr << "[BuildSizeAnalyzer] " << m_statusMsg << "\n";
            }
        }
    }

    if (!hasSelection) ImGui::EndDisabled();

    // Status message row
    if (!m_statusMsg.empty())
    {
        ImVec4 colour = m_statusIsError
            ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
            : ImVec4(0.35f, 1.0f, 0.35f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, colour);
        ImGui::TextWrapped("%s", m_statusMsg.c_str());
        ImGui::PopStyleColor();
    }

    // Detect X-button close → persist selections.
    if (wasOpen && !m_isOpen) SaveSelections();

    ImGui::End();
}

// =============================================================================
//  Private helpers
// =============================================================================

void BuildSizeAnalyzer::ScanAssets()
{
    m_assets.clear();

    const fs::path root(k_assetsRoot);
    if (!fs::exists(root) || !fs::is_directory(root))
    {
        std::cerr << "[BuildSizeAnalyzer] Assets root not found: " << root << "\n";
        return;
    }

    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(
            root, fs::directory_options::skip_permission_denied, ec))
    {
        if (ec) { ec.clear(); continue; }
        if (!entry.is_regular_file(ec)) { ec.clear(); continue; }

        const fs::path& filePath = entry.path();

        // Skip hidden files / system files (names starting with '.')
        const std::string fname = filePath.filename().string();
        if (!fname.empty() && fname.front() == '.') continue;

        const uintmax_t size = entry.file_size(ec);
        if (ec) { ec.clear(); }

        fs::path rel = fs::relative(filePath, root, ec);
        if (ec) { ec.clear(); rel = filePath; }

        AssetEntry a;
        a.fullPath  = filePath.string();
        a.relPath   = rel.string();
        // Normalise path separators to '/' for consistent display.
        for (char& c : a.relPath) if (c == '\\') c = '/';
        a.sizeBytes = (ec ? 0 : size);
        a.selected  = true;   // default: selected (may be overridden by LoadSelections)

        m_assets.push_back(std::move(a));
    }

    // Sort alphabetically by relative path.
    std::sort(m_assets.begin(), m_assets.end(),
        [](const AssetEntry& x, const AssetEntry& y) {
            return x.relPath < y.relPath;
        });

    std::cout << "[BuildSizeAnalyzer] Scanned " << m_assets.size()
              << " assets from " << root << "\n";
}

void BuildSizeAnalyzer::UpdateTotal()
{
    m_totalSelectedBytes = 0;
    for (const auto& a : m_assets)
        if (a.selected)
            m_totalSelectedBytes += a.sizeBytes;
}

void BuildSizeAnalyzer::SaveSelections() const
{
    // We persist the list of DESELECTED paths.
    // Files absent from this list are treated as selected on next load,
    // so newly added assets automatically default to included.
    nlohmann::json j;
    j["version"] = 1;

    std::vector<std::string> deselected;
    deselected.reserve(m_assets.size());
    for (const auto& a : m_assets)
        if (!a.selected)
            deselected.push_back(a.relPath);
    j["deselected"] = deselected;

    std::ofstream f(k_selectionsFile);
    if (f.is_open())
    {
        f << j.dump(2);
        std::cout << "[BuildSizeAnalyzer] Saved selections (" << deselected.size()
                  << " deselected) to " << k_selectionsFile << "\n";
    }
    else
    {
        std::cerr << "[BuildSizeAnalyzer] Could not write " << k_selectionsFile << "\n";
    }
}

void BuildSizeAnalyzer::LoadSelections()
{
    std::ifstream f(k_selectionsFile);
    if (!f.is_open()) return;   // first run — all assets stay selected

    try
    {
        const nlohmann::json j = nlohmann::json::parse(f);
        if (!j.contains("deselected")) return;

        std::unordered_set<std::string> deselectedPaths;
        for (const auto& entry : j["deselected"])
            deselectedPaths.insert(entry.get<std::string>());

        // Deselect explicitly saved paths; everything else (including new files)
        // stays selected.
        for (auto& a : m_assets)
            if (deselectedPaths.count(a.relPath))
                a.selected = false;

        std::cout << "[BuildSizeAnalyzer] Restored selections ("
                  << deselectedPaths.size() << " deselected).\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[BuildSizeAnalyzer] Could not parse selections file: "
                  << ex.what() << " — using defaults.\n";
    }
}

std::string BuildSizeAnalyzer::BrowseForFolder()
{
    std::string result;

#ifdef _WIN32
    // CoInitializeEx is safe to call even if COM is already initialised.
    const HRESULT hrInit = CoInitializeEx(nullptr,
                                          COINIT_APARTMENTTHREADED |
                                          COINIT_DISABLE_OLE1DDE);
    const bool comInitedHere = SUCCEEDED(hrInit);

    IFileDialog* pfd = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                                   CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        DWORD opts = 0;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);
        pfd->SetTitle(L"Select Export Destination Folder");

        if (SUCCEEDED(pfd->Show(nullptr)))
        {
            IShellItem* psi = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                {
                    // Convert wide string → UTF-8.
                    const int len = WideCharToMultiByte(
                        CP_UTF8, 0, pszPath, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0)
                    {
                        result.resize(static_cast<size_t>(len) - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pszPath, -1,
                                            result.data(), len, nullptr, nullptr);
                    }
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }

    if (comInitedHere) CoUninitialize();
#endif

    return result;
}

bool BuildSizeAnalyzer::ExportSelected(const std::string& destFolder)
{
    const fs::path dest(destFolder);
    bool allOk = true;
    int  copied = 0;

    for (const auto& a : m_assets)
    {
        if (!a.selected) continue;

        const fs::path src(a.fullPath);
        const fs::path dst = dest / a.relPath;

        std::error_code ec;
        fs::create_directories(dst.parent_path(), ec);
        if (ec)
        {
            std::cerr << "[BuildSizeAnalyzer] mkdir failed for "
                      << dst.parent_path() << ": " << ec.message() << "\n";
            allOk = false;
            continue;
        }

        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec)
        {
            std::cerr << "[BuildSizeAnalyzer] copy failed: "
                      << src << " -> " << dst << ": " << ec.message() << "\n";
            allOk = false;
        }
        else
        {
            ++copied;
        }
    }

    std::cout << "[BuildSizeAnalyzer] Exported " << copied
              << " file(s) to " << destFolder << "\n";
    return allOk;
}

std::string BuildSizeAnalyzer::FormatBytes(uintmax_t bytes)
{
    char buf[32];
    constexpr uintmax_t KB = 1024;
    constexpr uintmax_t MB = 1024 * KB;
    constexpr uintmax_t GB = 1024 * MB;

    if      (bytes < KB) snprintf(buf, sizeof(buf), "%llu B",   static_cast<unsigned long long>(bytes));
    else if (bytes < MB) snprintf(buf, sizeof(buf), "%.1f KB",  static_cast<double>(bytes) / static_cast<double>(KB));
    else if (bytes < GB) snprintf(buf, sizeof(buf), "%.2f MB",  static_cast<double>(bytes) / static_cast<double>(MB));
    else                 snprintf(buf, sizeof(buf), "%.2f GB",  static_cast<double>(bytes) / static_cast<double>(GB));

    return buf;
}
