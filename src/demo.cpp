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
static bool skipVram = false;
static bool backbuffer = true;

static uint32 nframe = 0;
static bool quit = false;

uint8 *VGAptr = (uint8*)0xA0000;
uint8 *TXTptr = (uint8*)0xB8000;
uint16 *my_clock = (uint16*)0x046C;

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

	printf("0) 640x400\n");
	printf("1) 640x480\n");
	printf("2) 800x600\n");
	printf("3) 1024x768\n");
	printf("4) 1280x1024\n");
	printf("9) 320x200\n");

	uint8 answer = getch();

	bool isSvga = true;

	switch(answer)
	{
		case '0':
			video_width = 640;
			video_height = 400;
		break;

		case '1':
			video_width = 640;
			video_height = 480;
		break;

		case '2':
			video_width = 800;
			video_height = 600;
		break;

		case '3':
			video_width = 1024;
			video_height = 768;
		break;

		case '4':
			video_width = 1280;
			video_height = 1024;
		break;

		case '9':
			video_width = 320;
			video_height = 200;
			isSvga = false;
		break;

		default:
		break;
	}

	printf("\nVsync? (Y/N)\n");
	answer = getch();
	vsync = (answer == 'y' | answer == 'Y');

	printf("\nBackbuffer? (Y/N)\n");
	answer = getch();
	backbuffer = isSvga | (answer == 'y' | answer == 'Y');

	printf("\nDon't write to vram? (Y/N)\n");
	answer = getch();
	skipVram = (answer == 'y' | answer == 'Y');
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

	vm = setVideoMode(video_width, video_height, 8, backbuffer);
	if (vm==0) {
		printf("Video Mode not found\n");
		return 0;
	}

	init(vm);
	clearFrame(vm);

    uint16 time0 = getTime();
	while(!quit)
	{
        script(nframe);
		updateFrame(vm, vsync, skipVram);
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
