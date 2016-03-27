#ifndef GRAPH_BITMAP_H
#define GRAPH_BITMAP_H

typedef struct{
	short sign;
	unsigned bitmap_size;
	int reserved;
	unsigned offset;
	unsigned header_size;
	unsigned width;
	unsigned height;
	short planes;
	short bpp;			//bits per pixel 1, 4, 8, 24, 32?
	int compression;	//0=none 1=RLE-8 2=RLE-4
	unsigned size;
	unsigned h_resulution;
	unsigned v_resulution;
	unsigned colors;
	unsigned important_colors;
	
}__attribute((packed)) BitmapHeader_t, *BitmapHeader_p;

#define BITMAP_SIGNATURE 0x4D42

#define IsBitmap(hbmp) (hbmp->sign == BITMAP_SIGNATURE)

#endif
