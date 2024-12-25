/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 24/01/2023 */
/**************************************/

#ifndef ERR_MESSAGES_H_INCLUDED
#define ERR_MESSAGES_H_INCLUDED

/**
 * \file
 * \author Pavlo Nykolyn
 * a collection of the program error messages
 */

#define WRC_MSG_HEAPMANFAIL  "[CRIT] heap manipulation failure\n"
#define WRC_MSG_UNSCINIT     "[ERR] unsuccessful curl initialization\n"
#define WRC_MSG_INCCHARR     "[ERR] inconsistent character array\n"
#define WRC_MSG_INVPAR       "[ERR] invalid parameter\n"
#define WRC_MSG_WRPPAR       "[ERR] wrong program parameter\n"
#define WRC_MSG_WRUSRI       "[ERR] wrong user input\n"
#define WRC_MSG_UNSCEH       "[ERR] unsuccessful creation of a curl easy handle\n"
#define WRC_MSG_HLPROT       "[ERR] libcurl does not supported at least one required protocol\n"

#endif // ERR_MESSAGES_H_INCLUDED
