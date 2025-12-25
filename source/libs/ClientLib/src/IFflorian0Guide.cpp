#include "IFflorian0Guide.h"
#include <BSLib/Debug.h>
#include "GEffSoundBody.h"
#include "GInterface.h"
#include "Game.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <windows.h>

// =============================================================================
// D3D9 Viewport Resolution (updated every frame from EndScene hook)
// Defined in DevKit_DLL/src/hooks/Hooks.cpp
// =============================================================================
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

GFX_IMPLEMENT_DYNCREATE(CIFflorian0Guide, CIFDecoratedStatic)

// Track hover state for texture swapping
static bool g_isHovering = false;

// Debug log function
static void DebugLog(const char* format, ...)
{
    const char* logPath = "clientlog.txt";
    
    char buffer[2048];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';
    
    time_t now = time(0);
    struct tm tstruct;
    char timeBuf[80];
    localtime_s(&tstruct, &now);
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tstruct);
    
    FILE* fp = fopen(logPath, "a");
    if (fp) {
        fprintf(fp, "[%s] %s\n", timeBuf, buffer);
        fclose(fp);
    }
    
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}

bool CIFflorian0Guide::OnCreate(long ln)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", ln);
	
	// Call parent first
	CIFDecoratedStatic::OnCreate(ln);

	// IRM LOADER - Using g_pCGInterface->m_IRM
	if (g_pCGInterface) {
		g_pCGInterface->m_IRM.LoadFromFile("resinfo\\ifflorian0guide.txt");
		g_pCGInterface->m_IRM.CreateInterfaceSection("Create", (CObj*)g_pCGInterface);
	}
	
	// Load the icon texture 
	TB_Func_13("icon\\etc\\bugle_icon_1.ddj", 0, 0);
	
	// Set animation parameters
	set_N00009BD4(2);
	set_N00009BD3(500);

	// Force widget bounds to exactly 40x40
	SetGWndSize(40, 40);
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x44) = 40; // bounds.size.width
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x48) = 40; // bounds.size.height

	return true;
}

int CIFflorian0Guide::OnMouseLeftUp(int a1, int x, int y)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
	
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	const int BUTTON_WIDTH = 40;
	const int BUTTON_HEIGHT = 40;
	
	if (x < posX || x >= posX + BUTTON_WIDTH ||
	    y < posY || y >= posY + BUTTON_HEIGHT) {
		return 0;
	}
	
	DebugLog("[CIFflorian0Guide] OnClick at (%d, %d)", x, y);

	// TODO: Open florian0 main window when clicked
	// CGEffSoundBody::get()->PlaySound(L"snd_quest");

	return 1;
}

int CIFflorian0Guide::OnMouseLeftDown(int a1, int x, int y)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
	
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	const int BUTTON_WIDTH = 40;
	const int BUTTON_HEIGHT = 40;
	
	if (x < posX || x >= posX + BUTTON_WIDTH ||
	    y < posY || y >= posY + BUTTON_HEIGHT) {
		return 0;
	}

	return 1;
}

int CIFflorian0Guide::OnMouseMove(int a1, int x, int y)
{
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	const int BUTTON_WIDTH = 40;
	const int BUTTON_HEIGHT = 40;
	
	if (x < posX || x >= posX + BUTTON_WIDTH ||
	    y < posY || y >= posY + BUTTON_HEIGHT) {
		// Mouse is outside button area - reset hover state
		if (g_isHovering) {
			g_isHovering = false;
			set_N00009BD0(0);
		}
		return 0;
	}
	
	// Mouse is inside the button area
	if (!g_isHovering) {
		g_isHovering = true;
		DebugLog("[CIFflorian0Guide] OnEnter");
		set_N00009BD0(1);
	}
	
	return 1;
}

void CIFflorian0Guide::OnUpdate()
{
	CIFDecoratedStatic::OnUpdate();
	
	// ==========================================================================
	// DYNAMIC RESOLUTION-AWARE REPOSITIONING
	// Detect resolution changes and reposition button automatically
	// ==========================================================================
	static int s_lastWidth = 0;
	static int s_lastHeight = 0;
	
	int currentWidth = g_D3DViewportWidth;
	int currentHeight = g_D3DViewportHeight;
	
	// Check if resolution changed (and is valid)
	if (currentWidth > 0 && currentHeight > 0 &&
	    (currentWidth != s_lastWidth || currentHeight != s_lastHeight)) {
		
		s_lastWidth = currentWidth;
		s_lastHeight = currentHeight;
		
		// Reposition button: right side, below minimap area
		int posX = currentWidth - 60;
		int posY = 200;
		
		MoveGWnd(posX, posY);
	}
}

void CIFflorian0Guide::OnCIFReady()
{
	static bool s_positionSet = false;
	if (s_positionSet) {
		CIFDecoratedStatic::OnCIFReady();
		return;
	}
	s_positionSet = true;
	
	BS_DEBUG_LOW("> " __FUNCTION__);

	CIFDecoratedStatic::OnCIFReady();
	sub_633990();

	// Initial positioning using D3D9 viewport resolution
	int screenWidth = g_D3DViewportWidth;
	int screenHeight = g_D3DViewportHeight;
	
	// Fallback to system metrics if D3D9 not ready yet
	if (screenWidth <= 0 || screenWidth > 7680 || screenHeight <= 0 || screenHeight > 4320) {
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
	}
	
	// Position: right side, below minimap area
	int posX = screenWidth - 60;
	int posY = 200;
	
	MoveGWnd(posX, posY);
}

bool CIFflorian0Guide::IsInside(int x, int y)
{
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	const int BUTTON_WIDTH = 40;
	const int BUTTON_HEIGHT = 40;
	
	return (x >= posX && x < posX + BUTTON_WIDTH &&
	        y >= posY && y < posY + BUTTON_HEIGHT);
}
