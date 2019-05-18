#include "gfxtools.h"
#include "video.h"

void makePalTab(int numCols, int r0, int g0, int b0, int r1, int g1, int b1, unsigned char *myPalTab)
{
    float dr = (float)(r1 - r0) / (float)(numCols - 1);
    float dg = (float)(g1 - g0) / (float)(numCols - 1);
    float db = (float)(b1 - b0) / (float)(numCols - 1);
    float cr = (float)r0;
    float cg = (float)g0;
    float cb = (float)b0;
    for (int i=0; i<numCols; i++)
    {
        *myPalTab++ = (unsigned char)cr;
        *myPalTab++ = (unsigned char)cg;
        *myPalTab++ = (unsigned char)cb;
        cr += dr;
        cg += dg;
        cb += db;
    }
}

void makeAndSetPal(int colStart, int colEnd, int r0, int g0, int b0, int r1, int g1, int b1)
{
    int numCols = colEnd - colStart + 1;
    unsigned char *tempPal = new unsigned char[3 * numCols];

    makePalTab(numCols, r0, g0, b0, r1, g1, b1, tempPal);
    setPalFromTab(colStart, tempPal, numCols);

    delete tempPal;
}
