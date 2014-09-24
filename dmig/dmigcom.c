/**CFile***********************************************************************

  FileName    [dmigCom.c]

  PackageName [dmig]

  Synopsis    [The command interface for the dmig algorithm.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "dmigInt.h"
#include "intf.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Procedure implementing the dmig command.]

  Description        []

  SideEffects        []

  SeeAlso            []

  CommandName        [dmig]

  CommandSynopsis    []  

  CommandArguments   []

  CommandDescription []

******************************************************************************/
int
dmig_Com(
  network_t **network,
  int argc,
  char **argv)
{
 
  dmigRun(*network);
  
  return 0;
}

/**Function********************************************************************

  Synopsis           [Initialization procedure.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
dmig_Init()
{
  COM_ADD_COMMAND_SIS("dmig", dmig_Com, "decomp");
}

/**Function********************************************************************

  Synopsis           [Cleanup procedure.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
dmig_End()
{

}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
