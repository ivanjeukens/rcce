/**CFile***********************************************************************

  FileName    [placeMain.c]

  PackageName [place]

  Synopsis    [The core of the algorithm.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "placeInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define CWX 100
#define CWY 100
#define NINF 1000


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern st_table *blocks;
float **chanx_place_cost_fac, **chany_place_cost_fac;

const float cross_count[50] = {   
 1.0,    1.0,    1.0,    1.0828, 1.1536, 1.2206, 1.2823, 1.3385, 1.3991, 1.4493,
 1.4974, 1.5455, 1.5937, 1.6418, 1.6899, 1.7304, 1.7709, 1.8114, 1.8519, 1.8924,
 1.9288, 1.9652, 2.0015, 2.0379, 2.0743, 2.1061, 2.1379, 2.1698, 2.2016, 2.2334,
 2.2646, 2.2958, 2.3271, 2.3583, 2.3895, 2.4187, 2.4479, 2.4772, 2.5064, 2.5356,
 2.5610, 2.5864, 2.6117, 2.6371, 2.6625, 2.6887, 2.7148, 2.7410, 2.7671, 2.7933};

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static double PlaceT(netl_clbl_t *clbl, double cost, int nx, int ny, int io_rat, int Lm);
static float ComputeOffset(float p);
static float evaluate(float x);
static float midexp(float aa, float bb, int n);
static float normal(float x);
static void RemoveIoGaps(array_t ***io_occ, int io_rat, int nx, int ny);
static double GenerateSpace(netl_clbl_t *temp, int nx, int ny, double *sum_of_squares, int io_rat);
static void load_chan_fac(float ex, int nx, int ny);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [The main procedure.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
place(
  netl_clbl_t **clbl,
  array_t **netl,
  int nx,
  int ny,
  short heu,
  int io_rat)
{
netl_clb_t ***clbs_occ;
array_t ***io_occ;
int i,j;
float ht = 0., at, t, cost, tct = 0., ci;
array_t *ia;
bb_t **bboxes, **bedges;
bb_t **bbox_new, **bedge_new;
int *net_move;
netl_net_t **anets;
  
  clbs_occ = (netl_clb_t ***) ALLOC(netl_clb_t ***, nx);
  for(i = 0;i < nx;i++) {
    clbs_occ[i] = (netl_clb_t **) ALLOC(netl_clb_t **, ny);
  }
  for(i = 0;i < nx;i++) {
    for(j = 0;j < ny;j++) {
      clbs_occ[i][j] = NIL(netl_clb_t);
    }  
  }  
  io_occ = (array_t ***) ALLOC(array_t ***, 4);
  io_occ[0] = (array_t **) ALLOC(array_t **, ny);
  for(i = 0;i < ny;i++) {
    io_occ[0][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[0][i], j, NIL(netl_clb_t));
    }
  }
  io_occ[1] = (array_t **) ALLOC(array_t **, nx);
  for(i = 0;i < nx;i++) {
    io_occ[1][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[1][i], j, NIL(netl_clb_t));
    }    
  }
  io_occ[2] = (array_t **) ALLOC(array_t **, ny);
  for(i = 0;i < ny;i++) {
    io_occ[2][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[2][i], j, NIL(netl_clb_t));
    }    
  }
  io_occ[3] = (array_t **) ALLOC(array_t **, nx);
  for(i = 0;i < nx;i++) {
    io_occ[3][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[3][i], j, NIL(netl_clb_t));
    }    
  }
  load_chan_fac(1.0, nx, ny);
  bboxes = (bb_t **) ALLOC(bb_t *, array_n(*netl));
  bedges = (bb_t **) ALLOC(bb_t *, array_n(*netl));
  for(i = 0;i < array_n(*netl);i++) {
    bboxes[i] = (bb_t *) ALLOC(bb_t, 1);
    bedges[i] = (bb_t *) ALLOC(bb_t, 1);
  }  
  bbox_new = (bb_t **) ALLOC(bb_t *, MAX_PIN_PER_CLB*2);
  bedge_new = (bb_t **) ALLOC(bb_t *, MAX_PIN_PER_CLB*2);
  for(i = 0;i < MAX_PIN_PER_CLB*2;i++) {
    bbox_new[i] = (bb_t *) ALLOC(bb_t, 1);
    bedge_new[i] = (bb_t *) ALLOC(bb_t, 1);  
  }
  net_move = (int *) ALLOC(int, MAX_PIN_PER_CLB*2);
  anets = (netl_net_t **) ALLOC(netl_net_t *, MAX_PIN_PER_CLB*2);
  
  if(heu == SIFT) {
    placeIniPlaRand(clbl, nx, ny, clbs_occ, io_occ, io_rat);
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    ci = cost;
    printf("Initial cost = %f\n", cost);  
    util2_StartTimer();
    placeSift(clbl, *netl, nx, ny, clbs_occ, io_occ, &cost, io_rat, bbox_new,
      bedge_new, bboxes, bedges, net_move, anets);
    util2_StopTimer();
    ht = util2_GetRtime();
    util2_StartTimer();
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    printf("RECOMPUTED FROM SCRATCH: %f\n", cost);
    t = PlaceT(*clbl, cost, nx, ny, io_rat,  INNER_NUM *
      pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
      array_n((*clbl)->outputs) ,1.3333));
    util2_StopTimer();
    tct = util2_GetRtime();
  }
  else 
  if((heu == GENERATIONAL) || (heu == STEADYSTATE)) {
    util2_StartTimer();  
    placeGA(clbl, *netl, &cost, nx, ny, io_rat, heu, clbs_occ, io_occ,
      bboxes, bedges, bbox_new, bedge_new, net_move, anets);
    util2_StopTimer();
    ht = util2_GetRtime();
    util2_StartTimer();
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    printf("RECOMPUTED FROM SCRATCH: %f\n", cost);      
    t = PlaceT(*clbl, cost, nx, ny, io_rat,  INNER_NUM *
      pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
      array_n((*clbl)->outputs) ,1.3333));
    util2_StopTimer();
    tct = util2_GetRtime();
  }
  else
  if(heu == SIMULATED_EVOLUTION) {
    placeIniPlaRand(clbl, nx, ny, clbs_occ, io_occ, io_rat);
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    ci = cost;
    printf("Initial cost = %f\n", cost);      
    util2_StartTimer();
    placeSE(clbl, *netl, nx, ny, clbs_occ, io_occ, &cost, io_rat,
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);
    util2_StopTimer();
    ht = util2_GetRtime();
    util2_StartTimer();
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);    
    printf("RECOMPUTED FROM SCRATCH: %f\n", cost);
    t = PlaceT(*clbl, cost, nx, ny, io_rat,  INNER_NUM *
      pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
      array_n((*clbl)->outputs) ,1.3333));    
    util2_StopTimer();
    tct = util2_GetRtime();     
  }  
  else
  if(heu == NOHEU) {
    placeIniPlaRand(clbl, nx, ny, clbs_occ, io_occ, io_rat);
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    ci = cost;
    printf("Initial cost = %f\n", cost);  
    t = placeStartingT(*clbl, *netl, nx, ny, &cost, 
      INNER_NUM*pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
      array_n((*clbl)->outputs) ,1.3333), clbs_occ, io_occ, io_rat,
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);
  }

  if(heu == HYBRID1) {
    placeIniPlaRand(clbl, nx, ny, clbs_occ, io_occ, io_rat);
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    ci = cost;
    printf("Initial cost = %f\n", cost);  
    t = placeStartingT(*clbl, *netl, nx, ny, &cost, 
      INNER_NUM*pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
      array_n((*clbl)->outputs) ,1.3333), clbs_occ, io_occ, io_rat,
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);
    util2_StartTimer();  
    placeH1(clbl, *netl, nx, ny, clbs_occ, io_occ, t, &cost, io_rat, 
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);
    util2_StopTimer();
    at = util2_GetRtime();    
  }
  else
  if(heu == TABU) {
    placeIniPlaRand(clbl, nx, ny, clbs_occ, io_occ, io_rat);
    cost = placeCompCost(*clbl, *netl, bboxes, bedges, nx, ny);
    ci = cost;
    printf("Initial cost = %f\n", cost);  
    util2_StartTimer();
    placeTabu(clbl, *netl, nx, ny, clbs_occ, io_occ, &cost, io_rat,
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);
    netl_Free(*netl);
    *netl = netl_Extr(*clbl);
    util2_StopTimer();
    ht = util2_GetRtime();
  }
  else {
    util2_StartTimer();
    placeAnneal(clbl, *netl, nx, ny, clbs_occ, io_occ, t, &cost, io_rat, 
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);
    util2_StopTimer();
    at = util2_GetRtime();  
  }

  printf("\nTOTAL HEURISTIC TIME: %f(%f)\n", ht, ht/(ht + at + tct));
  printf("TOTAL TEMPERATURE COMPUTATION TIME: %f(%f)\n", tct,
    tct/(ht + at + tct));
  printf("TOTAL ANNEALING TIME: %f(%f)\n", at, at/(ht + at + tct));
  printf("TOTAL TIME: %f\n\n",ht+at+tct);
  printf("INITIAL COST: %f\n", ci);
  printf("FINAL COST: %f\n", cost);
  printf("IMPROVEMENT: %f\n", 100*(ci - cost)/ci);
  
  RemoveIoGaps(io_occ, io_rat, nx, ny);

  placeCheck(*clbl, *netl, bboxes, bedges, nx, ny, io_rat, cost);

  FREE(net_move);
  FREE(anets);
  for(i = 0;i < MAX_PIN_PER_CLB*2;i++) {
    FREE(bbox_new[i]);
    FREE(bedge_new[i]);
  }
  FREE(bbox_new);
  FREE(bedge_new);
  for(i = 0;i < array_n(*netl);i++) {
    FREE(bboxes[i]);
    FREE(bedges[i]);
  }
  FREE(bedges);
  FREE(bboxes);
  for(i = 0;i < nx;i++) {
    FREE(clbs_occ[i]);
  }
  FREE(clbs_occ);
  for(i = 0;i < ny;i++) {
    ia = io_occ[0][i];
    array_free(ia);
  }
  for(i = 0;i < nx;i++) {
    ia = io_occ[1][i];
    array_free(ia);
  }
  for(i = 0;i < ny;i++) {
    ia = io_occ[2][i];
    array_free(ia);
  }
  for(i = 0;i < nx;i++) {
    ia = io_occ[3][i];
    array_free(ia);
  }
  FREE(io_occ[0]);
  FREE(io_occ[1]);
  FREE(io_occ[2]);
  FREE(io_occ[3]);
  FREE(io_occ);
}

/**Function********************************************************************

  Synopsis           [Return an index to the affected nets by a block swap.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
placeAffectedNets(
  netl_net_t ***anets,
  array_t *netl,
  netl_clb_t *from,
  netl_clb_t *to,
  int **net_move)
{
int i, j;
netl_pin_t *pin;
int count = 0;

  for(i = 0;i < array_n(from->cb->in);i++) {
    pin = array_fetch(netl_pin_t *, from->cb->in, i);
    if((pin->net->global == 0) && (pin->skip == 0)) {
      (*anets)[count] = pin->net;
      (*net_move)[count] = FROM;
      count++;
    }
  }

  for(i = 0;i < array_n(from->cb->out);i++) {
    pin = array_fetch(netl_pin_t *, from->cb->out, i);
    if((pin->net->global == 0) && (pin->skip == 0)) {
      (*anets)[count] = pin->net;
      (*net_move)[count] = FROM;
      count++;
    }
  }

  if(to != NIL(netl_clb_t)) {
    for(i = 0;i < array_n(to->cb->in);i++) {
      pin = array_fetch(netl_pin_t *, to->cb->in, i);
      if((pin->net->global == 1) || (pin->skip == 1)) 
        continue;
      for(j = 0;j < count;j++) {
        if((*anets)[j] == pin->net) {
          break;
        }
      }
      if(j == count) {
        (*anets)[count] = pin->net;
        (*net_move)[count] = TO;
        count++;        
      }
    }  
    for(i = 0;i < array_n(to->cb->out);i++) {
      pin = array_fetch(netl_pin_t *, to->cb->out, i);
      if((pin->net->global == 1) || (pin->skip == 1)) 
         continue;       
      for(j = 0;j < count;j++) {
        if((*anets)[j] == pin->net) {
          break;
        }
      }
      if(j == count) {
        (*anets)[count] = pin->net;
        (*net_move)[count] = TO;
        count++;
      }
    }
  }

  return count;
}

/**Function********************************************************************

  Synopsis           [Compute the placement cost.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
float
placeCompCost(
  netl_clbl_t *clbl,
  array_t *netl,
  bb_t **bboxes,
  bb_t **bedges,
  int nx,
  int ny)
{
float cost = 0,result;
int i;
netl_net_t *n;

  for(i = 0; i < array_n(netl);i++) {  
    n = array_fetch(netl_net_t *, netl, i);
    if(n->global == 0) {
      if(array_n(n->snet) > SMALL_NET) {
        placeGetUbb(n, &bboxes[n->id], &bedges[n->id], nx, ny);
      }
      else {
        placeGetNUbb(n, &bboxes[n->id], nx, ny);
      }
      result = placeNetCost(n, bboxes[n->id]);
      cost += result;
      n->cost = result;
    }
  }
  
  return cost;
}

/**Function********************************************************************

  Synopsis           [Return the bounding box of a net. This version is for
  the small nets.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeGetNUbb(
  netl_net_t *n,
  bb_t **box,
  int nx,
  int ny)
{
netl_signet_t *sn;
int xmax, xmin, ymax, ymin, j;

  sn = array_fetch(netl_signet_t *, n->snet, 0);
  xmax = sn->clb->x;
  xmin = sn->clb->x;
  ymin = sn->clb->y;
  ymax = sn->clb->y;
  for(j = 1;j < array_n(n->snet);j++) {
    sn = array_fetch(netl_signet_t *, n->snet, j);      
    if(sn->clb->x > xmax) {
      xmax = sn->clb->x;
    }
    else
      if(sn->clb->x < xmin) {
        xmin = sn->clb->x;    
      }

    if(sn->clb->y > ymax) {
      ymax = sn->clb->y;
    }
    else
      if(sn->clb->y < ymin) {
        ymin = sn->clb->y;
      }
  } 
  (*box)->xmax = MAX(MIN(xmax, nx), 1);
  (*box)->ymax = MAX(MIN(ymax, ny), 1);
  (*box)->xmin = MAX(MIN(xmin, nx), 1);
  (*box)->ymin = MAX(MIN(ymin, ny), 1);
}

/**Function********************************************************************

  Synopsis           [Return the bounding box of a net.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeGetUbb(
  netl_net_t *n,
  bb_t **box,
  bb_t **edges,
  int nx,
  int ny)
{
int xmin, xmax, ymin, ymax, xmin_edge, ymin_edge, xmax_edge, ymax_edge;
netl_signet_t *sn;
int i, x, y;

  xmin_edge = 1;
  ymin_edge = 1;
  xmax_edge = 1;
  ymax_edge = 1;  
  sn = array_fetch(netl_signet_t *, n->snet, 0);
  xmin = MAX(MIN(sn->clb->x, nx), 1);
  ymin = MAX(MIN(sn->clb->y, ny), 1);
  xmax = xmin;
  ymax = ymin;
  
  for(i = 1;i < array_n(n->snet);i++) {
    sn = array_fetch(netl_signet_t *, n->snet, i);
    x = MAX(MIN(sn->clb->x, nx), 1);
    y = MAX(MIN(sn->clb->y, ny), 1);

    if(x == xmin) {  
      xmin_edge++;
    }
    if(x == xmax) {
      xmax_edge++;
    }
    else
      if(x < xmin) {
        xmin = x;
        xmin_edge = 1;
      }
      else
        if(x > xmax) {
          xmax = x;
          xmax_edge = 1;
        }

    if(y == ymin) {
      ymin_edge++;
    }
    if(y == ymax) {
      ymax_edge++;
    }
    else
      if(y < ymin) {
        ymin = y;
        ymin_edge = 1;
      }
      else
        if(y > ymax) {
          ymax = y;
          ymax_edge = 1;
        }
  }
  (*box)->xmin = xmin;
  (*box)->xmax = xmax;
  (*box)->ymin = ymin;
  (*box)->ymax = ymax;

  (*edges)->xmin = xmin_edge;
  (*edges)->xmax = xmax_edge;
  (*edges)->ymin = ymin_edge;
  (*edges)->ymax = ymax_edge; 
}

/**Function********************************************************************

  Synopsis           [Compute the cost of a net.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
float
placeNetCost(
  netl_net_t *net,
  bb_t *box)
{
float cost = 0.;
float cross;

  if(array_n(net->snet) > 50) {
    cross = 2.7933 + 0.02616 * (array_n(net->snet) - 50); 
  }
  else {
    cross = cross_count[array_n(net->snet) - 1];
  }    
  cost = (box->xmax - box->xmin + 1) * cross * 
    chanx_place_cost_fac[box->ymax][box->ymin - 1];
  cost += (box->ymax - box->ymin + 1) * cross *
    chany_place_cost_fac[box->xmax][box->xmin - 1];
 
  return cost;
}

/**Function********************************************************************

  Synopsis           [Update the bouding box of a net.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeUpdateBb(
  bb_t **bb_coord_new,
  bb_t **bb_edge_new,
  bb_t *bbox,
  bb_t *bedge,
  netl_net_t *n,
  int xold,
  int yold,
  int xnew,
  int ynew,
  int nx,
  int ny)
{
  xnew = MAX(MIN(xnew,nx),1);
  ynew = MAX(MIN(ynew,ny),1);
  xold = MAX(MIN(xold,nx),1);
  yold = MAX(MIN(yold,ny),1);

  if(xnew < xold) {
    if(xold == bbox->xmax) {
      if(bedge->xmax == 1) {
        placeGetUbb(n, bb_coord_new, bb_edge_new, nx, ny);
        return;
      }
      else {
        (*bb_edge_new)->xmax = bedge->xmax - 1;
        (*bb_coord_new)->xmax = bbox->xmax; 
      }
    }
    else {
       (*bb_coord_new)->xmax = bbox->xmax; 
       (*bb_edge_new)->xmax = bedge->xmax;
    }
    if(xnew < bbox->xmin) {
      (*bb_coord_new)->xmin = xnew;
      (*bb_edge_new)->xmin = 1;
    }
    else
      if(xnew == bbox->xmin) {
        (*bb_coord_new)->xmin = xnew;
        (*bb_edge_new)->xmin = bedge->xmin + 1;
      }    
      else {
        (*bb_coord_new)->xmin = bbox->xmin;
        (*bb_edge_new)->xmin = bedge->xmin;
      }
  }
  else
    if(xnew > xold) {
      if(xold == bbox->xmin) {
        if(bedge->xmin == 1) {
          placeGetUbb(n, bb_coord_new, bb_edge_new, nx, ny);
          return;
        }
        else {
          (*bb_edge_new)->xmin = bedge->xmin - 1;
          (*bb_coord_new)->xmin = bbox->xmin;
        }
      }
      else {
        (*bb_coord_new)->xmin = bbox->xmin;
        (*bb_edge_new)->xmin = bedge->xmin;
      }

      if(xnew > bbox->xmax) {
        (*bb_coord_new)->xmax = xnew;
        (*bb_edge_new)->xmax = 1;   
      }     
      else
        if(xnew == bbox->xmax) {
          (*bb_coord_new)->xmax = xnew;
          (*bb_edge_new)->xmax = bedge->xmax + 1;
        }
        else {
          (*bb_coord_new)->xmax = bbox->xmax; 
          (*bb_edge_new)->xmax = bedge->xmax;   
        }
    }
    else {
      (*bb_coord_new)->xmin = bbox->xmin;
      (*bb_coord_new)->xmax = bbox->xmax;
      (*bb_edge_new)->xmin = bedge->xmin;
      (*bb_edge_new)->xmax = bedge->xmax;
    }

  if(ynew < yold) {
    if(yold == bbox->ymax) {
      if(bedge->ymax == 1) {
        placeGetUbb(n, bb_coord_new, bb_edge_new, nx, ny);
        return;
      }
      else {
        (*bb_edge_new)->ymax = bedge->ymax - 1;
        (*bb_coord_new)->ymax = bbox->ymax;
      }
    }            
    else {
      (*bb_coord_new)->ymax = bbox->ymax;
      (*bb_edge_new)->ymax = bedge->ymax;
    }     
    if(ynew < bbox->ymin) {
      (*bb_coord_new)->ymin = ynew;
      (*bb_edge_new)->ymin = 1;
    }         
    else
      if(ynew == bbox->ymin) {
        (*bb_coord_new)->ymin = ynew;
        (*bb_edge_new)->ymin = bedge->ymin + 1;
      }    
      else {
        (*bb_coord_new)->ymin = bbox->ymin;
        (*bb_edge_new)->ymin = bedge->ymin;
      }     
  } 
  else
    if(ynew > yold) {
      if(yold == bbox->ymin) {
        if(bedge->ymin == 1) {
          placeGetUbb(n, bb_coord_new, bb_edge_new, nx, ny);
          return;
       }
       else {
         (*bb_edge_new)->ymin = bedge->ymin - 1;
         (*bb_coord_new)->ymin = bbox->ymin;
       }
     }       
     else {
       (*bb_coord_new)->ymin = bbox->ymin;
       (*bb_edge_new)->ymin = bedge->ymin;
     }
     if(ynew > bbox->ymax) {
       (*bb_coord_new)->ymax = ynew;
       (*bb_edge_new)->ymax = 1;
     }    
     else
       if(ynew == bbox->ymax) {
         (*bb_coord_new)->ymax = ynew;
         (*bb_edge_new)->ymax = bedge->ymax + 1;
       }     
       else {
         (*bb_coord_new)->ymax = bbox->ymax;
         (*bb_edge_new)->ymax = bedge->ymax;
       }
   }
   else {
     (*bb_coord_new)->ymin = bbox->ymin;
     (*bb_coord_new)->ymax = bbox->ymax;
     (*bb_edge_new)->ymin = bedge->ymin;
     (*bb_edge_new)->ymax = bedge->ymax;
   }
}

/**Function********************************************************************

  Synopsis           [Check the placement.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeCheck(
  netl_clbl_t *clbl,
  array_t *netl,
  bb_t **bboxes,
  bb_t **bedges,  
  int nx,
  int ny,
  int io_rat,
  float cost)
{
float cost_check;
int i;
netl_clb_t *clb;

 cost_check = placeCompCost(clbl, netl, bboxes, bedges, nx, ny);
 printf("Cost recomputed from scratch is %f.\n", cost_check);
 if(fabs(cost_check - cost) > cost * ERROR_TOL) {
   printf("Error: cost_check: %f and cost: %f differ in check_place.\n",
      cost_check, cost);
 }
 
 for(i = 0;i < array_n(clbl->clbs);i++) {
   clb = array_fetch(netl_clb_t *, clbl->clbs, i);
   if((clb->x > nx) || (clb->x < 0)) {
     printf("Error: invalid x coordinate at CLB %d\n", i);
   }
   if((clb->y > ny) || (clb->y < 0)) {
     printf("Error: invalid y coordinate at CLB %d\n", i);
   }
   if(clb->nio != -1) {
     printf("Error: invalid io number at CLB %d\n", i);   
   } 
 }

 for(i = 0;i < array_n(clbl->inputs);i++) {
   clb = array_fetch(netl_clb_t *, clbl->inputs, i); 
   if((clb->nio > io_rat) || (clb->nio < 0)) {
     printf("Error: invalid io number at INPUT %d\n", i);
   } 
 }
 
 for(i = 0;i < array_n(clbl->outputs);i++) {
   clb = array_fetch(netl_clb_t *, clbl->outputs, i);  
   if((clb->nio > io_rat) || (clb->nio < 0)) {
     printf("Error: invalid io number at OUTPUT %d\n", i);
   }
 }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Calculate the temperature of a placement.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static double
PlaceT(
  netl_clbl_t *clbl,
  double cost,
  int nx, 
  int ny,
  int io_rat,
  int Lm)
{
netl_clbl_t *temp;
double Eoo, STDoo, sum_of_squares, Yoo;

    temp = netl_ClblCopy(clbl);
    Eoo = GenerateSpace(temp, nx, ny, &sum_of_squares, io_rat);
    netl_ClblFree(temp);
    STDoo = placeGetStdDev(NINF, sum_of_squares, Eoo);
    printf("Eoo = %f\n", Eoo);
    printf("STDoo = %f\n", STDoo);
    Yoo = ComputeOffset(1 - (float) 1/Lm);
    printf("Yoo = %f\n", Yoo);
    
    return ((STDoo * STDoo)/(Eoo - cost - Yoo*STDoo));
}

/**Function********************************************************************

  Synopsis           [Compute the offset parameter.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static float
ComputeOffset(float p)
{
float x1, x2, besterr = 100, err, Lm, i, besti;
float startt, stopt;

  if(p < 0.9900) {
    startt = 1.0;
    stopt = 2.60;
  }
  else
    if(p < 0.9974) {
      startt = 2.60;
      stopt = 3.0;
    }
    else 
      if(p < 0.9988) {
        startt = 3.0;
        stopt = 3.3;
      }
      else
        if(p < 0.9998) {
          startt = 3.3;
          stopt = 3.7;
        }
        else {
          startt = 3.7;
          stopt = 5.0;
        }
    
  for(i = startt;i < stopt;i = i + 0.1) {
    x1 = evaluate(i);
    x2 = evaluate(-i);
    err = x1 - x2 - Lm;
    if(fabs(err) < fabs(besterr)) {
      besti = i;
      besterr = err;
    }  
  }
  
  return besti;
}

/**Function********************************************************************

  Synopsis           [Evaluate the integral using the midpoint algorithm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static float
evaluate(float x)
{
float res;
int i;

  for(i = 1;i < 10;i++) {  
    res = midexp(-x, 1, i);
  }  
  return res;
}

/**Function********************************************************************

  Synopsis           [Evaluate the integral using the midpoint algorithm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static float
midexp(
  float aa,
  float bb,
  int n)
{
float x, tnm, sum, del, ddel, b, a;
static float s;
static int it;
int j;

  b = exp(-aa);
  a = 0.0;
  if(n == 1) {
    it = 1;
    s = (b - a) * FUNC(0.5*(a+b));
    return s;
  }
  else {
    tnm = it;
    del = (b - a)/(3.0*tnm);
    ddel = del + del;
    x = a + 0.5 * del;
    sum = 0.0;
    for(j = 1;j <= it;j++) {
      sum += FUNC(x);
      x += ddel;
      sum += FUNC(x);
      x += del;
    }
    it *= 3;
    s = (s + (b - a)*sum/tnm)/3.0;
    return s;
  }
}

/**Function********************************************************************

  Synopsis           [Evaluate the integral using the midpoint algorithm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static float
normal(float x)
{
  return ((1/sqrt(2*PI)) * exp(-pow(x, 2)/2));
}

/**Function********************************************************************

  Synopsis           [Remove possible gaps in io blocks.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
RemoveIoGaps(
  array_t ***io_occ,
  int io_rat,
  int nx,
  int ny)
{
array_t *clbs;
netl_clb_t *clb, *clb2;
int next, current, i;
short ins;

  for(i = 0;i < ny;i++) {
    clbs = io_occ[0][i];
    next = 0;
    for(current = 0;current < array_n(clbs);current++) {
      clb = array_fetch(netl_clb_t *, clbs, current);
      ins = 0;
      if(clb == NIL(netl_clb_t)) {
        for(next = current+1;next < array_n(clbs);next++) {
          clb2 = array_fetch(netl_clb_t *, clbs, next);
          if(clb2 != NIL(netl_clb_t)) {
            ins = 1;
            clb2->nio = current;            
            array_insert(netl_clb_t *, clbs, current, NIL(netl_clb_t));
            break;
          }
        }
        if(ins == 0) break;
      }    
    }
  }

  for(i = 0;i < ny;i++) {
    clbs = io_occ[1][i];
    next = 0;
    for(current = 0;current < array_n(clbs);current++) {
      clb = array_fetch(netl_clb_t *, clbs, current);
      ins = 0;
      if(clb == NIL(netl_clb_t)) {
        for(next = current+1;next < array_n(clbs);next++) {
          clb2 = array_fetch(netl_clb_t *, clbs, next);
          if(clb2 != NIL(netl_clb_t)) {
            ins = 1;
            clb2->nio = current;            
            array_insert(netl_clb_t *, clbs, current, NIL(netl_clb_t));
            break;
          }
        }
        if(ins == 0) break;
      }    
    }
  }

  for(i = 0;i < ny;i++) {
    clbs = io_occ[2][i];
    next = 0;
    for(current = 0;current < array_n(clbs);current++) {
      clb = array_fetch(netl_clb_t *, clbs, current);
      ins = 0;
      if(clb == NIL(netl_clb_t)) {
        for(next = current+1;next < array_n(clbs);next++) {
          clb2 = array_fetch(netl_clb_t *, clbs, next);
          if(clb2 != NIL(netl_clb_t)) {
            ins = 1;
            clb2->nio = current;            
            array_insert(netl_clb_t *, clbs, current, NIL(netl_clb_t));
            break;
          }
        }
        if(ins == 0) break;
      }    
    }
  }

  for(i = 0;i < nx;i++) {
    clbs = io_occ[3][i];
    next = 0;
    for(current = 0;current < array_n(clbs);current++) {
      clb = array_fetch(netl_clb_t *, clbs, current);
      ins = 0;
      if(clb == NIL(netl_clb_t)) {
        for(next = current+1;next < array_n(clbs);next++) {
          clb2 = array_fetch(netl_clb_t *, clbs, next);
          if(clb2 != NIL(netl_clb_t)) {
            ins = 1;
            clb2->nio = current;            
            array_insert(netl_clb_t *, clbs, current, NIL(netl_clb_t));
            break;
          }
        }
        if(ins == 0) break;
      }    
    }
  }
} 

/**Function********************************************************************

  Synopsis           [Generate NINF random solutions to find the espected
  cost over the solution space. Used in determining the initial TSSA
  temperature.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static double
GenerateSpace(
  netl_clbl_t *temp,
  int nx, 
  int ny,
  double *sum_of_squares, 
  int io_rat)
{
int i, j;
netl_clb_t ***clbs_occ;
array_t ***io_occ;
double cost, ret = 0.;
array_t *tn;
bb_t **bboxes, **bedges;

  tn = netl_Extr(temp);
  clbs_occ = (netl_clb_t ***) ALLOC(netl_clb_t ***, nx);
  for(i = 0;i < nx;i++) {
    clbs_occ[i] = (netl_clb_t **) ALLOC(netl_clb_t **, ny);
  }
  for(i = 0;i < nx;i++) {
    for(j = 0;j < ny;j++) {
      clbs_occ[i][j] = NIL(netl_clb_t);
    }  
  }  
  io_occ = (array_t ***) ALLOC(array_t ***, 4);
  io_occ[0] = (array_t **) ALLOC(array_t **, ny);
  for(i = 0;i < ny;i++) {
    io_occ[0][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[0][i], j, NIL(netl_clb_t));
    }
  }
  io_occ[1] = (array_t **) ALLOC(array_t **, nx);
  for(i = 0;i < nx;i++) {
    io_occ[1][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[1][i], j, NIL(netl_clb_t));
    }    
  }
  io_occ[2] = (array_t **) ALLOC(array_t **, ny);
  for(i = 0;i < ny;i++) {
    io_occ[2][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[2][i], j, NIL(netl_clb_t));
    }    
  }
  io_occ[3] = (array_t **) ALLOC(array_t **, nx);
  for(i = 0;i < nx;i++) {
    io_occ[3][i] = array_alloc(netl_clb_t *, io_rat);
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[3][i], j, NIL(netl_clb_t));
    }    
  }

  bboxes = (bb_t **) ALLOC(bb_t *, array_n(tn));
  bedges = (bb_t **) ALLOC(bb_t *, array_n(tn));
  for(i = 0;i < array_n(tn);i++) {
    bboxes[i] = (bb_t *) ALLOC(bb_t, 1);
    bedges[i] = (bb_t *) ALLOC(bb_t, 1);
  }    
  *sum_of_squares = 0.;
  for(i = 0;i < NINF;i++) {
    placeIniPlaRand(&temp, nx, ny, clbs_occ, io_occ, io_rat);
    cost = placeCompCost(temp, tn, bboxes, bedges, nx, ny);
    ret += cost;
    *sum_of_squares += cost * cost;  
  }

  for(i = 0;i < array_n(tn);i++) {
    FREE(bboxes[i]);
    FREE(bedges[i]);
  }
  FREE(bedges);
  FREE(bboxes);
  
  netl_Free(tn);
  for(i = 0;i < nx;i++) {  
    FREE(clbs_occ[i]);
  }
  FREE(clbs_occ);  
  for(i = 0;i < ny;i++) {
    tn = io_occ[0][i];
    array_free(tn);
  }
  for(i = 0;i < nx;i++) {
    tn = io_occ[1][i];
    array_free(tn);
  }
  for(i = 0;i < ny;i++) {
    tn = io_occ[2][i];
    array_free(tn);
  }
  for(i = 0;i < nx;i++) {
    tn = io_occ[3][i];
    array_free(tn);
  }
  FREE(io_occ[0]);
  FREE(io_occ[1]);
  FREE(io_occ[2]);
  FREE(io_occ[3]);
  FREE(io_occ);

  return ret/NINF;
}

/**Function********************************************************************

  Synopsis           [Set the channel factor of the cost function.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
load_chan_fac(
  float ex,
  int nx,
  int ny)
{
int low, high;

  chanx_place_cost_fac = (float **) ALLOC(float *, ny + 1);
  for(low = 0;low <= ny;low++) {
    chanx_place_cost_fac[low] = (float *) ALLOC(float, low + 1);
  }

  chany_place_cost_fac = (float **) ALLOC(float *, nx + 1);
  for(low = 0;low <= nx;low++) {
    chany_place_cost_fac[low] = (float *) ALLOC(float, low + 1);
  }

  chanx_place_cost_fac[0][0] = CWX;

  for(high = 1;high <= ny;high++) {
    chanx_place_cost_fac[high][high] = CWX;    
    for(low = 0;low < high;low++) {
      chanx_place_cost_fac[high][low] = chanx_place_cost_fac[high-1][low] + CWX;
     }
  }
 
  for(high = 0;high <= ny;high++) 
    for(low = 0;low <= high;low++) {
      chanx_place_cost_fac[high][low] = (high - low + 1.)/ 
        chanx_place_cost_fac[high][low];
      chanx_place_cost_fac[high][low] = 
        pow((double) chanx_place_cost_fac[high][low], (double) ex);
    }
 
  chany_place_cost_fac[0][0] = CWY;
 
  for(high = 1;high <= nx;high++) {
    chany_place_cost_fac[high][high] = CWY;
    for(low = 0;low < high;low++) {
      chany_place_cost_fac[high][low] = chany_place_cost_fac[high-1][low] + CWY;
    }
  }
  
  for(high = 0;high <= nx;high++) 
    for(low = 0;low <= high;low++) {
      chany_place_cost_fac[high][low] = (high - low + 1.) / 
        chany_place_cost_fac[high][low]; 
      chany_place_cost_fac[high][low] = 
        pow((double) chany_place_cost_fac[high][low], (double) ex);
     }    
}
