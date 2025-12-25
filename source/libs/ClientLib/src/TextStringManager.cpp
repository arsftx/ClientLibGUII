#include "TextStringManager.h"

#include <set>
#include <algorithm>

#ifdef CONFIG_TRANSLATIONS_DEBUG

#endif

const std::n_wstring*CTextStringManager::GetString(const wchar_t *identifier) {
    return reinterpret_cast<const std::n_wstring*(__thiscall *) (CTextStringManager *, const wchar_t *identifier)>(0x005FCE10)(this, identifier);
}
const wchar_t* CTextStringManager::GetString2(const char* identifier) {
	return reinterpret_cast<const wchar_t*(__thiscall*) (CTextStringManager*, const char* identifier)>(0x005FCE10)(this, identifier);
}
const char* CTextStringManager::GetString3(const char* identifier) {
	const char* result = reinterpret_cast<const char* (__thiscall*) (CTextStringManager*,const char*)>(0x005FCE10)(this, identifier);
	return result;  // Make sure the buffer is valid
}
const char CTextStringManager::GetString4(const char* identifier) {
	return reinterpret_cast<const char (__thiscall*) (CTextStringManager*, const char* identifier)>(0x005FCE10)(this, identifier);
}
const char CTextStringManager::GetString5(const char identifier) {
	return reinterpret_cast<const char (__thiscall*) (CTextStringManager*, const char identifier)>(0x005FCE10)(this, identifier);
}
const std::n_wstring &CTextStringManager::FUN_008c9bb0(const std::n_wstring &identifier) {
    return reinterpret_cast<const std::n_wstring &(__thiscall *) (CTextStringManager *, const std::n_wstring &)>(0x005FCE10)(this, identifier);
}

wchar_t* CTextStringManager::GetStringFromTextUIByCode( wchar_t* TextUISystemCode)
{
	return reinterpret_cast<wchar_t* (__thiscall*) (CTextStringManager*,  wchar_t*)>(0x005FCE10)(this, TextUISystemCode);
}
char* CTextStringManager::GetStringFromTextUIByCode2( char* TextUISystemCode)
{
	return reinterpret_cast<char* (__thiscall*) (CTextStringManager*,  char*)>(0x005FCE10)(this, TextUISystemCode);
}