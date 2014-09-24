/**CHeaderFile*****************************************************************

  FileName    [llibInt.h]

  PackageName [llib]

  Synopsis    [Internal declarations of a logic library.]

  Description []

  SeeAlso     [llib]

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _LLIBINT
#define _LLIBINT

#include "sis.h"
#include "comp.h"
#include "arch.h"
#include "llib.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Logic library structure. It's composed of a network describing
  the logic of the component, the compnent library where the component came
  from, and the input/output pins of the component.]

  Description []

  SeeAlso     []

******************************************************************************/
struct logic_library {
  char *label;
  char *lib;
  array_t *in;
  array_t *out;
  network_t *net;
};


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/

#endif /* _ */
