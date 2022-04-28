#ifndef VIDEO_H
#define VIDEO_H

#include "types.h"

enum {  VMODE_TEXT,
        VMODE_VGA_320x200,
        VMODE_SVGA_640x400,
        VMODE_SVGA_640x480,
        VMODE_SVGA_800x600,
        VMODE_SVGA_1024x768,
        VMODE_SVGA_1280x1024,
        VMODES_NUM};

struct vmode
{
    vmode(){};
    vmode(uint16 m, uint16 w, uint16 h, uint8 b, bool vs, uint8 *v) : 
        mode(m), width(w), height(h), bpp(b), vesa(vs), vram(v), buffer(0) {}

    uint16 mode;
    uint16 width;
    uint16 height;
    uint8 bpp;
    bool vesa;
    uint8 *vram;
    uint8 *buffer;
};

void initVideoModeInfo();
vmode *setVideoMode(uint16 width, uint16 height, uint8 bpp, bool needsBuffer = false);
void setTextMode();

void waitForVsync();
void clearFrame(vmode *vm);
void updateFrame(vmode *vm, bool vsync = false);
uint8 *getRenderBuffer(vmode *vm);

void setPalFromTab(uint8 colstart, uint8 *paltab, uint16 colnum);
void setSingleColorPal(uint8 color, uint8 r, uint8 g, uint8 b);

#endif
