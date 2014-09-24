/**CFile***********************************************************************

  FileName    [placeTabu.c]

  PackageName [place]

  Synopsis    [A tabu search procedure]

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
#define NONE 0

#define MAXITE 2

#define GREEDY 1
#define RNA 2

#define STATIC 1
#define RANDOM 2
#define INCREASING 3
#define SIZED 4

#define ONENEIGH 6

#define PROPORTIONAL 1

#define CONSTANT 1
#define BESTCOST 2
#define NOIMP 3

#define CLB_FROM_AND_TO 1
#define CLB_FROM 2
#define POS_FROM_AND_TO 3
#define POS_FROM 4
#define POS_TO 5

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
int ntabu;
int ttabu;
int asp;
int nlower;
netl_clb_t **stack;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static float GetDeltaCost(array_t *netl, netl_clb_t *clb, netl_clb_t *clb2, int x_from, int y_from, int x_to, int y_to, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, int nx, int ny, netl_net_t **anets, int *net_move);
static move_t * Explore2(move_t **recency, float cost, float bestcost, tabuconf_t *conf, float rlim, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, int *clb_freq_from, int *clb_freq_to, int *io_freq_from, int *io_freq_to, int tot_moves, int fhead, int ftail);
static move_t * Explore4(move_t **recency, float cost, float bestcost, tabuconf_t *conf, float rlim, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, int *clb_freq_from, int *clb_freq_to, int *io_freq_from, int *io_freq_to, int tot_moves, int fhead, int ftail);
static void EncodeSolution(netl_clbl_t *clbl, int **solution, int nx, int ny, int io_rat);
static void AcceptMove(move_t *mv, float *cost, float *bestcost, int *recompute, move_t **recency, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, tabuconf_t *conf, int *fhead, int *ftail, int *freq_from, int *freq_to);
static void Execute(move_t *mv, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, tabuconf_t *conf);
static void Escape1(int *firstsol, int *alltime, float *cost, int *recompute, move_t **recency, float rlim, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, tabuconf_t *conf, int *fhead, int *ftail, int *nrand);
static void Escape2(int *alltime, float *cost, int *recompute, move_t **recency, float rlim, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, tabuconf_t *conf, int *fhead, int *ftail, int *nrand);
static int Tabu(move_t **recency, netl_clb_t *cfrom, netl_clb_t *cto, int fpos, int tpos, int ternure, short attribute, int fhead, int ftail);
static void Rebalance(tabuconf_t *conf, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, float *cost, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
static tabuconf_t * ReadConf();

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
placeTabu(
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
move_t *mv, **recency;
int i, j, ret, recompute = 0;
float rlim, cost, bestcost = INFINITY, lastcost;
tabuconf_t *conf;
int *clb_freq_from, *clb_freq_to, *io_freq_from, *io_freq_to;
const int nclb = nx*ny, nio = 2*(nx + ny)*io_rat;
int tot_moves = 1, lowimp = 0, *firstsol, *alltime;
float imp, alltimecost, ci;
int tern, chaos = 0, nr = 0, ftail, fhead, noimp = 0, nrand = -1, zeromut = 0;
FILE *costfile, *impfile, *tfile;
int incb = 0;
netl_clb_t *clb;

  cost = *hfc;
  ci = *hfc;

  costfile = fopen("cost.dat","w");
  impfile = fopen("imp.dat","w");
  tfile = fopen("t.dat","w");
  
  firstsol = (int *) ALLOC(int, array_n((*clbl)->clbs) + 
    array_n((*clbl)->inputs) + array_n((*clbl)->outputs));
  alltime = (int *) ALLOC(int, array_n((*clbl)->clbs) + 
    array_n((*clbl)->inputs) + array_n((*clbl)->outputs));
    
  clb_freq_from = (int *) ALLOC(int, nclb);
  clb_freq_to = (int *) ALLOC(int, nclb);
  io_freq_from = (int *) ALLOC(int, nio);
  io_freq_to = (int *) ALLOC(int, nio);

  for(i = 0;i < nclb;i++)  {
    clb_freq_from[i] = 0;
    clb_freq_to[i] = 0;
  }
  for(i = 0;i < nio;i++) {
    io_freq_from[i] = 0;
    io_freq_to[i] = 0;
  }
  conf = ReadConf();
  if(conf == NIL(tabuconf_t)) return;

  if(conf->doimbalance > 0) {
    stack = (netl_clb_t **) ALLOC(netl_clb_t *, conf->maximbalance);  
    for(i = 0;i < conf->maximbalance;i++) {
      stack[i] = NIL(netl_clb_t);
    }
    conf->imbalance = 0;
  }
  else {
    conf->maximbalance = 0;
  }

  if(conf->exploretype == SIZED) {
    conf->neighsize = array_n((*clbl)->clbs) +
      array_n((*clbl)->inputs) + array_n((*clbl)->outputs);
  }  
  
  if(conf->ternure == SIZED) {
    conf->ternurevalue = sqrt(array_n((*clbl)->clbs) + array_n((*clbl)->inputs)
      + array_n((*clbl)->outputs));
    if(conf->ternurevalue < 5) conf->ternurevalue = 5;
    conf->ternuremax = conf->ternurevalue;
  }
  recency = (move_t **) ALLOC(move_t *, conf->ternuremax);
  for(ret = 0;ret < conf->ternuremax;ret++) {
    recency[ret] = NIL(move_t);
  }
  ftail = 0;
  fhead = 0;

/*
  if(conf->stopc == MAXITE) {
    conf->maxite = 5 * (array_n((*clbl)->clbs) + array_n((*clbl)->inputs)
      + array_n((*clbl)->outputs));
  }
*/

  rlim = (float) MAX(nx, ny);  
  
  EncodeSolution(*clbl, &firstsol, nx, ny, io_rat);
  EncodeSolution(*clbl, &alltime, nx, ny, io_rat);
      
  lastcost = cost;
  bestcost = cost;
  alltimecost = cost;
  ttabu = 0;

  i = 0;
  if(conf->escape == 1) {
    conf->probselect = 1;
    while(zeromut <= conf->zeromut) {
      ntabu = 0;
      asp = 0;
      if((conf->exploretype == RANDOM) || (conf->exploretype == SIZED)) {
        mv = Explore2(recency, cost, bestcost, conf, rlim, *clbl, netl,
          nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
          anets, net_move, clb_freq_from, clb_freq_to, io_freq_from, io_freq_to,
          tot_moves, fhead, ftail);      
      }
      else
      if(conf->exploretype == ONENEIGH) {
        mv = Explore4(recency, cost, bestcost, conf, rlim, *clbl, netl,
          nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
          anets, net_move, clb_freq_from, clb_freq_to, io_freq_from, io_freq_to,
          tot_moves, fhead, ftail);      
      }
               
      if(mv->clbfrom->nio == -1) {
        AcceptMove(mv, &cost, &bestcost, &recompute, recency, *clbl, 
          netl, nx, ny,
          io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
          anets, net_move, conf, &fhead, &ftail, clb_freq_from, clb_freq_to);          
          tot_moves++;
      }
      else {
        AcceptMove(mv, &cost, &bestcost, &recompute, recency, *clbl,
          netl, nx, ny,
          io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
          anets, net_move, conf, &fhead, &ftail, io_freq_from, io_freq_to);
          tot_moves++;
      }
      imp = (lastcost - cost)/lastcost;
    
      if(imp < conf->threshold) {
        lowimp++;  
      }
      else {
        lowimp = 0;
      }
    
      if(lowimp == conf->lowimp) {
        if(conf->ternure == RANDOM) {
          printf("Changing ternure value\n");
          tern = conf->ternurevalue;
          conf->ternurevalue = util2_Irand(conf->ternuremax - conf->ternuremin);
           conf->ternurevalue += conf->ternuremin;
          for(j = tern;j < conf->ternurevalue;j++) {
            recency[j] = NIL(move_t);
          }
        }
        else
          if(conf->ternure == INCREASING) {
            printf("Changing ternure value\n");
            conf->ternurevalue++;
            recency[conf->ternurevalue - 1] = NIL(move_t);
          }
      
        if((conf->escape == 1) && (chaos == conf->chaos)) {
          printf("Randomizing\n");
          nr++;
          nrand = 0;
          if(conf->escapetype == 1) {
            Escape1(firstsol, alltime, &cost, &recompute, recency, rlim,
              *clbl, netl, nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges,
              bbox_new, bedge_new, anets, net_move, conf, &fhead, &ftail, &nrand);
          }        
          else
          if(conf->escapetype == 2) {
            Escape2(alltime, &cost, &recompute, recency, rlim, *clbl, netl, nx, 
              ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
              anets, net_move, conf, &fhead, &ftail, &nrand);
          }
          if(nrand == 0) {
            zeromut++;
          }
          else {
            zeromut = 0;
          }
          conf->probmut *= conf->escapedecrease;

          EncodeSolution(*clbl, &firstsol, nx, ny, io_rat);
          bestcost = cost;
          lowimp = 0;
          chaos = 0;
          if(conf->escapereset == 1) {
            for(j = 0;j < conf->ternurevalue;j++) {
              FREE(recency[j]);
              recency[j] = NIL(move_t);
            }        
            fhead = 0;
            ftail = 0;
          }
        }
        else {
          chaos++;
        }
        lowimp = 0;
        imp = (float) (lastcost - cost)/lastcost;
      }    
      printf("%4d  %4.5f  %2d %2d %4.5f %4.5f %3d %4.5f %4.5f %3d\n", i, cost, 
        ntabu, asp, rlim, imp, conf->ternurevalue, (ci - cost)/ci,
        conf->probmut, nrand);

      fprintf(costfile,"%d %f\n", i, cost);
      fprintf(impfile,"%d %f\n", i, imp);

      lastcost = cost;    
      if(cost < alltimecost) {
        alltimecost = cost;
        EncodeSolution(*clbl, &alltime, nx, ny, io_rat);
      }       
      i++;
    }
  }

  if(conf->localini == 1) {
    printf("Aplicando otimizacao local\n");
    printf("Cost antes = %f\n",cost);
    if(conf->localopt == GREEDY) {
      placeSift(clbl, netl, nx, ny, clbs_occ, io_occ, &cost, io_rat, bbox_new,
        bedge_new, bboxes, bedges, net_move, anets);
    }
    else 
    if(conf->localopt == RNA) {
      placeSwap(conf->noimp, rlim, &cost, clbl, netl, nx, ny, io_rat,
        clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new, anets, net_move);
    }
    printf("Cost depois = %f\n",cost);    
  }
  
  noimp = 0;
  i = 0;
  conf->probselect = 0;
  while(((noimp < conf->noimp) && (conf->stopc == NOIMP)) ||
        ((i < conf->maxite) && (conf->stopc == MAXITE))) {
    ntabu = 0;
    asp = 0;
    nlower = 0;
    if((conf->exploretype == RANDOM) || (conf->exploretype == SIZED)) {
      mv = Explore2(recency, cost, bestcost, conf, rlim, *clbl, netl,
        nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
        anets, net_move, clb_freq_from, clb_freq_to, io_freq_from, io_freq_to,
        tot_moves, fhead, ftail);      
    }
    else
    if(conf->exploretype == ONENEIGH) {
      mv = Explore4(recency, cost, bestcost, conf, rlim, *clbl, netl,
        nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
        anets, net_move, clb_freq_from, clb_freq_to, io_freq_from, io_freq_to,
        tot_moves, fhead, ftail);      
    }

    if(mv->clbfrom->nio == -1) {
      AcceptMove(mv, &cost, &bestcost, &recompute, recency, *clbl, 
        netl, nx, ny,
        io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
        anets, net_move, conf, &fhead, &ftail, clb_freq_from, clb_freq_to);          
        tot_moves++;
    }
    else {
      AcceptMove(mv, &cost, &bestcost, &recompute, recency, *clbl,
        netl, nx, ny,
        io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
        anets, net_move, conf, &fhead, &ftail, io_freq_from, io_freq_to);
        tot_moves++;
    }

    imp = (lastcost - cost)/lastcost;

    printf("%4d %4.5f %3d %3d %3d %4.5f %4.5f %3d %4.5f %4.5f %3d %4.5f\n", i, cost, 
      ntabu, asp, nlower, rlim, imp, conf->ternurevalue, (ci - cost)/ci,
      conf->probmut, nrand, conf->temp);

    fprintf(costfile,"%d %f\n", i, cost);
    fprintf(impfile,"%d %f\n", i, imp);
    fprintf(tfile,"%d %f\n", i, conf->temp);
    
    lastcost = cost;
    
    if(cost < alltimecost) {
      alltimecost = cost;
      EncodeSolution(*clbl, &alltime, nx, ny, io_rat);
      noimp = 0;
      incb = 0;
    }       
    else {
      noimp++;
      incb++;
    }
    
    if((incb > conf->doimbalance) && (conf->doimbalance > 0)) {
      printf("Increasing imbalance\n");
      incb = 0;
      conf->imbalance++;
      if(conf->imbalance > conf->maximbalance) {
        conf->imbalance = 0;
        printf("Rebalancing\n");
        Rebalance(conf, netl, nx, ny, clbs_occ, io_occ, &cost, io_rat, bbox_new,
          bedge_new, bboxes, bedges, net_move, anets);
      }
    }
    i++;
  }

  if((conf->imbalance > 0) && (conf->doimbalance > 0)) {
    printf("Rebalancing\n");
    Rebalance(conf, netl, nx, ny, clbs_occ, io_occ, &cost, io_rat, bbox_new,
      bedge_new, bboxes, bedges, net_move, anets);
  }

  i = 0;
  while((recency[i] != NIL(move_t)) && (i < conf->ternuremax)) {
    FREE(recency[i]);
    i++;
  }
  FREE(recency);

  FREE(clb_freq_from);
  FREE(clb_freq_to);
  FREE(io_freq_from);
  FREE(io_freq_to);

  printf("Alltime best cost = %f\n", alltimecost);
  printf("Total tabu moves = %d\n", ttabu);
  printf("Total randomizations = %d\n", nr);

  for(i = 0;i < nx;i++) {
    for(j = 0;j < ny;j++) {
      clbs_occ[i][j] = NIL(netl_clb_t);
    }
  }
  
  for(i = 0;i < ny;i++) {
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[0][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < nx;i++) {
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[1][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < ny;i++) {
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[2][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < nx;i++) {
    for(j = 0;j < io_rat;j++) {
      array_insert(netl_clb_t *, io_occ[3][i], j, NIL(netl_clb_t));
    }
  }
  
  for(i = 0;i < array_n((*clbl)->clbs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->clbs, i);
    placeDecodePos(alltime[i], &clb->x, &clb->y, &clb->nio, nx, ny, io_rat);
    clbs_occ[clb->x - 1][clb->y - 1] = clb;
  }

  for(j = 0;j < array_n((*clbl)->inputs) + array_n((*clbl)->outputs);j++) {
    if(j < array_n((*clbl)->inputs)) {
      clb = array_fetch(netl_clb_t *, (*clbl)->inputs, j);
    }
    else {
      clb = array_fetch(netl_clb_t *, (*clbl)->outputs, j -
        array_n((*clbl)->inputs));
    }    
    placeDecodePos(alltime[i], &clb->x, &clb->y, &clb->nio, nx, ny, io_rat);   
    if(clb->x == 0) {
      array_insert(netl_clb_t *, io_occ[0][clb->y - 1], clb->nio, clb);
    }
    else
      if(clb->y == 0) {
        array_insert(netl_clb_t *, io_occ[1][clb->x - 1], clb->nio, clb);          
      }
      else 
        if(clb->x == nx+1) {
          array_insert(netl_clb_t *, io_occ[2][clb->y - 1], clb->nio, clb);            
        }
        else {
          array_insert(netl_clb_t *, io_occ[3][clb->x - 1], clb->nio, clb);            
        }    
    i++;
  }

  FREE(firstsol);
  FREE(alltime);  
  FREE(stack);
  
  *hfc = alltimecost;
  
  if(conf->localend == 1) {
    *hfc = placeCompCost(*clbl, netl, bboxes, bedges, nx, ny);    
    placeSift(clbl, netl, nx, ny, clbs_occ, io_occ, hfc, io_rat,
      bbox_new, bedge_new, bboxes, bedges, net_move, anets);    
    printf("After Local Optimization: %f\n", *hfc);
  }
  
  (void) fclose(costfile);
  (void) fclose(impfile);
  (void) fclose(tfile);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static float
GetDeltaCost(
  array_t *netl,
  netl_clb_t *clb,
  netl_clb_t *clb2,
  int x_from,
  int y_from,
  int x_to,
  int y_to,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  int nx,
  int ny,
  netl_net_t **anets,
  int *net_move)  
{
int nc, k;
float delta_cost = 0.;

  nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
  for(k = 0;k < nc;k++) {
    if(array_n(anets[k]->snet) <= SMALL_NET) {
      placeGetNUbb(anets[k], &bbox_new[k], nx, ny);
    }
    else {      
      if(net_move[k] == FROM) {
        placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id], 
          bedges[anets[k]->id], anets[k], x_from, y_from, x_to, y_to, nx,
          ny);   
        }
        else {
          placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id],
            bedges[anets[k]->id], anets[k], x_to, y_to, x_from, y_from, nx,
            ny);
        }
    }
    delta_cost += placeNetCost(anets[k], bbox_new[k]) - anets[k]->cost;       
  }
  
  return delta_cost;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static move_t *
Explore2(
  move_t **recency,
  float cost,
  float bestcost,
  tabuconf_t *conf,
  float rlim,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  netl_net_t **anets,
  int *net_move,
  int *clb_freq_from,
  int *clb_freq_to,
  int *io_freq_from,
  int *io_freq_to,
  int tot_moves,
  int fhead,
  int ftail)
{
int i, j, x_to, y_to, io, x_from, y_from, tpos, fpos, test;
int tposbest, fposbest;
move_t *move;
netl_clb_t *clb, *clb2, *cfrom, *cto;
float bestdelta = INFINITY, delta_cost;

  for(i = 0;i < conf->neighsize;i++) {
    delta_cost = util2_Frand();
    if(delta_cost < 0.5) {
      j = util2_Irand(array_n(clbl->inputs) + array_n(clbl->outputs) - 1);
      if(j < array_n(clbl->inputs)) {
        clb = array_fetch(netl_clb_t *, clbl->inputs, j);
      }
      else {
        clb = array_fetch(netl_clb_t *, clbl->outputs, 
          j - array_n(clbl->inputs));
      }
      
      x_from = clb->x;
      y_from = clb->y;      
      placeFindTo(x_from, y_from, 1, rlim, &x_to, &y_to, nx, ny);
      io = util2_Irand(io_rat - 1);
      
      if(x_to == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[0][y_to - 1], io);
      }
      else
        if(y_to == 0) {
          clb2 = array_fetch(netl_clb_t *, io_occ[1][x_to - 1], io);
        }
        else
          if(x_to == nx + 1) {
            clb2 = array_fetch(netl_clb_t *, io_occ[2][y_to - 1], io);
          }
          else {
            clb2 = array_fetch(netl_clb_t *, io_occ[3][x_to - 1], io);
          }

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_from;
        clb2->y = y_from;
      }    
      clb->x = x_to;
      clb->y = y_to;

      delta_cost = GetDeltaCost(netl, clb, clb2, x_from, y_from, x_to, y_to,
        bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

      tpos = placeEncodePos(x_to, y_to, io, nx, ny, io_rat);
      fpos = placeEncodePos(x_from, y_from, clb->nio, nx, ny, io_rat);

      if(conf->diversification == PROPORTIONAL) {
        delta_cost += delta_cost*((float) io_freq_to[tpos - io_rat]/tot_moves);
      }   
      if(conf->intensification == PROPORTIONAL) {
        delta_cost -= delta_cost*((float)io_freq_to[tpos - ny]/tot_moves);
      }

      test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
        conf->attribute, fhead, ftail);
      if(delta_cost < bestdelta) {
        if(test == 0) {        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          tposbest = tpos;
          fposbest = fpos;
        }
        else {
          if((cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;          
            asp++;
          }
          ntabu++;
          ttabu++;
        }
      }        

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_to;
        clb2->y = y_to;
      }
      clb->x = x_from;
      clb->y = y_from;           
    }
    else {
      j = util2_Irand(array_n(clbl->clbs) - 1);
      clb = array_fetch(netl_clb_t *, clbl->clbs, j);
      x_from = clb->x;
      y_from = clb->y;      
      placeFindTo(x_from, y_from, 0, rlim, &x_to, &y_to, nx, ny);     
      clb2 = clbs_occ[x_to - 1][y_to - 1];        

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_from;
        clb2->y = y_from;
       }

      clb->x = x_to;
      clb->y = y_to;

      delta_cost = GetDeltaCost(netl, clb, clb2, x_from, y_from, x_to, y_to,
        bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

      tpos = placeEncodePos(x_to, y_to, -1, nx, ny, io_rat);
      fpos = placeEncodePos(x_from, y_from, -1, nx, ny, io_rat);
      
      if(conf->diversification == PROPORTIONAL) {
        delta_cost += delta_cost*((float)clb_freq_to[tpos - ny]/tot_moves);
      }
      if(conf->intensification == PROPORTIONAL) {
        delta_cost -= delta_cost*((float)clb_freq_to[tpos - ny]/tot_moves);
      }
  
      test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
        conf->attribute, fhead, ftail);
      if(delta_cost < bestdelta) {
        if(test == 0) {        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          tposbest = tpos;
          fposbest = fpos;
        }
        else {
         if((cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;          
            asp++;
          }
          ntabu++;
          ttabu++;
        }
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_to;
        clb2->y = y_to;
      }
      clb->x = x_from;
      clb->y = y_from;
    }
  }

  move = (move_t *) ALLOC(move_t, 1);
  move->clbfrom = cfrom;
  move->from = fposbest;
  move->clbto = cto;
  move->to = tposbest;
  
  if(conf->diversification == PROPORTIONAL) {
    if(cfrom->nio == -1) {
      move->delta_cost = bestdelta - 
        bestdelta*((float)clb_freq_to[tposbest - ny]/tot_moves);
    }
    else {
      move->delta_cost = bestdelta - 
        bestdelta*((float)io_freq_to[tposbest - io_rat]/tot_moves);
    }   
  }

  if(conf->intensification == PROPORTIONAL) {
    if(cfrom->nio == -1) {
      move->delta_cost = bestdelta +
        bestdelta*((float)clb_freq_to[tposbest - ny]/tot_moves);           
    }
    else {
      move->delta_cost = bestdelta + 
        bestdelta*((float)io_freq_to[tposbest - io_rat]/tot_moves);        
    }      
  }    
  else {
    move->delta_cost = bestdelta;  
  }
  
  return move;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static move_t *
Explore4(
  move_t **recency,
  float cost,
  float bestcost,
  tabuconf_t *conf,
  float rlim,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  netl_net_t **anets,
  int *net_move,
  int *clb_freq_from,
  int *clb_freq_to,
  int *io_freq_from,
  int *io_freq_to,
  int tot_moves,
  int fhead,
  int ftail)
{
int j, io, origx, origy, tpos, fpos, test, k, i;
int tposbest, fposbest, m, n;
move_t *move;
netl_clb_t *clb, *clb2, *cfrom, *cto;
float bestdelta = INFINITY, delta_cost, r;
short imb = 0, cont;

  if((conf->imbalance > 0) && (stack[conf->imbalance - 1] == NIL(netl_clb_t))) 
    imb = 1;

  for(i = 0;i < conf->neighsize;i++) {    
  delta_cost = util2_Frand();
  if(delta_cost < 0.5) {
    do {
      j = util2_Irand(array_n(clbl->clbs) - 1);
      clb = array_fetch(netl_clb_t *, clbl->clbs, j);
      cont = 0;    
      for(k = 0;k < conf->maximbalance;k++) {
        if((stack[k] != NIL(netl_clb_t)) &&
           (stack[k]->x == clb->x) && (stack[k]->y == clb->y)) {
          cont = 1;
          break;
        } 
      }
    } while(cont == 1);

    origx = clb->x;
    origy = clb->y;
    for(m = 1;m <= nx;m++) {
      for(n = 1;n <= ny;n++) {
        if((m == origx) && (n == origy)) continue;
        if(imb == 0) {
          clb2 = clbs_occ[m - 1][n - 1];
          if(clb2 != NIL(netl_clb_t)) {
            clb2->x = origx;
            clb2->y = origy;
          }
        }
        else {
          clb2 = NIL(netl_clb_t);
        }
        clb->x = m;
        clb->y = n;

        delta_cost = GetDeltaCost(netl, clb, clb2, origx, origy, m, n,
          bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

        io = -1;
        tpos = placeEncodePos(m, n, io, nx, ny, io_rat);
        fpos = placeEncodePos(origx, origy, clb->nio, nx, ny, io_rat);

        if(delta_cost < bestdelta) {
          test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
            conf->attribute, fhead, ftail);
          if(test == 0) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;
          }
          else {
            if((cost + delta_cost) < bestcost) {
              bestdelta = delta_cost;
              cfrom = clb;
              cto = clb2;
              tposbest = tpos;
              fposbest = fpos;          
              asp++;
            }
            ntabu++;
            ttabu++;
          }
        }        
        else
          if(conf->probselect == 1) {
          r = util2_Frand();
          if((bestdelta != 0) && (r < (delta_cost - bestdelta)/bestdelta)) {
            test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
              conf->attribute, fhead, ftail);
            if(test == 0) {
              bestdelta = delta_cost;
              cfrom = clb;
              cto = clb2;
              tposbest = tpos;
              fposbest = fpos;
            }
          }
        }
        
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = m;
          clb2->y = n;
        }
        clb->x = origx;
        clb->y = origy;
      }
    }
  }
  else {
    do {
      j = util2_Irand(array_n(clbl->inputs) + array_n(clbl->outputs) - 1);
      if(j < array_n(clbl->inputs)) {
        clb = array_fetch(netl_clb_t *, clbl->inputs, j);
      }
      else {
        clb = array_fetch(netl_clb_t *, clbl->outputs, 
          j - array_n(clbl->inputs));
      }
      cont = 0;    
      for(k = 0;k < conf->maximbalance;k++) {
        if((stack[k] != NIL(netl_clb_t)) &&
           (stack[k]->x == clb->x) && (stack[k]->y == clb->y) && 
           (stack[k]->nio == clb->nio)) {
          cont = 1;
          break;
        } 
      }
    } while(cont == 1);      

    origx = clb->x;
    origy = clb->y;      
    imb = 0;
    for(n = 1;n <= ny;n++) {
      io = util2_Irand(io_rat - 1);
      if((origx == 0) && (origy == n)) continue;
      if(imb == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[0][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = origx;
          clb2->y = origy;
        }  
      }
      else {
        clb2 = NIL(netl_clb_t);
      }
      clb->x = 0;
      clb->y = n;

      delta_cost = GetDeltaCost(netl, clb, clb2, origx, origy, 0, n,
        bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

      tpos = placeEncodePos(0, n, io, nx, ny, io_rat);
      fpos = placeEncodePos(origx, origy, clb->nio, nx, ny, io_rat);

      if(delta_cost < bestdelta) {
        test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
          conf->attribute, fhead, ftail);
        if(test == 0) {        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          tposbest = tpos;
          fposbest = fpos;
        }
        else {
          if((cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;          
            asp++;
          }
          ntabu++;
          ttabu++;
        }
      }
      else
        if(conf->probselect == 1) {
          if(bestdelta == 0) continue;
          r = util2_Frand();
          if((bestdelta != 0) && (r < (delta_cost - bestdelta)/bestdelta)) {
            test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
              conf->attribute, fhead, ftail);
            if(test == 0) {
              bestdelta = delta_cost;
              cfrom = clb;
              cto = clb2;
              tposbest = tpos;
              fposbest = fpos;
            }
          }
        }

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = 0;
        clb2->y = n;
      }
      clb->x = origx;
      clb->y = origy;           
    }
    for(n = 1;n <= nx;n++) {
      io = util2_Irand(io_rat - 1);
      if((origx == n) && (origy == 0)) continue;
      if(imb == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[1][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = origx;
          clb2->y = origy;
        }  
      }
      else {
        clb2 = NIL(netl_clb_t);
      }
      clb->x = n;
      clb->y = 0;

      delta_cost = GetDeltaCost(netl, clb, clb2, origx, origy, n, 0,
        bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

      tpos = placeEncodePos(n, 0, io, nx, ny, io_rat);
      fpos = placeEncodePos(origx, origy, clb->nio, nx, ny, io_rat);

      if(delta_cost < bestdelta) {
        test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
          conf->attribute, fhead, ftail);
        if(test == 0) {        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          tposbest = tpos;
          fposbest = fpos;
        }
        else {
          if((cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;          
            asp++;
          }
          ntabu++;
          ttabu++;
        }
      }
      else
        if(conf->probselect == 1) {
          if(bestdelta == 0) continue;
          r = util2_Frand();
          if((bestdelta != 0) && (r < (delta_cost - bestdelta)/bestdelta)) {
            test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
              conf->attribute, fhead, ftail);
            if(test == 0) {
              bestdelta = delta_cost;
              cfrom = clb;
              cto = clb2;
              tposbest = tpos;
              fposbest = fpos;
            }
          }
        }

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = n;
        clb2->y = 0;
      }
      clb->x = origx;
      clb->y = origy;
    }
    for(n = 1;n <= ny;n++) {
      io = util2_Irand(io_rat - 1);
      if((origx == nx+1) && (origy == n)) continue;
      if(imb == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[2][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = origx;
          clb2->y = origy;
        }  
      }
      else {
        clb2 = NIL(netl_clb_t);
      }
      clb->x = nx+1;
      clb->y = n;

      delta_cost = GetDeltaCost(netl, clb, clb2, origx, origy, nx+1, n,
        bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

      tpos = placeEncodePos(nx+1, n, io, nx, ny, io_rat);
      fpos = placeEncodePos(origx, origy, clb->nio, nx, ny, io_rat);

      if(delta_cost < bestdelta) {
        test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
          conf->attribute, fhead, ftail);
        if(test == 0) {        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          tposbest = tpos;
          fposbest = fpos;
        }
        else {
          if((cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;          
            asp++;
          }
          ntabu++;
          ttabu++;
        }
      }
      else
        if(conf->probselect == 1) {
          if(bestdelta == 0) continue;
          r = util2_Frand();
          if((bestdelta != 0) && (r < (delta_cost - bestdelta)/bestdelta)) {
            test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
              conf->attribute, fhead, ftail);
            if(test == 0) {
              bestdelta = delta_cost;
              cfrom = clb;
              cto = clb2;
              tposbest = tpos;
              fposbest = fpos;
            }
          }
        }

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = nx+1;
        clb2->y = n;
      }
      clb->x = origx;
      clb->y = origy;
    }
    for(n = 1;n <= nx;n++) {
      io = util2_Irand(io_rat - 1);
      if((origx == n) && (origy == ny+1)) continue;
      if(imb == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[3][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = origx;
          clb2->y = origy;
        } 
      }
      else  {
        clb2 = NIL(netl_clb_t);      
      }
      clb->x = n;
      clb->y = ny+1;

      delta_cost = GetDeltaCost(netl, clb, clb2, origx, origy, n, ny+1,
        bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

      tpos = placeEncodePos(n, ny+1, io, nx, ny, io_rat);
      fpos = placeEncodePos(origx, origy, clb->nio, nx, ny, io_rat);

      if(delta_cost < bestdelta) {
        test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
          conf->attribute, fhead, ftail);
        if(test == 0) {        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          tposbest = tpos;
          fposbest = fpos;
        }
        else {
          if((cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            tposbest = tpos;
            fposbest = fpos;          
            asp++;
          }
          ntabu++;
          ttabu++;
        }
      }
      else
        if(conf->probselect == 1) {
          if(bestdelta == 0) continue;
          r = util2_Frand();
          if((bestdelta != 0) && (r < (delta_cost - bestdelta)/bestdelta)) {
            test = Tabu(recency, clb, clb2, fpos, tpos, conf->ternurevalue,
              conf->attribute, fhead, ftail);
            if(test == 0) {
              bestdelta = delta_cost;
              cfrom = clb;
              cto = clb2;
              tposbest = tpos;
              fposbest = fpos;
            }
          }
        }

      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = n;
        clb2->y = ny+1;
      }
      clb->x = origx;
      clb->y = origy;
    }
  }
  }
  
  move = (move_t *) ALLOC(move_t, 1);
  move->clbfrom = cfrom;
  move->from = fposbest;
  move->clbto = cto;
  move->to = tposbest;
  move->delta_cost = bestdelta;
  
  return move;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
EncodeSolution(
  netl_clbl_t *clbl,
  int **solution,
  int nx,
  int ny,
  int io_rat)
{
int i;
netl_clb_t *clb;
    
  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    (*solution)[i] = placeEncodePos(clb->x, clb->y, clb->nio, nx, ny, io_rat);
  }
  for(;i < array_n(clbl->inputs) + array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i - array_n(clbl->clbs));
    (*solution)[i] = placeEncodePos(clb->x, clb->y, clb->nio, nx, ny, io_rat);
  }
  for(;i < array_n(clbl->outputs) + array_n(clbl->inputs) + 
    array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i - array_n(clbl->clbs) -
      array_n(clbl->inputs));
    (*solution)[i] = placeEncodePos(clb->x, clb->y, clb->nio, nx, ny, io_rat);
  }
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
AcceptMove(
  move_t *mv,
  float *cost,
  float *bestcost,
  int *recompute,
  move_t **recency,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  netl_net_t **anets,
  int *net_move,
  tabuconf_t *conf,
  int *fhead,
  int *ftail,
  int *freq_from,
  int *freq_to)
{
float newcost;
int j;
netl_net_t *n;

  Execute(mv, netl, nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges,
    bbox_new, bedge_new, anets, net_move, conf);
          
  *cost += mv->delta_cost;

  (*recompute)++;
  if((*recompute) > MAX_MOVES) {
    newcost = 0;
    for(j = 0;j < array_n(netl);j++) {
      n = array_fetch(netl_net_t *, netl, j);
      newcost += n->cost;
    }
    if(fabs(newcost - *cost) > (*cost) * ERROR_TOL) {
      printf("placeTabu Error: new_cost = %f, old cost = %f.\n",
                newcost, *cost);
      exit(1);
    }
    recompute = 0;
    *cost = newcost;
  }

  FREE(recency[*fhead]);
  recency[*fhead] = mv;
  (*fhead)++;
  if((*fhead) > conf->ternurevalue - 1) {
    (*fhead) = 0;
  }
  if((*fhead) == (*ftail)) (*ftail)++;
  if((*ftail) > conf->ternurevalue - 1) {
    (*ftail) = 0;
  }
  
  if(mv->clbfrom->nio == -1) {
    freq_from[mv->from - ny]++;
    freq_to[mv->to - ny]++;
  }
  else {
    freq_from[mv->from - io_rat]++;
    freq_to[mv->to - io_rat]++;
  }
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Execute(
  move_t *mv,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  netl_net_t **anets,
  int *net_move,
  tabuconf_t *conf)
{
int x_from, y_from, x_to, y_to, fio, tio, nc, j;
netl_clb_t *clbt;

  fio = mv->clbfrom->nio;
  tio = fio;
  placeDecodePos(mv->from, &x_from, &y_from, &fio, nx, ny, io_rat);
  placeDecodePos(mv->to, &x_to, &y_to, &tio, nx, ny, io_rat);

  mv->clbfrom->x = x_to;
  mv->clbfrom->y = y_to;
  mv->clbfrom->nio = tio;

  if(mv->clbfrom->nio == -1) {
    if(mv->clbto != NIL(netl_clb_t)) {
      mv->clbto->x = x_from;
      mv->clbto->y = y_from;
      mv->clbto->nio = -1;
      clbs_occ[x_from - 1][y_from - 1] = mv->clbto;
      clbs_occ[x_to - 1][y_to - 1] = mv->clbfrom;
    }
    else {
      clbt = clbs_occ[x_to - 1][y_to - 1];
      if(clbt == NIL(netl_clb_t)) {
        clbs_occ[x_to - 1][y_to - 1] = mv->clbfrom;        
      }
      else {
        stack[conf->imbalance - 1] = mv->clbfrom;
      }
      clbs_occ[x_from - 1][y_from - 1] = NIL(netl_clb_t);
    }
  }
  else {
    if(mv->clbto != NIL(netl_clb_t)) {
      mv->clbto->x = x_from;
      mv->clbto->y = y_from;
      mv->clbto->nio = fio;
      if(x_from == 0) {
        array_insert(netl_clb_t *, io_occ[0][y_from - 1], fio, mv->clbto);
      }
      else 
        if(y_from == 0) {
          array_insert(netl_clb_t *, io_occ[1][x_from - 1], fio, mv->clbto);
        }
        else
          if(x_from == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][y_from - 1], fio, mv->clbto);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][x_from - 1], fio, mv->clbto);
          }
      
      if(x_to == 0) {
        array_insert(netl_clb_t *, io_occ[0][y_to - 1], tio, mv->clbfrom);
      }
      else 
        if(y_to == 0) {
          array_insert(netl_clb_t *, io_occ[1][x_to - 1], tio, mv->clbfrom);
        }
        else
          if(x_to == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][y_to - 1], tio, mv->clbfrom);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][x_to - 1], tio, mv->clbfrom);
          }
    }
    else {
      if(x_from == 0) {
        array_insert(netl_clb_t *, io_occ[0][y_from - 1], fio, 
          NIL(netl_clb_t));
      }
      else
        if(y_from == 0) {
          array_insert(netl_clb_t *, io_occ[1][x_from - 1], fio,
            NIL(netl_clb_t));
        }
        else
          if(x_from == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][y_from - 1], fio, 
              NIL(netl_clb_t));
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][x_from - 1], fio, 
              NIL(netl_clb_t));
          }

      if(x_to == 0) {
        clbt = array_fetch(netl_clb_t *, io_occ[0][y_to - 1], tio);
      }
      else
        if(y_to == 0) {
          clbt = array_fetch(netl_clb_t *, io_occ[1][x_to - 1], tio);
        }
        else
          if(x_to == nx + 1) {
            clbt = array_fetch(netl_clb_t *, io_occ[2][y_to - 1], tio);
          }
          else {
            clbt = array_fetch(netl_clb_t *, io_occ[3][x_to - 1], tio);
          }
          
      if(clbt == NIL(netl_clb_t)) {
        if(x_to == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_to - 1], tio, mv->clbfrom);
        }
        else 
          if(y_to == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_to - 1], tio, mv->clbfrom);
          }
          else
            if(x_to == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_to - 1], tio, mv->clbfrom);
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_to - 1], tio, mv->clbfrom);
            }
      }
      else {
        stack[conf->imbalance - 1] = mv->clbfrom;
      }
    }
  }

  nc = placeAffectedNets(&anets, netl, mv->clbfrom, mv->clbto, &net_move);    
  for(j = 0;j < nc;j++) {
    if(array_n(anets[j]->snet) <= SMALL_NET) {
      placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
    }
    else {      
      if(net_move[j] == FROM) {
        placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id], 
          bedges[anets[j]->id], anets[j], x_from, y_from, x_to, y_to, nx, ny);   
       }
       else {
         placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
           bedges[anets[j]->id], anets[j], x_to, y_to, x_from, y_from, nx, ny);
       }
    }
    anets[j]->cost = placeNetCost(anets[j], bbox_new[j]);
    if(array_n(anets[j]->snet) > SMALL_NET) {
      bedges[anets[j]->id]->xmax = bedge_new[j]->xmax;
      bedges[anets[j]->id]->ymax = bedge_new[j]->ymax;
      bedges[anets[j]->id]->xmin = bedge_new[j]->xmin;
      bedges[anets[j]->id]->ymin = bedge_new[j]->ymin;
    }
    bboxes[anets[j]->id]->xmax = bbox_new[j]->xmax;
    bboxes[anets[j]->id]->ymax = bbox_new[j]->ymax;
    bboxes[anets[j]->id]->xmin = bbox_new[j]->xmin;
    bboxes[anets[j]->id]->ymin = bbox_new[j]->ymin;      
  }  
  
}

/**Function********************************************************************

  Synopsis           [Select a position to be moved to.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Escape1(
  int *firstsol,
  int *alltime,
  float *cost,
  int *recompute,
  move_t **recency,
  float rlim,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  netl_net_t **anets,
  int *net_move,
  tabuconf_t *conf,
  int *fhead,
  int *ftail,
  int *nrand)
{
int i,x_to,y_to, x_from, y_from,nc,j, k, io, fio;
float r, newcost, oldcost;
netl_clb_t *clb, *clb2;
netl_net_t *n;
move_t *move;
int nmodclb = 0, nmodio = 0, *modifyclb = NIL(int), *modifyio = NIL(int);

  for(i = 0;i < nx;i++) {
    for(j = 0;j < ny;j++) {
      clbs_occ[i][j] = NIL(netl_clb_t);
    }
  }
  
  for(i = 0;i < ny;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[0][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < nx;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[1][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < ny;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[2][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < nx;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[3][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    if(firstsol[i] == alltime[i]) {
      nmodclb++;
      modifyclb = (int *) REALLOC(int, modifyclb, nmodclb);
      modifyclb[nmodclb - 1] = i;
    }
    placeDecodePos(alltime[i], &clb->x, &clb->y, &clb->nio, nx, ny, io_rat);
    clbs_occ[clb->x - 1][clb->y - 1] = clb;    
  }

  for(j = 0;j < array_n(clbl->inputs) + array_n(clbl->outputs);j++) {
    if(j < array_n(clbl->inputs)) {
      clb = array_fetch(netl_clb_t *, clbl->inputs, j);
    }
    else {
      clb = array_fetch(netl_clb_t *, clbl->outputs, j -
        array_n(clbl->inputs));
    }    
    if(firstsol[i] == alltime[i]) {
      nmodio++;
      modifyio = (int *) REALLOC(int, modifyio, nmodio);
      modifyio[nmodio - 1] = j;
    }
    placeDecodePos(alltime[i], &clb->x, &clb->y, &clb->nio, nx, ny, io_rat);   
    i++;
    if(clb->x == 0) {
      array_insert(netl_clb_t *, io_occ[0][clb->y - 1], clb->nio, clb);
    }
    else
    if(clb->y == 0) {
      array_insert(netl_clb_t *, io_occ[1][clb->x - 1], clb->nio, clb);          
    }
    else 
      if(clb->x == nx+1) {
        array_insert(netl_clb_t *, io_occ[2][clb->y - 1], clb->nio, clb);            
      }
      else {
        array_insert(netl_clb_t *, io_occ[3][clb->x - 1], clb->nio, clb);            
      }    
  }
  *cost = placeCompCost(clbl, netl, bboxes, bedges, nx, ny);  
  *recompute = 0;
  
  for(i = 0;i < nmodio;i++) {
    if(modifyio[i] < array_n(clbl->inputs)) {
      clb = array_fetch(netl_clb_t *, clbl->inputs, modifyio[i]);
    }
    else {
      clb = array_fetch(netl_clb_t *, clbl->outputs, modifyio[i] -
        array_n(clbl->inputs));
    }
    r = util2_Frand();
    if(r < conf->probmut) {
      (*nrand)++;
      x_from = clb->x;
      y_from = clb->y;      
      fio = clb->nio;
      placeFindTo(x_from, y_from, 1, rlim, &x_to, &y_to, nx, ny);
      io = util2_Irand(io_rat - 1);
      if(x_to == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[0][y_to - 1], io);
      }
      else
        if(y_to == 0) {
          clb2 = array_fetch(netl_clb_t *, io_occ[1][x_to - 1], io);
        }
        else
          if(x_to == nx + 1) {
            clb2 = array_fetch(netl_clb_t *, io_occ[2][y_to - 1], io);
          }
          else {
            clb2 = array_fetch(netl_clb_t *, io_occ[3][x_to - 1], io);
          }          
      move = (move_t *) ALLOC(move_t, 1);
      move->clbfrom = clb;
      move->from = placeEncodePos(clb->x, clb->y, clb->nio, nx, ny, io_rat);
      move->clbto = clb2;
      move->to = placeEncodePos(x_to, y_to, io, nx, ny, io_rat);
      move->delta_cost = 0.;
        
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_from;
        clb2->y = y_from;
        clb2->nio = fio;
        if(x_from == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_from - 1], fio, clb2);
        }
        else 
          if(y_from == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_from - 1], fio, clb2);
          }
          else
            if(x_from == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_from - 1], fio, clb2);
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_from - 1], fio, clb2);
            }
      }
      else {
        if(x_from == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_from - 1], fio, 
            NIL(netl_clb_t));
        }
        else
          if(y_from == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_from - 1], fio,
              NIL(netl_clb_t));
          }
          else
            if(x_from == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_from - 1], fio, 
                NIL(netl_clb_t));
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_from - 1], fio, 
                NIL(netl_clb_t));
            }                
      }
      clb->x = x_to;
      clb->y = y_to;
      clb->nio = io;
      if(x_to == 0) {
        array_insert(netl_clb_t *, io_occ[0][y_to - 1], clb->nio, clb);
      }
      else 
        if(y_to == 0) {
          array_insert(netl_clb_t *, io_occ[1][x_to - 1], clb->nio, clb);
        }
        else
          if(x_to == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][y_to - 1], clb->nio, clb);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][x_to - 1], clb->nio, clb);
          }

      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);    
      for(k = 0;k < nc;k++) {
        if(array_n(anets[k]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[k], &bbox_new[k], nx, ny);
        }
        else {      
          if(net_move[k] == FROM) {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id], 
              bedges[anets[k]->id], anets[k], x_from, y_from, x_to, y_to, nx,
              ny);   
           }
           else {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id],
              bedges[anets[k]->id], anets[k], x_to, y_to, x_from, y_from, nx,
              ny);
          }
        }
        oldcost = anets[k]->cost;
        anets[k]->cost = placeNetCost(anets[k], bbox_new[k]);
        if(array_n(anets[k]->snet) > SMALL_NET) {
          bedges[anets[k]->id]->xmax = bedge_new[k]->xmax;
          bedges[anets[k]->id]->ymax = bedge_new[k]->ymax;
          bedges[anets[k]->id]->xmin = bedge_new[k]->xmin;
          bedges[anets[k]->id]->ymin = bedge_new[k]->ymin;
        }
        bboxes[anets[k]->id]->xmax = bbox_new[k]->xmax;
        bboxes[anets[k]->id]->ymax = bbox_new[k]->ymax;
        bboxes[anets[k]->id]->xmin = bbox_new[k]->xmin;
        bboxes[anets[k]->id]->ymin = bbox_new[k]->ymin;
        move->delta_cost += anets[k]->cost - oldcost;
      }
      *cost += move->delta_cost;

      (*recompute)++;
      if((*recompute) > MAX_MOVES) {
        newcost = 0;
        for(j = 0;j < array_n(netl);j++) {
          n = array_fetch(netl_net_t *, netl, j);
          newcost += n->cost;
        }
        if(fabs(newcost - *cost) > (*cost) * ERROR_TOL) {
          printf("placeTabu Error: new_cost = %f, old cost = %f.\n",
                    newcost, *cost);
          exit(1);
        }
        recompute = 0;
        *cost = newcost;
      }

      FREE(recency[*fhead]);
      recency[*fhead] = move;
      (*fhead)++;
      if((*fhead) > conf->ternurevalue - 1) {
        (*fhead) = 0;
      }
      if((*fhead) == (*ftail)) (*ftail)++;
      if((*ftail) > conf->ternurevalue - 1) {
        (*ftail) = 0;
      }
    }  
  }

  for(i = 0;i < nmodclb;i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, modifyclb[i]);
    r = util2_Frand();
    if(r < conf->probmut) {
      (*nrand)++;
      x_from = clb->x;
      y_from = clb->y;
      placeFindTo(x_from, y_from, 0, rlim, &x_to, &y_to, nx, ny);
      clb2 = clbs_occ[x_to - 1][y_to - 1];
      
      move = (move_t *) ALLOC(move_t, 1);
      move->clbfrom = clb;
      move->from = placeEncodePos(clb->x, clb->y, -1, nx, ny, io_rat);
      move->clbto = clb2;
      move->to = placeEncodePos(x_to, y_to, -1, nx, ny, io_rat);
      move->delta_cost = 0.;
        
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_from;
        clb2->y = y_from;
        clbs_occ[x_from - 1][y_from - 1] = clb2;
      }
      else {
        clbs_occ[x_from - 1][y_from - 1] = NIL(netl_clb_t);      
      }
      clb->x = x_to;
      clb->y = y_to;
      clbs_occ[x_to - 1][y_to - 1] = clb;

      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);    
      for(k = 0;k < nc;k++) {
        if(array_n(anets[k]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[k], &bbox_new[k], nx, ny);
        }
        else {      
          if(net_move[k] == FROM) {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id], 
              bedges[anets[k]->id], anets[k], x_from, y_from, x_to, y_to, nx,
              ny);   
           }
           else {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id],
              bedges[anets[k]->id], anets[k], x_to, y_to, x_from, y_from, nx,
              ny);
          }
        }
        oldcost = anets[k]->cost;
        anets[k]->cost = placeNetCost(anets[k], bbox_new[k]);
        if(array_n(anets[k]->snet) > SMALL_NET) {
          bedges[anets[k]->id]->xmax = bedge_new[k]->xmax;
          bedges[anets[k]->id]->ymax = bedge_new[k]->ymax;
          bedges[anets[k]->id]->xmin = bedge_new[k]->xmin;
          bedges[anets[k]->id]->ymin = bedge_new[k]->ymin;
        }
        bboxes[anets[k]->id]->xmax = bbox_new[k]->xmax;
        bboxes[anets[k]->id]->ymax = bbox_new[k]->ymax;
        bboxes[anets[k]->id]->xmin = bbox_new[k]->xmin;
        bboxes[anets[k]->id]->ymin = bbox_new[k]->ymin;
        move->delta_cost += anets[k]->cost - oldcost;
      }
      *cost += move->delta_cost;

      (*recompute)++;
      if((*recompute) > MAX_MOVES) {
        newcost = 0;
        for(j = 0;j < array_n(netl);j++) {
          n = array_fetch(netl_net_t *, netl, j);
          newcost += n->cost;
        }
        if(fabs(newcost - *cost) > (*cost) * ERROR_TOL) {
          printf("placeTabu Error: new_cost = %f, old cost = %f.\n",
                    newcost, *cost);
          exit(1);
        }
        recompute = 0;
        *cost = newcost;
      }

      FREE(recency[*fhead]);
      recency[*fhead] = move;
      (*fhead)++;
      if((*fhead) > conf->ternurevalue - 1) {
        (*fhead) = 0;
      }
      if((*fhead) == (*ftail)) (*ftail)++;
      if((*ftail) > conf->ternurevalue - 1) {
        (*ftail) = 0;
      }
    }
  } 
  FREE(modifyclb);
  FREE(modifyio);
}

/**Function********************************************************************

  Synopsis           [Select a position to be moved to.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Escape2(
  int *alltime,
  float *cost,
  int *recompute,
  move_t **recency,
  float rlim,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  netl_net_t **anets,
  int *net_move,
  tabuconf_t *conf,
  int *fhead,
  int *ftail,
  int *nrand)
{
int i,x_to,y_to, x_from, y_from,nc,j, k, io, fio;
float r, newcost, oldcost;
netl_clb_t *clb, *clb2;
netl_net_t *n;
move_t *move;

  for(i = 0;i < nx;i++) {
    for(j = 0;j < ny;j++) {
      clbs_occ[i][j] = NIL(netl_clb_t);
    }
  }
  
  for(i = 0;i < ny;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[0][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < nx;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[1][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < ny;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[2][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < nx;i++) {
    for(j = 0;j < 2;j++) {
      array_insert(netl_clb_t *, io_occ[3][i], j, NIL(netl_clb_t));
    }
  }

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    placeDecodePos(alltime[i], &clb->x, &clb->y, &clb->nio, nx, ny, io_rat);
    clbs_occ[clb->x - 1][clb->y - 1] = clb;    
  }

  for(j = 0;j < array_n(clbl->inputs) + array_n(clbl->outputs);j++) {
    if(j < array_n(clbl->inputs)) {
      clb = array_fetch(netl_clb_t *, clbl->inputs, j);
    }
    else {
      clb = array_fetch(netl_clb_t *, clbl->outputs, j -
        array_n(clbl->inputs));
    }    
    placeDecodePos(alltime[i], &clb->x, &clb->y, &clb->nio, nx, ny, io_rat);   
    i++;
    if(clb->x == 0) {
      array_insert(netl_clb_t *, io_occ[0][clb->y - 1], clb->nio, clb);
    }
    else
    if(clb->y == 0) {
      array_insert(netl_clb_t *, io_occ[1][clb->x - 1], clb->nio, clb);          
    }
    else 
      if(clb->x == nx+1) {
        array_insert(netl_clb_t *, io_occ[2][clb->y - 1], clb->nio, clb);            
      }
      else {
        array_insert(netl_clb_t *, io_occ[3][clb->x - 1], clb->nio, clb);            
      }    
  }
  *cost = placeCompCost(clbl, netl, bboxes, bedges, nx, ny);  
  *recompute = 0;
  
  for(i = 0;i < array_n(clbl->inputs) + array_n(clbl->outputs);i++) {
    if(i < array_n(clbl->inputs)) {
      clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    }
    else {
      clb = array_fetch(netl_clb_t *, clbl->outputs,i - array_n(clbl->inputs));
    }
    r = util2_Frand();
    if(r < conf->probmut) {
      (*nrand)++;
      x_from = clb->x;
      y_from = clb->y;      
      fio = clb->nio;
      placeFindTo(x_from, y_from, 1, rlim, &x_to, &y_to, nx, ny);
      io = util2_Irand(io_rat - 1);
      if(x_to == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[0][y_to - 1], io);
      }
      else
        if(y_to == 0) {
          clb2 = array_fetch(netl_clb_t *, io_occ[1][x_to - 1], io);
        }
        else
          if(x_to == nx + 1) {
            clb2 = array_fetch(netl_clb_t *, io_occ[2][y_to - 1], io);
          }
          else {
            clb2 = array_fetch(netl_clb_t *, io_occ[3][x_to - 1], io);
          }          
      move = (move_t *) ALLOC(move_t, 1);
      move->clbfrom = clb;
      move->from = placeEncodePos(clb->x, clb->y, clb->nio, nx, ny, io_rat);
      move->clbto = clb2;
      move->to = placeEncodePos(x_to, y_to, io, nx, ny, io_rat);
      move->delta_cost = 0.;
        
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_from;
        clb2->y = y_from;
        clb2->nio = fio;
        if(x_from == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_from - 1], fio, clb2);
        }
        else 
          if(y_from == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_from - 1], fio, clb2);
          }
          else
            if(x_from == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_from - 1], fio, clb2);
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_from - 1], fio, clb2);
            }
      }
      else {
        if(x_from == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_from - 1], fio, 
            NIL(netl_clb_t));
        }
        else
          if(y_from == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_from - 1], fio,
              NIL(netl_clb_t));
          }
          else
            if(x_from == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_from - 1], fio, 
                NIL(netl_clb_t));
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_from - 1], fio, 
                NIL(netl_clb_t));
            }                
      }
      clb->x = x_to;
      clb->y = y_to;
      clb->nio = io;
      if(x_to == 0) {
        array_insert(netl_clb_t *, io_occ[0][y_to - 1], clb->nio, clb);
      }
      else 
        if(y_to == 0) {
          array_insert(netl_clb_t *, io_occ[1][x_to - 1], clb->nio, clb);
        }
        else
          if(x_to == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][y_to - 1], clb->nio, clb);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][x_to - 1], clb->nio, clb);
          }

      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);    
      for(k = 0;k < nc;k++) {
        if(array_n(anets[k]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[k], &bbox_new[k], nx, ny);
        }
        else {      
          if(net_move[k] == FROM) {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id], 
              bedges[anets[k]->id], anets[k], x_from, y_from, x_to, y_to, nx,
              ny);   
           }
           else {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id],
              bedges[anets[k]->id], anets[k], x_to, y_to, x_from, y_from, nx,
              ny);
          }
        }
        oldcost = anets[k]->cost;
        anets[k]->cost = placeNetCost(anets[k], bbox_new[k]);
        if(array_n(anets[k]->snet) > SMALL_NET) {
          bedges[anets[k]->id]->xmax = bedge_new[k]->xmax;
          bedges[anets[k]->id]->ymax = bedge_new[k]->ymax;
          bedges[anets[k]->id]->xmin = bedge_new[k]->xmin;
          bedges[anets[k]->id]->ymin = bedge_new[k]->ymin;
        }
        bboxes[anets[k]->id]->xmax = bbox_new[k]->xmax;
        bboxes[anets[k]->id]->ymax = bbox_new[k]->ymax;
        bboxes[anets[k]->id]->xmin = bbox_new[k]->xmin;
        bboxes[anets[k]->id]->ymin = bbox_new[k]->ymin;
        move->delta_cost += anets[k]->cost - oldcost;
      }
      *cost += move->delta_cost;

      (*recompute)++;
      if((*recompute) > MAX_MOVES) {
        newcost = 0;
        for(j = 0;j < array_n(netl);j++) {
          n = array_fetch(netl_net_t *, netl, j);
          newcost += n->cost;
        }
        if(fabs(newcost - *cost) > (*cost) * ERROR_TOL) {
          printf("placeTabu Error: new_cost = %f, old cost = %f.\n",
                    newcost, *cost);
          exit(1);
        }
        recompute = 0;
        *cost = newcost;
      }

      FREE(recency[*fhead]);
      recency[*fhead] = move;
      (*fhead)++;
      if((*fhead) > conf->ternurevalue - 1) {
        (*fhead) = 0;
      }
      if((*fhead) == (*ftail)) (*ftail)++;
      if((*ftail) > conf->ternurevalue - 1) {
        (*ftail) = 0;
      }
    }  
  }

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    r = util2_Frand();
    if(r < conf->probmut) {
      (*nrand)++;
      x_from = clb->x;
      y_from = clb->y;
      placeFindTo(x_from, y_from, 0, rlim, &x_to, &y_to, nx, ny);
      clb2 = clbs_occ[x_to - 1][y_to - 1];
      
      move = (move_t *) ALLOC(move_t, 1);
      move->clbfrom = clb;
      move->from = placeEncodePos(clb->x, clb->y, -1, nx, ny, io_rat);
      move->clbto = clb2;
      move->to = placeEncodePos(x_to, y_to, -1, nx, ny, io_rat);
      move->delta_cost = 0.;
        
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_from;
        clb2->y = y_from;
        clbs_occ[x_from - 1][y_from - 1] = clb2;
      }
      else {
        clbs_occ[x_from - 1][y_from - 1] = NIL(netl_clb_t);      
      }
      clb->x = x_to;
      clb->y = y_to;
      clbs_occ[x_to - 1][y_to - 1] = clb;

      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);    
      for(k = 0;k < nc;k++) {
        if(array_n(anets[k]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[k], &bbox_new[k], nx, ny);
        }
        else {      
          if(net_move[k] == FROM) {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id], 
              bedges[anets[k]->id], anets[k], x_from, y_from, x_to, y_to, nx,
              ny);   
           }
           else {
            placeUpdateBb(&bbox_new[k], &bedge_new[k], bboxes[anets[k]->id],
              bedges[anets[k]->id], anets[k], x_to, y_to, x_from, y_from, nx,
              ny);
          }
        }
        oldcost = anets[k]->cost;
        anets[k]->cost = placeNetCost(anets[k], bbox_new[k]);
        if(array_n(anets[k]->snet) > SMALL_NET) {
          bedges[anets[k]->id]->xmax = bedge_new[k]->xmax;
          bedges[anets[k]->id]->ymax = bedge_new[k]->ymax;
          bedges[anets[k]->id]->xmin = bedge_new[k]->xmin;
          bedges[anets[k]->id]->ymin = bedge_new[k]->ymin;
        }
        bboxes[anets[k]->id]->xmax = bbox_new[k]->xmax;
        bboxes[anets[k]->id]->ymax = bbox_new[k]->ymax;
        bboxes[anets[k]->id]->xmin = bbox_new[k]->xmin;
        bboxes[anets[k]->id]->ymin = bbox_new[k]->ymin;
        move->delta_cost += anets[k]->cost - oldcost;
      }
      *cost += move->delta_cost;

      (*recompute)++;
      if((*recompute) > MAX_MOVES) {
        newcost = 0;
        for(j = 0;j < array_n(netl);j++) {
          n = array_fetch(netl_net_t *, netl, j);
          newcost += n->cost;
        }
        if(fabs(newcost - *cost) > (*cost) * ERROR_TOL) {
          printf("placeTabu Error: new_cost = %f, old cost = %f.\n",
                    newcost, *cost);
          exit(1);
        }
        recompute = 0;
        *cost = newcost;
      }

      FREE(recency[*fhead]);
      recency[*fhead] = move;
      (*fhead)++;
      if((*fhead) > conf->ternurevalue - 1) {
        (*fhead) = 0;
      }
      if((*fhead) == (*ftail)) (*ftail)++;
      if((*ftail) > conf->ternurevalue - 1) {
        (*ftail) = 0;
      }
    }
  } 
}

/**Function********************************************************************

  Synopsis           [Select a position to be moved to.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
Tabu(
  move_t **recency,
  netl_clb_t *cfrom,
  netl_clb_t *cto,
  int fpos,
  int tpos,
  int ternure,
  short attribute,
  int fhead,
  int ftail)
{
int i = ftail;

  switch(attribute) {
    case CLB_FROM_AND_TO: {
      while(i != fhead) {
        if((cfrom == recency[i]->clbto) &&
           (cto == recency[i]->clbfrom)) {
          return 1;  
        }
        i++;  
        if(i > ternure - 1) i = 0;
      }      
    } break;

    case CLB_FROM: {
      while(i != fhead) {
        if(cto == recency[i]->clbfrom) {
          return 1;  
        }
        i++;
        if(i > ternure - 1) i = 0;
      }
    } break;

    case POS_FROM_AND_TO: {
      while(i != fhead) {
        if((fpos == recency[i]->to) &&
           (tpos == recency[i]->from)) {
          return 1;  
        }
        i++;  
        if(i > ternure - 1) i = 0;
      }          
    } break;

    case POS_FROM: {
      while(i != fhead) {
        if(tpos == recency[i]->from) {
          return 1;  
        }
        i++;  
        if(i > ternure - 1) i = 0;
      }
    } break;

    case POS_TO: {
      while(i != fhead) {
        if(fpos == recency[i]->to) {
          return 1;
        }
        i++;  
        if(i > ternure - 1) i = 0;
      }    
    } break;    
  }

  return 0;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Rebalance(
  tabuconf_t *conf,
  array_t *netl,
  int nx,
  int ny,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  float *cost,
  int io_rat,
  bb_t **bbox_new,
  bb_t **bedge_new,
  bb_t **bboxes,
  bb_t **bedges,
  int *net_move,
  netl_net_t **anets)
{
int i, m, n, origx, origy, nc, j, bestx, besty, k, io, bestio;
netl_clb_t *clb, *clb2;
float delta, bestdelta;
short cont;

  for(i = 0;i < conf->maximbalance;i++) {
    clb = stack[i];
    if(clb == NIL(netl_clb_t)) continue;
    origx = clb->x;
    origy = clb->y;
    bestdelta = INFINITY;
    if(clb->nio == -1) {
      for(m = 1;m <= nx;m++) {
        for(n = 1;n <= ny;n++) {
          if((m == origx) && (n == origy)) continue;
          clb2 = clbs_occ[m - 1][n - 1];
          if(clb2 != NIL(netl_clb_t)) continue;
          cont = 0;
          for(k = 0;k < conf->maximbalance;k++) {
            if((stack[k] != NIL(netl_clb_t)) &&
               (stack[k]->x == m) && (stack[k]->y == n)) cont = 1;
          }
          if(cont == 1) continue;
          if(clb2 != NIL(netl_clb_t)) {
            clb2->x = origx;
            clb2->y = origy;
          }
          clb->x = m;
          clb->y = n;

          delta = GetDeltaCost(netl, clb, clb2, origx, origy, m, n,
            bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);
          if(delta < bestdelta) {
            bestdelta = delta;
            bestx = m;
            besty = n;
          }
          if(clb2 != NIL(netl_clb_t)) {
            clb2->x = m;
            clb2->y = n;
          }
        }
      }
      (*cost) += bestdelta;
      clb->x = bestx;
      clb->y = besty;
/*
      clb2 = clbs_occ[bestx - 1][besty - 1];
*/
      clbs_occ[bestx - 1][besty - 1] = clb;
/*
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
        clbs_occ[origx - 1][origy - 1] = clb2;
      }
*/      
      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      for(j = 0;j < nc;j++) {
       if(array_n(anets[j]->snet) <= SMALL_NET) {
         placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
       }
       else {
         if(net_move[j] == FROM) {
           placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
             bedges[anets[j]->id], anets[j], origx, origy, bestx, besty,
             nx, ny);   
         }
         else {
           placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
             bedges[anets[j]->id], anets[j], bestx, besty, origx, origy,
             nx, ny);
         }
       }
       anets[j]->cost = placeNetCost(anets[j], bbox_new[j]);
       if(array_n(anets[j]->snet) > SMALL_NET) {
         bedges[anets[j]->id]->xmax = bedge_new[j]->xmax;
         bedges[anets[j]->id]->ymax = bedge_new[j]->ymax;
         bedges[anets[j]->id]->xmin = bedge_new[j]->xmin;
         bedges[anets[j]->id]->ymin = bedge_new[j]->ymin;
       }
       bboxes[anets[j]->id]->xmax = bbox_new[j]->xmax;
       bboxes[anets[j]->id]->ymax = bbox_new[j]->ymax;
       bboxes[anets[j]->id]->xmin = bbox_new[j]->xmin;
       bboxes[anets[j]->id]->ymin = bbox_new[j]->ymin;        
      }      
    }
    else {
      for(n = 1;n <= ny;n++) {
        io = util2_Irand(io_rat - 1);
        if((origx == 0) && (origy == n)) continue;
        clb2 = array_fetch(netl_clb_t *, io_occ[0][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) continue;
        cont = 0;
        for(k = 0;k < conf->maximbalance;k++) {
          if((stack[k] != NIL(netl_clb_t)) &&
             (stack[k]->x == 0) && (stack[k]->y == n) &&
             (stack[k]->nio = io)) cont = 1;
        }
        if(cont == 1) continue;
        clb->x = 0;
        clb->y = n;

        delta = GetDeltaCost(netl, clb, clb2, origx, origy, 0, n,
          bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

        if(delta < bestdelta) {
          bestdelta = delta;
          bestx = 0;
          besty = n;  
          bestio = io;
        }
        clb->x = origx;
        clb->y = origy;           
      }
      for(n = 1;n <= nx;n++) {
        io = util2_Irand(io_rat - 1);
        if((origx == n) && (origy == 0)) continue;
        clb2 = array_fetch(netl_clb_t *, io_occ[1][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) continue;
        cont = 0;
        for(k = 0;k < conf->maximbalance;k++) {
          if((stack[k] != NIL(netl_clb_t)) &&
             (stack[k]->x == n) && (stack[k]->y == 0) &&
             (stack[k]->nio = io)) {
            cont = 1;
            break;
          }
        }
        if(cont == 1) continue;
        clb->x = n;
        clb->y = 0;
        delta = GetDeltaCost(netl, clb, clb2, origx, origy, n, 0,
          bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

        if(delta < bestdelta) {
          bestdelta = delta;
          bestx = n;
          besty = 0;  
          bestio = io;
        }
        clb->x = origx;
        clb->y = origy;           
      }      
      for(n = 1;n <= ny;n++) {
        io = util2_Irand(io_rat - 1);
        if((origx == nx+1) && (origy == n)) continue;
        clb2 = array_fetch(netl_clb_t *, io_occ[2][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) continue;
        cont = 0;
        for(k = 0;k < conf->maximbalance;k++) {
          if((stack[k] != NIL(netl_clb_t)) &&
             (stack[k]->x == nx+1) && (stack[k]->y == n) &&
             (stack[k]->nio = io)) cont = 1;
        }
        if(cont == 1) continue;

        clb->x = nx+1;
        clb->y = n;
        delta = GetDeltaCost(netl, clb, clb2, origx, origy, nx+1, n,
          bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

        if(delta < bestdelta) {
          bestdelta = delta;
          bestx = nx+1;
          besty = n;  
          bestio = io;
        }
        clb->x = origx;
        clb->y = origy;           
      }      
      for(n = 1;n <= nx;n++) {
        io = util2_Irand(io_rat - 1);
        if((origx == n) && (origy == ny+1)) continue;
        clb2 = array_fetch(netl_clb_t *, io_occ[3][n - 1], io);
        if(clb2 != NIL(netl_clb_t)) continue;
        cont = 0;
        for(k = 0;k < conf->maximbalance;k++) {
          if((stack[k] != NIL(netl_clb_t)) &&
             (stack[k]->x == n) && (stack[k]->y == ny+1) &&
             (stack[k]->nio = io)) cont = 1;
        }
        if(cont == 1) continue;
        clb->x = n;
        clb->y = ny+1;
        delta = GetDeltaCost(netl, clb, clb2, origx, origy, n, ny+1,
          bboxes, bedges, bbox_new, bedge_new, nx, ny, anets, net_move);

        if(delta < bestdelta) {
          bestdelta = delta;
          bestx = n;
          besty = ny+1;
          bestio = io;
        }
        clb->x = origx;
        clb->y = origy;           
      }       
          
      (*cost) += bestdelta;
      clb->x = bestx;
      clb->y = besty;
      clb->nio = bestio;
      if(bestx == 0) {
        array_insert(netl_clb_t *, io_occ[0][besty - 1], bestio, 
          clb);
      }
      else
        if(besty == 0) {
          array_insert(netl_clb_t *, io_occ[1][bestx - 1], bestio,
            clb);
        }
        else
          if(bestx == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][besty - 1], bestio, 
              clb);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][bestx - 1], bestio, 
              clb);
          }                
      clb2 = NIL(netl_clb_t);
 
      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      for(j = 0;j < nc;j++) {
       if(array_n(anets[j]->snet) <= SMALL_NET) {
         placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
       }
       else {
         if(net_move[j] == FROM) {
           placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
             bedges[anets[j]->id], anets[j], origx, origy, bestx, besty,
             nx, ny);   
         }
         else {
           placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
             bedges[anets[j]->id], anets[j], bestx, besty, origx, origy,
             nx, ny);
         }
       }
       anets[j]->cost = placeNetCost(anets[j], bbox_new[j]);
       if(array_n(anets[j]->snet) > SMALL_NET) {
         bedges[anets[j]->id]->xmax = bedge_new[j]->xmax;
         bedges[anets[j]->id]->ymax = bedge_new[j]->ymax;
         bedges[anets[j]->id]->xmin = bedge_new[j]->xmin;
         bedges[anets[j]->id]->ymin = bedge_new[j]->ymin;
       }
       bboxes[anets[j]->id]->xmax = bbox_new[j]->xmax;
       bboxes[anets[j]->id]->ymax = bbox_new[j]->ymax;
       bboxes[anets[j]->id]->xmin = bbox_new[j]->xmin;
       bboxes[anets[j]->id]->ymin = bbox_new[j]->ymin;        
      }      
    }   
    stack[i] = NIL(netl_clb_t);
  }
}

/**Function********************************************************************

  Synopsis           [Select a position to be moved to.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static tabuconf_t *
ReadConf()
{
tabuconf_t *conf;
char buf[100];
FILE *fp;

  conf = (tabuconf_t *) ALLOC(tabuconf_t, 1);
  fp = fopen("tabu.conf","r");
  if(fp == NIL(FILE)) {
    fprintf(stderr,"couldn't open file tabu.conf\n");
    return NIL(tabuconf_t);
  }
  conf->stopc = NOIMP;
  conf->maxite = 2000;
  conf->ternure = STATIC;
  conf->noimp = 30;
  conf->ternurevalue = 7;
  conf->ioexplore = 2;
  conf->clbexplore = 2;
  conf->attribute = POS_FROM_AND_TO;
  conf->diversification = NONE;
  conf->intensification = NONE;
  conf->lowimp = 20;
  conf->threshold = .01;
  conf->probmut = 0.5;
  conf->escape = 0;
  conf->escapetype = 1;
  conf->escapereset = 0;
  conf->localini = 0;
  conf->localend = 0;
  conf->localopt = RNA;
  conf->zeromut = 0;
  conf->exploretype = SIZED;
  conf->neighsize = 0;
  conf->maximbalance = 0;
  conf->doimbalance = INFINITY;
  
  (void) fscanf(fp, "%s", buf);
  while(!feof(fp)) {
    if(strcmp(buf,"stopc:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"noimp") == 0) {
        conf->stopc = NOIMP;
      }
      else
      if(strcmp(buf,"maxite") == 0) {
        conf->stopc = MAXITE;
      }
    }
    else
    if(strcmp(buf,"maxite:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->maxite = atoi(buf);
    }
    else
    if(strcmp(buf,"noimp:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->noimp = atoi(buf);      
    }
    else
    if(strcmp(buf,"localini:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"on") == 0) {
        conf->localini = 1;
      }
      else 
      if(strcmp(buf,"off") == 0) {
        conf->localini = 0;
      }      
    }
    else
    if(strcmp(buf,"localend:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"on") == 0) {
        conf->localend = 1;
      }
      else 
      if(strcmp(buf,"off") == 0) {
        conf->localend = 0; 
      }
    }
    else
    if(strcmp(buf, "localopt:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"rna") == 0) {
        conf->localopt = RNA;
      }
      else
      if(strcmp(buf,"greedy") == 0) {
        conf->localopt = GREEDY;
      }
    }
    else
    if(strcmp(buf, "ternure:") == 0) {    
      fscanf(fp,"%s", buf);
      if(strcmp(buf,"static") == 0) {
        conf->ternure = STATIC;
      }
      else
      if(strcmp(buf,"random") == 0) {
        conf->ternure = RANDOM;
      }
      else
      if(strcmp(buf,"increasing") == 0) {
        conf->ternure = INCREASING;
      }
      else
      if(strcmp(buf,"sized") == 0) {
        conf->ternure = SIZED;
      }
    }
    else
    if(strcmp(buf,"ternurevalue:") == 0) {
      fscanf(fp,"%s",buf);
      conf->ternurevalue = atoi(buf);
    }
    else
    if(strcmp(buf,"ternuremax:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->ternuremax = atoi(buf);
    }
    else
     if(strcmp(buf,"ternuremin:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->ternuremin = atoi(buf);
    }
    else
    if(strcmp(buf,"ioexplore:") == 0) {
      fscanf(fp,"%s",buf);
      conf->ioexplore = atoi(buf);
    }
    else
    if(strcmp(buf,"clbexplore:") == 0) {
      fscanf(fp,"%s",buf);
      conf->clbexplore = atoi(buf);
    }
    else
    if(strcmp(buf,"exploretype:") == 0) {
      fscanf(fp,"%s",buf);
      if(strcmp(buf,"random") == 0) {
        conf->exploretype = RANDOM;
      }
      else
      if(strcmp(buf,"sized") == 0) {
        conf->exploretype = SIZED;
      }
      else
      if(strcmp(buf,"oneneigh") == 0) {
        conf->exploretype = ONENEIGH;
      }
    }
    else
    if(strcmp(buf,"neighsize:") == 0) {
      fscanf(fp,"%s",buf);
      conf->neighsize = atoi(buf);
    }
    else
    if(strcmp(buf,"attribute:") == 0) {
      fscanf(fp,"%s",buf);
      if(strcmp(buf,"clb_from_and_to") == 0) {
        conf->attribute = CLB_FROM_AND_TO;
      }
      else
      if(strcmp(buf,"clb_from") == 0) {
        conf->attribute = CLB_FROM;      
      }
      else
      if(strcmp(buf,"pos_from_and_to") == 0) {
        conf->attribute = POS_FROM_AND_TO;
      }
      else
      if(strcmp(buf,"pos_from") == 0) {
        conf->attribute = POS_FROM;
      }
      else
      if(strcmp(buf,"pos_to") == 0) {
        conf->attribute = POS_TO;      
      }
    }
    else
    if(strcmp(buf,"diversification:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"proportional") == 0) {
        conf->diversification = PROPORTIONAL;
      }
      else
      if(strcmp(buf,"none") == 0) {
        conf->diversification = NONE;
      }
    }
    else
     if(strcmp(buf,"intensification:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"proportional") == 0) {
        conf->intensification = PROPORTIONAL;
      }
      else
      if(strcmp(buf,"none") == 0) {
        conf->diversification = NONE;
      }
    }
    else
    if(strcmp(buf,"lowimp:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->lowimp = atoi(buf);
    }
    else
    if(strcmp(buf,"threshold:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->threshold = atof(buf);
    }
    else
    if(strcmp(buf,"probmut:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->probmut = atof(buf);
    }
    else
    if(strcmp(buf,"escape:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"on") == 0) {
        conf->escape = 1;
      }
      else
      if(strcmp(buf,"off") == 0) {
        conf->escape = 0;
      }
    }
    else
    if(strcmp(buf,"chaos:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->chaos = atoi(buf);
    }
    else
    if(strcmp(buf,"escapetype:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->escapetype = atoi(buf);
    }
    else
    if(strcmp(buf,"escapereset:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->escapereset = atoi(buf);
    }
    else
    if(strcmp(buf,"escapedecrease:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->escapedecrease = atof(buf);
    }
    else
    if(strcmp(buf,"zeromut:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->zeromut = atoi(buf);
    }
    else
    if(strcmp(buf,"maximbalance:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->maximbalance = atoi(buf);      
    }
    else
    if(strcmp(buf,"doimbalance:") == 0) {
      (void) fscanf(fp,"%s",buf);
      conf->doimbalance = atoi(buf);    
    }
    fscanf(fp,"%s", buf);
  }
  (void) fclose(fp);
  
  return conf;
}
