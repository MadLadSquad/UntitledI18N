#include "cui18n.h"
#include <UI18N.hpp>
#include <new>

#define cast(x) ((UI18N::TranslationEngine*)x)

namespace UI18N
{
    class Internal
    {
    public:
        static std::deque<ui18nstring>& getCAPITmpStorage(UI18N_CTranslationEngine* engine) noexcept
        {
            return cast(engine)->cAPITmpResultStorage;
        }
    };
}

const char* UI18N_languageCodeToString(const UI18N_LanguageCodes code)
{
    return UI18N::languageCodeToString(code);
}

UI18N_LanguageCodes UI18N_stringToLanguageCode(const char* code)
{
    return UI18N::stringToLanguageCode(code);
}

UI18N_CTranslationEngine* UI18N_TranslationEngine_Construct()
{
    // Nothrow: the library never throws, so report an allocation failure the C way, with a null handle
    return new(std::nothrow) UI18N::TranslationEngine();
}

void UI18N_TranslationEngine_Free(UI18N_CTranslationEngine* engine)
{
    delete (UI18N::TranslationEngine*)engine;
}

UI18N_InitialisationResult UI18N_TranslationEngine_init(UI18N_CTranslationEngine* engine, const char* directory, const UI18N_LanguageCodes defaultLocale)
{
    return cast(engine)->init(directory, defaultLocale);
}

const char* UI18N_TranslationEngine_get(UI18N_CTranslationEngine* engine, const char* id, char** pargv, const size_t pargc, UI18N_Pair* argv, const size_t argc)
{
    // Every other null here is answerable with an empty result, but a null engine owns no storage to put that
    // result in, so it is the one case that cannot hand back a readable pointer. A null id is fine: get() reports
    // an unknown id as an empty string
    if (engine == nullptr)
        return nullptr;

    ui18nmap<ui18nstring, ui18nstring> map;
    std::vector<ui18nstring> vec;
    if (argv != nullptr)
        for (size_t i = 0; i < argc; i++)
            // Constructing a string from a null pointer is undefined, so a pair missing either half is dropped
            // rather than guessed at: an argument with no name matches no placeholder, and one with no value has
            // nothing to substitute. Dropping it leaves the placeholder to the switch default, exactly as if the
            // caller had not passed it - the same thing the loop below already does for a null positional argument
            if (argv[i].key != nullptr && argv[i].val != nullptr)
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

void UI18N_TranslationEngine_setCurrentLocale(UI18N_CTranslationEngine* engine, const UI18N_LanguageCodes locale)
{
    cast(engine)->setCurrentLocale(locale);
}

const UI18N_LanguageCodes* UI18N_TranslationEngine_getExistingLocales(UI18N_CTranslationEngine* engine, size_t* size)
{
    auto& locales = cast(engine)->getExistingLocales();
    *size = locales.size();
    return locales.data();
}