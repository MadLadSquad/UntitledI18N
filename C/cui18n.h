#pragma once
#include <Common.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void UI18N_CTranslationEngine;

    typedef struct UI18N_Pair
    {
        char* key;
        char* val;
    } UI18N_Pair;

    UVK_PUBLIC_API const char* UI18N_languageCodeToString(UI18N_LanguageCodes code);
    UVK_PUBLIC_API UI18N_LanguageCodes UI18N_stringToLanguageCode(const char* code);

    UVK_PUBLIC_API UI18N_CTranslationEngine* UI18N_TranslationEngine_Construct();
    UVK_PUBLIC_API void UI18N_TranslationEngine_Free(UI18N_CTranslationEngine* engine);

    // The C++ API sets the "defaultLocale" argument to be equal to "en_US" by default
    UVK_PUBLIC_API UI18N_InitialisationResult UI18N_TranslationEngine_init(UI18N_CTranslationEngine* engine,
                                                                           const char* directory,
                                                                           UI18N_LanguageCodes defaultLocale);

    UVK_PUBLIC_API const char* UI18N_TranslationEngine_get(UI18N_CTranslationEngine* engine, const char* id, char** pargv, size_t pargc, UI18N_Pair* argv, size_t argc);
    UVK_PUBLIC_API void UI18N_TranslationEngine_pushVariable(UI18N_CTranslationEngine* engine, const char* name, const char* val);

#ifdef __cplusplus
}
#endif