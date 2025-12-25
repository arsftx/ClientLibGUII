#include "IFWorldMap.h"
const DWORD off_BA3ECC = 0x00BA3ECC;
const DWORD movAddr = 0x00CB2B98;
const DWORD sub_75FFB0 = 0x075FFB0;
const DWORD sub_74EF80 = 0x74EF80;
const DWORD off_BA3E90 = 0x00BA3E90;
void __declspec(naked) CIFWorldMap::DisableManualButton()
{

	__asm {
		push esi                         // Save ESI
		push edi                         // Save EDI
		mov esi, ecx                     // Load this pointer into ESI

		// Force the default state to Manual Move
		mov byte ptr[esi + 0x7D5], 1    // Set [ESI+7D5] to 1 (Manual Move)

		// Perform Manual Move initialization logic
		lea ecx, [esi + 0x1BC]           // Load map control into ECX
		push 1                           // Push 1 (Manual Move parameter)
		push 0x14                        // Push 20 (hex 14, Manual Move parameter)
		call sub_74EF80                  // Call the Manual Move function

		mov edi, eax                     // Store the result in EDI
		push 0x00BA3E90                  // Push Manual Move identifier string (UIIT_STT_WORLDMAP_MANUAL_MOVE)
		mov ecx, 0x00CB2B98              // Load necessary context
		call sub_75FFB0                  // Call supporting function

		// Final transition logic (optional cleanup)
		cmp dword ptr[esi + 0x61A8], 1  // Check if transition state is 1
		jne SkipTransition               // Skip transition if not 1
		push 0                           // Push 0 (optional transition parameter)
		mov ecx, dword ptr[esi + 0x61F0] // Load transition control
		mov edx, dword ptr[ecx]         // Load function table
		mov eax, dword ptr[edx + 0x58]  // Get transition function
		call eax                         // Call transition function

		SkipTransition :
		// Final cleanup
		pop edi                          // Restore EDI
			pop esi                          // Restore ESI
			ret                              // Return
	}
}

bool CIFWorldMap::OnCreateIMPL(long ln) {


	bool b = reinterpret_cast<bool(__thiscall*)(CIFWorldMap*, long)>(0x00563BF0)(this, ln);
	/*CIFButton* btn = m_IRM.GetResObj<CIFButton>(20, 1);
	btn->ShowWnd(false);*/
	return b;
}		