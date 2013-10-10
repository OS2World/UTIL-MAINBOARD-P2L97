/* LM78 voltage parameters for the Asus P2L97 mainboard.
 *
 * These parameters are based on
 *  - the LM78 datasheet,
 *  - similar programs for other platforms,
 *  - but mostly on testing and comparing to the BIOS values on my P2L97-S.
 *
 * Let me know if you have exact information/docs on this subject.
 *
 *   bjorn@mork.no
 */
#ifndef P2L97_H
#define P2L97_H
#include "lm78.h"

/* P2L97 voltages */
#define P2L97_VCORE ((float)LM78_READ(LM78_IN0)     * 0.016)
#define P2L97_3_3V  ((float)LM78_READ(LM78_IN2)     * 0.016)
#define P2L97_5V    ((float)LM78_READ(LM78_IN3)     * 0.0265)
#define P2L97_12V   ((float)LM78_READ(LM78_IN4)     * 0.061)
#define P2L97_M12V  ((float)LM78_READ(LM78_IN5_NEG) * -0.0564)
#define P2L97_M5V   ((float)LM78_READ(LM78_IN6_NEG) * -0.0242)

#endif /* P2L97_H */
