/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 21/05/2023 */
/**************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "ctrl.h"
#include "err_wrapper.h"

// input parameter keys
#define WRC_HELP_KEY    "--help"
#define WRC_IPV4_KEY    "--ipv4"
#define WRC_PORT_KEY    "--port"
#define WRC_BEH_KEY     "--behaviour"
#define WRC_MNEMCD_KEY  "--mnemonic-code"
#define WRC_MODEL_KEY   "--model"
// error messages
#define WRC_WRIPV4LEN_MSG  "[ERR] The length of an IPv4 address is not correct\n"
#define WRC_WRIPV4SEQ_MSG  "[ERR] More than three digits or an unrecognised character belong to an IPv4 address sequence\n"
// program behaviour
#define WRC_SINGLE  "single"
#define WRC_ITER    "iter"
// supported values for the --model key
#define WRC_KMTRONIC  "KMTronic_wr"
#define WRC_NC800     "NC800"
// generic macros
#define WRC_MINSZSTR_IPV4   8U  // minimum size of the string that contains an IPv4 address (the null character is included)
#define WRC_MAXSZSTR_IPV4  16U  // maximum size of the string that contains an IPv4 address (the null character is included)
#define WRC_MAXSZSTR_PRT    6U  // maximum size of the string that contains a port number
// macros related to initial checks
// bit masks
#define WRC_PROT_NONE   0x00  // no protocol is supported
#define WRC_PROT_VALID  0x03  // both http and https are supported
#define WRC_PROT_HTTP   0x01  // http is supported
#define WRC_PROT_HTTPS  0x02  // https is supported

typedef unsigned char wRC_supProt_t; // indicates the protocols needed by this tool that are supported by the underlying libcurl

enum wRC_keyCodes {wRC_ipv4,      /**< IPv4 address of the web relay */
                   wRC_port,      /**< port associated to a web relay */
                   wRC_beh,       /**< behaviour adopted by the program */
                   wRc_mnemCode,  /**< mnemonic code */
                   wRC_model,     /**< model of the web relay */
                   wRC_help,      /**< information on how to use the program */
                   wRC_maxNumCds  /**< maximum number of codes */
                  };

typedef struct wRC_iPar {
// indicates whether a parameter has been specified on the command line
   bool wRC_fDef;
// index of the first character belonging to the parameter value
// if the value is not needed, this member must be set to zero
   size_t wRC_posVal;
// index of the input parameter in the parameter array
   int wRC_idxPar;
} wRC_iPar;

static void wRC_usage(void)
{
   fputs("wRCtrl --ipv4=<address> [--port=<port>] --model=<model> [--behaviour=<type> [--mnemonic-code=<code>]]\n\
          wRCtrl --help\n\
          --port has to be defined only for specific models;\n\
          --behaviour can be one of two types: single, meaning that the program\n\
          will attempt to perform a single operation and then will quit execution;\n\
          iter, meaning that the program will provide the ability to perform an\n\
          undefined number of operations sequentially.\n\
          The following commands are supported:\n\
          1) turn [on|off] <relay-ID>\n\
             switches the current state of a relay. It is assumed that\n\
             its identifier is an Arab digit (either 0 or 9 will not be accepted);\n\
          2) quit\n\
             terminates an iterative session;\n\
          if the user enters an unrecognized command, an appropriate error will be displayed but,\n\
          the session will not be terminated;\n\
          --mnemonic-code indicates the code used to convey a command. It makes sense only when --behaviour\n\
          is set to single. As such, using this parameter within an iterative instance of the controller will\n\
          cause an error. The structure of the code is the following:\n\
          <action>_<relay-ID> where:\n\
          a) <action> <= t_(on|off)\n\
             1) t_on indicates that the default state of relay has to switch;\n\
             2) t_off indicates that the default state of relay has to be restored;\n\
          b) <relay-ID> <= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 ;\n\
          --model defines the web relay that is to be queried. The supported devices are:\n\
          a) KMTronic_wr\n\
          b) NC800 (requires the --port option);\n\
          it should be noted that both the \"turn [on|off] <relay-ID>\" and <action>_<relay-ID>\n\
          are absolute commands. That is, if multiple instances of the same command are invoked in\n\
          a row, only the first one will result in its intended action\n", stdout);
}

static enum wRC_keyCodes wRC_getIParType(const char* const wRC_strIParID)
{
   if (!strcmp(wRC_strIParID, WRC_HELP_KEY))
      return wRC_help;
   else if (!strcmp(wRC_strIParID, WRC_IPV4_KEY))
      return wRC_ipv4;
   else if (!strcmp(wRC_strIParID, WRC_PORT_KEY))
      return wRC_port;
   else if (!strcmp(wRC_strIParID, WRC_BEH_KEY))
      return wRC_beh;
   else if (!strcmp(wRC_strIParID, WRC_MNEMCD_KEY))
      return wRc_mnemCode;
   else if (!strcmp(wRC_strIParID, WRC_MODEL_KEY))
      return wRC_model;
   return wRC_maxNumCds;
}

static bool wRC_chkIPv4(const size_t wRC_szStrIPv4, const char* const wRC_strIPv4)
{
   if (wRC_szStrIPv4 < WRC_MINSZSTR_IPV4 ||
       wRC_szStrIPv4 > WRC_MAXSZSTR_IPV4) {
      fputs(WRC_WRIPV4LEN_MSG, stderr);
      return false;
   }
   unsigned wRC_numDgs = 0; // number of digits of an IPv4 section (resets on each iteration)
   for (size_t i = 0; i < wRC_szStrIPv4 &&
                      wRC_strIPv4[i]; i++) {
      if (isdigit(wRC_strIPv4[i]))
         wRC_numDgs++;
      if (!wRC_numDgs ||
          wRC_numDgs == 4) {
         fputs(WRC_WRIPV4SEQ_MSG, stderr);
         return false;
      }
      else if (wRC_numDgs &&
               wRC_strIPv4[i] == '.')
         wRC_numDgs = 0;
   }
   return true;
}

int main(int argc, char* argv[])
{
   wRC_iPar wRC_iParColl[wRC_maxNumCds] = {0}; // has a key been defined?
   if (argc == 1 ||
       argc > 6) {
      wRC_usage();
      return EXIT_FAILURE;
   }
   for (size_t i = 1; i < argc; i++) {
      const size_t wRC_next = strcspn(argv[i], "=");
      char wRC_strKey[wRC_next + 1];
      wRC_strKey[wRC_next] = '\0';
      memcpy(wRC_strKey, argv[i], wRC_next);
      const enum wRC_keyCodes wRC_keyType = wRC_getIParType(wRC_strKey);
      if (argc > 2 &&
          wRC_keyType == wRC_help) {
         wRC_usage();
         return EXIT_FAILURE;
      }
      wRC_iParColl[wRC_keyType].wRC_fDef = true;
      wRC_iParColl[wRC_keyType].wRC_idxPar = i;
      if (wRC_keyType != wRC_help ) {
         if (*(argv[i] + wRC_next) &&
             wRC_keyType != wRC_maxNumCds)
            wRC_iParColl[wRC_keyType].wRC_posVal = wRC_next + 1;
         else {
            fputs(WRC_MSG_WRPPAR, stderr);
            return false;
         }
      }
   }
   size_t wRC_szStrIPv4 = WRC_MAXSZSTR_IPV4;
   char wRC_strIPv4[WRC_MAXSZSTR_IPV4] = {0};
   size_t wRC_szStrPort = WRC_MAXSZSTR_PRT;
   char wRC_strPort[WRC_MAXSZSTR_PRT] = {0};
   bool wRC_fBeh = false;
   char wRC_strMnemCd[P_CST_MAXSZSTR_MNEMCD] = {0};
   enum r_mCodes wRC_hwModel = r_numMod;
   for (size_t i = 0; i < wRC_maxNumCds; i++) {
      if (wRC_iParColl[i].wRC_fDef) {
         const size_t wRC_lenVal = strlen(argv[wRC_iParColl[i].wRC_idxPar]) - wRC_iParColl[i].wRC_posVal;
         if (i != wRC_help &&
             !wRC_lenVal) {
            fputs(WRC_MSG_WRPPAR, stderr);
            return EXIT_FAILURE;
         }
         const char* wRC_pVal = argv[wRC_iParColl[i].wRC_idxPar] + wRC_iParColl[i].wRC_posVal;
         switch (i) {
            case     wRC_help: wRC_usage();
                               return EXIT_SUCCESS;
            case     wRC_ipv4: if (!wRC_chkIPv4(wRC_lenVal + 1, wRC_pVal))
                                  return EXIT_FAILURE;
                               memcpy(wRC_strIPv4, wRC_pVal, wRC_lenVal);
                               wRC_szStrIPv4 = wRC_lenVal + 1;
                               break;
            case     wRC_port: if (strspn(wRC_pVal, "0123456789") != wRC_lenVal ||
                                   strtoul(wRC_pVal, 0, 10) > 65535) {
                                 fputs(WRC_MSG_WRPPAR, stderr);
                                 return EXIT_FAILURE;
                               }
                               memcpy(wRC_strPort, wRC_pVal, wRC_lenVal);
                               wRC_szStrPort = wRC_lenVal + 1;
                               break;
            case      wRC_beh: if (!strcmp(wRC_pVal, WRC_ITER))
                                  wRC_fBeh = true;
                               else if (strcmp(wRC_pVal, WRC_SINGLE)) {
                                  fputs(WRC_MSG_WRPPAR, stderr);
                                  return EXIT_FAILURE;
                               }
                               break;
            case wRc_mnemCode: if (wRC_lenVal > P_CST_MAXLEN_MNEMCD) {
                                  fputs(WRC_MSG_WRPPAR, stderr);
                                  return EXIT_FAILURE;
                               }
                               memcpy(wRC_strMnemCd, wRC_pVal, wRC_lenVal);
                               break;
            case    wRC_model: if (!strcmp(wRC_pVal, WRC_KMTRONIC))
                                  wRC_hwModel = r_kmTronic;
                               else if (!strcmp(wRC_pVal, WRC_NC800))
                                  wRC_hwModel = r_nc800;
                               else {
                                  fputs(WRC_MSG_WRPPAR, stderr);
                                  return EXIT_FAILURE;
                               }
         }
      }
   }
   if ((wRC_fBeh &&
        wRC_strMnemCd[0]) ||
       (!wRC_fBeh &&
        !wRC_strMnemCd[0])) {
      fputs(WRC_MSG_WRPPAR, stderr);
      return EXIT_FAILURE;
   }
   if (wRC_hwModel == r_nc800 &&
       !(*wRC_strPort)) {
      fputs(WRC_MSG_WRPPAR, stderr);
      return EXIT_FAILURE;
   }
   if (curl_global_init(CURL_GLOBAL_NOTHING)) {
      fputs(WRC_MSG_UNSCINIT, stderr);
      return EXIT_FAILURE;
   }
   wRC_supProt_t wRC_protInd = WRC_PROT_NONE;
   curl_version_info_data* wRC_pCurlInfo = curl_version_info(CURLVERSION_NOW);
   const char* const* wRC_pStrArr_prot = wRC_pCurlInfo -> protocols;
   while (*wRC_pStrArr_prot) {
      if (!strcmp(*wRC_pStrArr_prot, "http"))
         wRC_protInd |= WRC_PROT_HTTP;
      else if (!strcmp(*wRC_pStrArr_prot, "https"))
         wRC_protInd |= WRC_PROT_HTTPS;
      else if (wRC_protInd == WRC_PROT_VALID)
         break;
      wRC_pStrArr_prot++;
   }
   if (wRC_protInd == WRC_PROT_VALID) {
      if (!wRC_fBeh)
         rC_doSingleOperation(wRC_szStrIPv4, wRC_strIPv4,
                              wRC_szStrPort, wRC_strPort,
                              wRC_strMnemCd,
                              wRC_hwModel);
      else
         rC_doMultipleOperations(wRC_szStrIPv4, wRC_strIPv4,
                                 wRC_szStrPort, wRC_strPort,
                                 wRC_hwModel);
   }
   else
      fputs(WRC_MSG_HLPROT, stderr);
   curl_global_cleanup();
   return EXIT_SUCCESS;
}
