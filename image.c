/* $RCSfile$
 * 
 * $Author$
 * $Date$
 * $Revision$
 * 
 * $Log$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "utils.h"

#include "readpic.h"
#include "writepic.h"

#include "image.h"

// =============================================================
// = #DEFINES =================================================
// ===========================================================

#define INT_PIX(_d_, _sa_, _sb_, _wa_, _wb_) {\
	*(((uint8 *)(_d_))+0)=(uint8)(((double) *(((uint8 *)(_sa_))+0))*(_wa_) + \
								((double) *(((uint8 *)(_sb_))+0))*(_wb_)); \
	*(((uint8 *)(_d_))+1)=(uint8)(((double) *(((uint8 *)(_sa_))+1))*(_wa_) + \
								((double) *(((uint8 *)(_sb_))+1))*(_wb_)); \
	*(((uint8 *)(_d_))+2)=(uint8)(((double) *(((uint8 *)(_sa_))+2))*(_wa_) + \
								((double) *(((uint8 *)(_sb_))+2))*(_wb_)); \
	*(((uint8 *)(_d_))+3)=(uint8)(((double) *(((uint8 *)(_sa_))+3))*(_wa_) + \
								((double) *(((uint8 *)(_sb_))+3))*(_wb_)); \
	}

// =============================================================
// = TYPEDEFS =================================================
// ===========================================================

typedef struct {
	int		index;
	double	wa;
	double	wb;
} LLutEntry;

// =============================================================
// = GLOBAL FUNCTIONS =========================================
// ===========================================================

Image *IMG_new(int width, int height, uint32 col)
{
	Image	*image;
	
	image = malloc(sizeof(Image));
	image->width = width;
	image->height = height;
	image->premult = FALSE;
	if(col == 0)
		image->bitmap = calloc(width * height, sizeof(uint32));
	else {
		int		i;
		
		image->bitmap = malloc(width * height * sizeof(uint32));
		for(i = 0; i < width * height; i++)
			image->bitmap[i] = col;
	}
	return image;
}

Image *IMG_load(char *path)
{
	// XXX: Only Softimage pics supported at the moment
	
	return FF_PIC_load(path);
}

bool IMG_save(char *path, Image *image)
{
	// XXX: Only Softimage pics supported at the moment
	
	return FF_PIC_save(path, image);
}

void IMG_delete(Image *image)
{
	if(!image)
		return;
	
	if(image->bitmap)
		free(image->bitmap);
	free(image);
}

bool IMG_getPreMult(Image *image)
{
	return image->premult;
}

void IMG_setPreMult(Image *image, bool pm)
{
	image->premult = pm;
}


void IMG_paste(Image *image, Image *brush, int xp, int yp)
{
	int		w, h;
	int		xi, xb, yi, yb;
	int		x, y;
	
	xi = xp;
	xb = 0;
	if(xp < 0) {
		// Completely outside
		if(xp <= -brush->width)
			return;
		w = brush->width + xp;
		xi = 0;
		xb = -xp;
	} else
	if((xp + brush->width) > image->width) {
		if(xp >= image->width)
			return;
		w = image->width - xp;
	} else
		w = brush->width;

	yi = yp;
	yb = 0;
	if(yp < 0) {
		// Completely outside
		if(yp <= -brush->height)
			return;
		h = brush->height + yp;
		yi = 0;
		yb = -yp;
	} else
	if((yp + brush->height) > image->height) {
		if(yp >= image->height)
			return;
		h = image->height - yp;
	} else
		h = brush->height;
	
	if(image->premult)
		printf("WARNING: IMG_Paste(): Pre-multiplied lower layer images not compenstated.\n");
	
	if(brush->premult) {
		for(y = 0; y < h; y++) {
			uint32	*dst, *src;
			
			dst = image->bitmap + (y + yi) * image->width;
			src = brush->bitmap + (y + yb) * brush->width;
			
			for(x = 0; x < w; x++) {
				int		r, g, b, a;
				int		xs, xd;
				
				xs = x + xb;
				xd = x + xi;
				
				a = (src[xs] >> 24);
				
				r = (( ((dst[xd]) & 0xFF) * (255 - a)) + ((src[xs]) & 0xFF) * 255) / 255;
				if(r > 255) r = 255;
				g = (( ((dst[xd] >> 8) & 0xFF) * (255 - a)) + ((src[xs] >> 8) & 0xFF) * 255) / 255;
				if(g > 255) g = 255;
				b = (( ((dst[xd] >> 16) & 0xFF) * (255 - a)) + ((src[xs] >> 16) & 0xFF) * 255) / 255;
				if(b > 255) b = 255;
				a = (( ((dst[xd] >> 24) & 0xFF) * (255 - a)) + ((src[xs] >> 24) & 0xFF) * 255) / 255;
				if(a > 255) a = 255;
				dst[xd] = (a << 24) | (b << 16) | (g << 8) | (r);
			}
		}
	} else {
		for(y = 0; y < h; y++) {
			uint32	*dst, *src;
			
			dst = image->bitmap + (y + yi) * image->width;
			src = brush->bitmap + (y + yb) * brush->width;
			
			for(x = 0; x < w; x++) {
				int		r, g, b, a;
				int		xs, xd;
				
				xs = x + xb;
				xd = x + xi;
				
				a = (src[xs] >> 24);
				
				r = (( ((dst[xd]) & 0xFF) * (255 - a)) + (((src[xs]) & 0xFF) * a)) / 255;
				if(r > 255) r = 255;
				g = (( ((dst[xd] >> 8) & 0xFF) * (255 - a)) + (((src[xs] >> 8) & 0xFF) * a)) / 255;
				if(g > 255) g = 255;
				b = (( ((dst[xd] >> 16) & 0xFF) * (255 - a)) + (((src[xs] >> 16) & 0xFF) * a)) / 255;
				if(b > 255) b = 255;
				a = (( ((dst[xd] >> 24) & 0xFF) * (255 - a)) + (((src[xs] >> 24) & 0xFF) * a)) / 255;
				if(a > 255) a = 255;
				dst[xd] = (a << 24) | (b << 16) | (g << 8) | (r);
			}
		}
	}
}

void IMG_scaleChannels(Image *image, int rs, int gs, int bs, int as)
{
	int		x, y;
	
	for(y = 0; y < image->height; y++) {
		uint32	*line = &image->bitmap[y * image->width];
		
		for(x = 0; x < image->width; x++) {
			int	r, g, b, a;
			r = (((line[x] & 0xFF)) * rs) / 255;
			r = (r < 0)?(0):( (r > 255)?(255):(r) );
			g = (((line[x] >> 8) & 0xFF) * gs) / 255;
			g = (g < 0)?(0):( (g > 255)?(255):(g) );
			b = (((line[x] >> 16) & 0xFF) * bs) / 255;
			b = (b < 0)?(0):( (b > 255)?(255):(b) );
			a = (((line[x] >> 24) & 0xFF) * as) / 255;
			a = (a < 0)?(0):( (a > 255)?(255):(a) );
			
			line[x] = (a << 24) | (b << 16) | (g << 8) | r;
		}
	}
}

void IMG_fullAlpha(Image *image)
{
	int		x, y;
	
	for(y = 0; y < image->height; y++) {
		uint32	*line = &image->bitmap[y * image->width];
		
		for(x = 0; x < image->width; x++) {
			line[x] |= 0xFF000000;
		}
	}
}


/* this function sets up a lookup table for line scaling. */

static LLutEntry * createLLut(int from, int to)
{
	LLutEntry	*lut;
	int			i;
	double		x, tmp;
	
	lut = (LLutEntry *)malloc(to * sizeof(LLutEntry));
	
	for(i = 0; i < to; i++) {
		x = (double)i / (double)(to - 1);
		if(x < 1.0f) {
			tmp = x * (double)(from - 1);
			lut[i].index = (int)floor(tmp);
			lut[i].wb = tmp - (double)lut[i].index;
			lut[i].wa = 1.0 - lut[i].wb;
		} else {
			lut[i].index = from-2;
			lut[i].wb = 1.0;
			lut[i].wa = 0.0;
		}
	}
	return lut;
}

static void linescale(uint32 *dest, uint32 *src, int n, LLutEntry *lut)
{
	int i;
	
	for(i=0;i<n;i++)
		INT_PIX(dest + i, src + lut[i].index, src + lut[i].index + 1, lut[i].wa, lut[i].wb);
}

void IMG_scale(Image *image, int width, int height)
{
	uint32		*work;
	int			i, j, a, b, olda, oldb;
	double		x, tmp, wa, wb;
	LLutEntry	*lut;
	uint32		*bufa, *bufb, *tmpp;
	
	work = malloc(width * height * sizeof(uint32));
	
	if(width == image->width && height == image->height) {
		free(work);
		return;
	}
	
	lut = createLLut((int)image->width, width);

	olda = oldb = -1;
	bufa = (uint32 *)malloc(width * sizeof(uint32));
	bufb = (uint32 *)malloc(width * sizeof(uint32));

	for(i = 0; i < height; i++) {
		x = (double)i / (double)(height - 1);
		if (x < 1.0) {
			tmp = x * (double)(image->height - 1);
			a = (int)floor(tmp);
			b = a + 1;
			wb = tmp - (double)a;
			wa = 1.0 - wb;
		} else {
			b = image->height - 1;
			a = b - 1;
			wa = 0.0;
			wb = 1.0;
		}
		if(a != olda) {
			if(a == oldb) {
				tmpp = bufa;
				bufa = bufb;
				bufb = tmpp;
			} else {
				linescale(bufa, image->bitmap + a * image->width, width, lut);
			}
		}
		if(b != oldb)
			linescale(bufb, image->bitmap + b * image->width, width, lut);
		
		if(!wa) {
			memcpy(work + i * width, bufa, width * sizeof(uint32));
		}else {
			for(j = 0; j < width; j++){
				INT_PIX(work + i * width + j, bufa + j, bufb + j, wa, wb);
			}
		}

		olda = a;
		oldb = b;
	}
	free(bufa);
	free(bufb);
	free(lut);
	free(image->bitmap);
	image->bitmap = work;
	image->width = width;
	image->height = height;
}
