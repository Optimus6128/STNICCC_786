#include <string.h>
#include <stdlib.h>
#include "video.h"
#include "demo.h"

#include <dos.h>

static vmode vmodes[VMODES_NUM];

void initVideoModeInfo()
{
    vmodes[VMODE_TEXT] = vmode(0x0003, 80, 50, 4, false, VGA); /*(uint8*)0xB8000);*/
    vmodes[VMODE_VGA_320x200] = vmode(0x0013, 320, 200, 8, false, VGA);

    vmodes[VMODE_SVGA_640x400] = vmode(0x0100, 640, 400, 8, true, VGA);
    vmodes[VMODE_SVGA_640x480] = vmode(0x0101, 640, 480, 8, true, VGA);
    vmodes[VMODE_SVGA_800x600] = vmode(0x0103, 800, 600, 8, true, VGA);
    vmodes[VMODE_SVGA_1024x768] = vmode(0x0105, 1024, 768, 8, true, VGA);
    vmodes[VMODE_SVGA_1280x1024] = vmode(0x0107, 1280, 1024, 8, true, VGA);
}

void setMode(uint16 mode)
{
    /*_asm
	{
		mov ax,mode
		int 10h
    }*/

    union REGS regs;

    regs.h.ah = mode >> 8;
    regs.h.al = mode & 255;
    int86(0x10, &regs, &regs);
}

void setVesaMode(uint16 mode)
{
	#ifdef __DJGPP__
		union REGS regs;

		regs.h.ah = 0x4F;
		regs.h.al = 0x02;
		regs.h.bh = mode >> 8;
		regs.h.bl = mode & 255;
		int86(0x10, &regs, &regs);
	#else
		_asm
		{
			mov ax,4F02h
			mov bx,mode
			int 10h
		}
	#endif
}

void setTextMode()
{
	setMode(vmodes[VMODE_TEXT].mode);
}

vmode *setVideoMode(uint16 width, uint16 height, uint8 bpp, bool needsBuffer)
{
	vmode *vm;
	for (uint8 i=0; i<VMODES_NUM; ++i)
	{
		vm = &vmodes[i];
		if (vm->width==width && vm->height==height && vm->bpp==bpp) {
			if (needsBuffer) {
                if (vm->buffer!=0) free(vm->buffer);
				vm->buffer = (uint8*)malloc(((width * height * bpp) >> 3) * sizeof(uint8));
			}
			if (vm->vesa)
				setVesaMode(vm->mode);
			else
				setMode(vm->mode);
			return vm;
		}
	}
	return 0;
}

void setSvgaWindow(uint16 window)
{
	#ifdef __DJGPP__
		union REGS regs;

		regs.h.ah = 0x4F;
		regs.h.al = 0x05;
		regs.h.bh = 0;
		regs.h.bl = 0;
		regs.h.dh = window >> 8;
		regs.h.dl = window & 255;
		int86(0x10, &regs, &regs);
	#else
		_asm
		{
			mov ax,4F05h
			xor bx,bx
			mov dx,window
			int 10h
		}
	#endif
}

void copyBufferToSvga(vmode *vm)
{
	uint16 window = 0;
	int32 bytesLeft = (vm->width * vm->height * vm->bpp) >> 3;
	uint8 *src = (uint8*)vm->buffer;
	uint8 *dst = (uint8*)vm->vram;

	while (bytesLeft > 65535)
	{
		setSvgaWindow(window++);
		memcpy(dst, src, 65536);
		bytesLeft -= 65536;
		src += 65536;
	}
	setSvgaWindow(window);
	memcpy(dst, src, bytesLeft);
}

void clearFrame(vmode *vm)
{
	uint8 *vram = getRenderBuffer(vm);
	const uint32 size = vm->width * vm->height;
	memset(vram, 0, size);
}

void updateFrame(vmode *vm, bool vsync, bool skipVram)
{
	if (vsync) waitForVsync();
	if (vm->buffer==0) return;

	if (vm->vesa) {
		if (!skipVram) {
			copyBufferToSvga(vm);
		} else {
			memcpy(vm->vram, vm->buffer, vm->width);    // copy one line as a test
		}
	}
	else {
		uint32 size = vm->width * vm->height;
		if (skipVram) size = vm->width; // copy one line as above (we don't want to render to vram but want to update very little so that the user knows it's running)
		memcpy(vm->vram, vm->buffer, size);   // Just for VGA 320*200 now
	}
}

uint8 *getRenderBuffer(vmode *vm)
{
	if (vm->buffer==0)
		return vm->vram;
	else
		return vm->buffer;
}

void waitForVsync()
{
	#ifdef __DJGPP__
	#else
		_asm
		{
			mov dx,3dah

			vsync_in:
				in al,dx
				and al,8
			jnz vsync_in

			vsync_out:
				in al,dx
				and al,8
			jz vsync_out
		}
	#endif
}

void setPalFromTab(uint8 colstart, uint8 *paltab, uint16 colnum)
{
	#ifdef __DJGPP__
		outportb(0x3c8, colstart);
		for (uint16 i=0; i<3 * colnum; ++i)
			outportb(0x3c9, paltab[i]);
	#else
		_asm
		{
			mov dx,03c8h
			mov al,colstart
			out dx,al
			inc dx

			mov ax,colnum
			lea ecx,[eax + eax*2]

			mov ebx,paltab
			paltab_loop:
				mov al,[ebx]
				inc ebx
				out dx,al
			loopw paltab_loop
		}
	#endif
}

void setSingleColorPal(uint8 color, uint8 r, uint8 g, uint8 b)
{
	#ifdef __DJGPP__
		outportb(0x3c8, color);
		outportb(0x3c9, r);
		outportb(0x3c9, g);
		outportb(0x3c9, b);
	#else
		_asm
		{
			mov dx,03c8h
			mov al,color
			out dx,al
			inc dx

			mov al,r
			out dx,al

			mov al,g
			out dx,al

			mov al,b
			out dx,al
		}
	#endif
}
