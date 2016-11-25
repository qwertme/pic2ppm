/* $RCSfile$
 * 
 * $Author$
 * $Date$
 * $Revision$
 * 
 * $Log$
 */

#ifndef IMAGE_H
#define IMAGE_H

#include "utils.h"

// =============================================================
// = PUBLIC TYPES =============================================
// ===========================================================

typedef struct {
	int		width, height;
	bool	premult;			// If alpha is premultiplied
	uint32	*bitmap;
} Image;

// =============================================================
// = PUBLIC FUNCTIONS =========================================
// ===========================================================

Image *IMG_new(int width, int height, uint32 col);
Image *IMG_load(char *path);
bool IMG_save(char *path, Image *image);
void IMG_delete(Image *image);
bool IMG_getPreMult(Image *image);
void IMG_setPreMult(Image *image, bool pm);
void IMG_paste(Image *image, Image *brush, int x, int y);
void IMG_scaleChannels(Image *image, int r, int g, int b, int a);
void IMG_fullAlpha(Image *image);
void IMG_scale(Image *image, int width, int height);

#endif // IMAGE_H
