/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 21/05/2023 */
/**************************************/

#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

/**
 * \file
 * \author Pavlo Nykolyn
 * the interface of the parsing services:
 * - user input;
 * - web relay response;
 */

#include <stdbool.h>
#include "status.h"
#include "parser_constants.h"

enum P_oActCds {oAct_quit,    /**< quits an iterative session */
                oAct_numOAct  /**< number of other actions */
               };

typedef struct P_out {
// relay that is to be commanded
   int p_rID;
// action on the relay
   bool p_fAct;
// other actions
   enum P_oActCds p_oAct;
} P_out;

/** \brief parses an input line
 * \param[in,out] p_pIntData a pointer to a structure that will contain information about the parsed command
 * \return an error code
 *
 * one of the following error codes will be returned:
 * - \a wRC_Cd_noError ;
 * - \a wRC_Cd_invP ;
 * - \a wRC_Cd_wrI
 */
int P_parseInput(P_out* restrict p_pIntData);

/**
 * \brief attempts to parse a mnemonic code
 * \param[in,out] p_pIntData a pointer to a structure that will contain information about the parsed mnemonic code
 * \param[in] p_strMnemCd string containing the mnemonic code
 * \return error code
 * \warning the caller HAS TO ignore the data of the parsed mnemonic code in case an error is returned
 *
 * one of the following error codes will be returned:
 * - \a wRC_Cd_noError ;
 * - \a wRC_Cd_invP ;
 * - \a wRC_Cd_wrI
 */
int P_parseMnemCode(P_out* restrict p_pIntData,
                    const char p_strMnemCd[static P_CST_MAXSZSTR_MNEMCD]);

/** \brief parses the html response of a web relay
 * \param[in] p_szStrResp size of the string holding the response
 * \param[in] p_strResp string holding the response
 * \param[in] p_hwMod model of the web relay
 * \return the status of the relay
 *
 * currently, the following web relays are supported:
 * - KMTronic;
 * - NC800;
 * the status of the relay labeled one is the most significant bit of the returned value while
 * the status of the relay labeled eight is the least significant bit of the returned value;
 * bit set to 0 -> the relay is close;
 * bit set to 1 -> the relay is open
 */
r_stat P_parseHtmlResp(size_t p_szStrResp, const char* const p_strResp,
                       const enum r_mCodes p_hwMod);

#endif // PARSER_H_INCLUDED
