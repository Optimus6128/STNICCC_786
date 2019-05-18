#ifndef GFXTOOLS_H
#define GFXTOOLS_H

void makePalTab(int numCols, int r0, int g0, int b0, int r1, int g1, int b1, unsigned char *myPalTab);
void makeAndSetPal(int colStart, int colEnd, int r0, int g0, int b0, int r1, int g1, int b1);

#endif
