/**CFile***********************************************************************

  FileName    [intfBatch.c]

  PackageName [intf]

  Synopsis    [Run command in batch.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/scrollbar.h>
#include <fcntl.h>

#undef va_start

#include "intfInt.h"

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


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Read a script, run the commands and exit.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfBatch(char *fname)
{
FILE *fp, *fo;
network_t *net = NIL(network_t);
array_t *llib = NIL(array_t);
char buf[512];
char *data;
arch_graph_t *ag = NIL(arch_graph_t);
netl_clbl_t *clbl = NIL(netl_clbl_t);

  system("rm data.out");
  fp = fopen(fname, "r");
  if(fp == NIL(FILE)) {
    fprintf(stderr,"Couldn't open file %s for reading.\n", fname);
    exit(1);
  } 
  buf[0] = '\0';
  while(feof(fp) == 0) {
    fgets(buf, 512, fp);
    data = intfClearCmd(buf);
    if((data != NIL(char)) && (data[0] != '\0')) {
      (void) intf_Com(data, &net, &llib, &clbl, &ag, 1);
      fo = fopen("data.out","a");
      fprintf(fo,"command %s\n", data);
      (void) fclose(fo);
      FREE(data);
    }
    buf[0] = '\0';
  }      
  if(net != NIL(network_t)) {
    network_free(net);
  }
  if(llib != NIL(array_t)) {
    llib_Free(llib);
  }
  if(ag != NIL(arch_graph_t)) {
    arch_Free(ag);
  }
  if(clbl != NIL(netl_clbl_t)) {
    netl_ClblFree(clbl);
  }  
  intfMainResetGlobals();
  (void) fclose(fp);
  exit(0);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
