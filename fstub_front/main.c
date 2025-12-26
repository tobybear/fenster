#define FENSTER_IMPLEMENTATION
#include "../fenster.h"
#include "../sharedmem.h"

int main(int argc, char** argv) {
	struct fenster f = { 0 };
	int64_t now, time;
	const char* mapfile1 = "/tmp/fstub";
	const char* mapfile2 = "/tmp/fstub_pix";
	uint8_t* pMem;
	struct hdr* pHdr;
	char title[128];
	volatile uint8_t* sync;
	struct fenster_input_data* inp;
	uint32_t* pPixBuf;
	volatile int W, H;
	uint32_t hdr_sz = sizeof(struct hdr);
	uint32_t inp_sz = sizeof(struct fenster_input_data);
	uint32_t size1 = hdr_sz + inp_sz;
	uint32_t size2;
	void* hdl1;
	void* hdl2;
	int fs = 0;

	pMem = (uint8_t*)createSharedMem(mapfile1, size1, 0, &hdl1);
	pHdr = (struct hdr*)pMem;
	sync = &pHdr->sync;
	W = pHdr->w;
	H = pHdr->h;
	inp = (struct fenster_input_data*)&pMem[hdr_sz];
	while (W * H == 0) { usleep(1000); }
	size2 = W * H * sizeof(int32_t);
	pPixBuf = (uint32_t*)createSharedMem(mapfile2, size2, 0, &hdl2);
	
	strcpy_s(title, 128, (const char *)pHdr->title);
	f.title = title;
	f.width = W;
	f.height = H;
	f.buf = pPixBuf;
	fenster_open(&f);
	now = fenster_time();
	while (fenster_loop(&f) == 0) {
		if (*sync == 2) break;
		if (*sync == 3) {
			int m = pHdr->win_w * pHdr->win_h;
			if (m > 0) {
				fs = 1 - fs;
				if (m == 1) {
					fenster_fullscreen(&f, fs);
				} else {
					fenster_resize(&f, pHdr->win_w, pHdr->win_h);
				}
				pHdr->win_w = pHdr->win_h = 0;
			}
			*sync = 0;
		}
		*inp = f.inp;

		*sync = 1;
		while (*sync == 1) { fenster_sleep(1); }
		time = fenster_time();
		if (time - now < 1000 / 60) {
			fenster_sleep(time - now);
		}
		now = time;
	}
	*sync = 2;
	fenster_close(&f);

	destroySharedMem(pMem, &hdl1);
	destroySharedMem(pPixBuf, &hdl2);
	return 0;
}

