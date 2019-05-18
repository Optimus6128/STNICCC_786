#ifndef MAIN_H
#define MAIN_H

enum {PART_CLEAR, PART_START, PART_PLASMA, PART_GALAXY0, PART_GALAXY1, PART_ASTRONAUT, PART_CREDITS, PART_GROUPS, PART_END, NUM_PARTS};

#define ubyte unsigned char
#define uint32 unsigned int

#define UNROLL_2(X) X X
#define UNROLL_4(X) UNROLL_2(X) UNROLL_2(X)
#define UNROLL_8(X) UNROLL_4(X) UNROLL_4(X)
#define UNROLL_16(X) UNROLL_8(X) UNROLL_8(X)
#define UNROLL_64(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X)
#define UNROLL_80(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X)
#define UNROLL_78(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X) UNROLL_16(X) UNROLL_8(X) UNROLL_4(X) UNROLL_2(X)
#define UNROLL_256(X) UNROLL_64(X) UNROLL_64(X) UNROLL_64(X) UNROLL_64(X)

#define PI 3.14159265359f


#define MUSIC_ON

extern bool quit;

extern unsigned int oddEvenFrame;

extern unsigned int partTime;
extern unsigned int partFrame;
extern int refreshes;

void switchPart(int part);

void printMusicInfo();

#endif
