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

#include "utils.h"
#include "image.h"

#include "readpic.h"

// ===============================================
// = #DEFINES ===================================
// =============================================

// Data type
#define PIC_UNSIGNED_INTEGER	0x00
#define PIC_SIGNED_INTEGER		0x10	// XXX: Not implemented
#define PIC_SIGNED_FLOAT		0x20	// XXX: Not implemented


// Compression type
#define PIC_UNCOMPRESSED		0x00
#define PIC_PURE_RUN_LENGTH		0x01
#define PIC_MIXED_RUN_LENGTH	0x02

// Channel types (OR'd)
#define PIC_RED_CHANNEL			0x80
#define PIC_GREEN_CHANNEL		0x40
#define PIC_BLUE_CHANNEL		0x20
#define PIC_ALPHA_CHANNEL		0x10
#define PIC_SHADOW_CHANNEL		0x08	// XXX: Not implemented
#define PIC_DEPTH_CHANNEL		0x04	// XXX: Not implemented
#define PIC_AUXILIARY_1_CHANNEL	0x02	// XXX: Not implemented
#define PIC_AUXILIARY_2_CHANNEL	0x01	// XXX: Not implemented

// ===============================================
// = TYPEDEFS ===================================
// =============================================

typedef struct __Channel {
	uint8	size;
	uint8	type;
	uint8	channels;
	struct __Channel *next;
} Channel;

// ===============================================
// = DECLARATIONS ===============================
// =============================================


static uint32 readScanlines(FILE *file, uint32 *image, int32 width, int32 height, Channel *channel, uint32 alpha);
static uint32 readScanline(FILE *file, uint8 *scan, int32 width, Channel *channel,  int32 bytes);
static uint32 channelReadRaw(FILE *file, uint8 *scan, int32 width, int32 noCol, int32 *off, int32 bytes);
static uint32 channelReadPure(FILE *file, uint8 *scan, int32 width, int32 noCol, int32 *off, int32 bytes);
static uint32 channelReadMixed(FILE *file, uint8 *scan, int32 width, int32 noCol, int32 *off, int32 bytes);


// ===============================================
// = PIC READ FUNCTIONS =========================
// =============================================

Image *FF_PIC_load(char *path)
{
	FILE		*file = NULL;
	Image		*img;
	uint32		tmp, status;
	uint32		width, height;
	uint32		alpha = FALSE;
	uint8		chained;
	Channel		*channel = NULL;
	
	if((file = fopen(path, "rb")) == NULL) {
		fprintf(stderr, "ERROR: Can't open '%s' for reading.\n", path);
		goto error;
	}
	
	tmp = readInt(file);
	if(tmp != 0x5380F634) {		// 'S' + 845-1636 (SI's phone no :-)
		fprintf(stderr, "ERROR: '%s' has invalid magic number in the header.\n", path);
		goto error;
	}
	
	tmp = readInt(file);
	fseek(file, 88, SEEK_SET);	// Skip over creator string
	
	tmp = readInt(file);		// File identifier 'PICT'
	/* pdb - if(tmp != 'PICT') { */
   if (tmp != 0x50494354) {
		fprintf(stderr, "ERROR: '%s' is a Softimage file, but not a PIC file.\n", path);
		goto error;
	}
	
	width  = readShort(file);
	height = readShort(file);
	
	readInt(file);			// Aspect ratio (ignored)
	readShort(file);		// Interlace type (ignored)
	readShort(file);		// Read padding
	
	// Read channels
	do {
		Channel	*c;
		
		if(channel == NULL)
			channel = c = malloc(sizeof(Channel));
		else {
			c->next = malloc(sizeof(Channel));
			c = c->next;
		}
		c->next = NULL;
		
		chained = fgetc(file);
		c->size = fgetc(file);
		c->type = fgetc(file);
		c->channels = fgetc(file);
		
		// See if we have an alpha channel in there
		if(c->channels & PIC_ALPHA_CHANNEL)
			alpha = TRUE;
		
	} while(chained);
	
	// Create the image and the background layer
	img = IMG_new(width, height, 0);

	if(!readScanlines(file, img->bitmap, width, height, channel, alpha))
		goto error;
	
	status = TRUE;
	goto noerror;

error:
	status = FALSE;
noerror:
	
	while(channel) {
		Channel		*prev;

		prev = channel;
		channel = channel->next;
		free(prev);
	}
	if(!status) {
		IMG_delete(img);
		img = NULL;
	}
	
	fclose(file);
	return img;
}

static uint32 readScanlines(FILE *file, uint32 *image,
							 int32 width, int32 height, 
							 Channel *channel, uint32 alpha)
{
	int32		i;
	
	(void)alpha;
	
	for(i = height - 1; i >= 0; i--) {
		uint32	*scan = image + i * width;
		
		if(!readScanline(file, (uint8 *)scan, width, channel, 4))
			return FALSE;
	}
	
	return TRUE;
}

static uint32 readScanline(FILE *file, uint8 *scan, int32 width, Channel *channel,  int32 bytes)
{
	int32		noCol, status;
	int32		off[4];
	
	while(channel) {
		noCol = 0;
#ifndef sgi
		if(channel->channels & PIC_RED_CHANNEL) {
			off[noCol] = 0;
			noCol++;
		}
		if(channel->channels & PIC_GREEN_CHANNEL) {
			off[noCol] = 1;
			noCol++;
		}
		if(channel->channels & PIC_BLUE_CHANNEL) {
			off[noCol] = 2;
			noCol++;
		}
		if(channel->channels & PIC_ALPHA_CHANNEL) {
			off[noCol] = 3;
			noCol++;
		}
#else
		if(channel->channels & PIC_RED_CHANNEL) {
			off[noCol] = 3;
			noCol++;
		}
		if(channel->channels & PIC_GREEN_CHANNEL) {
			off[noCol] = 2;
			noCol++;
		}
		if(channel->channels & PIC_BLUE_CHANNEL) {
			off[noCol] = 1;
			noCol++;
		}
		if(channel->channels & PIC_ALPHA_CHANNEL) {
			off[noCol] = 0;
			noCol++;
		}
#endif
		
		switch(channel->type & 0x0F) {
			case PIC_UNCOMPRESSED:
				status = channelReadRaw(file, scan, width, noCol, off, bytes);
				break;
			case PIC_PURE_RUN_LENGTH:
				status = channelReadPure(file, scan, width, noCol, off, bytes);
				break;
			case PIC_MIXED_RUN_LENGTH:
				status = channelReadMixed(file, scan, width, noCol, off, bytes);
				break;
		}
		if(!status)
			break;
		
		channel = channel->next;
	}
	return status;
}

static uint32 channelReadRaw(FILE *file, uint8 *scan, int32 width, int32 noCol, int32 *off, int32 bytes)
{
	int			i, j;
	
	for(i = 0; i < width; i++) {
		if(feof(file))
			return FALSE;
		for(j = 0; j < noCol; j++)
			scan[off[j]] = (uint8)getc(file);
		scan += bytes;
	}
	return TRUE;
}

static uint32 channelReadPure(FILE *file, uint8 *scan, int32 width, int32 noCol, int32 *off, int32 bytes)
{
	uint8		col[4];
	int32		count;
	int			i, j, k;
	
	for(i = width; i > 0; ) {
		count = (unsigned char)getc(file);
		if(count > width)
			count = width;
		i -= count;
		
		if(feof(file))
			return FALSE;
		
		for(j = 0; j < noCol; j++)
			col[j] = (uint8)getc(file);
		
		for(k = 0; k < count; k++, scan += bytes) {
			for(j = 0; j < noCol; j++)
				scan[off[j] + k] = col[j];
		}
	}
	return TRUE;
}

static uint32 channelReadMixed(FILE *file, uint8 *scan, int32 width, int32 noCol, int32 *off, int32 bytes)
{
	int32	count;
	int		i, j, k;
	uint8	col[4];
	
	for(i = 0; i < width; i += count) {
		if(feof(file))
			return FALSE;
		
		count = (uint8)fgetc(file);
		
		if(count >= 128) {		// Repeated sequence
			if(count == 128)	// Long run
				count = readShort(file);
			else
				count -= 127;
			
			// We've run past...
			if((i + count) > width) {
				fprintf(stderr, "ERROR: FF_PIC_load(): Overrun scanline (Repeat) [%d + %d > %d] (NC=%d)\n", i, count, width, noCol);
				return FALSE;
			}
			
			for(j = 0; j < noCol; j++)
				col[j] = (uint8)fgetc(file);
			
			for(k = 0; k < count; k++, scan += bytes) {
				for(j = 0; j < noCol; j++)
					scan[off[j]] = col[j];
			}
		} else {				// Raw sequence
			count++;
			if((i + count) > width) {
				fprintf(stderr, "ERROR: FF_PIC_load(): Overrun scanline (Raw) [%d + %d > %d] (NC=%d)\n", i, count, width, noCol);
				return FALSE;
			}
			
			for(k = count; k > 0; k--, scan += bytes) {
				for(j = 0; j < noCol; j++)
					scan[off[j]] = (uint8)fgetc(file);
			}
		}
	}
	return TRUE;
}
