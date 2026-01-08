#pragma once
#include "Common.h"
#include "parallel-hashmap/parallel_hashmap/phmap.h"
#include <array>
#include <vector>
#include <ryml.hpp>

#ifdef UI18N_CUSTOM_STRING
    #ifdef UI18N_CUSTOM_STRING_INCLUDE
        #include UI18N_CUSTOM_STRING_INCLUDE
        typedef UI18N_CUSTOM_STRING ui18nstring;
    #else
        #error UI18N_CUSTOM_STRING defined but UI18N_CUSTOM_STRING_INCLUDE not defined, it is needed to include the necessary headers for the string, and should contain the name of the header wrapped in ""
    #endif
#else
    #include <string>
    typedef std::string ui18nstring;
#endif

template<typename K, typename V>
using ui18nmap = phmap::parallel_flat_hash_map<K, V>;

namespace UI18N
{
    typedef UI18N_LanguageCodes LanguageCodes;
    typedef UI18N_InitialisationResult InitialisationResult;

    // Returns an empty string if out of bounds
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API const char* languageCodeToString(LanguageCodes code) noexcept;

    // Returns UI18N_LANGUAGE_CODES_COUNT if it does not fit
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API LanguageCodes stringToLanguageCode(const char* code) noexcept;

    // UntitledImGuiFramework Event Safety - Any time
    class MLS_PUBLIC_API TranslationEngine
    {
    public:
        TranslationEngine() = default;
        InitialisationResult init(const char* directory, LanguageCodes defaultLocale = en_US) noexcept;

        void setCurrentLocale(LanguageCodes locale) noexcept;

        ui18nstring get(const char* id, const std::vector<ui18nstring>& positionalArgs = {}, const ui18nmap<ui18nstring, ui18nstring>& args = {}) noexcept;
        void pushVariable(const ui18nstring& name, const ui18nstring& val) noexcept;

        // Returns a list of all locales that contain at least 1 translation
        const std::vector<LanguageCodes>& getExistingLocales() noexcept;

        ~TranslationEngine() = default;
    private:
        friend class Internal;

        // Used for the pattern-matching functionality
        struct Switch
        {
            bool bExists = false;
            ui18nstring defaultValue;
            ui18nmap<ui18nstring, ui18nstring> patterns{};
        };

        struct Variable
        {
            ui18nstring text;
            ui18nmap<ui18nstring, Switch> references;
        };

        static void replaceVariableInString(ui18nstring& str, const ui18nstring& replaceName, const ui18nstring& replace) noexcept;

        InitialisationResult parseConfig(const char* directory);
        InitialisationResult parseTranslations(const char* file, size_t lc);
        void parseVariablePatternMatching(ryml::NodeRef node, Variable& variable) noexcept;

        static void getHandlePositionalArguments(ui18nstring& text, const std::vector<ui18nstring>& args) noexcept;
        void getHandleVariables(ui18nstring& text, const ui18nmap<ui18nstring, Switch>& references, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept;
        void getHandleReplaceWithVal(const Switch& switchA, ui18nstring& text, const std::pair<ui18nstring, ui18nstring>& variable, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept;

        LanguageCodes currentLocale = en_US;

        std::vector<ui18nstring> cAPITmpResultStorage;

        std::vector<LanguageCodes> existingLocales{};

        ui18nmap<ui18nstring, ui18nstring> terms{};
        ui18nmap<ui18nstring, ui18nstring> variables{};
        std::array<ui18nmap<ui18nstring, Variable>, UI18N_LANGUAGE_CODES_COUNT> translations{};
    };
}