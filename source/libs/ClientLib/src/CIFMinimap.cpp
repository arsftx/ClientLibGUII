#include "CIFMinimap.h"
#include "GInterface.h"
#include "TextStringManager.h"
#include <stdio.h>

// GDR_MINIMAP ID from ginterface.txt (ID=10, NOT 15!)
#define GDR_MINIMAP 10

CIFMinimap* CIFMinimap::GetInstance() {
    if (!g_pCGInterface) return NULL;
    
    // Get from CGInterface IRM - GDR_MINIMAP = 15
    return (CIFMinimap*)g_pCGInterface->m_IRM.GetResObj(GDR_MINIMAP, 1);
}

// Get region name from TextStringManager using region ID
// Native minimap uses: sprintf(buffer, "%d", regionID); TSM->GetString(buffer);
const char* CIFMinimap::GetRegionName() const {
    static char regionIdBuffer[16];
    
    // Combine region X and Y into full region ID (16-bit value)
    int regionID = GetRegionID();
    
    // Convert to string - native uses sprintf with format "%d"
    sprintf(regionIdBuffer, "%d", regionID);
    
    // Get region name from TextStringManager
    return g_CTextStringManager->GetString3(regionIdBuffer);
}
