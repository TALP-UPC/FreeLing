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
 * \file   fgen-srl-types.h
 * \brief  Defines types for feature templates
 * \author Xavier Carreras, Terry Koo
 */
#ifndef TREELER_FGEN_SRL_TYPES_H
#define TREELER_FGEN_SRL_TYPES_H

namespace treeler {

  /** 
   * \brief Defines a number of feature template types for token features for SRL parsing
   */
  typedef enum {
      //Johansson secondary features
  JS_PRED_WORD = 128, 
  JS_PRED_LEMMA,
  JS_PRED_CPOS,
  JS_PRED_FPOS,

  JS_PRED_W_PRED_CPOS,

  JS_PRED_WORD_LABEL,
  JS_PRED_LEMMA_LABEL,
  JS_PRED_CPOS_LABEL,
  JS_PRED_W_PRED_CPOS_LABEL,

  JS_ARG_WORD,
  JS_ARG_LEMMA,
  JS_ARG_CPOS,
  JS_ARG_FPOS,
  JS_ARG_W_ARG_CPOS,
  JS_ARG_WORD_LABEL,
  JS_ARG_LEMMA_LABEL,
  JS_ARG_CPOS_LABEL,
  JS_ARG_FPOS_LABEL,
  JS_ARG_W_ARG_CPOS_LABEL,

  JS_PRED_ARG_W,
  JS_PRED_ARG_CPOS,
  JS_PRED_C_ARG_W_C,
  JS_PRED_W_ARG_W_C,
  JS_PRED_W_C_ARG_W,
  JS_PRED_W_C_ARG_C,
  JS_PRED_W_C_ARG_W_C,

  JS_TK_WORD,
  JS_TK_LEMMA,
  JS_TK_CPOS,
  JS_TK_FPOS,
  JS_TK_W_CPOS,

  JS_TK_WORD_LABEL,
  JS_TK_CPOS_LABEL,
  JS_PRED_ARG_W_LABEL,
  JS_PRED_ARG_CP_LABEL,
  JS_PRED_W_ARG_W_M1_LABEL,
  JS_PRED_W_ARG_W_P1_LABEL,


  JS_PRED_ARG_W_EM1,
  JS_PRED_ARG_CPOS_EM1,
  JS_PRED_ARG_W_EP1,
  JS_PRED_ARG_CPOS_EP1,

  JS_PRED_ARG_W_SM1,
  JS_PRED_ARG_CPOS_SM1,
  JS_PRED_ARG_W_SP1,
  JS_PRED_ARG_CPOS_SP1,
  JS_PRED_ARG_W_SM1_FIRST,
  JS_PRED_ARG_W_EP1_LAST,

  JS_TK_FIRST,
  JS_TK_LAST,

  JS_I_NODE_PATH,
  JS_I_POS,
  JS_I_UPDOWN,
  JS_I_LABELED_PATH,
  JS_I_LEMMA_PATH,

  GJ_POSITION,

  GJ_PW_TW,
  GJ_POS_VOICE_SYNL,
  GJ_POS_VOICE_SYNL_PRED,

  JS_TK_IS_VERB,
  JS_TK_IS_PASSIVE,

  //last role dynamic features
  JS_TK_WORD_LAST_ROLE,
  JS_TK_CPOS_LAST_ROLE,
  JS_TK_LAST_ROLE,
  JS_TK_LAST_ROLE_DIR,
  JS_TK_LAST_ROLE_POS,

  JS_TK_LAST_ROLE_BITSTRING,


  JS_TK_FINAL,

  J_PRED_WORD,
  J_PRED_POS,
  J_ARG_WORD,
  J_ARG_POS,
  J_PRED_ARG_POS,
  J_PRED_WORD_LABEL,
  J_PRED_POS_LABEL,
  J_ARG_WORD_LABEL,
  J_ARG_POS_LABEL,
  J_PRED_ARG_POS_LABEL,
  J_PATH,
  J_PATH_ARG_POS,
  J_PATH_PRED_POS,
  J_PATH_ARG_WORD,
  J_PATH_PRED_WORD,
  J_PATH_LABEL,
  J_PATH_ARG_POS_LABEL,
  J_PATH_PRED_POS_LABEL,
  J_PATH_ARG_WORD_LABEL,
  J_PATH_PRED_WORD_LABEL,

  J_BIAS,

  J_PATH_TOKEN_HEAD_WORD,
  J_PATH_TOKEN_HEAD_POS,
  J_PATH_TOKEN_HEAD_CPOS,
  J_PATH_TOKEN_MOD_WORD,
  J_PATH_TOKEN_MOD_POS,
  J_PATH_TOKEN_MOD_CPOS,
  J_PATH_TOKEN_WORD_WORD,
  J_PATH_TOKEN_POS_POS,
  J_PATH_TOKEN_DIR,
  J_PATH_TOKEN_DOWN,
  J_PATH_TOKEN_WWP,
  J_PATH_TOKEN_PWW,
  J_PATH_TOKEN_WPP,
  J_PATH_TOKEN_WWPP,
  J_PATH_TOKEN_NO_PREV_MOD,
  J_PATH_TOKEN_NO_PREV_HEAD,
  J_PATH_TOKEN_NO_PREV_2,
  J_PATH_TOKEN_LAST,
  J_PATH_TOKEN_LAST_2,
  J_PATH_TOKEN_CTX_B1,
  J_PATH_TOKEN_CTX_B2,
  J_PATH_TOKEN_DIST,
  J_PATH_TOKEN_BTW_POS,
  J_PATH_TOKEN_BTW_PRED,
  J_PATH_TOKEN_BTW_PUNCT,
  J_PATH_TOKEN_BTW_COORD,
  J_PATH_TOKEN_VOICE,
  J_PATH_TOKEN_VOICE_POS,
  J_PATH_TOKEN_VOICE_DIR,
  J_PATH_TOKEN_PP_HEAD,
  J_PATH_TOKEN_HOLES


  } FGenFTemplateTypesSrl;
}

#endif 
