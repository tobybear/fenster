#include "../../sharedmem.h"
#include "doomgeneric.h"
#include "doomkeys.h"

#include <string.h>
#include <time.h>
#include <ctype.h>

struct fenster_input_data {
	uint8_t key_down[256];          // keys are mostly ASCII, but arrows are 17..20 (persisent until release)
	uint8_t key[256];               // like key_down, but one press signal only (one click)
	uint8_t key_mod[4];             // ctrl, shift, alt, meta
	uint32_t mouse_pos[2];          // mouse x, y
	uint8_t mouse_button[5];        // left, right, middle, scroll up, scroll down (one click)
	uint8_t mouse_button_down[3];   // left, right, middle (persistent until release)
};

uint8_t* pMem;
struct hdr* pHdr;
volatile uint8_t* fenster_sync;
struct fenster_input_data* inp;
uint32_t* pPixBuf;
uint32_t size1, size2;
void* hdl1;
void* hdl2;
const int W = DOOMGENERIC_RESX;
const int H = DOOMGENERIC_RESY;

void DG_Init() {
	const char* title = "CosmoDOOM";

	const char* mapfile1 = "/tmp/fstub";
	const char* mapfile2 = "/tmp/fstub_pix";
	uint32_t hdr_sz = sizeof(struct hdr);
	uint32_t inp_sz = sizeof(struct fenster_input_data);
	size1 = hdr_sz + inp_sz;
	size2 = W * H * sizeof(int32_t);

	pMem = (uint8_t*)createSharedMem(mapfile1, size1, 1, &hdl1);
	pHdr = (struct hdr*)pMem;
	memcpy(pHdr->title, title, strlen(title));
	fenster_sync = &pHdr->sync;
	pHdr->w = W;
	pHdr->h = H;
	inp = (struct fenster_input_data*)&pMem[hdr_sz];
	pPixBuf = (uint32_t*)createSharedMem(mapfile2, size2, 1, &hdl2);
 	
	printf("Waiting for framebuffer client...");
	while (*fenster_sync == 0) { usleep(1000); }
	printf("OK\n");
//	destroySharedMem(pMem, &hdl1);
//	destroySharedMem(pPixBuf, &hdl2);
}

void DG_DrawFrame() { 
	int has_keys = 0;
	char s[32] = { 0 };
	char *p = s;
	int i;

	while (*fenster_sync == 0) { usleep(1000); }
	if (*fenster_sync == 2) exit(0);

	memcpy(pPixBuf, DG_ScreenBuffer, size2);

	for (i = 32; i < 128; i++) {
		if (inp->key_down[i]) {
			has_keys = i;
			*p++ = i;
		}
	}
	*p = '\0';
	if (inp->key_down[27]) { 
		exit(0); 
	}
	*fenster_sync = 0;
	if (inp->key_mod[0] > 0) {
		if (inp->key_down['1']) { 
			pHdr->win_w = W;
			pHdr->win_h = H;
			*fenster_sync = 3;
		} else if (inp->key_down['2']) { 
			pHdr->win_w = W * 2;
			pHdr->win_h = H * 2;
			*fenster_sync = 3;
		} else if (inp->key_down['3']) { 
			pHdr->win_w = W * 4;
			pHdr->win_h = H * 4;
			*fenster_sync = 3;
		} else if (inp->key_down['4']) { 
			pHdr->win_w = 1;
			pHdr->win_h = 1;
			*fenster_sync = 3;
		}
	}
}

void DG_SleepMs(uint32_t ms) { 
	usleep(ms * 1000); 
}

uint32_t DG_GetTicksMs() {
#ifdef _WIN32
	LARGE_INTEGER freq, count;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&count);
	return (int64_t)(count.QuadPart * 1000.0 / freq.QuadPart);
#else
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec * 1000 + (time.tv_nsec / 1000000);
#endif
}

void DG_SetWindowTitle(const char *title) { 
	(void)title;
}

unsigned char toDoomKey(int k) {
  switch (k) {
  case '\n':
    return KEY_ENTER;
  case '\x1b':
    return KEY_ESCAPE;
  case '\x11':
    return KEY_UPARROW;
  case '\x12':
    return KEY_DOWNARROW;
  case '\x13':
    return KEY_RIGHTARROW;
  case '\x14':
    return KEY_LEFTARROW;
  case 'Z':
    return KEY_FIRE;
  case 'Y':
    return KEY_FIRE;
  case 'X':
    return KEY_RSHIFT;
  case ' ':
    return KEY_USE;
  }
  return tolower(k);
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
  static int old[128] = {0};
  for (int i = 0; i < 128; i++) {
    if ((inp->key_down[i] && !old[i]) || (!inp->key_down[i] && old[i])) {
      *pressed = old[i] = inp->key_down[i];
      *doomKey = toDoomKey(i);
      return 1;
    }
  }
  return 0;
}
