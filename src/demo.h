#ifndef DEMO_H
#define DEMO_H

#include "types.h"

enum {  PART_PLASMA,
        PART_ROTOZOOMER,
        PART_GRID4C,
        NUM_PARTS };

extern uint8 *VGA;
extern uint16 *my_clock;

#define UNROLL_2(X) X X
#define UNROLL_4(X) UNROLL_2(X) UNROLL_2(X)
#define UNROLL_8(X) UNROLL_4(X) UNROLL_4(X)
#define UNROLL_16(X) UNROLL_8(X) UNROLL_8(X)
#define UNROLL_32(X) UNROLL_16(X) UNROLL_16(X)
#define UNROLL_64(X) UNROLL_32(X) UNROLL_32(X)
#define UNROLL_160(X) UNROLL_64(X) UNROLL_64(X) UNROLL_32(X)


#endif
