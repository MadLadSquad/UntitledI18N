#pragma once

#ifdef MLS_EXPORT_LIBRARY
    #ifdef _WIN32
        #ifdef MLS_LIB_COMPILE
            #define MLS_PUBLIC_API __declspec(dllexport)
        #else
            #define MLS_PUBLIC_API __declspec(dllimport)
        #endif
    #else
        #define MLS_PUBLIC_API
    #endif
#else
    #define MLS_PUBLIC_API
#endif

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum [[maybe_unused]] UI18N_LanguageCodes
    {
        aa_DJ,aa_ER,aa_ET,af_ZA,agr_PE,ak_GH,am_ET,an_ES,anp_IN,ar_AE,ar_BH,ar_DZ,ar_EG,ar_IN,ar_IQ,ar_JO,ar_KW,ar_LB,
        ar_LY,ar_MA,ar_OM,ar_QA,ar_SA,ar_SD,ar_SS,ar_SY,ar_TN,ar_YE,as_IN,ast_ES,ayc_PE,az_AZ,az_IR,be_BY,bem_ZM,ber_DZ,
        ber_MA,bg_BG,bho_IN,bho_NP,bi_VU,bn_BD,bn_IN,bo_CN,bo_IN,br_FR,brx_IN,bs_BA,byn_ER,ca_AD,ca_ES,ca_FR,ca_IT,
        ce_RU,chr_US,ckb_IQ,cmn_TW,crh_UA,cs_CZ,csb_PL,cv_RU,cy_GB,da_DK,de_AT,de_BE,de_CH,de_DE,de_IT,de_LU,doi_IN,
        dsb_DE,dv_MV,dz_BT,el_CY,el_GR,en_AG,en_AU,en_BW,en_CA,en_DK,en_GB,en_HK,en_IE,en_IL,en_IN,en_NG,en_NZ,en_PH,
        en_SG,en_US,en_ZA,en_ZM,en_ZW,eo,es_AR,es_BO,es_CL,es_CO,es_CR,es_CU,es_DO,es_EC,es_ES,es_GT,es_HN,es_MX,es_NI,
        es_PA,es_PE,es_PR,es_PY,es_SV,es_US,es_UY,es_VE,et_EE,eu_ES,fa_IR,ff_SN,fi_FI,fil_PH,fo_FO,fr_BE,fr_CA,fr_CH,
        fr_FR,fr_LU,fur_IT,fy_DE,fy_NL,ga_IE,gd_GB,gez_ER,gez_ET,gl_ES,gu_IN,gv_GB,ha_NG,hak_TW,he_IL,hi_IN,hif_FJ,
        hne_IN,hr_HR,hsb_DE,ht_HT,hu_HU,hy_AM,ia_FR,id_ID,ig_NG,ik_CA,is_IS,it_CH,it_IT,iu_CA,jp_JP,ka_GE,kab_DZ,kk_KZ,kl_GL,
        km_KH,kn_IN,kok_IN,ks_IN,ku_TR,kw_GB,ky_KG,lb_LU,lg_UG,li_BE,li_NL,lij_IT,ln_CD,lo_LA,lt_LT,lv_LV,lzh_TW,mag_IN,
        mai_IN,mai_NP,mfe_MU,mg_MG,mhr_RU,mi_NZ,miq_NI,mjw_IN,mk_MK,ml_IN,mn_MN,mni_IN,mnw_MM,mr_IN,ms_MY,mt_MT,my_MM,
        nan_TW,nb_NO,nds_DE,nds_NL,ne_NP,nhn_MX,niu_NU,niu_NZ,nl_AW,nl_BE,nl_NL,nn_NO,nr_ZA,nso_ZA,oc_FR,om_ET,om_KE,
        or_IN,os_RU,pa_IN,pa_PK,pap_AW,pap_CW,pl_PL,ps_AF,pt_BR,pt_PT,quz_PE,raj_IN,ro_RO,ru_RU,ru_UA,rw_RW,sa_IN,
        sah_RU,sat_IN,sc_IT,sd_IN,se_NO,sgs_LT,shn_MM,shs_CA,si_LK,sid_ET,sk_SK,sl_SI,sm_WS,so_DJ,so_ET,so_KE,so_SO,
        sq_AL,sq_MK,sr_ME,sr_RS,ss_ZA,st_ZA,sv_FI,sv_SE,sw_KE,sw_TZ,szl_PL,ta_IN,ta_LK,te_IN,tg_TJ,th_TH,the_NP,ti_ER,
        ti_ET,tig_ER,tk_TM,tl_PH,tn_ZA,to_TO,tpi_PG,tr_CY,tr_TR,ts_ZA,tt_RU,ug_CN,uk_UA,unm_US,ur_IN,ur_PK,uz_UZ,ve_ZA,
        vi_VN,wa_BE,wae_CH,wal_ET,wo_SN,xh_ZA,yi_US,yo_NG,yue_HK,yuw_PG,zh_CN,zh_HK,zh_SG,zh_TW,zu_ZA,
        UI18N_LANGUAGE_CODES_COUNT
    } UI18N_LanguageCodes;

    typedef enum UI18N_InitialisationResult
    {
        UI18N_INIT_RESULT_SUCCESS,
        UI18N_INIT_RESULT_INVALID_CONFIG,
        UI18N_INIT_RESULT_INVALID_TRANSLATION,
    } UI18N_InitialisationResult;

#ifdef __cplusplus
}
#endif