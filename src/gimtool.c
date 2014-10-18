//*                                                                 *//
//* gimconv.c - Rough GIM <-> BMP converter. Not fully complete.    *//
//* Converts GIM to BMP and viceversa, intended to be used within   *//
//* the RCOmage fork I'm coding                                     *//

//* Mostly based on the code in GIMSharp - useful, but C#-y. That's *//
//* not a good thing, IMO. There's far too much support code for    *//
//* data reads in C-ish ways, and using streams like fread          *//
//*                                                                 *//

//* Things being derpy at the moment - Argb8888. Why? IDK.          *//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "gimtool.h"

char* unswizzle(char* data, int width, int height, int bpp) {
	width = (width * bpp) / 8;

	char* dest = (char*)calloc(sizeof(char), width*height);

	int rowblocks = (width / 16);

	int dstoff = 0;

	int x, y;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int blockX = x / 16;
			int blockY = y / 8;

			int blockIndex = blockX + ((blockY) * rowblocks);
			int blockAddress = blockIndex * 16 * 8;

			dest[dstoff] = data[blockAddress + (x - blockX * 16) + ((y - blockY * 8) * 16)];
			dstoff++;
		}
	}

	return dest;
}

void dump_bitmap(char* fname, char* data32bpp, int width, int height) {

	// Bitmaps are bottom-up for rows - so first thing's first.
	char* data_new = (char*)calloc(sizeof(char), width*height*4);
	// FLIP IT.
	int i;
	for(i=0; i<height; i++) {
		memcpy(&data_new[i*width*4], &data32bpp[(height-1-i)*width*4], width*4);
	}

	// Bitmap info.
	bitmapv4_header bmp_inh;

	bmp_inh.bmpinfohead_size = sizeof(bitmapv4_header);
	bmp_inh.width = width;
	bmp_inh.height = height;
	bmp_inh.color_planes = 1;
	bmp_inh.bpp = 32;
	bmp_inh.compression = 3;
	bmp_inh.image_size = width*height*4;
	bmp_inh.ppm_w = 0;
	bmp_inh.ppm_h = 0;
	bmp_inh.colors = 0;
	bmp_inh.important_col = 0;
	bmp_inh.redmask = 0;
	((char*)&bmp_inh.redmask)[0] = 0xff;
	bmp_inh.bluemask = 0;
	((char*)&bmp_inh.bluemask)[1] = 0xff;
	bmp_inh.greenmask = 0;
	((char*)&bmp_inh.greenmask)[2] = 0xff;
	bmp_inh.alphamask = 0;
	((char*)&bmp_inh.alphamask)[3] = 0xff;
	bmp_inh.colsptype = 0x206E6957;
	bmp_inh.reserved0 = bmp_inh.reserved1 = bmp_inh.reserved2 = bmp_inh.reserved3 = bmp_inh.reserved4 = bmp_inh.reserved5 = bmp_inh.reserved6 = bmp_inh.reserved7 = bmp_inh.reserved8 = 0;
	bmp_inh.gamma_r = 0;
	bmp_inh.gamma_g = 0;
	bmp_inh.gamma_b = 0;

	// File header.
	bitmap_header bmp_h;

	bmp_h.magic1 = 'B';
	bmp_h.magic2 = 'M';
	bmp_h.reserved = 0;
	bmp_h.offset = sizeof(bitmap_header) + sizeof(bitmapv4_header);
	bmp_h.filesize = sizeof(bitmap_header) + sizeof(bitmapv4_header) + (width*height*4);

	// Write shit out.
	FILE* out = fopen(fname, "wb");
	fwrite(&bmp_h, 1, sizeof(bitmap_header), out);
	fwrite(&bmp_inh, 1, sizeof(bitmapv4_header), out);
	fwrite(data_new, 1, (width*4*height), out);
	fclose(out);
}

// Used for palletes too.
char* decode_Rgb565(char* data, int width, int height) {
	int size = (width * height);

	char* output = (char*)calloc(sizeof(char), 4*size); // 32bpp so 4

	// Cast data to a ushort pointer.
	uint16_t* pointer = (uint16_t*)data;

	int i;
	for(i=0; i < size; i++) {
		output[i*4 + 3] = 0xFF;
		output[i*4 + 2] = (char)(((pointer[i] >> 0)  & 0x1F) * 0xFF / 0x1F);
		output[i*4 + 1] = (char)(((pointer[i] >> 5)  & 0x3F) * 0xFF / 0x3F);
		output[i*4 + 0] = (char)(((pointer[i] >> 11) & 0x1F) * 0xFF / 0x1F);
	}

	return output;
}

char* decode_Argb1555(char* data, int width, int height) {
	int size = (width * height);

	char* output = (char*)calloc(sizeof(char), 4*size); // 32bpp so 4

	// Cast data to a ushort pointer.
	uint16_t* pointer = (uint16_t*)data;
	
	int i;
	for(i=0; i < size; i++) {
		output[i*4 + 3] = (char)(((pointer[i] >> 15) & 0x01) * 0xFF);
		output[i*4 + 2] = (char)(((pointer[i] >> 0)  & 0x1F) * 0xFF / 0x1F);
		output[i*4 + 1] = (char)(((pointer[i] >> 5)  & 0x1F) * 0xFF / 0x1F);
		output[i*4 + 0] = (char)(((pointer[i] >> 10) & 0x1F) * 0xFF / 0x1F);
	}

	return output;
}

char* decode_Argb4444(char* data, int width, int height) {
	int size = (width * height);

	char* output = (char*)calloc(sizeof(char), 4*size); // 32bpp so 4

	// Cast data to a ushort pointer.
	uint16_t* pointer = (uint16_t*)data;

	int i;
	for(i=0; i < size; i++) {
		output[i*4 + 3] = (char)(((pointer[i] >> 12) & 0x0F) * 0xFF / 0x0F);
		output[i*4 + 2] = (char)(((pointer[i] >> 0)  & 0x0F) * 0xFF / 0x0F);
		output[i*4 + 1] = (char)(((pointer[i] >> 4)  & 0x0F) * 0xFF / 0x0F);
		output[i*4 + 0] = (char)(((pointer[i] >> 8)  & 0x0F) * 0xFF / 0x0F);
	}

	return output;
}

char* decode_Argb8888(char* data, int width, int height) {
	int size = (width * height);

	char* output = (char*)calloc(sizeof(char), 4*size); // 32bpp so 4

	int i;
	for(i=0; i < size; i++) {
		output[i*4+0] = data[i*4+0];
		output[i*4+1] = data[i*4+1];
		output[i*4+2] = data[i*4+2];
		output[i*4+3] = data[i*4+3];
	}

	return output;
}

char* decode_Index4(char* data, char* palette, int width, int height) {
	int size = (width * height);

	char* output = (char*)calloc(sizeof(char), 4*size); // 32bpp so 4

	uint32_t* out = (uint32_t*)output;
	uint32_t* pal = (uint32_t*)palette;

	int i;
	for(i=0; i < size; i += 2) {
		int px1  = data[i / 2] & 0xF;
		int px2  = data[i / 2] >> 4 & 0xF;
		out[i]   = pal[px1];
		out[i+1] = pal[px2];
	}

	return output;
}

char* decode_Index8(char* data, char* palette, int width, int height) {
	int size = (width * height);

	char* output = (char*)calloc(sizeof(char), 4*size); // 32bpp so 4

	int i;
	for(i=0; i < size; i++) {
		output[i*4+0] = palette[data[i]*4+0];
		output[i*4+1] = palette[data[i]*4+1];
		output[i*4+2] = palette[data[i]*4+2];
		output[i*4+3] = palette[data[i]*4+3];
	}

	return output;
}

OutData* ReadData(char* data, int length) {

	// Data Used Here
	int32_t offset = 0x10;
	int32_t prevOffset = offset;
	int32_t textureOffset = 0;
	int32_t palleteOffset = 0;
	uint16_t width = 0, height = 0, widthOff = 0, heightOff = 0;
	char datFmt = Unknown;
	char palFmt = Unknown;
	uint16_t pal_ents = 0;
	int32_t eofOffset = 0;
	int32_t hasMeta = 0;
	int32_t swizzled = 0;

	// Read all the meta blocks.
	while (offset < length) {

		// EOF Block
		if (data[offset] == 0x02) {
			eofOffset = ((int32_t*)(&data[offset+0x04]))[0] + 16;

			//fprintf(stderr, "[%x] eofOffset: %x\n", offset, eofOffset);

			offset +=   ((int32_t*)(&data[offset+0x08]))[0];
		}

		// Metadata Chunk
		else if (data[offset] == 0x03) {
			// We actually skip this.
			offset +=   ((int32_t*)(&data[offset+0x08]))[0];
		}

		// Texture Data Chunk
		else if (data[offset] == 0x04) {
			datFmt = (char)data[offset + 0x14];

			swizzled = ((uint16_t*)(&data[offset+0x16]))[0];

			width  = ((uint16_t*)(&data[offset+0x18]))[0];
			height = ((uint16_t*)(&data[offset+0x1A]))[0];

			textureOffset = offset + 0x50;

			//fprintf(stderr, "[%x] texture: f:%u w:%i h:%i swz:%i off:%x\n", offset, datFmt, width, height, swizzled, textureOffset);

			offset +=   ((int32_t*)(&data[offset+0x08]))[0];

		}

		// Pallete Chunk
		else if (data[offset] == 0x05) {
			palFmt = (char)data[offset + 0x14];

			pal_ents = ((uint16_t*)(&data[offset+0x18]))[0];

			palleteOffset = offset + 0x50;

			//fprintf(stderr, "[%x] pallete: f:%u ent:%u off:%x\n", offset, palFmt, pal_ents, palleteOffset);

			offset +=   ((int32_t*)(&data[offset+0x08]))[0];

		}

		// Metadata Chunk
		else if (data[offset] == 0xff) {
			hasMeta = 1;
			size_t metaOff = 0x10;

			// Seek to end of string for each piece of data
			int i=0;

			while(data[offset+metaOff+i]) {
				if(data[offset+metaOff+i] == 0)
					break;
				i++;
			}
			metaOff += i + 1; i = 0;

			while(data[offset+metaOff+i]) {
				if(data[offset+metaOff+i] == 0)
					break;
				i++;
			}
			metaOff += i + 1; i = 0;

			while(data[offset+metaOff+i]) {
				if(data[offset+metaOff+i] == 0)
					break;
				i++;
			}
			metaOff += i + 1; i = 0;

			while(data[offset+metaOff+i]) {
				if(data[offset+metaOff+i] == 0)
					break;
				i++;
			}
			metaOff += i + 1; i = 0;

			//fprintf(stderr, "[%x] Meta\n", offset);

			offset +=   ((int32_t*)(&data[offset+0x08]))[0];
		}

		// Invalid Chunk
		else {
			fprintf(stderr, "Sanity test: Chunk type failed.\n");
			//fprintf(stderr, "Type: %u\n", data[offset]);
			return NULL;
		}

		// Make sure chunks aren't moving us backwards.
		if ((int)offset <= (int)prevOffset) {
			fprintf(stderr, "Sanity test: forwards offset failed.\n");
			//fprintf(stderr, "Off:%x Fwd:%x\n", offset, prevOffset);
			return NULL;
		}
		prevOffset = offset;
	}

	// Sanity test
	if (offset != eofOffset) {
		fprintf(stderr, "Sanity test: Offset != eof failed.\n");
		//printf("Off: %u EOF: %u\n", offset, eofOffset);
		return NULL;
	}

	char* texture_data = &data[textureOffset];

	if (width % 16 != 0) {
		widthOff = (16 - (width % 16));
	}
	if (height % 8 != 0) {
		heightOff = (8 - (height % 8));
	}

	printf("GIM->BMP (");

	if(swizzled) {
		printf("SWIZ, ");
		switch (datFmt) {
			case Rgb565:
			case Argb1555:
			case Argb4444:
				texture_data = unswizzle(texture_data, width+widthOff, height+heightOff, 16);
				break;
			case Argb8888:
				texture_data = unswizzle(texture_data, width+widthOff, height+heightOff, 32);
				break;
			case Index4:
				texture_data = unswizzle(texture_data, width+widthOff, height+heightOff, 4);
				break;
			case Index8:
				texture_data = unswizzle(texture_data, width+widthOff, height+heightOff, 8);
				break; 
		}
	}

	char* newData = NULL;

	char* palData = NULL;

	switch (datFmt) {
		case Rgb565:
			printf("RGB565)");
			newData = decode_Rgb565(texture_data, width+widthOff, height+heightOff);
			break;
		case Argb1555:
			printf("ARGB1555)");
			newData = decode_Argb1555(texture_data, width+widthOff, height+heightOff);
			break;
		case Argb4444:
			printf("ARGB4444)");
			newData = decode_Argb4444(texture_data, width+widthOff, height+heightOff);
			break;
		case Argb8888:
			printf("ARGB8888)");
			newData = decode_Argb8888(texture_data, width+widthOff, height+heightOff);
			break;
		case Index4:
			// Extract pallete
			switch (palFmt) {
				case Rgb565:
					printf("4bpp, RGB565)");
					palData = decode_Rgb565( &data[palleteOffset],  16, 1);
					break;
				case Argb1555:
					printf("4bpp, ARGB1555)");
					palData = decode_Argb1555(&data[palleteOffset], 16, 1);
					break;
				case Argb4444:
					printf("4bpp, ARGB4444)");
					palData = decode_Argb4444(&data[palleteOffset], 16, 1);
					break;
				case Argb8888:
					printf("4bpp, ARGB8888)");
					palData = decode_Argb8888(&data[palleteOffset], 16, 1);
					break;
			}

			newData = decode_Index4(texture_data, palData, width+widthOff, height+heightOff);			
			break;
		case Index8:
			// Extract pallete
			switch (palFmt) {
				case Rgb565:
					printf("8bpp, RGB565)");
					palData = decode_Rgb565( &data[palleteOffset],  256, 1);
					break;
				case Argb1555:
					printf("8bpp, ARGB1555)");
					palData = decode_Argb1555(&data[palleteOffset], 256, 1);
					break;
				case Argb4444:
					printf("8bpp, ARGB4444)");
					palData = decode_Argb4444(&data[palleteOffset], 256, 1);
					break;
				case Argb8888:
					printf("8bpp, ARGB8888)");
					palData = decode_Argb8888(&data[palleteOffset], 256, 1);
					break;
			}
			newData = decode_Index8(texture_data, palData, width+widthOff, height+heightOff);
			break;
		case Unknown:
			// Invalid format.
			return NULL;
	}

	OutData* out = (OutData*)calloc(sizeof(OutData), 1);

	out->data = newData;
	out->width = width+widthOff;
	out->height = height+heightOff;

	return out;
}

uint8_t GIMExport(char* data, size_t len, char* file_out) {
	// Convert to a more rational format.
	OutData* data_converted = ReadData(data, len);

	if(data_converted == NULL) {
		printf(" - Failed.\n");
		return -1;
	}

	// Export to bitmap.
	dump_bitmap(file_out, data_converted->data, data_converted->width, data_converted->height);

	printf(" - OK.\n");

	return 0;
}


uint8_t GIMToBMP(char* file_in, char* file_out) {
	FILE* input = fopen(file_in, "rb");
	
	// Get length.
	fseek(input, 0, SEEK_END);
	size_t size = ftell(input);
	fseek(input, 0, SEEK_SET);

	// Read in RAW data.
	char* input_data = (char*)calloc(sizeof(char), size);
	fread(input_data, 1, size, input);

	printf("gimtool-exp: %s, ", file_out);
	
	return GIMExport(input_data, size, file_out);
}

#ifdef STANDALONE_GIMTOOL

int main(int argc, char** argv) {
	if(argc < 3) {
		printf("Usage: %s GIM BMP\n", argv[0]);
		return 1;
	}
	else {
		GIMToBMP(argv[1], argv[2]);
		return 0;
	}
}

#endif
