#pragma once
#include "../Common.h"

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

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API const char* UI18N_languageCodeToString(UI18N_LanguageCodes code);
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API UI18N_LanguageCodes UI18N_stringToLanguageCode(const char* code);

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API UI18N_CTranslationEngine* UI18N_TranslationEngine_Construct();
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void UI18N_TranslationEngine_Free(UI18N_CTranslationEngine* engine);

    // The C++ API sets the "defaultLocale" argument to be equal to "en_US" by default
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API UI18N_InitialisationResult UI18N_TranslationEngine_init(UI18N_CTranslationEngine* engine,
                                                                           const char* directory,
                                                                           UI18N_LanguageCodes defaultLocale);

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API const char* UI18N_TranslationEngine_get(UI18N_CTranslationEngine* engine, const char* id, char** pargv, size_t pargc, UI18N_Pair* argv, size_t argc);
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void UI18N_TranslationEngine_pushVariable(UI18N_CTranslationEngine* engine, const char* name, const char* val);

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void UI18N_TranslationEngine_setCurrentLocale(UI18N_CTranslationEngine* engine, UI18N_LanguageCodes locale);
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API const UI18N_LanguageCodes* UI18N_TranslationEngine_getExistingLocales(UI18N_CTranslationEngine* engine, size_t* size);

#ifdef __cplusplus
}
#endif