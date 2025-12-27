#include "../fenster.h"
#include "../sharedmem.h"

#define set_pixel(x, y) (buf[((y) * W) + (x)])

static void fenster_rect(uint32_t* buf, int W, int x, int y, int width, int height, uint32_t c) {
	int row, col;
	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			set_pixel(x + col, y + row) = c;
		}
	}
}

static uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
static void fenster_text(uint32_t* buf, int W, int x, int y, char *s, int scale, uint32_t c) {
	int dx, dy;
	while (*s) {
		char chr = *s++;
		if (chr > 32) {
			uint16_t bmp = font5x3[chr - 32];
			for (dy = 0; dy < 5; dy++) {
				for (dx = 0; dx < 3; dx++) {
					if (bmp >> (dy * 3 + dx) & 1) {
						fenster_rect(buf, W, x + dx * scale, y + dy * scale, scale, scale, c);
					}
				}
			}
		}
		x = x + 4 * scale;
	}
}

int main(int argc, char** argv) {
	uint8_t* pMem;
	struct hdr* pHdr;
	const char* title = "This is a test";
	volatile uint8_t* sync;
	struct fenster_input_data* inp;
	uint32_t* pPixBuf;

	const char* mapfile1 = "/tmp/fstub";
	const char* mapfile2 = "/tmp/fstub_pix";
	const int W = 320;
	const int H = 200;
	uint32_t hdr_sz = sizeof(struct hdr);
	uint32_t inp_sz = sizeof(struct fenster_input_data);
	uint32_t size1 = hdr_sz + inp_sz;
	uint32_t size2 = W * H * sizeof(int32_t);
	void* hdl1;
	void* hdl2;

	pMem = (uint8_t*)createSharedMem(mapfile1, size1, 1, &hdl1);
	pHdr = (struct hdr*)pMem;
	memcpy(pHdr->title, title, strlen(title));
	sync = &pHdr->sync;
	pHdr->w = W;
	pHdr->h = H;
	inp = (struct fenster_input_data*)&pMem[hdr_sz];
	pPixBuf = (uint32_t*)createSharedMem(mapfile2, size2, 1, &hdl2);

	do {
		int has_keys = 0;
		char s[32] = { 0 };
		char *p = s;
		int i;

		while (*sync == 0) { usleep(1000); }
		if (*sync == 2) break;

		memset(pPixBuf, 0, size2);

		for (i = 32; i < 128; i++) {
			if (inp->key_down[i]) {
				has_keys = i;
				*p++ = i;
			}
		}
		*p = '\0';
		fenster_text(pPixBuf, W, 50, 50, s, 2, 0xff00ffff);
//		if (strlen(s) > 0) printf("%d %s %d %d %d %d\n", has_keys, s, inp->key_mod[0], inp->key_mod[1], inp->key_mod[2], inp->key_mod[3]);
		if (inp->key[27]) { 
			break; 
		}
		*sync =	0;
		if (inp->key_mod[0] > 0) {
			if (inp->key_down['1']) { 
				pHdr->win_w = W;
				pHdr->win_h = H;
				*sync = 3;
			} else if (inp->key_down['2']) { 
				pHdr->win_w = W * 2;
				pHdr->win_h = H * 2;
				*sync = 3;
			} else if (inp->key_down['3']) { 
				pHdr->win_w = W * 4;
				pHdr->win_h = H * 4;
				*sync = 3;
			} else if (inp->key_down['4']) { 
				pHdr->win_w = 1;
				pHdr->win_h = 1;
				*sync = 3;
			}
		}
	} while (1);
	*sync = 2;

	destroySharedMem(pMem, &hdl1);
	destroySharedMem(pPixBuf, &hdl2);
	return 0;
}

