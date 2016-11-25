/* $RCSfile$
 * 
 * $Author$
 * $Date$
 * $Revision$
 * 
 * $Log$
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "utils.h"
#include "image.h"

// =============================================================
// = PRIVATE FUNCTIONS ========================================
// ===========================================================

static bool ff_pic_writeScanline(FILE *file, uint32 *line, uint32 width)
{
	int		same, seqSame, count;
	int		i, k;
	uint8	pixel[128][3], col[3];
	
	count = 0;
	for(k = 0; k < width; k++) {
		col[0] = (line[k]) & 0xFF;
		col[1] = (line[k] >> 8) & 0xFF;
		col[2] = (line[k] >> 16) & 0xFF;
		
		if(count == 0) {
			pixel[0][0] = col[0];
			pixel[0][1] = col[1];
			pixel[0][2] = col[2];
			count++;
		} else
		if(count == 1) {
			seqSame  = (col[0] == pixel[0][0]);
			seqSame &= (col[1] == pixel[0][1]);
			seqSame &= (col[2] == pixel[0][2]);
			
			if(!seqSame) {
				pixel[count][0] = col[0];
				pixel[count][1] = col[1];
				pixel[count][2] = col[2];
			}
			count++;
		} else
		if(count > 1) { 
			if(seqSame) {
				same  = (col[0] == pixel[0][0]);
				same &= (col[1] == pixel[0][1]);
				same &= (col[2] == pixel[0][2]);
			} else {
				same  = (col[0] == pixel[count - 1][0]);
				same &= (col[1] == pixel[count - 1][1]);
				same &= (col[2] == pixel[count - 1][2]);
			}
			
			if(same ^ seqSame) {
				if(!seqSame) {
					putc((uint8)(count - 2), file);
					for(i = 0; i < count - 1; i++) {
						putc(pixel[i][0], file);
						putc(pixel[i][1], file);
						putc(pixel[i][2], file);
					}
					pixel[0][0] = pixel[1][0] = col[0];
					pixel[0][1] = pixel[1][1] = col[1];
					pixel[0][2] = pixel[1][2] = col[2];
					count = 2;
					seqSame = TRUE;
				} else {
					if(count < 128)
						putc((uint8)(count + 127), file);
					else {
						putc(128, file);
						writeShort(file, count);
					}
					putc(pixel[0][0], file);
					putc(pixel[0][1], file);
					putc(pixel[0][2], file);
					pixel[0][0] = col[0];
					pixel[0][1] = col[1];
					pixel[0][2] = col[2];
					count = 1;
				}
			} else {
				if(!same) {
					pixel[count][0] = col[0];
					pixel[count][1] = col[1];
					pixel[count][2] = col[2];
				}
				count++;
				if((count == 128) && !seqSame) {
					putc(127, file);
					for(i = 0; i < count; i++) {
						putc(pixel[i][0], file);
						putc(pixel[i][1], file);
						putc(pixel[i][2], file);
					}
					count = 0;
				}
				if((count == 65536) && seqSame) {
					putc(128, file);
					writeShort(file, count);
					putc(pixel[0][0], file);
					putc(pixel[0][1], file);
					putc(pixel[0][2], file);
					count = 0;
				}
			}
		}
		if(ferror(file))
			return FALSE;
	}
	if(count) {
		if((count == 1) || (!seqSame)) {
			putc((uint8)(count - 1), file);
			for(i = 0; i < count; i++) {
				putc(pixel[i][0], file);
				putc(pixel[i][1], file);
				putc(pixel[i][2], file);
			}
		} else {
			if(count < 128)
				putc((uint8)(count + 127), file);
			else {
				putc(128, file);
				writeShort(file, count);
			}
			putc(pixel[0][0], file);
			putc(pixel[0][1], file);
			putc(pixel[0][2], file);
		}
		if(ferror(file))
			return FALSE;
	}

	count = 0;
	for(k = 0; k < width; k++) {
		col[0] = (line[k] >> 24) & 0xFF;
		
		if(count == 0) {
			pixel[0][0] = col[0];
			count++;
		} else
		if(count == 1) {
			seqSame  = (col[0] == pixel[0][0]);
			
			if(!seqSame) {
				pixel[count][0] = col[0];
			}
			count++;
		} else
		if(count > 1) { 
			if(seqSame) {
				same  = (col[0] == pixel[0][0]);
			} else {
				same  = (col[0] == pixel[count - 1][0]);
			}
			
			if(same ^ seqSame) {
				if(!seqSame) {
					putc((uint8)(count - 2), file);
					for(i = 0; i < count - 1; i++) {
						putc(pixel[i][0], file);
					}
					pixel[0][0] = pixel[1][0] = col[0];
					count = 2;
					seqSame = TRUE;
				} else {
					if(count < 128)
						putc((uint8)(count + 127), file);
					else {
						putc(128, file);
						writeShort(file, count);
					}
					putc(pixel[0][0], file);
					pixel[0][0] = col[0];
					count = 1;
				}
			} else {
				if(!same) {
					pixel[count][0] = col[0];
				}
				count++;
				if((count == 128) && !seqSame) {
					putc(127, file);
					for(i = 0; i < count; i++) {
						putc(pixel[i][0], file);
					}
					count = 0;
				}
				if((count == 65536) && seqSame) {
					putc(128, file);
					writeShort(file, count);
					putc(pixel[0][0], file);
					count = 0;
				}
			}
		}
		if(ferror(file))
			return FALSE;
	}
	if(count) {
		if((count == 1) || (!seqSame)) {
			putc((uint8)(count - 1), file);
			for(i = 0; i < count; i++) {
				putc(pixel[i][0], file);
			}
		} else {
			if(count < 128)
				putc((uint8)(count + 127), file);
			else {
				putc(128, file);
				writeShort(file, count);
			}
			putc(pixel[0][0], file);
		}
		if(ferror(file))
			return FALSE;
	}
	
	return TRUE;
}

// =============================================================
// = PUBLIC FUNCTIONS =========================================
// ===========================================================

bool FF_PIC_save(char *path, Image *image)
{
	FILE	*file;
	char	str[80], myPath[4096];
	int		line;
	
	strcpy(myPath, path);
	if(strlen(myPath) < 4 || strcasecmp(myPath + strlen(myPath) - 4, ".pic") != 0)
		strcat(myPath, ".pic");
	
	if((file = fopen(myPath, "wb")) == NULL) {
		fprintf(stderr, "ERROR: Couldn't open '%s' for writing. Reason: %s\n", myPath, strerror(errno));
		return FALSE;
	}
	
	// Write Softimage file header
	writeInt(file, 0x5380F634);
	writeInt(file, 0x406001A3);
	memset(str, 0, 80);
	sprintf(str, "File written by slm. (C) Pison Ltd. 1999");
	fwrite(str, 1, 80, file);
	
	// Write picture file header
	writeInt(file, 0x50494354);
	writeShort(file, image->width);
	writeShort(file, image->height);
	writeInt(file, 0x3F800000);
	writeShort(file, 3);
	writeShort(file, 0);
	
	// Info for RGB stream.
	fputc(1, file);
	fputc(8, file);
	fputc(2, file);
	fputc(0xE0, file);
	
	// Info for alpha channel stream
	fputc(0, file);
	fputc(8, file);
	fputc(2, file);
	fputc(0x10, file);
	
	if(ferror(file)) {
		fprintf(stderr, "ERROR: Couldn't write out to '%s'. Reason: %s\n", path, strerror(errno));
		goto error;
	}
	
	for(line = image->height - 1; line >= 0; line--) {
		uint32		*linePtr;
		
		linePtr = image->bitmap + line * image->width;
		
		if(!ff_pic_writeScanline(file, linePtr, image->width)) {
			fprintf(stderr, "ERROR: Couldn't write out to '%s'. Reason: %s\n", path, strerror(errno));
		}
	}
	fclose(file);
	return TRUE;

error:
	fclose(file);
	return FALSE;
}

