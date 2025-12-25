#pragma once

#include <string>
#include <BSLib/BSLib.h>

class CTextStringManager {
public:
    //void ReadTranslationFile(char *filename, int lang, IFileManager *fm, int a5, int a6);
    //void sub_8CA8F0(const char * file, IFileManager *fm);

    /// Get translated string by identifier-token
    /// \address 0x0075ffb0
    /// \param identifier Identifier for the string to translate. Example: "UIIT_STT_CANT_CHATTING"
    /// \returns The translated string
    const std::n_wstring *GetString(const wchar_t *identifier);
	const wchar_t* GetString2(const char* identifier);
	const char* GetString3(const char* identifier);
	const char GetString4(const char* identifier);
	const char GetString5(const char identifier);

	wchar_t* GetStringFromTextUIByCode( wchar_t* TextUISystemCode);
	char* GetStringFromTextUIByCode2( char* TextUISystemCode);
    /// \address 008c9bb0
    const std::n_wstring &FUN_008c9bb0(const std::n_wstring &identifier);
};

#define g_CTextStringManager ((CTextStringManager*)0x0A00F8C)

#define TSM_GETTEXTPTR(text) (g_CTextStringManager->GetString(text)->c_str())
#define TSM_GETTEXT(text) (g_CTextStringManager->GetString(text))
