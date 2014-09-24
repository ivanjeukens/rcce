/**CHeaderFile*****************************************************************

  FileName    [placeInt.h]

  PackageName [place]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _placeINT
#define _placeINT

#include <math.h>
#include "sis.h"
#include "comp.h"
#include "arch.h"
#include "netlInt.h"
#include "intf.h"
#include "util2.h"
#include "place.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define SMALL_NET 4
#define FROM 0
#define TO 1
#define MAX_PIN_PER_CLB 6
#define ERROR_TOL .001
#define MAX_MOVES 1000000

#define NOHEU 0
#define SIFT 1
#define GENERATIONAL 2
#define STEADYSTATE 3
#define TABU 4
#define SIMULATED_EVOLUTION 5
#define HYBRID1 6

#define INNER_NUM 1

#define FUNC(x) (normal(-log(x))/(x))

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct bb bb_t;
typedef struct allele allele_t;
typedef struct cromossome cromossome_t;
typedef struct population population_t;
typedef struct xy xy_t;
typedef struct gaconf gaconf_t;
typedef struct move move_t;
typedef struct tabuconf tabuconf_t;
typedef struct seconf seconf_t;
typedef struct greedyconf greedyconf_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct xy { 
  int x;
  int y;
  int nio;
};
/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct bb {
  int xmin;
  int ymin;
  int xmax;
  int ymax;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct allele {
  int x;
  int y;
  int nio;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct cromossome {
  array_t *al;
  float fitness;
  float cost;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct population {
  array_t *cromo;
  float maxfit;
  float avgfit;
  float minfit;
  float sumfit;
  float maxcost;
  int maxpos;
  float mincost;
  int minpos;
  float avgcost;
  double squared_sumcost;
  double stddev;
  int nx;
  int ny;
  int io_rat;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct gaconf {
  short scale;
  short select1;
  short select2;
  int popsize;
  float probcross;
  float probmut;
  int nsite;
  int niter;
  float scalecoef;
  short local;
  short localstart;
  float loclb;
  float loio;
  short fitness;
  short replacement;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct move {
  netl_clb_t *clbfrom;
  int from;
  netl_clb_t *clbto;
  int to;
  float delta_cost;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct tabuconf {
  short stopc;
  int maxite;
  int noimp;  
  short ternure;
  int ternurevalue;
  int ternuremax;
  int ternuremin;
  int clbexplore;
  int ioexplore;
  int neighsize;
  short exploretype;
  short attribute;
  short diversification;
  short intensification;
  int lowimp;
  float threshold;
  float probmut;
  short escape;
  short escapetype;
  short escapereset;
  int chaos;
  short localini;
  short localend;
  short localopt;
  float escapedecrease;
  int zeromut;
  float temp;
  int maximbalance;
  int doimbalance;
  int imbalance;
  short probselect;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct seconf {
  int niter;

};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct greedyconf {
  short first;
  short second;
  short third;
  short order;
};

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

EXTERN void placeStatistics(netl_clbl_t *clbl, array_t *netl);
EXTERN void placeGA(netl_clbl_t **clbl, array_t *netl, float *cost, int nx, int ny, int io_rat, short generational, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, int *net_move, netl_net_t **anets);
EXTERN int placeEncodePos(int x, int y, int nio, int nx, int ny, int io_rat);
EXTERN void placeDecodePos(int pos, int *x, int *y, int *nio, int nx, int ny, int io_rat);
EXTERN void placeH1(netl_clbl_t **clbl, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, float tini, float *costini, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
EXTERN void placeSift(netl_clbl_t **clbl, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, float *hfc, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
EXTERN void placeSwap(int maxnoimp, float rlim, float *cost, netl_clbl_t **clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move);
EXTERN void place(netl_clbl_t **clbl, array_t **netl, int nx, int ny, short heu, int io_rat);
EXTERN int placeAffectedNets(netl_net_t ***anets, array_t *netl, netl_clb_t *from, netl_clb_t *to, int **net_move);
EXTERN float placeCompCost(netl_clbl_t *clbl, array_t *netl, bb_t **bboxes, bb_t **bedges, int nx, int ny);
EXTERN void placeGetNUbb(netl_net_t *n, bb_t **box, int nx, int ny);
EXTERN void placeGetUbb(netl_net_t *n, bb_t **box, bb_t **edges, int nx, int ny);
EXTERN float placeNetCost(netl_net_t *net, bb_t *box);
EXTERN void placeUpdateBb(bb_t **bb_coord_new, bb_t **bb_edge_new, bb_t *bbox, bb_t *bedge, netl_net_t *n, int xold, int yold, int xnew, int ynew, int nx, int ny);
EXTERN void placeCheck(netl_clbl_t *clbl, array_t *netl, bb_t **bboxes, bb_t **bedges, int nx, int ny, int io_rat, float cost);
EXTERN void placeCluster(netl_clbl_t *clbl);
EXTERN void placeAnneal(netl_clbl_t **clbl, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, float tini, float *costini, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
EXTERN void placeIniPlaRand(netl_clbl_t **clbl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, int io_rat);
EXTERN float placeStartingT(netl_clbl_t *clbl, array_t *netl, int nx, int ny, float *cost, int max_moves, netl_clb_t ***clbs_occ, array_t ***io_occ, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
EXTERN double placeGetStdDev(int n, double sum_x_squared, double av_x);
EXTERN void placeFindTo(int x_from, int y_from, int type, float rlim, int *x_to, int *y_to, int nx, int ny);
EXTERN void placeSE(netl_clbl_t **clbl, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, float *hfc, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
EXTERN float * placeComputeOptCost(array_t *netl);
EXTERN void placeSelectOpt(netl_clbl_t *clbl, float *optimum);
EXTERN void placeTabu(netl_clbl_t **clbl, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, float *hfc, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);

/**AutomaticEnd***************************************************************/

#endif /* _ */
