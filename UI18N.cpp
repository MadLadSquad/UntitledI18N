#include "UI18N.hpp"
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace UI18N
{
    // Strings that map 1:1 to the language codes
    constexpr const char* LanguageCodesAsStrings[UI18N_LANGUAGE_CODES_COUNT] =
    {
        "aa_DJ","aa_ER","aa_ET","af_ZA","agr_PE","ak_GH","am_ET","an_ES","anp_IN","ar_AE","ar_BH","ar_DZ","ar_EG",
        "ar_IN","ar_IQ","ar_JO","ar_KW","ar_LB","ar_LY","ar_MA","ar_OM","ar_QA","ar_SA","ar_SD","ar_SS","ar_SY",
        "ar_TN","ar_YE","as_IN","ast_ES","ayc_PE","az_AZ","az_IR","be_BY","bem_ZM","ber_DZ","ber_MA","bg_BG","bho_IN",
        "bho_NP","bi_VU","bn_BD","bn_IN","bo_CN","bo_IN","br_FR","brx_IN","bs_BA","byn_ER","ca_AD","ca_ES","ca_FR",
        "ca_IT","ce_RU","chr_US","ckb_IQ","cmn_TW","crh_UA","cs_CZ","csb_PL","cv_RU","cy_GB","da_DK","de_AT","de_BE",
        "de_CH","de_DE","de_IT","de_LU","doi_IN","dsb_DE","dv_MV","dz_BT","el_CY","el_GR","en_AG","en_AU","en_BW",
        "en_CA","en_DK","en_GB","en_HK","en_IE","en_IL","en_IN","en_NG","en_NZ","en_PH","en_SG","en_US","en_ZA","en_ZM",
        "en_ZW","eo","es_AR","es_BO","es_CL","es_CO","es_CR","es_CU","es_DO","es_EC","es_ES","es_GT","es_HN","es_MX",
        "es_NI","es_PA","es_PE","es_PR","es_PY","es_SV","es_US","es_UY","es_VE","et_EE","eu_ES","fa_IR","ff_SN","fi_FI",
        "fil_PH","fo_FO","fr_BE","fr_CA","fr_CH","fr_FR","fr_LU","fur_IT","fy_DE","fy_NL","ga_IE","gd_GB","gez_ER",
        "gez_ET","gl_ES","gu_IN","gv_GB","ha_NG","hak_TW","he_IL","hi_IN","hif_FJ","hne_IN","hr_HR","hsb_DE","ht_HT",
        "hu_HU","hy_AM","ia_FR","id_ID","ig_NG","ik_CA","is_IS","it_CH","it_IT","iu_CA","ka_GE","kab_DZ","kk_KZ",
        "kl_GL","km_KH","kn_IN","kok_IN","ks_IN","ku_TR","kw_GB","ky_KG","lb_LU","lg_UG","li_BE","li_NL","lij_IT",
        "ln_CD","lo_LA","lt_LT","lv_LV","lzh_TW","mag_IN","mai_IN","mai_NP","mfe_MU","mg_MG","mhr_RU","mi_NZ","miq_NI",
        "mjw_IN","mk_MK","ml_IN","mn_MN","mni_IN","mnw_MM","mr_IN","ms_MY","mt_MT","my_MM","nan_TW","nb_NO","nds_DE",
        "nds_NL","ne_NP","nhn_MX","niu_NU","niu_NZ","nl_AW","nl_BE","nl_NL","nn_NO","nr_ZA","nso_ZA","oc_FR","om_ET",
        "om_KE","or_IN","os_RU","pa_IN","pa_PK","pap_AW","pap_CW","pl_PL","ps_AF","pt_BR","pt_PT","quz_PE","raj_IN",
        "ro_RO","ru_RU","ru_UA","rw_RW","sa_IN","sah_RU","sat_IN","sc_IT","sd_IN","se_NO","sgs_LT","shn_MM","shs_CA",
        "si_LK","sid_ET","sk_SK","sl_SI","sm_WS","so_DJ","so_ET","so_KE","so_SO","sq_AL","sq_MK","sr_ME","sr_RS",
        "ss_ZA","st_ZA","sv_FI","sv_SE","sw_KE","sw_TZ","szl_PL","ta_IN","ta_LK","te_IN","tg_TJ","th_TH","the_NP",
        "ti_ER","ti_ET","tig_ER","tk_TM","tl_PH","tn_ZA","to_TO","tpi_PG","tr_CY","tr_TR","ts_ZA","tt_RU","ug_CN",
        "uk_UA","unm_US","ur_IN","ur_PK","uz_UZ","ve_ZA","vi_VN","wa_BE","wae_CH","wal_ET","wo_SN","xh_ZA","yi_US",
        "yo_NG","yue_HK","yuw_PG","zh_CN","zh_HK","zh_SG","zh_TW","zu_ZA"
    };
}

UI18N::InitialisationResult UI18N::TranslationEngine::init(const char* directory, LanguageCodes defaultLocale) noexcept
{
    currentLocale = defaultLocale;

    // Parser the ui18n-config.yaml file, which contains terms and other configuration variables
    auto result = parseConfig(directory);
    if (result != UI18N_INIT_RESULT_SUCCESS)
        return result;

    const ui18nstring ymlExt = ".yaml";
    const ui18nstring ymlExtShort = ".yml";
    for (auto& a : std::filesystem::directory_iterator(std::filesystem::path(directory)))
    {
        if (!a.is_directory() && (a.path().string().ends_with(".yaml") || a.path().string().ends_with(".yml")))
        {
            auto filename = a.path().filename().string();
            for (size_t i = 0; i < UI18N_LANGUAGE_CODES_COUNT; i++)
            {
                auto& lc = LanguageCodesAsStrings[i];
                if (filename == (lc + ymlExt) || filename == (lc + ymlExtShort))
                {
                    if (parseTranslations(absolute(a.path()).string().c_str(), i) == UI18N_INIT_RESULT_INVALID_TRANSLATION)
                        result = UI18N_INIT_RESULT_INVALID_TRANSLATION;
                    goto exit_inner_loop;
                }
                else
                {
                    auto tmp = filename;
                    for (auto& f : tmp)
                        if (f == '-')
                            f = '_';
                    if (tmp == (lc + ymlExt) || filename == (lc + ymlExtShort))
                    {
                        if (parseTranslations(absolute(a.path()).string().c_str(), i) == UI18N_INIT_RESULT_INVALID_TRANSLATION)
                            result = UI18N_INIT_RESULT_INVALID_TRANSLATION;
                        goto exit_inner_loop;
                    }
                }
            }
        }
exit_inner_loop:;
    }

    // Record the locales that have at least 1 translation
    for (size_t i = 0; i < UI18N_LANGUAGE_CODES_COUNT; i++)
        if (!translations[i].empty())
            existingLocales.push_back((LanguageCodes)i);

    return result;
}

ui18nstring UI18N::TranslationEngine::get(const char* id, const std::vector<ui18nstring>& positionalArgs, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    // Note: We intentionally use the C++ standard map access syntax, because we don't want to be doing exception
    // handling, and we also want to return an empty string if accessed in the future.
    auto rval = translations[currentLocale][id];
    if (rval.text.empty())
        return rval.text;

    // Handle positional arguments with {} syntax
    getHandlePositionalArguments(rval.text, positionalArgs);

    // Handle variable arguments with {var} syntax
    getHandleVariables(rval.text, rval.references, args);

    return rval.text;
}

UI18N::InitialisationResult UI18N::TranslationEngine::parseTranslations(const char* file, size_t lc)
{
    YAML::Node out;
    try
    {
        out = YAML::LoadFile(file);
    }
    catch (YAML::BadFile&)
    {
        return UI18N_INIT_RESULT_INVALID_TRANSLATION;
    }

    if (!out["translations"])
        return UI18N_INIT_RESULT_INVALID_TRANSLATION;

    for (const auto& a : out["translations"])
    {
        if (a["id"] && a["text"])
        {
            Variable variable = { .text = a["text"].as<ui18nstring>(), .references = {} };
            bool bIteratingVariable = false;
            size_t beginCut = 0;

            // Handle variable references
            for (size_t i = 0; i < variable.text.size(); i++)
            {
                auto& it = variable.text[i];
                if ((i + 1) != variable.text.size())
                {
                    auto& nit = variable.text[i + 1];
                    if (it == '{')
                    {
                        if (nit == '}')
                            goto exit_next_it;
                        else
                        {
                            bIteratingVariable = true;
                            // Got to the next character position
                            beginCut = i + 1;
                        }
                    }
                }

                if (it == '}' && bIteratingVariable)
                {
                    bIteratingVariable = false;
                    variable.references.insert(std::pair<ui18nstring, Switch>{ variable.text.substr(beginCut, i - beginCut), {} });
                }
exit_next_it:;
            }

            // Handle terms
            for (auto& f : variable.references)
            {
                for (auto& h : terms)
                {
                    if (f.first == h.first)
                    {
                        replaceVariableInString(variable.text, h.first, h.second);
                        goto exit_inner_loop_init_2;
                    }
                }
exit_inner_loop_init_2:;
            }

            if (a["switch"])
                parseVariablePatternMatching(a["switch"], variable);

            translations[lc].insert(std::pair<ui18nstring, Variable>{ a["id"].as<ui18nstring>(), variable });
        }
    }

    return UI18N_INIT_RESULT_SUCCESS;
}

void UI18N::TranslationEngine::parseVariablePatternMatching(const YAML::Node& node, Variable& variable) noexcept
{
    for (auto& f : node)
    {
        auto var = f["var"];
        if (!var)
            continue;

        const auto variableString = var.as<ui18nstring>();
        Switch& vswitch = variable.references[variableString];

        ui18nstring defaultVal;
        if (f["default"])
            defaultVal = f["default"].as<ui18nstring>();
        // Replace terms in the default string
        for (auto& a : terms)
            replaceVariableInString(defaultVal, a.first, a.second);

        vswitch.defaultValue = defaultVal;
        vswitch.bExists = true;

        if (f["cases"])
        {
            for (auto& h : f["cases"])
            {
                ui18nstring result;
                if (!h["case"] || !h["result"])
                    goto pattern_match_skip_inner;

                result = h["result"].as<ui18nstring>();
                // Replace terms in the result string
                for (auto& a : terms)
                    replaceVariableInString(result, a.first, a.second);

                vswitch.patterns.insert(std::pair<ui18nstring, ui18nstring>{ h["case"].as<ui18nstring>(), result });
pattern_match_skip_inner:;
            }
        }
    }
}

UI18N::InitialisationResult UI18N::TranslationEngine::parseConfig(const char* directory)
{
    YAML::Node out;
    try
    {
        out = YAML::LoadFile(ui18nstring(directory) + "/ui18n-config.yaml");
    }
    catch (YAML::BadFile&)
    {
        return UI18N_INIT_RESULT_INVALID_CONFIG;
    }

    auto terms_l = out["terms"];

    // Wow, this is inefficient as shit
    if (terms_l)
        for (auto& a : terms_l.as<std::vector<ui18nmap<ui18nstring, ui18nstring>>>())
            for (auto& f : a)
                terms.insert(std::pair<ui18nstring, ui18nstring>{ f.first, f.second });

    return UI18N_INIT_RESULT_SUCCESS;
}

void UI18N::TranslationEngine::pushVariable(const ui18nstring& name, const ui18nstring& val) noexcept
{
    variables.insert(std::pair<ui18nstring, ui18nstring>{ name, val });
}

const char* UI18N::languageCodeToString(LanguageCodes code) noexcept
{
    if (code >= UI18N_LANGUAGE_CODES_COUNT)
        return "";
    return LanguageCodesAsStrings[code];
}

ui18nstring toLower(const ui18nstring& arg) noexcept;

UI18N::LanguageCodes UI18N::stringToLanguageCode(const char* code) noexcept
{
    for (size_t i = 0; i < UI18N_LANGUAGE_CODES_COUNT; i++)
    {
        ui18nstring tmp = toLower(code);
        ui18nstring languageCode = toLower(LanguageCodesAsStrings[i]);

        if (tmp == languageCode)
            return static_cast<LanguageCodes>(i);
        else
        {
            for (auto& a : tmp)
                if (a == '-')
                    a = '_';
            if (tmp == languageCode)
                return static_cast<LanguageCodes>(i);
        }
    }
    return UI18N_LANGUAGE_CODES_COUNT;
}

void UI18N::TranslationEngine::replaceVariableInString(ui18nstring& str, const ui18nstring& replaceName, const ui18nstring& replace) noexcept
{
    ui18nstring pattern = "{" + replaceName + "}";
    for (size_t offset = str.find(pattern); offset != ui18nstring::npos; offset = str.find(pattern, offset))
        str.replace(offset, pattern.size(), replace);
}

void UI18N::TranslationEngine::getHandlePositionalArguments(ui18nstring& text, const std::vector<ui18nstring>& args) noexcept
{
    size_t ppos = 0;
    for (const auto& f : args)
    {
        auto pos = text.find("{}", ppos);
        ppos = pos;
        if (pos == ui18nstring::npos)
            break;
        text.replace(pos, 2, f);
    }
}

void UI18N::TranslationEngine::getHandleVariables(ui18nstring& text, const ui18nmap<ui18nstring, Switch>& references, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    for (const auto& f : references)
    {
        // Engine variables, added for long-term storage by the "pushVariable" method
        for (auto& var : variables)
        {
            // If the reference and variable names match
            if (f.first == var.first)
            {
                getHandleReplaceWithVal(f.second, text, var, {});
                goto exit_inner_loop_get_1;
            }
        }
        // Variables as argument, added by the "args" argument of this function
        for (auto& var : args)
        {
            if (f.first == var.first)
            {
                getHandleReplaceWithVal(f.second, text, var, args);
                goto exit_inner_loop_get_1;
            }
        }
        // Both  functions skip to this point because we ignore duplicates, so if one loop matches a variable reference
        // we should directly escape, not reiterate through the other variables again, just to waste cycles
exit_inner_loop_get_1:;
    }
}

void UI18N::TranslationEngine::getHandleReplaceWithVal(const Switch& switchA, ui18nstring& text, const std::pair<ui18nstring, ui18nstring>& variable, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    // If we have a switch statement
    if (switchA.bExists)
    {
        // Iterate patterns
        for (auto& a : switchA.patterns)
        {
            // If the case key is equal to the value of the variable
            if (a.first == variable.second)
            {
                auto resultTmp = a.second;
                // Replace any variables that may be templated into the result key
                for (auto& f : variables)
                    replaceVariableInString(resultTmp, f.first, f.second);
                // Also from the temporary variable list if provided
                for (auto& f : args)
                    replaceVariableInString(resultTmp, f.first, f.second);

                replaceVariableInString(text, variable.first, resultTmp);
                return;
            }
        }
        // Replace any variables in the default variable
        auto defaultValTmp = switchA.defaultValue;
        for (auto& a : variables)
            replaceVariableInString(defaultValTmp, a.first, a.second);
        // Also from the temporary variable list if provided
        for (auto& a : args)
            replaceVariableInString(defaultValTmp, a.first, a.second);

        // Finally replace everything in the default value
        replaceVariableInString(text, variable.first, defaultValTmp);
    }
    else // Default behaviour
        replaceVariableInString(text, variable.first, variable.second);
}

void UI18N::TranslationEngine::setCurrentLocale(UI18N::LanguageCodes locale) noexcept
{
    currentLocale = locale;
}

const std::vector<UI18N::LanguageCodes>& UI18N::TranslationEngine::getExistingLocales() noexcept
{
    return existingLocales;
}

ui18nstring toLower(const ui18nstring& arg) noexcept
{
    ui18nstring tmp = arg;
    for (auto& a : tmp)
        a = (char)tolower(a);
    return tmp;
}