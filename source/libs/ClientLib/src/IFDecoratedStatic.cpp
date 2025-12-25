#include "IFDecoratedStatic.h"
#include <BSLib/Debug.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFDecoratedStatic, 0x009FE4E0)

// CIFDecoratedStatic::OnCreate(int) .text 0042DB90
bool CIFDecoratedStatic::OnCreate(long ln)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", ln);

	// Call parent class OnCreate - don't call native address to avoid vtable issues
	return CIFStatic::OnCreate(ln);
}

// CIFDecoratedStatic::OnUpdate() .text 0042DBC0 (thunk to CIFWnd::OnUpdate)
void CIFDecoratedStatic::OnUpdate()
{
	CIFWnd::OnUpdate();
}

// CIFDecoratedStatic::OnTimer(void) .text (nullsub)
void CIFDecoratedStatic::OnTimer(int)
{
	// empty
}

// Mouse event handlers - default implementations that allow child class overrides
int CIFDecoratedStatic::OnMouseLeftUp(int a1, int x, int y)
{
	// Default: do nothing, return 0
	// Child classes should override this
	return 0;
}

int CIFDecoratedStatic::OnMouseLeftDown(int a1, int x, int y)
{
	// Default: do nothing, return 0
	// Child classes should override this
	return 0;
}

int CIFDecoratedStatic::OnMouseMove(int a1, int x, int y)
{
	// Default: do nothing, return 0
	// Child classes should override this
	return 0;
}

// CIFDecoratedStatic::RenderMyself(void) .text 0042DBD0
void CIFDecoratedStatic::RenderMyself()
{
	// Debug logging disabled - button working correctly
	/*
	static int g_renderCount = 0;
	if (g_renderCount < 3) {
		FILE* fp = fopen("clientlog.txt", "a");
		if (fp) {
			fprintf(fp, "[CIFDecoratedStatic::RenderMyself] count=%d, this=0x%p, vtable+0x6C=0x%08X\n", 
				g_renderCount, this, *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 0x6C));
			fclose(fp);
		}
		g_renderCount++;
	}
	*/
	reinterpret_cast<void(__thiscall*)(CIFDecoratedStatic*)>(0x0042DBD0)(this);
}

// CIFDecoratedStatic::OnCIFReady(void) .text 0042E7E0
void CIFDecoratedStatic::OnCIFReady()
{
	reinterpret_cast<void(__thiscall*)(CIFDecoratedStatic*)>(0x0042E7E0)(this);
}

// CIFDecoratedStatic::Func_52(void) .text 0042E7C0 (estimated)
bool CIFDecoratedStatic::Func_52()
{
	return false;
}

// CIFDecoratedStatic::CIFDecoratedStatic(void) .text 0042D8B0
CIFDecoratedStatic::CIFDecoratedStatic()
{
	// Call native constructor - it initializes all members that RenderMyself needs
	// The native constructor will also call CIFStatic constructor internally
	reinterpret_cast<void(__thiscall*)(CIFDecoratedStatic*)>(0x0042D8B0)(this);
}

// CIFDecoratedStatic::~CIFDecoratedStatic(void) .text 0042D9F0
CIFDecoratedStatic::~CIFDecoratedStatic()
{
	// Empty - don't call sub_634470 which uses std::n_string
}

// CIFDecoratedStatic::set_N00009BDD(char) .text 0042E290
void CIFDecoratedStatic::set_N00009BDD(char a1)
{
	N00009BDD = a1;
}

// CIFDecoratedStatic::set_N00009C18(char) .text 0042E2A0
void CIFDecoratedStatic::set_N00009C18(char a1)
{
	N00009C18 = a1;
}

// CIFDecoratedStatic::set_N00009BD0(char) .text 0042DBB0 - Hover state setter
void CIFDecoratedStatic::set_N00009BD0(char a1)
{
	N00009BD0 = a1;
}

// CIFDecoratedStatic::set_N00009BD4(char) .text 0042DBB0
void CIFDecoratedStatic::set_N00009BD4(char a1)
{
	N00009BD4 = a1;
}

// CIFDecoratedStatic::set_N00009BD3(int) .text 0042E2B0
void CIFDecoratedStatic::set_N00009BD3(int a1)
{
	N00009BD3 = a1;
}

// CIFDecoratedStatic::sub_634470 - hover texture setter
// Native address 0x0042E100 - FIXED: uses 4 separate parameters, not struct!
// Signature: unsigned int __thiscall sub_42E100(this, char* lpMem, char* a3, int a4)
void CIFDecoratedStatic::sub_634470(std::n_string a1)
{
	// Debug log
	FILE* fp = fopen("clientlog.txt", "a");
	if (fp) {
		fprintf(fp, "[sub_634470] Called with path: %s\n", a1.empty() ? "(empty)" : a1.c_str());
		fprintf(fp, "[sub_634470] this = 0x%p\n", this);
		fclose(fp);
	}
	
	if (a1.empty()) {
		// Empty string case
		fp = fopen("clientlog.txt", "a");
		if (fp) {
			fprintf(fp, "[sub_634470] Calling native with NULL params...\n");
			fclose(fp);
		}
		
		reinterpret_cast<void(__thiscall*)(CIFDecoratedStatic*, char*, char*, int)>(0x0042E100)
			(this, NULL, NULL, 0);
		
		fp = fopen("clientlog.txt", "a");
		if (fp) {
			fprintf(fp, "[sub_634470] Native call with NULL completed\n");
			fclose(fp);
		}
	} else {
		size_t len = a1.length();
		char* buffer = (char*)malloc(len + 1);
		if (!buffer) {
			fp = fopen("clientlog.txt", "a");
			if (fp) {
				fprintf(fp, "[sub_634470] ERROR: malloc failed!\n");
				fclose(fp);
			}
			return;
		}
		strcpy(buffer, a1.c_str());
		
		char* dataPtr = buffer;
		char* endPtr = buffer + len;
		int capacityVal = (int)(buffer + len + 1);
		
		fp = fopen("clientlog.txt", "a");
		if (fp) {
			fprintf(fp, "[sub_634470] Calling native with: data=0x%p, end=0x%p, capacity=0x%X\n", 
				dataPtr, endPtr, capacityVal);
			fclose(fp);
		}
		
		// Call with 3 separate parameters: data, end, capacity
		reinterpret_cast<void(__thiscall*)(CIFDecoratedStatic*, char*, char*, int)>(0x0042E100)
			(this, dataPtr, endPtr, capacityVal);
		
		fp = fopen("clientlog.txt", "a");
		if (fp) {
			fprintf(fp, "[sub_634470] Native call completed successfully\n");
			fclose(fp);
		}
		
		// Note: Don't free buffer - native code may keep reference
	}
}

// CIFDecoratedStatic::sub_633990(void) .text 0042E2F0 (VSRO: 0x633990)
void CIFDecoratedStatic::sub_633990()
{
	reinterpret_cast<void(__thiscall*)(CIFDecoratedStatic*)>(0x0042E2F0)(this);
}
