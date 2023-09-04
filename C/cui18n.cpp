#include "cui18n.h"
#include <UI18N.hpp>
#include <cstring>

#define cast(x) ((UI18N::TranslationEngine*)x)

namespace UI18N
{
    class Internal
    {
    public:
        static std::vector<ui18nstring>& getCAPITmpStorage(UI18N_CTranslationEngine* engine) noexcept
        {
            return cast(engine)->cAPITmpResultStorage;
        }
    };
}

const char* UI18N_languageCodeToString(UI18N_LanguageCodes code)
{
    return UI18N::languageCodeToString(code);
}

UI18N_LanguageCodes UI18N_stringToLanguageCode(const char* code)
{
    return UI18N::stringToLanguageCode(code);
}

UI18N_CTranslationEngine* UI18N_TranslationEngine_Construct()
{
    return new UI18N::TranslationEngine();
}

void UI18N_TranslationEngine_Free(UI18N_CTranslationEngine* engine)
{
    delete (UI18N::TranslationEngine*)engine;
}

UI18N_InitialisationResult UI18N_TranslationEngine_init(UI18N_CTranslationEngine* engine, const char* directory, UI18N_LanguageCodes defaultLocale)
{
    return cast(engine)->init(directory, defaultLocale);
}

const char* UI18N_TranslationEngine_get(UI18N_CTranslationEngine* engine, const char* id, char** pargv, size_t pargc, UI18N_Pair* argv, size_t argc)
{
    ui18nmap<ui18nstring, ui18nstring> map;
    std::vector<ui18nstring> vec;
    if (argv != nullptr)
        for (size_t i = 0; i < argc; i++)
            map.insert(std::pair<ui18nstring, ui18nstring>{ argv[i].key, argv[i].val });
    if (pargv != nullptr)
        for (size_t i = 0; i < pargc; i++)
            if (pargv[i] != nullptr)
                vec.emplace_back(pargv[i]);

    auto& storage = UI18N::Internal::getCAPITmpStorage(engine);
    storage.push_back(cast(engine)->get(id, vec, map));
    return storage.back().c_str();
}

void UI18N_TranslationEngine_pushVariable(UI18N_CTranslationEngine* engine, const char* name, const char* val)
{
    cast(engine)->pushVariable(name, val);
}
