/**************************************/
/* Author: Pavlo Nykolyn              */
/* Last modification date: 23/01/2023 */
/**************************************/

#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

/**
 * \file
 * \author Pavlo Nykolyn
 * a collection of macros that define a internal statuses of relays
 */

typedef unsigned char r_stat;

enum r_mCodes {r_kmTronic,  /**< KMTronic web relay */
               r_nc800,     /**< NC800 */
               r_numMod     /**< number of supported models */
              };

// eight relays
// ON statuses
#define R_ON_1     0x10
#define R_ON_2     0x20
#define R_ON_3     0x40
#define R_ON_4     0x80
#define R_ON_5     0x01
#define R_ON_6     0x02
#define R_ON_7     0x04
#define R_ON_8     0x08
#define R_DEF      0x00  // DEFAULT state: All relays are off
// only for NC800
#define R_1R_MASK  0xF0  // mask for the relays positioned on the first row
#define R_2R_MASK  0x0F  // mask for the relays positioned on the second row
// messages
#define R_ON_1_MSG   "** Relay 1 on **\n"
#define R_OFF_1_MSG  "** Relay 1 off **\n"
#define R_ON_2_MSG   "** Relay 2 on **\n"
#define R_OFF_2_MSG  "** Relay 2 off **\n"
#define R_ON_3_MSG   "** Relay 3 on **\n"
#define R_OFF_3_MSG  "** Relay 3 off **\n"
#define R_ON_4_MSG   "** Relay 4 on **\n"
#define R_OFF_4_MSG  "** Relay 4 off **\n"
#define R_ON_5_MSG   "** Relay 5 on **\n"
#define R_OFF_5_MSG  "** Relay 5 off **\n"
#define R_ON_6_MSG   "** Relay 6 on **\n"
#define R_OFF_6_MSG  "** Relay 6 off **\n"
#define R_ON_7_MSG   "** Relay 7 on **\n"
#define R_OFF_7_MSG  "** Relay 7 off **\n"
#define R_ON_8_MSG   "** Relay 8 on **\n"
#define R_OFF_8_MSG  "** Relay 8 off **\n"

#endif // STATUS_H_INCLUDED
