#ifndef __GIMTOOL_H__
#define __GIMTOOL_H__

#define Rgb565    0x00
#define Argb1555  0x01
#define Argb4444  0x02
#define Argb8888  0x03
#define Index4    0x04
#define Index8    0x05
#define Unknown   0xFF

typedef struct __attribute__((packed)) {
	char magic1;
	char magic2;
	uint32_t filesize;
	uint32_t reserved;
	uint32_t offset;
} bitmap_header;

typedef struct __attribute__((packed)) {
	uint32_t bmpinfohead_size;	// Should be 40.
	int32_t width;
	int32_t height;
	uint16_t color_planes;		// Should be 1.
	uint16_t bpp;				// For our purposes, 32bpp
	uint32_t compression;		// Compression Method
	uint32_t image_size;		// RAW image size, e.g. w*h*bpp
	int32_t ppm_w;				// PPM width
	int32_t ppm_h;				// PPM height
	uint32_t colors;			// Pallete colors - 0 for 2^n
	uint32_t important_col;		// Important colors. Set to 0.
	uint32_t redmask;			// 44
	uint32_t greenmask;			// 48
	uint32_t bluemask;			// 52
	uint32_t alphamask;			// 56
	uint32_t colsptype;			// 60
	uint32_t reserved0;			// 64
	uint32_t reserved1;			// 68
	uint32_t reserved2;			// 72
	uint32_t reserved3;			// 76
	uint32_t reserved4;			// 80
	uint32_t reserved5;			// 84
	uint32_t reserved6;			// 88
	uint32_t reserved7;			// 92
	uint32_t reserved8;			// 96
	uint32_t gamma_r;
	uint32_t gamma_g;	
	uint32_t gamma_b;
} bitmapv4_header;

typedef struct {
	int width;
	int height;
	uint8_t* data;
} OutData;

uint8_t* unswizzle(uint8_t* data, int width, int height, int bpp, int fix);
void dump_bitmap(char* fname, uint8_t* data32bpp, int width, int height);

uint8_t* decode_Rgb565(uint8_t* data, int width, int height);
uint8_t* decode_Argb1555(uint8_t* data, int width, int height);
uint8_t* decode_Argb4444(uint8_t* data, int width, int height);
uint8_t* decode_Argb8888(uint8_t* data, int width, int height);
uint8_t* decode_Index4(uint8_t* data, uint8_t* palette, int width, int height);
uint8_t* decode_Index8(uint8_t* data, uint8_t* palette, int width, int height);

OutData* ReadData(uint8_t* data, int length);
uint8_t GIMExport(uint8_t* data, size_t len, char* file_out);
uint8_t GIMToBMP(char* file_in, char* file_out);

#endif
