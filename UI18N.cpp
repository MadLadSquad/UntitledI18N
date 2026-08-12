#include "UI18N.hpp"
#include <filesystem>
#include <fstream>
#include <string_view>
#include <ryml.hpp>
#include <ryml_std.hpp>

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
        "hu_HU","hy_AM","ia_FR","id_ID","ig_NG","ik_CA","is_IS","it_CH","it_IT","iu_CA","ja_JP","ka_GE","kab_DZ","kk_KZ",
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

// Deliberately not std::tolower: everything folded here is pure ASCII, whereas tolower() is undefined for negative
// (that is, non-ASCII UTF-8) bytes and answers to whatever LC_CTYPE the host program happens to have set
static char asciiToLower(const char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// Matches a file extension the same way the language code in front of it is matched: without regard to case. A
// file name belongs to whoever wrote it, and "en_GB.YAML" names the locale that "en_GB.yaml" does - on Windows and
// macOS it may even be the same file
static bool endsWithNoCase(const ui18nstring& str, const std::string_view suffix) noexcept
{
    if (str.size() < suffix.size())
        return false;

    const size_t offset = str.size() - suffix.size();
    for (size_t i = 0; i < suffix.size(); i++)
        if (asciiToLower(str[offset + i]) != asciiToLower(suffix[i]))
            return false;
    return true;
}

// Returns false only when the file cannot be opened at all, so that a missing or unreadable file stays
// distinguishable from one that is merely empty - an empty config is legal, a missing one is not
static bool loadFileToString(const std::filesystem::path& file, ui18nstring& out) noexcept
{
    out.clear();

    // Binary mode is required, not cosmetic: in text mode Windows collapses every CRLF to a single character, so
    // fewer characters are read than tellg() reported and the tail of the buffer would keep its padding
    // Streams signal failure through their state, never by throwing, as long as no exception mask is set on them
    std::ifstream in(file, std::ios::binary);
    if (!in.is_open())
        return false;

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    if (size <= 0)
        return true;

    out.resize(static_cast<size_t>(size));

    in.seekg(0);
    in.read(out.data(), size);
    // Trust what was actually read over what was reported, so a short read truncates instead of leaving junk
    out.resize(static_cast<size_t>(in.gcount()));
    in.close();
    return true;
}

// std::filesystem::path::string() converts between encodings and throws when the conversion fails, so it cannot be
// used here. Language code file names are pure ASCII, which needs no conversion at all; anything else can never
// match a language code, so rejecting it outright loses nothing
static bool pathToASCII(const std::filesystem::path& path, ui18nstring& out) noexcept
{
    const auto& native = path.native();

    // clear() keeps the capacity, so a caller that reuses one string across a directory walk allocates at most once
    out.clear();
    out.reserve(native.size());
    for (const auto a : native)
    {
        if (a <= 0 || a > 127)
            return false;
        out += static_cast<char>(a);
    }
    return true;
}

// A node is only safe to read from once it passes this. find_child() returns an invalid ref rather than erroring
// when the key is absent, so this is what turns a missing field into a skipped field
static bool keyValid(const ryml::ConstNodeRef ref) noexcept
{
    return !ref.invalid() && ref.readable() && !ref.empty();
}

UI18N::InitialisationResult UI18N::TranslationEngine::init(const char* directory, const LanguageCodes defaultLocale) noexcept
{
    currentLocale = defaultLocale;

    // Everything loaded from a translation directory is dropped before loading the next one. Without this a second
    // init() merges the two directories: insert() keeps the older entry on a duplicate id, so stale translations
    // win over the ones just read, and existingLocales gains a duplicate entry per locale.
    // Deliberately not cleared: "variables" is pushed through the public API rather than read from the directory,
    // and cAPITmpResultStorage backs pointers the C API promised would live as long as the engine
    for (auto& a : translations)
        a.clear();
    terms.clear();
    existingLocales.clear();

    const std::filesystem::path directoryPath{ directory };

    // Parser the ui18n-config.yaml file, which contains terms and other configuration variables
    auto result = parseConfig(directoryPath);
    if (result != UI18N_INIT_RESULT_SUCCESS)
        return result;

    // Every std::filesystem call below is the std::error_code overload of its function: the plain ones throw a
    // filesystem_error, which this engine cannot handle
    std::error_code iterationError{};
    std::error_code entryError{};

    auto it = std::filesystem::directory_iterator(directoryPath, iterationError);
    // The config file was just read out of this directory, so failing to walk it means the directory itself, and
    // therefore the configuration pointing at it, is unusable
    if (iterationError)
        return UI18N_INIT_RESULT_INVALID_CONFIG;

    constexpr std::string_view ymlExt = ".yaml";
    constexpr std::string_view ymlExtShort = ".yml";

    // Hoisted out of the loop so that the whole directory walk reuses one buffer rather than allocating per entry
    ui18nstring filename{};
    for (const std::filesystem::directory_iterator end{}; it != end; it.increment(iterationError))
    {
        if (iterationError)
            break;
        if (it->is_directory(entryError) || entryError)
            continue;

        if (!pathToASCII(it->path().filename(), filename))
            continue;

        // Cut the extension off, leaving the language code to look up
        if (endsWithNoCase(filename, ymlExt))
            filename.resize(filename.size() - ymlExt.size());
        else if (endsWithNoCase(filename, ymlExtShort))
            filename.resize(filename.size() - ymlExtShort.size());
        else
            continue;

        // Deliberately the same lookup the public API uses, so that a file name and a call to stringToLanguageCode
        // always agree on which spellings of a code are valid
        const auto code = stringToLanguageCode(filename.c_str());
        if (code == UI18N_LANGUAGE_CODES_COUNT)
            continue;

        auto file = std::filesystem::absolute(it->path(), entryError);
        if (entryError)
            file = it->path();

        if (parseTranslations(file, code) == UI18N_INIT_RESULT_INVALID_TRANSLATION)
            result = UI18N_INIT_RESULT_INVALID_TRANSLATION;
    }

    // Record the locales that have at least 1 translation
    for (size_t i = 0; i < UI18N_LANGUAGE_CODES_COUNT; i++)
        if (!translations[i].empty())
            existingLocales.push_back(static_cast<LanguageCodes>(i));

    return result;
}

ui18nstring UI18N::TranslationEngine::get(const char* id, const std::vector<ui18nstring>& positionalArgs, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    if (id == nullptr || currentLocale >= UI18N_LANGUAGE_CODES_COUNT)
        return {};

    // find(), never operator[]: an unknown id has to return "" without inserting anything. Inserting would make a
    // lookup a write, so two threads calling get() would race the moment either one misses, and a program that
    // resolves ids it does not have would grow the map without bound. Reading a hash map from several threads at
    // once is safe; this is what keeps get() a read
    const auto& locale = translations[currentLocale];
    const auto translation = locale.find(id);
    if (translation == locale.end())
        return {};

    // Only the text is copied, and the copy is what gets mutated, so the stored translation is never consumed
    ui18nstring text = translation->second.text;
    if (text.empty())
        return text;

    // Handle positional arguments with {} syntax
    getHandlePositionalArguments(text, positionalArgs);

    // Handle variable arguments with {var} syntax
    getHandleVariables(text, translation->second.references, args);

    return text;
}

UI18N::InitialisationResult UI18N::TranslationEngine::parseTranslations(const std::filesystem::path& file, const size_t lc) noexcept
{
    // Unlike the config, an empty translation file is an error rather than a degenerate case: it is named after a
    // locale, so it promises translations, and it declares none
    ui18nstring string{};
    if (!loadFileToString(file, string) || string.empty())
        return UI18N_INIT_RESULT_INVALID_TRANSLATION;

    const auto tree = ryml::parse_in_arena(string.c_str());
    if (tree.empty())
        return UI18N_INIT_RESULT_INVALID_TRANSLATION;

    const auto root = tree.crootref();

    const auto trs = root.find_child("translations");
    if (!keyValid(trs) || !trs.is_seq())
        return UI18N_INIT_RESULT_INVALID_TRANSLATION;

    for (const auto a : trs.children())
    {
        const auto id = a.find_child("id");
        const auto text = a.find_child("text");

        if (keyValid(text) && keyValid(id))
        {
            Variable variable = { .text = {}, .references = {} };
            text.load(&variable.text);

            bool bIteratingVariable = false;
            size_t beginCut = 0;

            // Handle variable references
            for (size_t i = 0; i < variable.text.size(); i++)
            {
                const auto& it = variable.text[i];
                if ((i + 1) != variable.text.size())
                {
                    const auto& nit = variable.text[i + 1];
                    if (it == '{')
                    {
                        if (nit == '}')
                            goto exit_next_it;
                        bIteratingVariable = true;
                        // Got to the next character position
                        beginCut = i + 1;
                    }
                }

                if (it == '}' && bIteratingVariable)
                {
                    bIteratingVariable = false;
                    variable.references.insert(std::pair<ui18nstring, Switch>{ variable.text.substr(beginCut, i - beginCut), {} });
                }
exit_next_it:;
            }

            // Handle terms. "terms" is a hash map, so each reference is one probe: scanning every term for every
            // reference made this O(references * terms) for a result a single lookup gives
            for (const auto& f : variable.references)
            {
                const auto term = terms.find(f.first);
                if (term != terms.end())
                    replaceVariableInString(variable.text, term->first, term->second);
            }

            const auto sw = a.find_child("switch");
            if (keyValid(sw) && sw.is_seq())
                parseVariablePatternMatching(sw, variable);

            ui18nstring idstr{};
            id.load(&idstr);

            translations[lc].insert(std::pair{ idstr, variable });
        }
    }

    return UI18N_INIT_RESULT_SUCCESS;
}

void UI18N::TranslationEngine::parseVariablePatternMatching(ryml::ConstNodeRef node, Variable& variable) noexcept
{
    for (const auto f : node.children())
    {
        const auto var = f.find_child("var");
        if (!keyValid(var))
            continue;

        ui18nstring variableString{};
        var.load(&variableString);

        ui18nstring defaultVal{};
        const auto defaultNode = f.find_child("default");
        if (keyValid(defaultNode))
            defaultNode.load(&defaultVal);

        // Replace terms in the default string
        for (const auto& a : terms)
            replaceVariableInString(defaultVal, a.first, a.second);

        Switch& vswitch = variable.references[variableString];
        vswitch.defaultValue = defaultVal;
        vswitch.bExists = true;

        const auto cases = f.find_child("cases");
        if (keyValid(cases) && cases.is_seq())
        {
            for (const auto h : cases.children())
            {
                ui18nstring result{};
                ui18nstring caseStr{};

                const auto cc = h.find_child("case");
                const auto cr = h.find_child("result");
                if (!keyValid(cc) || !keyValid(cr))
                    goto pattern_match_skip_inner;

                cc.load(&caseStr);
                cr.load(&result);

                // Replace terms in the result string
                for (const auto& a : terms)
                    replaceVariableInString(result, a.first, a.second);

                vswitch.patterns.insert(std::pair{ caseStr, result });
pattern_match_skip_inner:;
            }
        }
    }
}

namespace c4::yml
{
    // Provided as a plain (internal-linkage) overload rather than an explicit specialisation: since the
    // rapidyaml ReadResult update the primary read() template returns ReadResult, so a bool result is now
    // routed through ReadResult's legacy adapter constructor. static keeps the symbol local to this TU,
    // avoiding a clash with the framework's own read(std::string*) when i18n is compiled into it.
    static bool read(ConstNodeRef const& ref, ui18nstring* t)
    {
        // val() on a node that has none is a ryml error, and a ryml error aborts. Reporting it as a failed read
        // instead fails just this field and leaves the rest of the document usable
        if (!keyValid(ref) || !ref.has_val())
            return false;

        const auto val = ref.val();
        t->resize(val.len);
        memcpy(t->data(), val.data(), val.size());
        return true;
    }

    // Check if similar to std::string
    template<typename T>
    concept IsStringLike = requires(T s, std::size_t n)
    {
        typename T::value_type;

        { s.data() } -> std::same_as<typename T::value_type*>;
        { std::as_const(s).data() } -> std::same_as<const typename T::value_type*>;
        { s.resize(n) } -> std::same_as<void>;
    } && std::is_trivial_v<typename T::value_type>;

    // Takes the container type directly instead of a template-template parameter: map types such as
    // phmap::parallel_flat_hash_map mix type and non-type (size_t N) template parameters, which no
    // template-template form can match portably across GCC/Clang/MSVC. Key/Val come from the typedefs.
    template<typename C>
    bool read_dict(ConstNodeRef const& ref, C* t)
    {
        using Key = typename C::key_type;
        using Val = typename C::mapped_type;

        // A ryml key is a csubstr: a pointer and a length into the parse arena, with no terminator of its own, so
        // the only correct way to build a key from one is to copy exactly key.len bytes. This used to fall back to
        // assigning the bare pointer for any other key type, which hands out an unbounded, unterminated pointer
        // into the arena - it happened to be dead code, since every map read here is keyed by ui18nstring. Failing
        // to compile is the honest answer for a key type that cannot be filled that way
        static_assert(IsStringLike<Key>,
                      "read_dict can only fill string-like keys: a ryml key is not null-terminated, so it has to be "
                      "copied with its length. Give the new key type an explicit conversion here.");

        if (!keyValid(ref) || !ref.is_seq())
            return false;

        for (const auto& a : ref.children())
        {
            if (!a.is_map())
                continue;

            // Each sequence element is a single-pair map ('- key: value'), so descend into its
            // key/value pair instead of reading the keyless seq element itself.
            for (const auto& entry : a.children())
            {
                // key() on a node that has none is a ryml error, see the note in read() above
                if (!entry.has_key())
                    continue;

                auto k = entry.key();

                Key key{};
                key.resize(k.len);
                memcpy(key.data(), k.data(), k.len);

                Val val{};
                entry.load(&val);

                t->insert({key, val});
            }
        }

        return true;
    }

    template<typename Key, typename Val>
    static bool read(ConstNodeRef const& ref, ui18nmap<Key, Val>* t)
    {
        return read_dict(ref, t);
    }
}


UI18N::InitialisationResult UI18N::TranslationEngine::parseConfig(const std::filesystem::path& directory) noexcept
{
    ui18nstring string{};
    if (!loadFileToString(directory / "ui18n-config.yaml", string))
        return UI18N_INIT_RESULT_INVALID_CONFIG;

    // An empty config file is valid and means exactly what it says: this translation set declares no terms. Only a
    // missing or unreadable one, rejected above, is a configuration error
    if (string.empty())
        return UI18N_INIT_RESULT_SUCCESS;

    const auto tree = ryml::parse_in_arena(string.c_str());
    if (tree.empty())
        return UI18N_INIT_RESULT_SUCCESS;

    const auto terms_l = tree.crootref().find_child("terms");
    if (keyValid(terms_l))
        terms_l.load(&terms);
    return UI18N_INIT_RESULT_SUCCESS;
}

void UI18N::TranslationEngine::pushVariable(const ui18nstring& name, const ui18nstring& val) noexcept
{
    variables.insert(std::pair{ name, val });
}

const char* UI18N::languageCodeToString(const LanguageCodes code) noexcept
{
    if (code >= UI18N_LANGUAGE_CODES_COUNT)
        return "";
    return LanguageCodesAsStrings[code];
}

// Compares a code against one canonical spelling without allocating. Lowercasing both sides into fresh strings, as
// this used to, cost two allocations for every one of the 298 candidates - ~600 per lookup - to answer a question
// that a character-wise comparison settles at the first letter for all but a handful of them
static bool localeCodeEquals(const char* code, const char* canonical) noexcept
{
    size_t i = 0;
    for (; code[i] != '\0' && canonical[i] != '\0'; i++)
    {
        // Both spellings of the separator are accepted, so normalise the input's as it is read rather than
        // rewriting the whole string first
        const char a = (code[i] == '-') ? '_' : asciiToLower(code[i]);
        if (a != asciiToLower(canonical[i]))
            return false;
    }
    // Equal only if both ended together, otherwise one is a prefix of the other
    return code[i] == '\0' && canonical[i] == '\0';
}

UI18N::LanguageCodes UI18N::stringToLanguageCode(const char* code) noexcept
{
    if (code == nullptr)
        return UI18N_LANGUAGE_CODES_COUNT;

    for (size_t i = 0; i < UI18N_LANGUAGE_CODES_COUNT; i++)
        if (localeCodeEquals(code, LanguageCodesAsStrings[i]))
            return static_cast<LanguageCodes>(i);

    return UI18N_LANGUAGE_CODES_COUNT;
}

void UI18N::TranslationEngine::replaceVariableInString(ui18nstring& str, const ui18nstring& replaceName, const ui18nstring& replace) noexcept
{
    const ui18nstring pattern = "{" + replaceName + "}";
    // Resume searching after the text we just inserted, never at its start: a replacement that itself contains
    // "{replaceName}" would otherwise be re-matched forever
    for (size_t offset = str.find(pattern); offset != ui18nstring::npos; offset = str.find(pattern, offset + replace.size()))
        str.replace(offset, pattern.size(), replace);
}

void UI18N::TranslationEngine::getHandlePositionalArguments(ui18nstring& text, const std::vector<ui18nstring>& args) noexcept
{
    size_t ppos = 0;
    for (const auto& f : args)
    {
        const auto pos = text.find("{}", ppos);
        if (pos == ui18nstring::npos)
            break;
        text.replace(pos, 2, f);
        // Resume past the argument just inserted, never at its start: an argument whose own value contains "{}"
        // would otherwise be rescanned, and the next argument would land inside the previous one instead of in
        // the next real placeholder
        ppos = pos + f.size();
    }
}

void UI18N::TranslationEngine::getHandleVariables(ui18nstring& text, const ui18nmap<ui18nstring, Switch>& references, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    // Both containers are hash maps, so each reference is resolved with a probe rather than by scanning them. The
    // scans this replaces were O(references * (variables + args)) and, because they yielded the map's own
    // value_type (whose key is const), every match also copied both of its strings into the pair parameter that
    // used to be passed on from here. A duplicate match is still ignored: the first container to answer wins
    for (const auto& f : references)
    {
        // Engine variables, added for long-term storage by the "pushVariable" method
        const auto variable = variables.find(f.first);
        if (variable != variables.end())
        {
            getHandleReplaceWithVal(f.second, text, variable->first, variable->second, {});
            continue;
        }

        // Variables as argument, added by the "args" argument of this function
        const auto arg = args.find(f.first);
        if (arg != args.end())
        {
            getHandleReplaceWithVal(f.second, text, arg->first, arg->second, args);
            continue;
        }

        // No variable was supplied for this reference. If it declares a switch, fall back to the switch's default
        // value, since an absent variable is exactly what a default is for. Without a switch there is nothing to
        // substitute, so the placeholder is left untouched
        if (f.second.bExists)
            getHandleReplaceWithDefault(f.second, text, f.first, args);
    }
}

// The variable is taken as a separate name and value rather than as a pair: the caller's pair comes out of a hash
// map, whose value_type has a const key, so binding it to a pair with a non-const key copied both strings
void UI18N::TranslationEngine::getHandleReplaceWithVal(const Switch& switchA, ui18nstring& text, const ui18nstring& name, const ui18nstring& value, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    // Default behaviour, when there is no switch statement
    if (!switchA.bExists)
    {
        replaceVariableInString(text, name, value);
        return;
    }

    // The patterns are a hash map keyed by the case, so the matching one is a probe rather than a scan of them all
    const auto pattern = switchA.patterns.find(value);
    // No case matched the value of the variable, so use the default
    if (pattern == switchA.patterns.end())
    {
        getHandleReplaceWithDefault(switchA, text, name, args);
        return;
    }

    auto resultTmp = pattern->second;
    // Replace any variables that may be templated into the result key
    for (const auto& f : variables)
        replaceVariableInString(resultTmp, f.first, f.second);
    // Also from the temporary variable list if provided
    for (const auto& f : args)
        replaceVariableInString(resultTmp, f.first, f.second);

    replaceVariableInString(text, name, resultTmp);
}

void UI18N::TranslationEngine::getHandleReplaceWithDefault(const Switch& switchA, ui18nstring& text, const ui18nstring& name, const ui18nmap<ui18nstring, ui18nstring>& args) noexcept
{
    // Replace any variables in the default variable
    auto defaultValTmp = switchA.defaultValue;
    for (const auto& a : variables)
        replaceVariableInString(defaultValTmp, a.first, a.second);
    // Also from the temporary variable list if provided
    for (const auto& a : args)
        replaceVariableInString(defaultValTmp, a.first, a.second);

    // Finally replace everything in the default value
    replaceVariableInString(text, name, defaultValTmp);
}

void UI18N::TranslationEngine::setCurrentLocale(const LanguageCodes locale) noexcept
{
    currentLocale = locale;
}

const std::vector<UI18N::LanguageCodes>& UI18N::TranslationEngine::getExistingLocales() noexcept
{
    return existingLocales;
}
