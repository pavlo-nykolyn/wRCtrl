/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 21/05/2023 */
/**************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "ctrl.h"
#include "parser.h"
#include "constants.h"
#include "err_wrapper.h"

#define RC_MAXNUMCHS  1500L  // maximum number of characters that will be downloaded by curl
#define RC_SZCOMM_KMT    6U  // length of a KMTronic web relay command
#define RC_SZCOMM_NC8    2U  // length of a NC800 web relay command

#define RC_CURLERRCODE(rC_curlCode)  fprintf(stderr, "[NOT] A curl service returned error code: %d\n", rC_curlCode + 0)

// the user-defined data CURLOPT_WRITEDATA
typedef struct rC_html_buf {
// effective size of the buffer
   size_t rC_szBuf;
// HAS TO BE ASSIGNED TO A VALID ARRAY OF RC_MAXNUMCHS + 1 CHARACTERS BEFORE ANY ATTEMPT TO READ ANYTHING
  char* rC_buf;
} rC_html_buf;

// the call-back CURLOPT_WRITEFUNCTION
static size_t rC_dl(char* rC_currBuf,
                    size_t rC_chSz, // byte size of each element of rC_currBuf (HAS TO BE ONE)
                    size_t rC_currSzBuf,
                    void* rC_uD)
{
   if (rC_chSz != 1)
      return ~rC_currSzBuf;
   rC_html_buf* rC_pBuf = (rC_html_buf*) rC_uD;
   if ((rC_pBuf -> rC_szBuf) + rC_currSzBuf > RC_MAXNUMCHS)
      return ~rC_currSzBuf;
   memcpy(((rC_pBuf -> rC_buf) + (rC_pBuf -> rC_szBuf)), rC_currBuf, rC_currSzBuf);
   rC_pBuf -> rC_szBuf += rC_currSzBuf;
   return rC_currSzBuf;
}

// prints on stdout the status of each relay
// WRAPPER
static void rC_viewStat(const r_stat rC_stat,
                        enum r_mCodes);
static void rC_viewStat_KMTronic(const r_stat rC_stat);
static void rC_viewStat_NC800(const r_stat rC_stat);

// the first column holds the off commands for each relay. The second column
// holds the on commands
// commands for the KMTronic web relay
static const char* rC_KMTronic_comm[8][2] = {{"FF0100", "FF0101"},
                                             {"FF0200", "FF0201"},
                                             {"FF0300", "FF0301"},
                                             {"FF0400", "FF0401"},
                                             {"FF0500", "FF0501"},
                                             {"FF0600", "FF0601"},
                                             {"FF0700", "FF0701"},
                                             {"FF0800", "FF0801"}};
// commands for the NC800 web relay
static const char* rC_nc800_comm[8][2] = {{"00", "01"},
                                          {"02", "03"},
                                          {"04", "05"},
                                          {"06", "07"},
                                          {"08", "09"},
                                          {"10", "11"},
                                          {"12", "13"},
                                          {"14", "15"}};

int rC_doSingleOperation(size_t rC_szStr_IPv4, const char* const rC_str_IPv4,
                         size_t rC_szStr_port, const char* const rC_str_port,
                         const char rC_strMnemCd[static P_CST_MAXSZSTR_MNEMCD],
                         enum r_mCodes rC_hwMod)
{
   int rC_errCode = wRC_Cd_noError;
   CURL* rC_pHan = CST_PVOID;
   if (((!rC_szStr_IPv4 && rC_str_IPv4) ||
        (rC_szStr_IPv4 && !rC_str_IPv4)) ||
       ((!rC_szStr_port && rC_str_port) ||
        (rC_szStr_port && !rC_str_port))) {
      fputs(WRC_MSG_INCCHARR, stderr);
      rC_errCode = wRC_Cd_incChArr;
      goto RC_SINOP_EXIT;
   }
   P_out rC_comm = {.p_oAct = oAct_numOAct};
   rC_errCode = P_parseMnemCode(&rC_comm,
                                rC_strMnemCd);
   if (rC_errCode)
      goto RC_SINOP_EXIT;
   else if (rC_comm.p_oAct != oAct_quit) {
      // creating the URL
      unsigned rC_szStrUrl;
      switch (rC_hwMod) {
         case r_kmTronic: rC_szStrUrl = rC_szStr_IPv4 + RC_SZCOMM_KMT + 1;
                          break;
         case r_nc800:    rC_szStrUrl = rC_szStr_IPv4 + rC_szStr_port + RC_SZCOMM_NC8 + 1;
                          break;
         default:         rC_errCode = wRC_Cd_invP;
                          fputs(WRC_MSG_INVPAR, stderr);
                          goto RC_SINOP_EXIT;
      }
      char rC_strUrl[rC_szStrUrl];
      rC_strUrl[rC_szStr_IPv4 - 1] = '/';
      rC_strUrl[rC_szStrUrl - 1] = '\0';
      memcpy(rC_strUrl, rC_str_IPv4, rC_szStr_IPv4 - 1);
      // different URL components for different hardware models
      switch (rC_hwMod) {
         case r_kmTronic: memcpy(rC_strUrl + rC_szStr_IPv4, rC_KMTronic_comm[rC_comm.p_rID][rC_comm.p_fAct ? 1
                                                                                                           : 0], RC_SZCOMM_KMT);
                          break;
         case r_nc800:    memcpy(rC_strUrl + rC_szStr_IPv4, rC_str_port, rC_szStr_port - 1);
                          *(rC_strUrl + rC_szStr_IPv4 + rC_szStr_port - 1) = '/';
                          memcpy(rC_strUrl + rC_szStr_IPv4 + rC_szStr_port, rC_nc800_comm[rC_comm.p_rID][rC_comm.p_fAct ? 1
                                                                                                                        : 0], RC_SZCOMM_NC8);
                          break;
         default:         ; // suppresses a needless warning
      }
      rC_pHan = curl_easy_init();
      if (!rC_pHan) {
         fputs(WRC_MSG_UNSCEH, stderr);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      CURLcode rC_libCode = curl_easy_setopt(rC_pHan,
                                             CURLOPT_PROTOCOLS,
                                             CURLPROTO_HTTP | CURLPROTO_HTTPS);
      if (rC_libCode) {
         RC_CURLERRCODE(rC_libCode);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      rC_libCode = curl_easy_setopt(rC_pHan,
                                    CURLOPT_WRITEFUNCTION,
                                    rC_dl);
      if (rC_libCode) {
         RC_CURLERRCODE(rC_libCode);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      char rC_htmlBuf[RC_MAXNUMCHS + 1] = {0};
      rC_html_buf rC_dlData = {.rC_buf = rC_htmlBuf};
      rC_libCode = curl_easy_setopt(rC_pHan,
                                    CURLOPT_WRITEDATA,
                                    (void*) &rC_dlData);
      if (rC_libCode) {
         RC_CURLERRCODE(rC_libCode);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      rC_libCode = curl_easy_setopt(rC_pHan,
                                    CURLOPT_URL,
                                    rC_strUrl);
      if (rC_libCode) {
         RC_CURLERRCODE(rC_libCode);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      rC_libCode = curl_easy_perform(rC_pHan);
      if (rC_libCode) {
         RC_CURLERRCODE(rC_libCode);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      long rC_resCode = 200;
      rC_libCode = curl_easy_getinfo(rC_pHan,
                                    CURLINFO_RESPONSE_CODE,
                                    &rC_resCode);
      if (rC_libCode) {
         RC_CURLERRCODE(rC_libCode);
         rC_errCode = wRC_Cd_curl;
         goto RC_SINOP_EXIT;
      }
      if (rC_resCode != 200)
         fprintf(stdout, "[NOT] The last request yielded response code %ld\n", rC_resCode);
      else {
         rC_dlData.rC_szBuf++;
         const r_stat rC_stat = P_parseHtmlResp(rC_dlData.rC_szBuf, rC_dlData.rC_buf,
                                                rC_hwMod);
         rC_viewStat(rC_stat,
                     rC_hwMod);
      }
   }
   RC_SINOP_EXIT:
   curl_easy_cleanup(rC_pHan);
   return rC_errCode;
}

int rC_doMultipleOperations(size_t rC_szStr_IPv4, const char* const rC_str_IPv4,
                            size_t rC_szStr_port, const char* const rC_str_port,
                            enum r_mCodes rC_hwMod)
{
   int rC_errCode = wRC_Cd_noError;
   CURL* rC_pHan = CST_PVOID;
   char* rC_strUrl = CST_PVOID;
   if (((!rC_szStr_IPv4 && rC_str_IPv4) ||
        (rC_szStr_IPv4 && !rC_str_IPv4)) ||
       ((!rC_szStr_port && rC_str_port) ||
        (rC_szStr_port && !rC_str_port))) {
      fputs(WRC_MSG_INCCHARR, stderr);
      rC_errCode = wRC_Cd_incChArr;
      goto RC_MULTOP_EXIT;
   }
   P_out rC_comm = {0};
   // creating the URL
   unsigned rC_szStrUrl;
   switch (rC_hwMod) {
      case r_kmTronic: rC_szStrUrl = rC_szStr_IPv4 + RC_SZCOMM_KMT + 1;
                       break;
      case r_nc800:    rC_szStrUrl = rC_szStr_IPv4 + rC_szStr_port + RC_SZCOMM_NC8 + 1;
                       break;
      default:         rC_errCode = wRC_Cd_invP;
                       fputs(WRC_MSG_INVPAR, stderr);
                       goto RC_MULTOP_EXIT;
   }
   rC_strUrl = calloc(rC_szStrUrl, sizeof(char));
   if (!rC_strUrl) {
      fputs(WRC_MSG_HEAPMANFAIL, stderr);
      rC_errCode = wRC_Cd_heapManFail;
      goto RC_MULTOP_EXIT;
   }
   *(rC_strUrl + rC_szStr_IPv4 - 1) = '/';
   memcpy(rC_strUrl, rC_str_IPv4, rC_szStr_IPv4 - 1);
   switch (rC_hwMod) {
      case r_kmTronic: break;
      case r_nc800:    memcpy(rC_strUrl + rC_szStr_IPv4, rC_str_port, rC_szStr_port - 1);
                       *(rC_strUrl + rC_szStr_IPv4 + rC_szStr_port - 1) = '/';
                       break;
      default:         ; // suppresses a needless warning
   }
   rC_pHan = curl_easy_init();
   if (!rC_pHan) {
      fputs(WRC_MSG_UNSCEH, stderr);
      rC_errCode = wRC_Cd_curl;
      goto RC_MULTOP_EXIT;
   }
   CURLcode rC_libCode = curl_easy_setopt(rC_pHan,
                                          CURLOPT_PROTOCOLS,
                                          CURLPROTO_HTTP | CURLPROTO_HTTPS);
   if (rC_libCode) {
      RC_CURLERRCODE(rC_libCode);
      rC_errCode = wRC_Cd_curl;
      goto RC_MULTOP_EXIT;
   }
   rC_libCode = curl_easy_setopt(rC_pHan,
                                 CURLOPT_WRITEFUNCTION,
                                 rC_dl);
   if (rC_libCode) {
      RC_CURLERRCODE(rC_libCode);
      rC_errCode = wRC_Cd_curl;
      goto RC_MULTOP_EXIT;
   }
   char rC_htmlBuf[RC_MAXNUMCHS + 1] = {0};
   rC_html_buf rC_dlData = {.rC_buf = rC_htmlBuf};
   rC_libCode = curl_easy_setopt(rC_pHan,
                                 CURLOPT_WRITEDATA,
                                 (void*) &rC_dlData);
   if (rC_libCode) {
      RC_CURLERRCODE(rC_libCode);
      rC_errCode = wRC_Cd_curl;
      goto RC_MULTOP_EXIT;
   }
   fputs("** Author: Pavlo Nykolyn **\n\
** Powered by curl **\n\
                  ______    _____  _                   _\n\
                 /      |  /  ___|| |                 | |\n\
                |   _   | /  /    | |        _        | |\n\
__            __|  |_|  ||  |     | |______ | |______ | |\n\
\\ \\    __    / /|       ||  |     |  ______||  ______|| |\n\
 \\ \\  /  \\  / / |  ___  ||  |     | |       | |       | |\n\
  \\ \\/ /\\ \\/ /  | |   \\ \\ \\  \\___ | |______ | |       | |\n\
   \\__/  \\__/   |_|    \\_\\ \\_____| \\_______||_|       |_|\n", stdout);
   do {
      // obtaining a valid command
      do {
         rC_comm.p_oAct = oAct_numOAct;
         fputs("> ", stdout);
         rC_errCode = P_parseInput(&rC_comm);
         if (rC_errCode) {
            memset((void*) &rC_comm, 0, sizeof(P_out));
            if (rC_errCode != wRC_Cd_wrI)
               goto RC_MULTOP_EXIT;
         }
      } while (rC_errCode == wRC_Cd_wrI);
      if (rC_comm.p_oAct == oAct_numOAct) {
         // adding the command to the URL
         switch (rC_hwMod) {
            case r_kmTronic: memcpy(rC_strUrl + rC_szStr_IPv4, rC_KMTronic_comm[rC_comm.p_rID][rC_comm.p_fAct ? 1
                                                                                                              : 0], RC_SZCOMM_KMT);
                             break;
            case r_nc800:    memcpy(rC_strUrl + rC_szStr_IPv4 + rC_szStr_port, rC_nc800_comm[rC_comm.p_rID][rC_comm.p_fAct ? 1
                                                                                                                           : 0], RC_SZCOMM_NC8);
                             break;
            default:         ; // suppresses a needless warning
         }
         // performing the operation
         rC_libCode = curl_easy_setopt(rC_pHan,
                                       CURLOPT_URL,
                                       rC_strUrl);
         if (rC_libCode) {
            RC_CURLERRCODE(rC_libCode);
            rC_errCode = wRC_Cd_curl;
            goto RC_MULTOP_EXIT;
         }
         rC_libCode = curl_easy_perform(rC_pHan);
         if (rC_libCode) {
            RC_CURLERRCODE(rC_libCode);
            rC_errCode = wRC_Cd_curl;
            goto RC_MULTOP_EXIT;
         }
         long rC_resCode = 200;
         rC_libCode = curl_easy_getinfo(rC_pHan,
                                       CURLINFO_RESPONSE_CODE,
                                       &rC_resCode);
         if (rC_libCode) {
            RC_CURLERRCODE(rC_libCode);
            rC_errCode = wRC_Cd_curl;
            goto RC_MULTOP_EXIT;
         }
         if (rC_resCode != 200)
            fprintf(stdout, "[NOT] The last request yielded response code %ld\n", rC_resCode);
         else {
            rC_dlData.rC_szBuf++;
            const r_stat rC_stat = P_parseHtmlResp(rC_dlData.rC_szBuf, rC_dlData.rC_buf,
                                                   rC_hwMod);
            rC_viewStat(rC_stat,
                        rC_hwMod);
         }
         // resetting the shared variables
         memset(rC_htmlBuf, 0, rC_dlData.rC_szBuf);
         rC_dlData.rC_szBuf = 0;
         rC_comm.p_fAct = false;
         rC_comm.p_rID = 0;
      }
   } while (rC_comm.p_oAct != oAct_quit);
   RC_MULTOP_EXIT:
   curl_easy_cleanup(rC_pHan);
   free(rC_strUrl);
   rC_strUrl = CST_PVOID;
   return rC_errCode;
}

static void rC_viewStat(const r_stat rC_stat,
                        enum r_mCodes rC_hwMod)
{
   switch (rC_hwMod) {
      case r_kmTronic: rC_viewStat_KMTronic(rC_stat);
                       break;
      case r_nc800:    rC_viewStat_NC800(rC_stat);
                       break;
      default:         ; // suppresses a needless warning
   }
}

static void rC_viewStat_KMTronic(const r_stat rC_stat)
{
   fputs((rC_stat & R_ON_1) ? R_ON_1_MSG
                            : R_OFF_1_MSG, stdout);
   fputs((rC_stat & R_ON_2) ? R_ON_2_MSG
                            : R_OFF_2_MSG, stdout);
   fputs((rC_stat & R_ON_3) ? R_ON_3_MSG
                            : R_OFF_3_MSG, stdout);
   fputs((rC_stat & R_ON_4) ? R_ON_4_MSG
                            : R_OFF_4_MSG, stdout);
   fputs((rC_stat & R_ON_5) ? R_ON_5_MSG
                            : R_OFF_5_MSG, stdout);
   fputs((rC_stat & R_ON_6) ? R_ON_6_MSG
                            : R_OFF_6_MSG, stdout);
   fputs((rC_stat & R_ON_7) ? R_ON_7_MSG
                            : R_OFF_7_MSG, stdout);
   fputs((rC_stat & R_ON_8) ? R_ON_8_MSG
                            : R_OFF_8_MSG, stdout);
}

static void rC_viewStat_NC800(const r_stat rC_stat)
{
   if (rC_stat & R_1R_MASK) {
      fputs((rC_stat & R_ON_1) ? R_ON_1_MSG
                               : R_OFF_1_MSG, stdout);
      fputs((rC_stat & R_ON_2) ? R_ON_2_MSG
                               : R_OFF_2_MSG, stdout);
      fputs((rC_stat & R_ON_3) ? R_ON_3_MSG
                               : R_OFF_3_MSG, stdout);
      fputs((rC_stat & R_ON_4) ? R_ON_4_MSG
                               : R_OFF_4_MSG, stdout);
   }
   else if (rC_stat & R_2R_MASK) {
      fputs((rC_stat & R_ON_5) ? R_ON_5_MSG
                               : R_OFF_5_MSG, stdout);
      fputs((rC_stat & R_ON_6) ? R_ON_6_MSG
                               : R_OFF_6_MSG, stdout);
      fputs((rC_stat & R_ON_7) ? R_ON_7_MSG
                               : R_OFF_7_MSG, stdout);
      fputs((rC_stat & R_ON_8) ? R_ON_8_MSG
                               : R_OFF_8_MSG, stdout);
   }
}
