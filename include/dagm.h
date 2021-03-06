/**CHeaderFile*****************************************************************

  FileName    [dagm.h]

  PackageName [dagm]

  Synopsis    [Implement the dagmap algorithm.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

  Revision    [$Id: $]

******************************************************************************/

#ifndef _DAGM
#define _DAGM

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

EXTERN int dagm_Com(network_t **network, array_t *llib, int argc, char **argv);
EXTERN void dagm_Init();
EXTERN void dagm_End();

/**AutomaticEnd***************************************************************/

#endif /* _ */
