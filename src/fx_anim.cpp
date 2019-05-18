#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fx_anim.h"
#include "video.h"
#include "gfxtools.h"
#include "scene1.h"

static int frameNum = 0;
static uint32 block64index = 0;

static uint8 *data = scene1_bin;

static Point2D pt[16];

static QuadStore quads[1024];
static QuadStore *quadPtr;
static int numQuads = 0;

static bool mustClearScreen = false;
static bool endOfAllFrames = false;

static int *leftEdgeFlat;
static int *rightEdgeFlat;


void fxAnimInit(vmode *vm)
{
	makeAndSetPal(0, 255, 0, 0, 0, 0, 0, 0);

	leftEdgeFlat = (int*)malloc(vm->height * sizeof(int));
	rightEdgeFlat = (int*)malloc(vm->height * sizeof(int));
}

// ======== Specialized functions for simpler flat quad rasterizer ========

static inline void prepareEdgeListFlat(Point2D *p0, Point2D *p1, vmode *vm)
{
	if (p0->y == p1->y) return;

	// Assumes CCW
	int *edgeListToWriteFlat;
	if (p0->y < p1->y) {
		edgeListToWriteFlat = leftEdgeFlat;
	}
	else {
		edgeListToWriteFlat = rightEdgeFlat;

		Point2D *pTemp = p0;
		p0 = p1;
		p1 = pTemp;
	}

	const int x0 = p0->x; const int y0 = p0->y;
	const int x1 = p1->x; const int y1 = p1->y;

	const int screenHeight = (int)vm->height;
	const int dx = INT_TO_FIXED(x1 - x0, FP_BITS) / (y1 - y0);

	int xp = INT_TO_FIXED(x0, FP_BITS);
	int yp = y0;
	do
	{
		if (yp >= 0 && yp < screenHeight)
		{
			edgeListToWriteFlat[yp] = FIXED_TO_INT(xp, FP_BITS);
		}
		xp += dx;

	} while (yp++ != y1);
}

static void renderPolygons(vmode *vm)
{
	uint8 *vram = (uint8*)getRenderBuffer(vm);
	QuadStore *q = &quads[0];

	const int SCALER_SHR = 16;
	int scalerX = (vm->width << SCALER_SHR) / ANIM_WIDTH;
	int scalerY = (vm->height << SCALER_SHR) / ANIM_HEIGHT;

	for (int i=0; i<numQuads; ++i) {

		Point2D p0 = q->p3;
		Point2D p1 = q->p2;
		Point2D p2 = q->p1;
		Point2D p3 = q->p0;
		
		p0.x = (p0.x * scalerX) >> SCALER_SHR; p0.y = (p0.y * scalerY) >> SCALER_SHR;
		p1.x = (p1.x * scalerX) >> SCALER_SHR; p1.y = (p1.y * scalerY) >> SCALER_SHR;
		p2.x = (p2.x * scalerX) >> SCALER_SHR; p2.y = (p2.y * scalerY) >> SCALER_SHR;
		p3.x = (p3.x * scalerX) >> SCALER_SHR; p3.y = (p3.y * scalerY) >> SCALER_SHR;

		const int x0 = p0.x; const int y0 = p0.y;
		const int x1 = p1.x; const int y1 = p1.y;
		const int x2 = p2.x; const int y2 = p2.y;
		const int x3 = p3.x; const int y3 = p3.y;

		const int scrWidth = vm->width;
		const int scrHeight = vm->height;

		int yMin = y0;
		int yMax = yMin;
		if (y1 < yMin) yMin = y1;
		if (y1 > yMax) yMax = y1;
		if (y2 < yMin) yMin = y2;
		if (y2 > yMax) yMax = y2;
		if (y3 < yMin) yMin = y3;
		if (y3 > yMax) yMax = y3;

		if (yMin < 0) yMin = 0;
		if (yMax > scrHeight - 1) yMax = scrHeight - 1;

		prepareEdgeListFlat(&p1, &p0, vm);
		prepareEdgeListFlat(&p2, &p1, vm);
		prepareEdgeListFlat(&p3, &p2, vm);
		prepareEdgeListFlat(&p0, &p3, vm);

		uint8 *dst = vram + yMin * scrWidth;
		for (int y = yMin; y <= yMax; y++)
		{
			int xl = leftEdgeFlat[y];
			int xr = rightEdgeFlat[y];

			if (xl < 0) xl = 0;
			if (xr > scrWidth - 1) xr = scrWidth - 1;

			if (xl == xr) ++xr;

			/*for (int x = xl; x < xr; ++x) {
				*(dst + x) = q->c;
			}*/

			if (xr > xl) memset(dst+xl, q->c, xr-xl);
			dst += scrWidth;
		}
		++q;
	}
}

static bool isPolygonConvex(Point2D *pt, int numVertices)
{
	int zcross0;

	for (int i = 0; i < numVertices; ++i) {
		const int i0 = i;
		const int i1 = (i + 1) % numVertices;
		const int i2 = (i + 2) % numVertices;

		vec2i v0;   v0.x = pt[i1].x - pt[i0].x;  v0.y = pt[i1].y - pt[i0].y;
		vec2i v1;   v1.x = pt[i2].x - pt[i1].x;  v1.y = pt[i2].y - pt[i1].y;

		int zcross = v0.x * v1.y - v0.y * v1.x;
		if (i == 0) {
			zcross0 = zcross;
		} else {
			zcross *= zcross0;
			if (zcross < 0) {
				return false;
			}
		}
	}
	return true;
}

static void addPolygon(Point2D *pt, int numVertices, uint8 color)
{
	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

	//if (!isPolygonConvex(pt, numVertices)) return;

	while(pStartIndex < maxIndex)
	{
		quadPtr->p0.x = pt[pBaseIndex].x;       quadPtr->p0.y = pt[pBaseIndex].y;
		quadPtr->p1.x = pt[pStartIndex].x;      quadPtr->p1.y = pt[pStartIndex].y;
		quadPtr->p2.x = pt[pStartIndex+1].x;    quadPtr->p2.y = pt[pStartIndex + 1].y;

		pStartIndex += 2;
		if (pStartIndex > maxIndex) pStartIndex = maxIndex;
		quadPtr->p3.x = pt[pStartIndex].x;      quadPtr->p3.y = pt[pStartIndex].y;

		quadPtr->c = color;

		++quadPtr;
		++numQuads;
	}
}

static void interpretPaletteData()
{
	uint8 bitmaskH = *data++;
	uint8 bitmaskL = *data++;

	int bitmask = (bitmaskH << 8) | bitmaskL;

	for (int i = 0; i < 16; ++i) {
		int palNum = i;
		if (bitmask & 0x8000) {
			uint8 colorH = *data++;
			uint8 colorL = *data++;

			int color = (colorH << 8) | colorL;

			int r = (color >> 8) & 7;
			int g = (color >> 4) & 7;
			int b = color & 7;
			
			setSingleColorPal(palNum, r << 3, g << 3, b << 3);
		}
		bitmask <<= 1;
	}
}

static void interpretDescriptorSpecial(uint8 descriptor)
{
	switch (descriptor)
	{
	case 0xff:
	{
		// End of frame
	}
	break;

	case 0xfe:
	{
		// End of frame and skip at next 64k block
		++block64index;
		data = &scene1_bin[block64index << 16];
	}
	break;

	case 0xfd:
	{
		// That's all folks!

		// Option 1, restart
		data = &scene1_bin[0];
		block64index = 0;
		frameNum = 0;

		// Option 2, quit?
		endOfAllFrames = true;
	}
	break;
	}
}

static void interpretDescriptorNormal(uint8 descriptor, int &polyNumVertices, int &colorIndex)
{
	colorIndex = (int)((descriptor >> 4) & 15);
	polyNumVertices = (int)(descriptor & 15);
}

static void interpretIndexedMode()
{
	static Point2D vi[256];

	uint8 descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	int vertexNum = *data++;

	for (int i = 0; i < vertexNum; ++i) {
		vi[i].x = (int)*data++;
		vi[i].y = (int)*data++;
	}

	while(true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, polyNumVertices, polyPaletteIndex);

		for (int n = 0; n < polyNumVertices; ++n) {
			int vertexId = *data++;

			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void interpretNonIndexedMode()
{
	uint8 descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	while (true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, polyNumVertices, polyPaletteIndex);

		for (int n = 0; n < polyNumVertices; ++n) {
			pt[n].x = *data++;
			pt[n].y = *data++;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}


static void decodeFrame()
{
	uint8 flags = *data++;

	mustClearScreen = false;
	numQuads = 0;
	quadPtr = &quads[0];

	if (flags & 1) {
		mustClearScreen = true;
	} 
	if (flags & 2) {
		interpretPaletteData();
	}
	if (flags & 4) {
		interpretIndexedMode();
	}
	else {
		interpretNonIndexedMode();
	}
}

static void renderScript(vmode *vm)
{
	decodeFrame();
	++frameNum;

	if (mustClearScreen) clearFrame(vm);

	renderPolygons(vm);
}

void fxAnimRun(vmode *vm, uint32 fxFrame)
{
	renderScript(vm);

	//if (endOfAllFrames) quit or do something;
}
