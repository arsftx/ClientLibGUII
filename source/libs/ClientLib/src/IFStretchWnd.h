#pragma once

#include "IFWnd.h"
#include <Test/Test.h>

// =====================================================================
// CIFStretchWnd - Container with stretching edges
// IDA Analysis (ECSRO):
//   Runtime Class: 0x9FFF14
//   Size: 0x68C (1676 bytes)
//   Constructor: sub_5679E0
//   Parent: CIFWnd (0x9FE5C0)
// =====================================================================
/// \brief Container, automatically stretching to screen width (or parent width, unsure)
/// \details Container resizes and draws borders accordingly. Allows specifing textures
///          for top, left, bottom and right edges, as well as corners.
class CIFStretchWnd : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFStretchWnd, 0x9FFF14)

public: /* interface functions */

    /// \brief Load all 8 textures (4 corners + 4 edges) from DDJ prefix
    /// \details The prefix is like "interface\\ifcommon\\lattice_window\\com_lattice_outline_"
    ///          Appends: left_down.ddj, right_down.ddj, right_up.ddj, left_up.ddj (corners)
    ///                   left_side.ddj, right_side.ddj (edges)
    /// \address 0056A1B0
    void LoadTexturesFromPrefix(const char* ddjPrefix);

private: /* members */
    char pad_0x036C[0x34]; //0x036C
    char *N000062E6; //0x03A0
    char pad_0x03A4[0x18]; //0x03A4
    char *N000062F0; //0x03BC
    char pad_0x03C0[0x18]; //0x03C0
    char *N000062F7; //0x03D8
    char pad_0x03DC[0x18]; //0x03DC
    char *N000062FE; //0x03F4
    char pad_0x03F8[0x18]; //0x03F8
    char *N00006305; //0x0410
    char pad_0x0414[0x18]; //0x0414
    char *N0000630C; //0x042C
    char pad_0x0430[0x18]; //0x0430
    char *N00006313; //0x0448
    char pad_0x044C[0x18]; //0x044C
    char *N0000631A; //0x0464
    char pad_0x0468[0x35C]; //0x0468

private:
    /*BEGIN_FIXTURE()
        ENSURE_SIZE(0x07C4)
    END_FIXTURE()

    RUN_FIXTURE(CIFStretchWnd)*/
};
