/* $RCSfile$
 * 
 * $Author$
 * $Date$
 * $Revision$
 * 
 * $Log$
 */

#ifndef UTILS_H
#define UTILS_H

#include "types.h"

// =============================================================
// = UTILITY MACROS ===========================================
// ===========================================================

#define MAX(a, b)	(((a) > (b))?(a):(b))
#define MIN(a, b)	(((a) < (b))?(a):(b))

#ifdef DEBUG
#define TRACE	printf
#else
#define TRACE	(1)?(0):printf
#endif

// =============================================================
// = UTILITY FUNCTIONS ========================================
// ===========================================================

uint32	readInt(FILE *file);
uint32	readShort(FILE *file);
void	writeInt(FILE *file, uint32 v);
void	writeShort(FILE *file, uint32 v);
char	*readStr(FILE *file);

#endif // UTILS_H
