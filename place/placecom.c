/**CFile***********************************************************************

  FileName    [placeCom.c]

  PackageName [place]

  Synopsis    [Placement command.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include <math.h>
#include "placeInt.h" 

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

  Synopsis           [The command for annealing procedure.]

  Description        []

  SideEffects        []

  SeeAlso            []

  CommandName        [place] 

  CommandSynopsis    [-a [automatic arch]  -v -t]

  CommandArguments   [] 

  CommandDescription []  

******************************************************************************/
int
place_Com(
  netl_clbl_t **clbl,
  arch_graph_t *ag,
  int argc,
  char **argv)
{
array_t *netl;
short vpr = 0, noarch = 0, heu = NOHEU;
int c, nx, ny, io_rat = 2;
char name[100];
float aspect_ratio = 1.;
int seed = 1;

  if(*clbl == NIL(netl_clbl_t)) {
    fprintf(siserr,"No clb list loaded.\n");
    return 1;
  }
  util_getopt_reset();
  while((c = util_getopt(argc, argv, "vah:s:")) != EOF) {
    switch (c) {
      case 'v': {
        vpr = 1;
      } break;      
      case 'a': {
        noarch = 1;
      } break;
      case 'h': {
        if(strcmp(util_optarg, "sift") == 0) {
          heu = SIFT;
        }
        else
        if(strcmp(util_optarg, "generational") == 0) {
          heu = GENERATIONAL;
        }
        else
        if(strcmp(util_optarg, "steadystate") == 0) {
          heu = STEADYSTATE;
        }
        else
        if(strcmp(util_optarg, "tabu") == 0) {
          heu = TABU;
        }
        else
        if(strcmp(util_optarg, "se") == 0) {
          heu = SIMULATED_EVOLUTION;
        }
        else
        if(strcmp(util_optarg, "h1") == 0) {
          heu = HYBRID1;
        }
      } break;
      case 's': {
        seed = atoi(util_optarg);        
      } break;
      default: Usage();
    }
  }  
  srand48(seed);  
  if((ag == NIL(arch_graph_t)) && (noarch == 0)) {
    fprintf(siserr,"No architecture loaded.\n");
    return 1;    
  }
  if(noarch == 0) return 0;

  if(noarch == 1) {
    ny = (int) ceil (sqrt((double)(array_n((*clbl)->clbs)/aspect_ratio)));
    c = (int) ceil((array_n((*clbl)->inputs) + array_n((*clbl)->outputs))/
      (2 * io_rat * (1. + aspect_ratio)));
    ny = MAX(ny, c);
    nx = (int) ceil(ny*aspect_ratio);  
  }
  else {
  
  }  
  netl = netl_Extr(*clbl);
  
  placeStatistics(*clbl, netl);
  
  place(clbl, &netl, nx, ny, heu, io_rat);
  netl_Free(netl);

  if(vpr == 1) {
    strcpy(name, (*clbl)->name);
    strcat(name,".placed");
    netl_VprPlaceWrite(*clbl, nx, ny, name);
  }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Initialize the place command.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
place_Init()
{
  COM_ADD_COMMAND_PLAC("place", place_Com);
}

/**Function********************************************************************

  Synopsis           [Exit function]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
place_End()
{

}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Print the usage of the place command.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeStatistics(
  netl_clbl_t *clbl,
  array_t *netl)
{
int i, maxnet = -1, minnet = INFINITY, size;
float avgnet = 0.;
netl_net_t *n;

  size = array_n(clbl->clbs) + array_n(clbl->inputs) +
    array_n(clbl->outputs);
  printf("CLBS = %d(%f)\tINPUTS = %d(%f)\tOUTPUTS = %d(%f)\n",
    array_n(clbl->clbs), (float) array_n(clbl->clbs)/size,
    array_n(clbl->inputs), (float) array_n(clbl->inputs)/size,
    array_n(clbl->outputs), (float) array_n(clbl->outputs)/size);
  
  for(i = 0;i < array_n(netl);i++) {
    n = array_fetch(netl_net_t *, netl, i);
    size = array_n(n->snet);
    if(size > maxnet) {
      maxnet = size;
    }
    if(size < minnet) {
      minnet = size;
    }
    avgnet += size;
  }  
  avgnet /= array_n(netl);
  printf("Max.Net = %d\tMin.Net = %d\tAvg.Net = %f\n", maxnet, minnet,
    avgnet);
  size = array_n(clbl->clbs) + array_n(clbl->inputs) +
    array_n(clbl->outputs);    
  printf("Nets = %d\tProp = %f\n", array_n(netl), 
    (float) avgnet*array_n(netl)/size);    
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Print the usage of the place command.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Usage()
{
  fprintf(siserr,"usage: place [-n -a -t]\n\n");
  fprintf(siserr,"\t-v Write vpr placement file\n");
  fprintf(siserr,"\t-a Automatic sizing of the FPGA\n");
  fprintf(siserr,"\t-s Use the sifting heuristic as first stage.\n");
  fprintf(siserr,"\t-g Use the genetic algorithm as first stage.\n");
}
