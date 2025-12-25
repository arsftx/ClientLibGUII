#pragma once

//Bugfix for VS 2k5 bug with intrin include (when both windows.h / intrin.h included)
//https://blog.katastros.com/a?ID=00200-f6f4c70e-067a-45d1-a4ed-7c49c2655702

#if _MSC_VER <= 1400
#define _interlockedbittestandreset _interlockedbittestandreset_NAME_CHANGED_TO_AVOID_MSVS2005_ERROR
#define _interlockedbittestandset _interlockedbittestandset_NAME_CHANGED_TO_AVOID_MSVS2005_ERROR
#endif

#include <intrin.h>
#include <Windows.h>
#include <iostream>
#include <vector>

#pragma intrinsic(_ReturnAddress)

#define REMODEL_PLACEHOLDER

#define REMODEL_VOID_RET_PLACEHOLDER 

#define REMODEL_DECLARE_VIRTUAL_NOIMPL_DCTOR(className) \
	virtual ~className() { };

#define REMODEL_DECLARE_VIRTUAL_NOIMPL(ret_type, fn, ret_value) \
	virtual ret_type fn { return ret_value; }

#define REMODEL_GET_RETURN_ADDRESS() \
	_ReturnAddress()

#define REMODEL_SAVE_REGISTERS_BEFORE_STDCALL \
{\
    __asm push eax   \
    __asm push ecx   \
    __asm push edx   \
}

#define REMODEL_RESTORE_REGISTERS_AFTER_STDCALL \
{\
    __asm pop edx   \
    __asm pop ecx   \
    __asm pop eax   \
}


//Bit tricky, some magic with virtual function tables (can be improved later if necessary). 
//Works with function prototypes.

#define REMODEL_GET_VIRTUAL_FN(class_ptr, fn_type, fn_offset) \
    CRemodelUtility::GetVirtualFn<fn_type>(class_ptr, fn_offset)

#define REMODEL_GET_VIRTUAL_FN_BY_IDX(class_ptr, fn_type, fn_idx) \
    CRemodelUtility::GetVirtualFn<fn_type>(class_ptr, fn_idx * sizeof(uintptr_t))

class CRemodelUtility
{
public:
    template<typename T>
    static T GetVirtualFn(void* class_ptr, DWORD fn_offset)
    {
        uintptr_t dwVftFnAddr = *(uintptr_t*)(class_ptr)+fn_offset;
        return reinterpret_cast<T>(*(uintptr_t*)dwVftFnAddr);
    }
};
