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

#include "types.h"

#include "utils.h"

uint32 readInt(FILE *file)
{
	uint32	v;
	uint8	c;
	
	v = 0;
	c = fgetc(file);
	v |= (c << 24);
	c = fgetc(file);
	v |= (c << 16);
	c = fgetc(file);
	v |= (c << 8);
	c = fgetc(file);
	v |= c;
	
	return v;
}

uint32 readShort(FILE *file)
{
	int32	v;
	uint8	c;

	v = 0;
	c = fgetc(file);
	v |= (c << 8);
	c = fgetc(file);
	v |= c;
	
	return v;
}

void writeInt(FILE *file, uint32 v)
{
	fputc(((v >> 24) & 0xFF), file);
	fputc(((v >> 16) & 0xFF), file);
	fputc(((v >> 8) & 0xFF), file);
	fputc((v & 0xFF), file);
}

void writeShort(FILE *file, uint32 v)
{
	fputc((v >> 8) & 0xFF, file);
	fputc(v & 0xFF, file);
}

char *readStr(FILE *file)
{
	char	*result;
	int		size, med, c;
	
	med = size = 0;
	result = malloc(16);
	
	c = fgetc(file);
	
	if(c == EOF)
		return NULL;
	
	while(!(c == EOF || c == '\n' || c == '\0')) {
		if(size == med) {
			med += 64;
			result = realloc(result, med);
		}
		result[size++] = (char)c;
		c = fgetc(file);
	}
	
	// Trim allocated buffer to contain only the string and terminating '\0'
	result = realloc(result, size + 1);

	result[size] = '\0';
	
	return result;
}
