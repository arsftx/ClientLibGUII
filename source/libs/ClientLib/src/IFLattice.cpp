#include "IFLattice.h"

// Link to native runtime class at 0x9FFEB4
GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFLattice, 0x9FFEB4)

// Static member definitions
int CIFLattice::m_nColumns = 4;
int CIFLattice::m_nRows = 4;
int CIFLattice::m_nCellWidth = 32;
int CIFLattice::m_nCellHeight = 32;

CIFLattice::CIFLattice() {
    // Native constructor handles initialization
}

CIFLattice::~CIFLattice() {
}

void CIFLattice::SetGridSize(int cols, int rows) {
    m_nColumns = cols;
    m_nRows = rows;
}

void CIFLattice::SetCellSize(int width, int height) {
    m_nCellWidth = width;
    m_nCellHeight = height;
}

// IDA Analysis: Corner sizes control 9-slice grid rendering
// Offsets 0x638-0x654 store 8 values (4 textures × 2 dimensions each)
// sub_56B5A0 normally sets these from texture dimensions
// sub_56AFC0 (vertexCalc) reads these to calculate vertex positions
// sub_565CA0 (render) uses idiv [esi+638h] for grid line calculation
void CIFLattice::SetCornerSizes(int cellWidth, int cellHeight) {
    char* ptr = reinterpret_cast<char*>(this);
    
    // Set all 8 corner values to cell dimensions
    // This makes the 9-slice rendering use our cell size for grid calculations
    *reinterpret_cast<int*>(ptr + 0x638) = cellWidth;   // Corner 1 width
    *reinterpret_cast<int*>(ptr + 0x63C) = cellHeight;  // Corner 1 height
    *reinterpret_cast<int*>(ptr + 0x640) = cellWidth;   // Corner 2 width
    *reinterpret_cast<int*>(ptr + 0x644) = cellHeight;  // Corner 2 height
    *reinterpret_cast<int*>(ptr + 0x648) = cellWidth;   // Corner 3 width
    *reinterpret_cast<int*>(ptr + 0x64C) = cellHeight;  // Corner 3 height
    *reinterpret_cast<int*>(ptr + 0x650) = cellWidth;   // Corner 4 width
    *reinterpret_cast<int*>(ptr + 0x654) = cellHeight;  // Corner 4 height
}
