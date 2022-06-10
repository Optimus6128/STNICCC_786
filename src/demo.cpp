#include <stdio.h>
#include <conio.h>

#ifdef __DJGPP__
	#include <sys/nearptr.h>
#endif

#include "demo.h"
#include "video.h"
#include "timer.h"

#include "fx_anim.h"

static uint16 video_width;
static uint16 video_height;

static vmode *vm;
static bool vsync = false;

static uint32 nframe = 0;
static bool quit = false;

uint8 *VGAptr = (uint8*)0xA0000;
uint8 *TXTptr = (uint8*)0xB8000;
uint16 *my_clock = (uint16*)0x046C;

static bool videoOutputEnabled = true;

void init(vmode *vm)
{
	fxAnimInit(vm);
}

void input()
{
	if (kbhit())
	{
		uint8 key = getch();
		switch(key)
		{
			case 27:    // Escape
				quit = true;
			break;

			case 'f':
				vsync = !vsync;
			break;
		}
	}
}

void script(uint32 fxFrame)
{
	fxAnimRun(vm, fxFrame);
}

void askForMode()
{
	printf("Select video mode:\n\n");

	printf("0) 320x200\n");
	printf("1) 640x400\n");
	printf("2) 640x480\n");
	printf("3) 800x600\n");
	printf("4) 1024x768\n");
	printf("5) 1280x1024\n");

	uint8 answer = getch();

	bool isSvga = true;

	switch(answer)
	{
		case '0':
			video_width = 320;
			video_height = 200;
			isSvga = false;
		break;

		case '1':
			video_width = 640;
			video_height = 400;
		break;

		case '2':
			video_width = 640;
			video_height = 480;
		break;

		case '3':
			video_width = 800;
			video_height = 600;
		break;

		case '4':
			video_width = 1024;
			video_height = 768;
		break;

		case '5':
			video_width = 1280;
			video_height = 1024;
		break;

		default:
		break;
	}

	printf("\nBenchmark? (Y/N) \n");
	answer = getch();
	if (answer == 'y' | answer == 'Y') {
		vsync = false;

		printf("\nVideo Output? (Y/N) \n");
		answer = getch();
		videoOutputEnabled = (answer == 'y' | answer == 'Y');

		printf("\nRepeats? ");
		scanf("%d", &benchRepeats);
		if (benchRepeats < 1) benchRepeats = 1;
	} else {
		vsync = true;
	}
}

int main(int argc, char **argv)
{
#ifdef __DJGPP__
    __djgpp_nearptr_enable();

    VGAptr += __djgpp_conventional_base;
	TXTptr += __djgpp_conventional_base;
    my_clock = (uint16*)((uint8*)my_clock + __djgpp_conventional_base);
#endif

	initVideoModeInfo();
	askForMode();

	vm = setVideoMode(video_width, video_height, 8, true);
	if (vm==0) {
		printf("Video Mode not found\n");
		return 0;
	}

	init(vm);
	clearFrame(vm);

	// Hack to show something updating (through the changes of the palette later) if we don't copy animation frame to vram.
	if (!videoOutputEnabled) {
		uint8 *dst = vm->buffer;
		for (int i=0; i<16; ++i) {
			*dst++ = i;
		}
		updateFrame(vm, false);
	}

    uint16 time0 = getTime();
	while(!quit && benchRepeats != 0)
	{
        script(nframe);
		if (videoOutputEnabled) {
			updateFrame(vm, vsync);
		}
		input();
		++nframe;
	}
    uint16 time1 = getTime();

	setTextMode();

	float secs = (float)(time1 - time0) / TIME_FREQ;
	float fps = (float)nframe / secs;
	printf("%.2f Fps\n", fps);

#ifdef __DJGPP__
    __djgpp_nearptr_disable();
#endif

	return 0;
}
