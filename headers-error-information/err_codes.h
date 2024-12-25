/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 17/01/2023 */
/**************************************/

#ifndef ERR_CODES_H_INCLUDED
#define ERR_CODES_H_INCLUDED

/**
 * \file
 * \author Pavlo Nykolyn
 * a collection of the program error codes and ancillary macros
 */

#define WRC_CDS_NUMCRITERR     1  // number of critical errors
#define WRC_CDS_NUMNONCRITERR  5  // number of non-critical errors

enum {wRC_Cd_heapManFail = -WRC_CDS_NUMCRITERR, /**< heap manipulation failure */
      wRC_Cd_noError = 0,                       /**< no error */
      wRC_Cd_wrPPar,                            /**< a program parameter is not correct */
      wRC_Cd_incChArr,                          /**< inconsistent character array (either its size is non-zero and the array is not valid or the opposite is true) */
      wRC_Cd_invP,                              /**< a function parameter is not valid */
      wRC_Cd_curl,                              /**< curl encountered an error condition */
      wRC_Cd_wrI = WRC_CDS_NUMNONCRITERR,       /**< the user provided the wrong input in the iterative session */
     };

#endif // ERR_CODES_H_INCLUDED
