/* Start Header ************************************************************************/
/*!
\file       BuildSizeAnalyzer.h
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <cstdint>

/**
 * @struct AssetEntry
 * @brief Metadata for a single file discovered during the asset scan.
 */
struct AssetEntry
{
    std::string fullPath;   ///< Absolute (or working-dir-relative) path used for file I/O.
    std::string relPath;    ///< Path relative to the Assets root, displayed in the table.
    uintmax_t   sizeBytes;  ///< File size in bytes.
    bool        selected;   ///< Whether this file is currently marked for export.
};

/**
 * @class BuildSizeAnalyzer
 * @brief ImGui editor window for reviewing, selecting, and exporting project assets.
 *
 * Features
 * --------
 *  - Recursively scans ../Assets and lists every file with its size.
 *  - Per-file checkboxes plus Select All / Deselect All.
 *  - Dynamically updated total size of the current selection.
 *  - Persists the deselected-file list to ../build_size_selections.json so
 *    selections survive editor restarts.  New files added after the last save
 *    default to selected.
 *  - Export button opens a native folder-picker and copies all selected files
 *    to the chosen destination, preserving the relative directory structure.
 *  - Accessible from the Scene Manager panel (Tools section).
 *
 * Usage (from LevelEditor)
 * ------------------------
 * @code
 *   // In LevelEditor::Update(), every frame:
 *   m_buildSizeAnalyzer.Render();
 *
 *   // In RenderMenuBar() / Scene Manager button:
 *   if (ImGui::Button("Build Size Analyzer"))
 *       m_buildSizeAnalyzer.Open();
 * @endcode
 */
class BuildSizeAnalyzer
{
public:
    BuildSizeAnalyzer()  = default;
    ~BuildSizeAnalyzer() = default;

    /** Make the window visible.  Triggers a fresh asset scan on first open. */
    void Open();

    /**
     * @brief Render the ImGui window.  Must be called every frame from the editor update.
     *        Does nothing when the window is closed.
     */
    void Render();

    /** @return True while the window is open. */
    bool IsOpen() const { return m_isOpen; }

private:
    // ===== Internal helpers =====

    /** Recursively walk k_assetsRoot and populate m_assets. */
    void ScanAssets();

    /** Recompute m_totalSelectedBytes from the current selection. */
    void UpdateTotal();

    /**
     * @brief Persist the deselected-file list to k_selectionsFile.
     *        Files NOT in this list are treated as selected on next load,
     *        so newly added assets default to included.
     */
    void SaveSelections() const;

    /**
     * @brief Restore selections from k_selectionsFile.
     *        Must be called after ScanAssets() so m_assets is populated.
     */
    void LoadSelections();

    /**
     * @brief Show a native OS folder-picker dialog.
     * @return Chosen path, or empty string if cancelled.
     */
    static std::string BrowseForFolder();

    /**
     * @brief Copy all selected files to destFolder, reproducing the relative
     *        sub-directory structure under destFolder.
     * @return True if every copy succeeded.
     */
    bool ExportSelected(const std::string& destFolder);

    /** Format a byte count as a human-readable string (B / KB / MB / GB). */
    static std::string FormatBytes(uintmax_t bytes);

    // ===== State =====
    bool m_isOpen    = false;
    bool m_needsScan = true;   ///< Set on construction and after Refresh clicks.

    std::vector<AssetEntry> m_assets;
    uintmax_t               m_totalSelectedBytes = 0;

    char m_filter[128] = {};   ///< Live text filter applied to relPath display.

    std::string m_statusMsg;          ///< Last export result message shown in footer.
    bool        m_statusIsError = false;

    // ===== Constants =====
    static constexpr const char* k_assetsRoot     = "../Assets";
    static constexpr const char* k_selectionsFile = "../build_size_selections.json";
};
