/**CFile***********************************************************************

  FileName    [placeSE.c]

  PackageName [place]

  Synopsis    [Simulated Evolution]

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

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern float **chanx_place_cost_fac;
extern float **chany_place_cost_fac;
extern const float cross_count[];

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeSE(
  netl_clbl_t **clbl,
  array_t *netl,
  int nx,
  int ny,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  float *hfc,
  int io_rat,
  bb_t **bbox_new,
  bb_t **bedge_new,
  bb_t **bboxes,
  bb_t **bedges,
  int *net_move,
  netl_net_t **anets)
{
float *optimum, bestcost = INFINITY, cost;
int i;
seconf_t *conf;
FILE *fp;
char buf[100];
netl_clb_t *clb;
float t;

  conf = (seconf_t *) ALLOC(seconf_t, 1);
  fp = fopen("se.conf","r");
  if(fp == NIL(FILE)) {
    fprintf(stderr,"couldn't open file se.conf\n");
    return;
  }  
  conf->niter = 0;
 
  (void) fscanf(fp, "%s", buf);
  while(!feof(fp)) {
    if(strcmp(buf, "niter:") == 0) {
      fscanf(fp,"%s", buf);
      conf->niter= atoi(buf);
    }
    (void)fscanf(fp,"%s",buf);
  }
  
  optimum = placeComputeOptCost(netl);
  cost = placeCompCost(*clbl, netl, bboxes, bedges, nx, ny);
  t = placeStartingT(*clbl, netl, nx, ny, &cost, 
    INNER_NUM*pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
    array_n((*clbl)->outputs) ,1.3333), clbs_occ, io_occ, io_rat,
    bbox_new, bedge_new, bboxes, bedges, net_move, anets);
        
  for(i = 0;i < conf->niter;i++) {
    placeSelectOpt(*clbl, optimum);

    placeSift(clbl, netl, nx, ny, clbs_occ, io_occ, &cost, io_rat,
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);

    printf("Cost = %f\n", cost);
    if(cost < bestcost) {
      bestcost = cost;
    }
    t = t*0.95;
  }
  FREE(optimum);
  
  printf("Bestcost = %f\n", bestcost);
  
  for(i = 0;i < array_n((*clbl)->clbs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->clbs, i);
    clb->noswap = 0;
  }
  for(i = 0;i < array_n((*clbl)->inputs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->inputs, i);
    clb->noswap = 0;
  }
  for(i = 0;i < array_n((*clbl)->outputs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->outputs, i);
    clb->noswap = 0;
  }
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
float *
placeComputeOptCost(array_t *netl)
{
float *ret;
int i;
netl_net_t *n;
float cross;
int xmax, ymax;

  ret = (float *) ALLOC(float, array_n(netl));

  for(i = 0;i < array_n(netl);i++) {
    n = array_fetch(netl_net_t *, netl, i);  
    if(array_n(n->snet) > 50) {
      cross = 2.7933 + 0.02616 * (array_n(n->snet) - 50); 
    }
    else {
      cross = cross_count[array_n(n->snet) - 1];
    }    
    xmax = (int) ceil(sqrt(array_n(n->snet)));
    ymax = (int) ceil((float) array_n(n->snet)/xmax);
    ret[n->id] = xmax*cross*chanx_place_cost_fac[xmax][0];
    ret[n->id] += ymax*cross*chany_place_cost_fac[ymax][0];
  }
  
  return ret;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeSelectOpt(
  netl_clbl_t *clbl,
  float *optimum)
{
float select, score;
int i,j;
netl_pin_t *pin;
int tot;
netl_clb_t *clb;
  
  select = util2_Frand() * 100; 

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    score = 0.;
    tot = 0;
    for(j = 0;j < array_n(clb->cb->in);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->in, j);
      if(pin->net->cost < optimum[pin->net->id]) {
        optimum[pin->net->id] = pin->net->cost;
      }
      score += optimum[pin->net->id]/pin->net->cost;
      tot++;
    }
    pin = array_fetch(netl_pin_t *, clb->cb->out, 0);
    if(pin->net->cost < optimum[pin->net->id]) {
      optimum[pin->net->id] = pin->net->cost;
    }
    score += optimum[pin->net->id]/pin->net->cost;
    tot++;
    score = score/tot * 100;
    if(score >= select) {
      clb->noswap = 1;
    }
    else {
      clb->noswap = 0;
    }
  }  

  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    score = 0.;
    pin = array_fetch(netl_pin_t *, clb->cb->out, 0);
    if(pin->net->cost < optimum[pin->net->id]) {
      optimum[pin->net->id] = pin->net->cost;
    }
    score = optimum[pin->net->id]/pin->net->cost * 100;
    if(score >= select) {
      clb->noswap = 1;
    }
    else {
      clb->noswap = 0;
    }
  }  

  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    score = 0.;
    pin = array_fetch(netl_pin_t *, clb->cb->in, 0);
    if(pin->net->cost < optimum[pin->net->id]) {
      optimum[pin->net->id] = pin->net->cost;
    }
    score = optimum[pin->net->id]/pin->net->cost * 100;
    if(score >= select) {
      clb->noswap = 1;
    }
    else {
      clb->noswap = 0;
    }
  }  
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
