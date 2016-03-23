/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   fgen-ftemplate-types.h
 * \brief  Defines types for feature templates
 * \author Xavier Carreras, Terry Koo
 */
#ifndef TREELER_FGENFTEMPLATETYPES_H
#define TREELER_FGENFTEMPLATETYPES_H

namespace treeler {

  /** 
   * \brief Defines a number of feature template types for token features for dependency parsing
   */
  typedef enum {
    FG_TOK_WORD,
    FG_TOK_LEMMA,
    FG_TOK_CTAG,
    FG_TOK_FTAG,
    FG_TOK_WORD_CTAG,
    FG_TOK_MORPHO,
    FG_TOK_WORD_MORPHO,

    FG_TOK_PREV1,
    FG_TOK_PREV2,
    FG_TOK_NEXT1,
    FG_TOK_NEXT2,

    FG_TOK_PREVNULL,
    FG_TOK_NEXTNULL,

    FG_TOK_PREV_CTAG_BIGRAM,
    FG_TOK_PREV_CTAG_TRIGRAM,
    FG_TOK_NEXT_CTAG_BIGRAM,
    FG_TOK_NEXT_CTAG_TRIGRAM,

    FGC_TOK_CL4,
    FGC_TOK_CL6,
    FGC_TOK_CL8,
    FGC_TOK_CL,
    FGC_TOK_CH,

    FGC_TOK_PREV_CL4_BIGRAM,
    FGC_TOK_PREV_CL4_TRIGRAM,
    FGC_TOK_PREV_CL6_BIGRAM,
    FGC_TOK_PREV_CL6_TRIGRAM,
    FGC_TOK_NEXT_CL4_BIGRAM,
    FGC_TOK_NEXT_CL4_TRIGRAM,
    FGC_TOK_NEXT_CL6_BIGRAM,
    FGC_TOK_NEXT_CL6_TRIGRAM
  } FGenFTemplateTypesToken;

  /** 
   * \brief Defines a number of feature template types for first-order dependency features for dependency parsing
   */
  typedef enum {
    FG_DEP_SW_EW,
    FG_DEP_SWT_EWT,
    FG_DEP_SWT_EW,
    FG_DEP_SW_EWT,
    FG_DEP_SWT_ET,
    FG_DEP_ST_EWT,
    FG_DEP_ST_ET,

    FG_DEP_CTXT_ROOT_EP,
    FG_DEP_CTXT_ROOT_EN,
    FG_DEP_CTXT_ROOT_EPN,

    FG_DEP_CTXT_ADJ_S_EN,
    FG_DEP_CTXT_ADJ_SP_E,
    FG_DEP_CTXT_ADJ_SP_EN,

    FG_DEP_CTXT_S_EP,
    FG_DEP_CTXT_S_EN,
    FG_DEP_CTXT_SP_E,
    FG_DEP_CTXT_SN_E,
    FG_DEP_CTXT_SP_EP,
    FG_DEP_CTXT_SP_EN,
    FG_DEP_CTXT_SN_EP,
    FG_DEP_CTXT_SN_EN,

    FG_DEP_BETW_SEB,

    FG_DEP_BETW_SE_VCNT,
    FG_DEP_BETW_SE_PCNT,
    FG_DEP_BETW_SE_CCNT,

    FG_DEP_BETW_VCNT,
    FG_DEP_BETW_PCNT,
    FG_DEP_BETW_CCNT,

    FG_DEP_DIST_EXACT,
    FG_DEP_DIST_BINGT,
    FG_DEP_DIST_BINGT_SCTAG,
    FG_DEP_DIST_BINGT_ECTAG,
    FG_DEP_DIST_BINGT_SECTAG,

    FG_DEP_AUX_AGREE,
    FG_DEP_AUX_AGREE_HT,
    FG_DEP_AUX_AGREE_MT,
    FG_DEP_AUX_AGREE_HMT,

    FG_DEP_AUX_PAIR_IND,
    FG_DEP_AUX_PAIR_HT,
    FG_DEP_AUX_PAIR_MT,
    FG_DEP_AUX_PAIR_HMT,

    FG_DEP_AUX_UNANIMOUS,
    FG_DEP_AUX_UNANIMOUS_HT,
    FG_DEP_AUX_UNANIMOUS_MT,
    FG_DEP_AUX_UNANIMOUS_HMT,

    FG_DEP_AUX_MAJORITY,
    FG_DEP_AUX_MAJORITY_HT,
    FG_DEP_AUX_MAJORITY_MT,
    FG_DEP_AUX_MAJORITY_HMT,

    FG_DEP_SM_ET,
    FG_DEP_ST_EM,
    FG_DEP_SMT_EM,
    FG_DEP_SMT_ET,
    FG_DEP_ST_EMT,
    FG_DEP_SM_EMT,
    FG_DEP_SM_EM,
    FG_DEP_SMT_EMT,


    FGC_DEP_SW_ECL4,
    FGC_DEP_SW_ECL6,
    FGC_DEP_SW_ECL8,
    FGC_DEP_SW_ECL,
    FGC_DEP_SW_ECH,
    FGC_DEP_ST_ECL4,
    FGC_DEP_ST_ECL6,
    FGC_DEP_ST_ECL8,
    FGC_DEP_ST_ECL,
    FGC_DEP_ST_ECH,

    FGC_DEP_SCL4_EW,
    FGC_DEP_SCL6_EW,
    FGC_DEP_SCL8_EW,
    FGC_DEP_SCL_EW,
    FGC_DEP_SCH_EW,
    FGC_DEP_SCL4_ET,
    FGC_DEP_SCL6_ET,
    FGC_DEP_SCL8_ET,
    FGC_DEP_SCL_ET,
    FGC_DEP_SCH_ET,

    FGC_DEP_SCL4_ECL4,
    FGC_DEP_SCL6_ECL6,
    FGC_DEP_SCL8_ECL8,
    FGC_DEP_SCL_ECL,
    FGC_DEP_SCH_ECH,

    FGC_DEP_SM_ECL4,
    FGC_DEP_SM_ECL6,
    FGC_DEP_SCL4_EM,
    FGC_DEP_SCL6_EM,
    FGC_DEP_SMCL4_EM,
    FGC_DEP_SMCL6_EM,
    FGC_DEP_SM_EMCL4,
    FGC_DEP_SM_EMCL6,
    FGC_DEP_SMCL4_ECL4,
    FGC_DEP_SMCL6_ECL6,
    FGC_DEP_SCL4_EMCL4,
    FGC_DEP_SCL6_EMCL6,
    FGC_DEP_SMCL4_EMCL4,
    FGC_DEP_SMCL6_EMCL6,

    FGC_DEP_CTXT_ROOT_PREV,
    FGC_DEP_CTXT_ROOT_PREVNEXT,
    FGC_DEP_CTXT_ROOT_NEXT,

    FGC_DEP_CTXT_ADJ_PREV,
    FGC_DEP_CTXT_ADJ_PREVNEXT,
    FGC_DEP_CTXT_ADJ_NEXT,

    FGC_DEP_CTXT_SP_E,
    FGC_DEP_CTXT_SN_E,
    FGC_DEP_CTXT_SP_EP,
    FGC_DEP_CTXT_SP_EN,
    FGC_DEP_CTXT_SN_EP,
    FGC_DEP_CTXT_SN_EN,
    FGC_DEP_CTXT_S_EP,
    FGC_DEP_CTXT_S_EN,

    FGC_DEP_BETW_SEB,

    FGC_DEP_DIST_BINGT_S,
    FGC_DEP_DIST_BINGT_E,
    FGC_DEP_DIST_BINGT_SE,

    FGC_DEP_AUX_AGREE_H,
    FGC_DEP_AUX_AGREE_M,
    FGC_DEP_AUX_AGREE_HM,

    FGC_DEP_AUX_PAIR_H,
    FGC_DEP_AUX_PAIR_M,
    FGC_DEP_AUX_PAIR_HM,

    FGC_DEP_AUX_UNANIMOUS_H,
    FGC_DEP_AUX_UNANIMOUS_M,
    FGC_DEP_AUX_UNANIMOUS_HM,

    FGC_DEP_AUX_MAJORITY_H,
    FGC_DEP_AUX_MAJORITY_M,
    FGC_DEP_AUX_MAJORITY_HM
  } FGenFTemplateTypesDep1;

  /** 
   * \brief Defines a number of feature template types for second-order features for dependency parsing
   */
  typedef enum {
    FG_O2_HT_MT_CT,
    FG_O2_HT_CT,
    FG_O2_MT_CT,
    FG_O2_HW_CT,
    FG_O2_MW_CT,
    FG_O2_HT_CW,
    FG_O2_MT_CW,
    FG_O2_HW_CW,
    FG_O2_MW_CW,

    FG_O2_HT_MT,
    FG_O2_HW_MT,
    FG_O2_HT_MW,
    FG_O2_HW_MW,

    FG_O2_AUX_AGREE_HM,
    FG_O2_AUX_AGREE_HM_TT,
    FG_O2_AUX_AGREE_HM_TTT,
    FG_O2_AUX_AGREE_HC,
    FG_O2_AUX_AGREE_HC_TT,
    FG_O2_AUX_AGREE_HC_TTT,
    FG_O2_AUX_AGREE_MC,
    FG_O2_AUX_AGREE_MC_TT,
    FG_O2_AUX_AGREE_MC_TTT,

    FG_O2_AUX_PAIR_HM,
    FG_O2_AUX_PAIR_HM_TT,
    FG_O2_AUX_PAIR_HM_TTT,
    FG_O2_AUX_PAIR_HC,
    FG_O2_AUX_PAIR_HC_TT,
    FG_O2_AUX_PAIR_HC_TTT,
    FG_O2_AUX_PAIR_MC,
    FG_O2_AUX_PAIR_MC_TT,
    FG_O2_AUX_PAIR_MC_TTT,

    FG_O2_AUX_AGREE_CH,
    FG_O2_AUX_AGREE_CH_TTT,
    FG_O2_AUX_AGREE_CMI,
    FG_O2_AUX_AGREE_CMI_TTT,
    FG_O2_AUX_AGREE_CMO,
    FG_O2_AUX_AGREE_CMO_TTT,

    FG_O2_AUX_PAIR_CH,
    FG_O2_AUX_PAIR_CH_TTT,
    FG_O2_AUX_PAIR_CMI,
    FG_O2_AUX_PAIR_CMI_TTT,
    FG_O2_AUX_PAIR_CMO,
    FG_O2_AUX_PAIR_CMO_TTT,



    FGC_O2_HM,
    FGC_O2_HC,
    FGC_O2_MC,
    FGC_O2_HMC,

    FGC_O2_AUX_AGREE_HM,
    FGC_O2_AUX_AGREE_HM_TT,
    FGC_O2_AUX_AGREE_HM_TTT,
    FGC_O2_AUX_AGREE_HC,
    FGC_O2_AUX_AGREE_HC_TT,
    FGC_O2_AUX_AGREE_HC_TTT,
    FGC_O2_AUX_AGREE_MC,
    FGC_O2_AUX_AGREE_MC_TT,
    FGC_O2_AUX_AGREE_MC_TTT,

    FGC_O2_AUX_PAIR_HM,
    FGC_O2_AUX_PAIR_HM_TT,
    FGC_O2_AUX_PAIR_HM_TTT,
    FGC_O2_AUX_PAIR_HC,
    FGC_O2_AUX_PAIR_HC_TT,
    FGC_O2_AUX_PAIR_HC_TTT,
    FGC_O2_AUX_PAIR_MC,
    FGC_O2_AUX_PAIR_MC_TT,
    FGC_O2_AUX_PAIR_MC_TTT,

    FGC_O2_AUX_AGREE_CH,
    FGC_O2_AUX_AGREE_CH_TTT,
    FGC_O2_AUX_AGREE_CMI,
    FGC_O2_AUX_AGREE_CMI_TTT,
    FGC_O2_AUX_AGREE_CMO,
    FGC_O2_AUX_AGREE_CMO_TTT,

    FGC_O2_AUX_PAIR_CH,
    FGC_O2_AUX_PAIR_CH_TTT,
    FGC_O2_AUX_PAIR_CMI,
    FGC_O2_AUX_PAIR_CMI_TTT,
    FGC_O2_AUX_PAIR_CMO,
    FGC_O2_AUX_PAIR_CMO_TTT,


    FG_O2_HW_MT_CT,
    FG_O2_HT_MW_CT,
    FG_O2_HT_MT_CW,

    FG_O2_HT_MW_CW,
    FG_O2_HW_MT_CW,
    FG_O2_HW_MW_CT,

    FG_O2_HW_MW_CW

  } FGenFTemplateTypesDep2;
  
  /** 
   * \brief Defines a number of feature template types for McDonald dependency features for dependency parsing
   */
  typedef enum {
    FG_DEP_HW_HT,
    FG_DEP_HW,
    FG_DEP_HT,
    FG_DEP_MW_MT,
    FG_DEP_MW,
    FG_DEP_MT,
    
    FG_DEP_HW_HT_MW_MT,
    FG_DEP_HT_MW_MT,
    FG_DEP_HW_MW_MT,
    FG_DEP_HW_HT_MT,
    FG_DEP_HW_HT_MW,
    FG_DEP_HW_MW,
    FG_DEP_HT_MT,

    FG_DEP_BETW,

    FG_DEP_SURR_HN_MP,
    FG_DEP_SURR_HP_MP,
    FG_DEP_SURR_HN_MN,
    FG_DEP_SURR_HP_MN
  } FGenFTemplateTypesMcDonald;
}

#endif /* DEP_FGENFTEMPLATETYPES_H */
