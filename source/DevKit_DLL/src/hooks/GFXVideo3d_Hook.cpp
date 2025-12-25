#include "Hooks.h"
#include "GFXVideo3d_Hook.h"
#include <d3d9.h>

extern std::vector<endscene_handler_t> hooks_endscene;
extern std::vector<create_handler_t> hooks_create;
extern std::vector<setsize_handler_t> hooks_setsize_pre;
extern std::vector<setsize_handler_t> hooks_setsize_post;

// Global resolution variables (defined in Hooks.cpp)
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

bool CGFXVideo3D_Hook::CreateThingsHook(HWND hWindow, void* msghandler, int a3)
{
    bool a = reinterpret_cast<bool(__thiscall *)(CGFXVideo3d *, HWND, void *, int)>(0x008a97f0)(//ECSRO 
		this, hWindow, msghandler, a3);

	for (std::vector<create_handler_t>::iterator it = hooks_create.begin();
		it != hooks_create.end();
		++it)
	{
		(*it)(hWindow, msghandler, a3);
	}

	return a;
}

bool CGFXVideo3D_Hook::EndSceneHook()
{
	// ==========================================================================
	// UPDATE GLOBAL VIEWPORT RESOLUTION from D3D9
	// This runs every frame - used by IFflorian0Guide for dynamic positioning
	// ==========================================================================
	if (m_pd3dDevice) {
		D3DVIEWPORT9 vp;
		if (SUCCEEDED(m_pd3dDevice->GetViewport(&vp))) {
			g_D3DViewportWidth = (int)vp.Width;
			g_D3DViewportHeight = (int)vp.Height;
		}
	}

	for (std::vector<endscene_handler_t>::iterator it = hooks_endscene.begin();
		it != hooks_endscene.end();
		++it)
	{
		(*it)();
	}

	// Full qualified name to avoid redirection through the vftable
	//return CGFXVideo3D_Hook::EndScene();
	m_pd3dDevice->EndScene();
	return true;
}

bool CGFXVideo3D_Hook::SetSizeHook(int width, int height)
{
	for (std::vector<setsize_handler_t>::iterator it = hooks_setsize_pre.begin();
		it != hooks_setsize_pre.end();
		++it)
	{
		(*it)(width, height);
	}

	CGFXVideo3d::SetSize(width, height);

	for (std::vector<setsize_handler_t>::iterator it = hooks_setsize_post.begin();
		it != hooks_setsize_post.end();
		++it)
	{
		(*it)(width, height);
	}

	return true;
}
