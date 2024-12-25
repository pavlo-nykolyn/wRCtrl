/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 21/05/2023 */
/**************************************/

#ifndef CTRL_H_INCLUDED
#define CTRL_H_INCLUDED

/**
 * \file
 * \author Pavlo Nykolyn
 * interface for the controlling services
 */

#include "status.h"
#include "parser_constants.h"

/** \brief performs a single operation on a relay
 * \param[in] rC_szStr_IPv4 size of the string holding an IPv4 address
 * \param[in] rC_str_IPv4 string holding an IPv4 address
 * \param[in] rC_szStr_port size of the string holding a port number
 * \param[in] rC_szStr_port string holding a port number
 * \param[in] p_strMnemCd string containing the mnemonic code
 * \param[in] rC_hwMod model of the controlled hardware
 * \return error code
 * \attention the string holding the IPv4 address is checked only for consistency. The validity of
 *            what it holds HAS TO BE ensured by the caller
 *            \par
 *            the strings HAVE TO BE null-terminated
 *
 * one of the following error codes may be returned:
 * \a wRC_Cd_noError ;
 * \a wRC_Cd_incChArr ;
 * \a wRC_Cd_wrI ;
 * \a wRC_Cd_curl
 */
int rC_doSingleOperation(size_t rC_szStr_IPv4, const char* const rC_str_IPv4,
                         size_t rC_szStr_port, const char* const rC_str_port,
                         const char rC_strMnemCd[static P_CST_MAXSZSTR_MNEMCD],
                         enum r_mCodes rC_hwMod);

/** \brief same as \a rC_doSingleOperation but, provides a command line that supports multiple commands;
 *         \a quit has to be used to terminate the interactive session. There is no need to provide a
 *         mnemonic code
 * \attention \a wRC_Cd_wrI is dealt with internally
 */
int rC_doMultipleOperations(size_t rC_szStr_IPv4, const char* const rC_str_IPv4,
                            size_t rC_szStr_port, const char* const rC_str_port,
                            enum r_mCodes rC_hwMod);

#endif // CTRL_H_INCLUDED
