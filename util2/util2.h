/**CHeaderFile*****************************************************************

  FileName    [util2.h]

  PackageName [util2]

  Synopsis    [This package contain generic utilities.]

  Description [This package offers additional utilities to the sis util
  package. This functions are generic and should be kept together.]

  SeeAlso     [util]

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _UTIL2
#define _UTIL2

#include "ansi.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define UTIL2_MYRANDOM_MAX 2147483647


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN short util2_IsInt(char *number);
EXTERN short util2_IsFloat(char *number);
EXTERN int util2_Irand(int imax);
EXTERN float util2_Frand();
EXTERN short util2_ValidId(char *s);
EXTERN void util2_StartTimer();
EXTERN void util2_StopTimer();
EXTERN double util2_GetUtime();
EXTERN double util2_GetStime();
EXTERN double util2_GetRtime();
EXTERN short util2_InLine(int x, int y, int x1, int y1, int x2, int y2);
EXTERN short util2_InRect(int x, int y, int x1, int y1, int width, int height);

/**AutomaticEnd***************************************************************/

#endif /* _ */
