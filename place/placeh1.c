/**CFile***********************************************************************

  FileName    [placeH1.c]

  PackageName [place]

  Synopsis    []

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
#define TERNURE 10

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern st_table *blocks;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int exit_crit(float t, int nnets, float cost, double std_dev);
static void update_t(float *t, float std_dev, float rlim, float success_rat);
static void update_rlim(float *rlim, float success_rat, int nx, int ny);
static int Swap(move_t **recency, float bestcost, netl_clbl_t *clbl, array_t *netl, int nx, int ny, float *cost, float t, float rlim, netl_clb_t ***clbs_occ, array_t ***io_occ, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
static int Tabu(move_t **recency, netl_clb_t *cfrom, netl_clb_t *cto, int fpos, int tpos, int ternure, short attribute);
static void Execute(move_t *mv, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Annealing phase of the algorithm.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeH1(
  netl_clbl_t **clbl,
  array_t *netl,
  int nx,
  int ny,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ, 
  float tini,
  float *costini,
  int io_rat,
  bb_t **bbox_new,
  bb_t **bedge_new,
  bb_t **bboxes,
  bb_t **bedges,
  int *net_move,
  netl_net_t **anets)
{
float cost, t, rlim, success_rat, oldt, newcost, bestcost;
int tot_iter, success_sum, inner_iter, move_lim, i;
netl_net_t *n;
double std_dev = 1, av_cost, sum_of_squares;
int count = 1, recompute = 0;
move_t **recency;

  recency = (move_t **) ALLOC(move_t *, TERNURE);
  for(i = 0;i < TERNURE;i++) {
    recency[i] = NIL(move_t);
  }
  
  move_lim = INNER_NUM*pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
    array_n((*clbl)->outputs) ,1.3333);  
  rlim = (float) MAX(nx, ny);

  cost = *costini;
  t = tini;
  tot_iter = 0;
    
  bestcost = cost;
  
  while(exit_crit(t, array_n(netl), cost, std_dev) == 0) {
    av_cost = 0.;
    success_sum = 0;
    sum_of_squares = 0.;
    for(inner_iter = 0;inner_iter < move_lim; inner_iter++) {
      if(Swap(recency, bestcost, *clbl, netl, nx, ny, &cost, t, rlim, clbs_occ,
        io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges, net_move, 
        anets) == 1) {
        sum_of_squares += cost * cost;
        success_sum++;
        av_cost += cost;
        if(cost < bestcost) {
          bestcost = cost;
        }        
      }
    }    
    recompute += move_lim;
    if(recompute > MAX_MOVES) {
      newcost = 0;
      for(inner_iter = 0;inner_iter < array_n(netl);inner_iter++) {
        n = array_fetch(netl_net_t *, netl, inner_iter);
        newcost += n->cost;
      }
      if(fabs(newcost - cost) > cost * ERROR_TOL) {
        printf("placeAnneal Error: new_cost = %f, old cost = %f.\n",
          newcost, cost);
        exit(1);
      }
      recompute = 0;
      cost = newcost;
    }
    
    tot_iter += move_lim;
    success_rat = ((float) success_sum)/ move_lim;
    if(success_sum == 0) {
       av_cost = cost;
    }
    else {
      av_cost /= success_sum;
    }
    std_dev = placeGetStdDev(success_sum, sum_of_squares, av_cost);   

    oldt = t;  
    update_t(&t, std_dev, rlim, success_rat);
    update_rlim(&rlim, success_rat, nx, ny);
    
    printf("%f %f %f %f %f %d\n", t, av_cost, success_rat, 
    std_dev, rlim, tot_iter);
    
    count++;
  }  
  t = 0;
  av_cost = 0.;
  success_sum = 0;
  sum_of_squares = 0.;

  for(inner_iter = 0; inner_iter < move_lim; inner_iter++) {
    if(Swap(recency, bestcost, *clbl, netl, nx, ny, &cost, t, rlim, clbs_occ, 
      io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges, net_move,
      anets) == 1) {
      sum_of_squares += cost * cost;
      success_sum++;
      av_cost += cost;
      if(cost < bestcost) {
        bestcost = cost;
      }
    }
  }
  tot_iter += move_lim;
  success_rat = ((float) success_sum) / move_lim;
  if(success_sum == 0) {
    av_cost = cost;
  }
  else {
    av_cost /= success_sum;
  }
  std_dev = placeGetStdDev(success_sum, sum_of_squares, av_cost);
    
  printf("%f %f %f %f %f %d\n", t, av_cost, success_rat, 
    std_dev, rlim, tot_iter);

  *costini = cost;
  
  i = 0;
  while((recency[i] != NIL(move_t)) && (i < TERNURE)) {
    FREE(recency[i]);
    i++;
  }
  FREE(recency);  
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Determine the exit condition.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
exit_crit(
  float t,
  int nnets,
  float cost,
  double std_dev)
{
  if((t < (0.005 * cost/nnets)) || (std_dev < 0.001)) {
    return(1);
  }
  else {
    return(0);
  }
}

/**Function********************************************************************

  Synopsis           [Update the temperature.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void 
update_t(
  float *t,
  float std_dev,
  float rlim,
  float success_rat)
{
  if(success_rat > 0.96) {
    *t = (*t) * 0.5; 
  }
  else
    if(success_rat > 0.8) {
      *t = (*t) * 0.90;
    }
    else 
      if((success_rat > 0.15) || (rlim > 1.)) {
        *t = (*t) * 0.95;
      }
      else {
       *t = (*t) * 0.8; 
      }
}

/**Function********************************************************************

  Synopsis           [Update the rlim.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void 
update_rlim(
  float *rlim,
  float success_rat,
  int nx,
  int ny)
{
float upper_lim;

  *rlim = (*rlim) * (1. - 0.44 + success_rat);
   upper_lim = MAX(nx,ny);
   *rlim = MIN(*rlim,upper_lim);
   *rlim = MAX(*rlim,1.);  
}

/**Function********************************************************************

  Synopsis           [Try to swap two blocks. Do not support multiple block
  movement.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int 
Swap(
  move_t **recency,
  float bestcost,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  float *cost,
  float t,
  float rlim,
  netl_clb_t ***clbs_occ, 
  array_t ***io_occ,
  int io_rat,
  bb_t **bbox_new,
  bb_t **bedge_new,
  bb_t **bboxes,
  bb_t **bedges,
  int *net_move,
  netl_net_t **anets)
{
int x_to, y_to, x_from, y_from, keep_switch, i, j, k, io;
float fnum, prob_fac, delta_cost, bestdelta = INFINITY;
netl_clb_t *clb, *clb2 = NIL(netl_clb_t), *cfrom, *cto;
int nc, tpos, fpos, test, tposbest, fposbest;
move_t *move;

  for(i = 0;i < 1;i++) {
    for(j = 0;j < (array_n(clbl->inputs) + array_n(clbl->outputs));j++) {
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

      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      delta_cost = 0.;      
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
        anets[k]->tempcost = placeNetCost(anets[k], bbox_new[k]);
        delta_cost += anets[k]->tempcost - anets[k]->cost;       
      }
      if(delta_cost < bestdelta) {
        tpos = placeEncodePos(x_to, y_to, io, nx, ny, io_rat);
        fpos = placeEncodePos(x_from, y_from, clb->nio, nx, ny, io_rat);
/*        test = Tabu(recency, clb, clb2, fpos, tpos, 10, 4);
        if(test == 0) {        */
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          fposbest = fpos;
          tposbest = tpos;
/*
        }
        else {
          if((*cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            fposbest = fpos;
            tposbest = tpos;
          }
        }
*/
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_to;
        clb2->y = y_to;
      }
      clb->x = x_from;
      clb->y = y_from;           
    }
  }

  for(i = 0;i < 1;i++) {
    for(j = 0;j < array_n(clbl->clbs);j++) {
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

      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      delta_cost = 0.;    
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
        anets[k]->tempcost = placeNetCost(anets[k], bbox_new[k]);
        delta_cost += anets[k]->tempcost - anets[k]->cost;       
      }
      if(delta_cost < bestdelta) {
        tpos = placeEncodePos(x_to, y_to, -1, nx, ny, io_rat);
        fpos = placeEncodePos(x_from, y_from, -1, nx, ny, io_rat);
/*
        test = Tabu(recency, clb, clb2, fpos, tpos, 10, 4);
        if(test == 0) {
*/        
          bestdelta = delta_cost;
          cfrom = clb;
          cto = clb2;
          fposbest = fpos;
          tposbest = tpos;
/*
        }
        else {
          if((*cost + delta_cost) < bestcost) {
            bestdelta = delta_cost;
            cfrom = clb;
            cto = clb2;
            fposbest = fpos;
            tposbest = tpos;
          }
        }
*/        
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = x_to;
        clb2->y = y_to;
      }
      clb->x = x_from;
      clb->y = y_from;
    }
  }
  
  if(bestdelta <= 0) {
    keep_switch = 1;    
  }
  else
    if(t > 0.) {
      fnum = util2_Frand();
      prob_fac = exp(-bestdelta/t);
      if (prob_fac > fnum) {
        keep_switch = 1;
      }
      else {
        keep_switch = 0;
      }
    }
    else {
      keep_switch = 0;
    }

  if(keep_switch == 1) {
    move = (move_t *) ALLOC(move_t, 1);
    move->clbfrom = cfrom;
    move->from = fposbest;
    move->clbto = cto;
    move->to = tposbest;
    move->delta_cost = bestdelta;  

    Execute(move, netl, nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges,
      bbox_new, bedge_new, anets, net_move);

/*      
    if(recency[TERNURE - 1] != NIL(move_t)) {
      FREE(recency[TERNURE - 1]);
    }
    for(j = TERNURE - 1;j > 0;j--) {
       recency[j] = recency[j-1];
    }
    recency[0] = move;   
*/    
    *cost += bestdelta;    
  }  
  
  return(keep_switch);
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
  short attribute)
{
int i = 0;

  switch(attribute) {
    case 1: {
      while((recency[i] != NIL(move_t)) && (i < ternure)) {
        if((cfrom == recency[i]->clbto) &&
           (cto == recency[i]->clbfrom)) {
          return 1;  
        }
        i++;  
      }      
    } break;

    case 2: {
      while((recency[i] != NIL(move_t)) && (i < ternure)) {
        if(cto == recency[i]->clbfrom) {
          return 1;  
        }
        i++;  
      }
    } break;

    case 3: {
      while((recency[i] != NIL(move_t)) && (i < ternure)) {
        if((fpos == recency[i]->to) &&
           (tpos == recency[i]->from)) {
          return 1;  
        }
        i++;  
      }          
    } break;

    case 4: {
      while((recency[i] != NIL(move_t)) && (i < ternure)) {
        if(tpos == recency[i]->from) {
          return 1;  
        }
        i++;  
      }
    } break;

    case 5: {
      while((recency[i] != NIL(move_t)) && (i < ternure)) {
        if(fpos == recency[i]->to) {
          return 1;
        }
        i++;  
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
  int *net_move)
{
int x_from, y_from, x_to, y_to, fio, tio, nc, j;

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
    }
    else {
      clbs_occ[x_from - 1][y_from - 1] = NIL(netl_clb_t);
    }
    clbs_occ[x_to - 1][y_to - 1] = mv->clbfrom;
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

