#ifndef FX_ANIM_H
#define FX_ANIM_H

#include "types.h"
#include "video.h"

#define ANIM_WIDTH 256
#define ANIM_HEIGHT 200
#define ANIM_OFFSET_X 32

#define FP_BITS 12

#define INT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)

struct Point2D
{
	int x, y;
};

struct QuadStore
{
	Point2D p0, p1, p2, p3;
	uint8 c;
};

struct vec2i
{
	int x, y;
};

void fxAnimInit(vmode *vm);
void fxAnimRun(vmode *vm, uint32 fxFrame);

#endif
