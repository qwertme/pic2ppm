/* $RCSfile$
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 * $Log$
 */

#ifndef TYPES_H
#define TYPES_H

// =============================================================
// = #DEFINES =================================================
// ===========================================================

#ifndef TRUE
#define TRUE		(1)
#endif

#ifndef FALSE
#define FALSE		(0)
#endif

typedef int					bool;

// =============================================================
// = TYPEDEFS =================================================
// ===========================================================

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
//#ifdef unix
//typedef unsigned long long	uint64;
//#else
//typedef unsigned __int64	uint64;
//#endif

typedef signed char			int8;
typedef signed short		int16;
typedef signed int			int32;
//#ifdef unix
//typedef signed long long	int64;
//#else
//typedef signed __int64		int64;
//#endif


#endif // TYPES_H
