#pragma once

#include "IFTileWnd.h"

// =====================================================================
// CIFLattice - Grid/Lattice Window for Inventory-style Slots
// Used for skill grids, item inventory slots, etc.
// Renders a grid pattern using com_lattice_ textures
// =====================================================================
// IDA Analysis (ECSRO):
//   Runtime Class: 0x9FFEB4
//   Size: 0x678 (same as CIFTileWnd - no extra members)
//   Constructor: sub_565BB0
//   GetRuntimeClass: sub_565C10
//   Parent: CIFTileWnd (0x9FFF54)
// =====================================================================

class CIFLattice : public CIFTileWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFLattice, 0x9FFEB4)

public:
    CIFLattice();
    ~CIFLattice();

    // Note: CIFLattice inherits everything from CIFTileWnd
    // The grid functionality is handled by the native implementation
    // We don't add custom methods here to avoid size mismatch

    // These are helper methods that don't affect class size (they just store values)
    void SetGridSize(int cols, int rows);
    int GetColumns() const { return m_nColumns; }
    int GetRows() const { return m_nRows; }

    void SetCellSize(int width, int height);
    int GetCellWidth() const { return m_nCellWidth; }
    int GetCellHeight() const { return m_nCellHeight; }

    // IDA Analysis: Corner sizes at 0x638-0x654 control 9-slice rendering
    // sub_56B5A0 sets these from texture dimensions, but we need to override
    // for correct grid rendering with custom dimensions
    void SetCornerSizes(int cellWidth, int cellHeight);

private:
    // WARNING: CIFLattice and CIFTileWnd both are 0x678 bytes
    // These are NOT actual class members - they're stored separately
    // to avoid changing class size. Use static or external storage if needed.
    static int m_nColumns;
    static int m_nRows;
    static int m_nCellWidth;
    static int m_nCellHeight;
};
