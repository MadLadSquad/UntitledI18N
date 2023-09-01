#pragma once
#include "Common.h"
#include <array>
#include <vector>

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

#ifdef UI18N_CUSTOM_MAP
    #ifdef UI18N_CUSTOM_MAP_INCLUDE
		#include UI18N_CUSTOM_MAP_INCLUDE
        template<typename T, typename T2>
        using ui18nmap = UI18N_CUSTOM_MAP<T, T2>;

	#else
		#error UI18N_CUSTOM_MAP defined but UI18N_CUSTOM_MAP_INCLUDE not defined, it is needed to include the necessary headers for the string, and should contain the name of the header wrapped in ""
	#endif
#else
    #include <unordered_map>
    template<typename T, typename T2>
    using ui18nmap = std::unordered_map<T, T2>;
#endif

namespace UI18N
{
    typedef UI18N_LanguageCodes LanguageCodes;
    typedef UI18N_InitialisationResult InitialisationResult;

    class TranslationEngine
    {
    public:
        TranslationEngine() = default;
        InitialisationResult init(const char* directory, LanguageCodes defaultLocale = en_US) noexcept;

        ui18nstring get(const char* id, const ui18nmap<ui18nstring, ui18nstring>& args = {}) noexcept;
        void pushVariable(const ui18nstring& name, const ui18nstring& val) noexcept;

        ~TranslationEngine() = default;
    private:
        InitialisationResult parseConfig(const char* directory);
        InitialisationResult parseTranslations(const char* file);

        LanguageCodes currentLocale = en_US;

        ui18nmap<ui18nstring, ui18nstring> variables{};
        std::array<ui18nmap<ui18nstring, ui18nstring>, UI18N_LANGUAGE_CODES_COUNT> translations{};
    };
}