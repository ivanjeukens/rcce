/**CHeaderFile*****************************************************************

  FileName    [dagmInt.h]

  PackageName [dagm]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      [1996-1997 Ivan Jeukens]

  Copyright   []

  Revision    [$Id: $]

******************************************************************************/

#ifndef _DAGMINT
#define _DAGMINT

#include "sis.h"
#include "netlInt.h"
#include "llibInt.h"
#include "dagm.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN network_t * dagmRun(network_t *network, int k);

/**AutomaticEnd***************************************************************/

#endif /* _ */