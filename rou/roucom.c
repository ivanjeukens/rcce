/**CFile***********************************************************************

  FileName    [rouCom.c]

  PackageName [rou]

  Synopsis    [Command interface for the routing package.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   []

******************************************************************************/

#include "rouInt.h" 

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

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
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void Usage();

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [The command for routing procedure]

  Description        []

  SideEffects        []

  SeeAlso            []

  CommandName        [rou]

  CommandSynopsis    [-a -v]

  CommandArguments   [] 

  CommandDescription []  

******************************************************************************/
int
rou_Com(
  netl_clbl_t **clbl,
  int argc,
  char **argv)
{
array_t *netl;
short vpr = 0, noarch = 0;
int c, nx, ny, io_rat = 2, verify = 1;
float aspect_ratio = 1.;

  if(*clbl == NIL(netl_clbl_t)) {
    fprintf(siserr,"No clb list loaded.\n");
    return 1;
  }
  util_getopt_reset();
  while((c = util_getopt(argc, argv, "vabi:")) != EOF) {
    switch (c) {
      case 'v': {
        vpr = 1;
      } break;      
      case 'a': {
        noarch = 1;
      } break;
      case 'i': {
        io_rat = atoi(util_optarg);
      } break;
      case 'b': {
        verify = 1;
      } break;
      default: Usage();
    }
  }  

  ny = (int) ceil (sqrt((double)(array_n((*clbl)->clbs)/aspect_ratio)));
  c = (int) ceil((array_n((*clbl)->inputs) + array_n((*clbl)->outputs))/
    (2 * io_rat * (1. + aspect_ratio)));
  ny = MAX(ny, c);
  nx = (int) ceil(ny*aspect_ratio);  

  netl = netl_Extr(*clbl);
  rouMain(*clbl, netl, nx, ny, io_rat, vpr, verify);
  netl_Free(netl);

  return 0;
}

/**Function********************************************************************

  Synopsis           [Initialize the vprsa command.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
rou_Init()
{
  COM_ADD_COMMAND_ROU("rou", rou_Com);
}

/**Function********************************************************************

  Synopsis           [Exit function]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
rou_End()
{

}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Print the usage of the vprsa command.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Usage()
{
  fprintf(siserr,"usage: rou [ -v -a -i [io_rat] ]\n\n");
  fprintf(siserr,"\t-v Write vpr route file\n");
  fprintf(siserr,"\t-a Automatic sizing of the FPGA\n");
  fprintf(siserr,"\t-i Number of pad in an io position\n");
}


