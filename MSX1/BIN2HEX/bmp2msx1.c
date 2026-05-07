/* 16colorBitmap->MSX1 convert support */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

#pragma pack(1)
typedef struct {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
	DWORD biSize;
	long  biWidth;
	long  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	long  biXPelsPerMeter;
	long  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;


int main(int argc, char *argv[]) {
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	int WIDTH,HEIGHT;

	if (argc < 2) {
		printf("Usage: %s <input.bmp>\n", argv[0]);
		return 1;
	}

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) return 1;

	// BMPヘッダ・パレットの読み込み（簡易版：オフセット固定）
	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);

	WIDTH =(int)bih.biWidth;
	HEIGHT = (int)bih.biHeight;

	fseek(fp, bfh.bfOffBits, SEEK_SET);

	// 画像データ読み込み (4bit BMP)
	// 1ピクセル4bit = 1バイト2ピクセル。
	uint8_t raw_pixels[HEIGHT][WIDTH / 2];
	for (int y = HEIGHT - 1; y >= 0; y--) {
		fread(raw_pixels[y], 1, WIDTH / 2, fp);
	}
	fclose(fp);


	for (int y = 0; y < HEIGHT; y+=8) {
		for (int x_byte = 0; x_byte < WIDTH / 2; x_byte+=4) {
			for(int k = 0; k < 8; ++k){

				printf("\n\"");
				for (int j = 0; j < 4; j++) {
					uint8_t two_pix = raw_pixels[y+k][x_byte+j];
					uint8_t p[2];

					p[0] = two_pix >> 4;   // 上位4bit（左ピクセル）
					p[1] = two_pix & 0x0F; // 下位4bit（右ピクセル）

					for (int i = 0; i < 2; i++) {

						printf("%x",p[i]);

					}
				}
				printf("\"");
			}
			printf(",\n");
		}
	}

	printf("Done.\n");
	return 0;
}