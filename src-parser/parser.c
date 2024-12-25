/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 21/05/2023 */
/**************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "err_wrapper.h"

#define PARSE_LINELEN  101U  // length of an input line

// a dictionary of words recognized by the parser
// each line of the dictionary is associated with a word position in the input line
static const char* p_dict[2][2] = {{"turn", "quit"},
                                   {"on", "off"}};
// templates of mnemonic codes. The # character indicates a relay-ID
static const char* p_templ[2] = {"t_on_#",
                                 "t_off_#"};

enum p_fstWords {p_turn,  /**< turn */
                 p_quit,  /**< quit */
                 p_numFW  /**< number of words that occupy the first position */
                };

enum p_secWords {p_on,    /**< on */
                 p_off,   /**< off */
                 p_numSW  /**< number of words that occupy the second position */
                };

static enum p_fstWords p_checkFstW(const char* const p_pWord)
{
   if (!strcmp(p_pWord, p_dict[0][0]))
      return p_turn;
   else if (!strcmp(p_pWord, p_dict[0][1]))
      return p_quit;
   return p_numFW;
}

static enum p_fstWords p_checkSecW(const char* const p_pWord)
{
   if (!strcmp(p_pWord, p_dict[1][0]))
      return p_on;
   else if (!strcmp(p_pWord, p_dict[1][1]))
      return p_off;
   return p_numSW;
}
// tokenizes the input line and returns the number of words
// ANY SPACE CHARACTER IS CONVERTED INTO A NULL CHARACTERS
static unsigned p_tokenize(const unsigned szILine, char* const iLine);
// checks if a position contains a valid word and updates the internal data
// accordingly
// returns one of the following error codes:
// - wRC_Cd_noError;
// - wRC_Cd_wrI
static int p_checkWord(unsigned p_wPos,
                       const char* const p_pWord,
                       P_out* restrict p_pIntData);
// HTML RESPONSE PARSING
static r_stat p_parseKMTronicResp(size_t p_szStrResp, const char* const p_strResp);
static r_stat p_parseNC800Resp(size_t p_szStrResp, const char* const p_strResp);
// status extraction
static r_stat p_extrStat_KMTronic(unsigned p_szTarg, const char* const p_targ); // target string
static r_stat p_extrStat_NC800(const char p_chID, // relay identifier (an Arab digit)
                               r_stat p_rStat);

int P_parseInput(P_out* restrict p_pIntData)
{
   int p_errCode = wRC_Cd_noError;
   if (!p_pIntData) {
      fputs(WRC_MSG_INVPAR, stderr);
      p_errCode = wRC_Cd_invP;
      goto P_PARSEIN_EXIT;
   }
   char line[PARSE_LINELEN] = {0};
   unsigned numCh = 0;
   char p_ch = getchar();
   while (numCh < PARSE_LINELEN - 1 &&
          p_ch != '\n') {
      // multiple spaces are coalesced into a single space and leading spaces are ignored
      if (!isspace(p_ch) ||
          (numCh &&
           !isspace(line[numCh - 1]) &&
           isspace(p_ch))) {
         line[numCh] = p_ch;
         numCh++;
      }
      p_ch = getchar();
   }
   if (numCh == PARSE_LINELEN - 1 &&
       p_ch != '\n') {
      while (getchar() != '\n') ; // discarding the un-read characters
      fputs("[NOT] the internal buffer length has been reached. Every un-read character will be discarded\n", stderr);
   }
   if (!isspace(line[numCh - 1])) {
      line[numCh] = ' '; // needed for "tokenization"
      numCh++;
   }
   if (numCh) {
      unsigned p_numW = p_tokenize(numCh, line);
      unsigned p_off = 0; // an offset used to "select" the first character of a word that resulted from "tokenization"
      for (unsigned i = 0; i < p_numW; i++) {
         p_errCode = p_checkWord(i,
                                 line + p_off,
                                 p_pIntData);
         if (p_errCode == wRC_Cd_wrI ||
             (i &&
             (p_pIntData -> p_oAct == oAct_quit))) {
            fputs(WRC_MSG_WRUSRI, stderr);
            goto P_PARSEIN_EXIT;
         }
         p_off += strlen(line + p_off) + 1;
      }
   }
   P_PARSEIN_EXIT:
   return p_errCode;
}

int P_parseMnemCode(P_out* restrict p_pIntData,
                    const char p_strMnemCd[static P_CST_MAXSZSTR_MNEMCD])
{
   int p_errCode = wRC_Cd_noError;
   if (!p_pIntData) {
      fputs(WRC_MSG_INVPAR, stderr);
      p_errCode = wRC_Cd_invP;
      goto P_PARSEMNEMCODE_EXIT;
   }
   unsigned p_strLen = P_CST_MAXLEN_MNEMCD - 1;
   const char* p_pStrTempl = p_templ[1];
   if (!p_strMnemCd[p_strLen]) {
      p_strLen -= 1;
      p_pStrTempl = p_templ[0];
      p_pIntData -> p_fAct = true; // assuming that the user-inserted parameter is indeed a "turn on" indication
   }
   if (memcmp(p_strMnemCd, p_pStrTempl, p_strLen)) {
      fputs(WRC_MSG_WRUSRI, stderr);
      p_errCode = wRC_Cd_wrI;
      goto P_PARSEMNEMCODE_EXIT;
   }
   if (isdigit(p_strMnemCd[p_strLen]) &&
       p_strMnemCd[p_strLen] != '0' && p_strMnemCd[p_strLen] != '9')
      p_pIntData -> p_rID = p_strMnemCd[p_strLen] - '1';
   else {
      fputs(WRC_MSG_WRUSRI, stderr);
      p_errCode = wRC_Cd_wrI;
   }
   P_PARSEMNEMCODE_EXIT:
   return p_errCode;
}

r_stat P_parseHtmlResp(size_t p_szStrResp, const char* const p_strResp,
                       const enum r_mCodes p_hwMod)
{
   switch (p_hwMod) {
      case r_kmTronic: return p_parseKMTronicResp(p_szStrResp, p_strResp);
      case r_nc800: return p_parseNC800Resp(p_szStrResp, p_strResp);
      default: ; // suppresses a needless warning
   }
   return R_DEF;
}

static r_stat p_parseKMTronicResp(size_t p_szStrResp, const char* const p_strResp)
{
   size_t p_currPos = 0;
   while (p_currPos < p_szStrResp &&
          *(p_strResp + p_currPos)) {
      size_t p_lineLen = strcspn(p_strResp + p_currPos, "\n"); // assuming that each line is terminated with a new-line character
      if (p_lineLen > 6 &&
          !strncmp(p_strResp +  p_currPos, "Status", 6)) {
         char p_statLine[p_lineLen + 1];
         p_statLine[p_lineLen] = '\0';
         memcpy(p_statLine, p_strResp + p_currPos, p_lineLen);
         return p_extrStat_KMTronic(p_lineLen + 1, p_statLine);
      }
      p_currPos += p_lineLen + 1;
   }
   return R_DEF;
}

static r_stat p_parseNC800Resp(size_t p_szStrResp, const char* const p_strResp)
{
   r_stat p_rStat = R_DEF;
   size_t p_currPos = 0;
   while (p_currPos < p_szStrResp &&
          *(p_strResp + p_currPos)) {
      if (*(p_strResp + p_currPos) == 'R' &&
          !strncmp(p_strResp + p_currPos, "Relay-0", 7)) {
         size_t p_hopLen = p_currPos + 7;
         const char p_rID = *(p_strResp + p_hopLen);
         // now p_hopLen refers to the first position of the font color identifier
         p_hopLen += strcspn(p_strResp + p_hopLen + 1, "#") + 2;
         if (strncmp(p_strResp + p_hopLen, "FF0000", 6))
            p_rStat = p_extrStat_NC800(p_rID,
                                       p_rStat);
         p_currPos = p_hopLen + 6;
         continue;
      }
      p_currPos++;
   }
   return p_rStat;
}

static r_stat p_extrStat_KMTronic(unsigned p_szTarg, const char* const p_targ)
{
   r_stat p_rStat = R_DEF;
   unsigned p_currR = 0; // current relay
   for (size_t i = 7; i < p_szTarg; i++) {
      if (*(p_targ + i) == '1') {
         switch (p_currR) {
            case 0: p_rStat |= R_ON_1;
                    break;
            case 1: p_rStat |= R_ON_2;
                    break;
            case 2: p_rStat |= R_ON_3;
                    break;
            case 3: p_rStat |= R_ON_4;
                    break;
            case 4: p_rStat |= R_ON_5;
                    break;
            case 5: p_rStat |= R_ON_6;
                    break;
            case 6: p_rStat |= R_ON_7;
                    break;
            case 7: p_rStat |= R_ON_8;
                    break;
         }
         p_currR++;
      }
      if (*(p_targ + i) == '0')
         p_currR++;
   }
   return p_rStat;
}

static r_stat p_extrStat_NC800(const char p_chID,
                               r_stat p_rStat)
{
   switch (p_chID) {
      case '1': p_rStat |= R_ON_1;
                break;
      case '2': p_rStat |= R_ON_2;
                break;
      case '3': p_rStat |= R_ON_3;
                break;
      case '4': p_rStat |= R_ON_4;
                break;
      case '5': p_rStat |= R_ON_5;
                break;
      case '6': p_rStat |= R_ON_6;
                break;
      case '7': p_rStat |= R_ON_7;
                break;
      case '8': p_rStat |= R_ON_8;
   }
   return p_rStat;
}

static unsigned p_tokenize(const unsigned szILine, char* const iLine)
{
   unsigned p_numW = 0;
   for (size_t i = 0; i < szILine; i++) {
      if (isspace(*(iLine + i))) {
         *(iLine + i) = '\0';
         p_numW++;
      }
   }
   return p_numW;
}

static int p_checkWord(unsigned p_wPos,
                       const char* const p_pWord,
                       P_out* restrict p_pIntData)
{
   int p_errCode = wRC_Cd_noError;
   switch (p_wPos) {
      case 0:  switch (p_checkFstW(p_pWord)) {
                  case p_turn: break;
                  case p_quit: p_pIntData -> p_oAct = oAct_quit;
                               break;
                  default: p_errCode = wRC_Cd_wrI;
               }
               break;
      case 1:  switch (p_checkSecW(p_pWord)) {
                  case p_on:  p_pIntData -> p_fAct = true;
                              break;
                  case p_off: break;
                  default: p_errCode = wRC_Cd_wrI;
               }
               break;
      case 2:  if (strlen(p_pWord) == 1 &&
                   isdigit(*p_pWord) &&
                   *p_pWord != '0' && *p_pWord != '9')
                  p_pIntData -> p_rID = *p_pWord - '1';
               else
                  p_errCode = wRC_Cd_wrI;
               break;
      default: p_errCode = wRC_Cd_wrI;
   }
   return p_errCode;
}
