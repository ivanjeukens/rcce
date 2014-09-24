/**CHeaderFile*****************************************************************

  FileName    [llib.h]

  PackageName [llib]

  Synopsis    [This package is a specification for a logic library. Each
  library element is a description of a logic component of an FPGA. The
  library is extracted from the architecture graph, in combination with
  the corresponding component library.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _LLIB
#define _LLIB

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct logic_library llib_t;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void llib_Free(array_t *l);
EXTERN int llib_KlutFind(array_t *llib, int k);
EXTERN array_t * llib_Create(arch_graph_t *ag);
EXTERN int llib_PadFind(array_t *llib);

/**AutomaticEnd***************************************************************/

#endif /* _ */
