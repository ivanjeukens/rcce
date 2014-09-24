/**CFile***********************************************************************

  FileName    [placeSA.c]

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
#define INNER_NUM 10

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern st_table *blocks;

double tempo = 0;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int exit_crit(float t, int nnets, float cost);
static void update_t(float *t, float std_dev, float rlim, float success_rat);
static void update_rlim(float *rlim, float success_rat, int nx, int ny);
static int Swap(netl_clbl_t *clbl, array_t *netl, int nx, int ny, float *cost, float t, float rlim, netl_clb_t ***clbs_occ, array_t ***io_occ, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);

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
placeAnneal(
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
float cost, t, rlim, success_rat, oldt, newcost, lastcost;
int tot_iter, success_sum, inner_iter, move_lim;
netl_net_t *n;
double std_dev, av_cost, sum_of_squares;
int count = 1, recompute = 0;
FILE *scost, *simp;

  scost = fopen("scost.dat","w");
  simp = fopen("simp.dat","w");
  
  move_lim = INNER_NUM*pow(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) +
    array_n((*clbl)->outputs) ,1.3333);  
  rlim = (float) MAX(nx, ny);

  cost = *costini;
  t = tini;
  tot_iter = 0;

  lastcost = cost;
    
  while(exit_crit(t, array_n(netl), cost) == 0) {
    av_cost = 0.;
    success_sum = 0;
    sum_of_squares = 0.;
    for(inner_iter = 0;inner_iter < move_lim; inner_iter++) {
      if(Swap(*clbl, netl, nx, ny, &cost, t, rlim, clbs_occ, io_occ,
        io_rat, bbox_new, bedge_new, bboxes, bedges, net_move, anets) == 1) {
        sum_of_squares += cost * cost;
        success_sum++;
        av_cost += cost;
      }
    }    
    fprintf(scost,"%d %f\n",count,cost);
    fprintf(simp, "%d %f\n",count, (float) (lastcost - cost)/lastcost);
    lastcost = cost;
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
    
    printf("%f %f %f %f %f %d %f\n", t, av_cost, success_rat, 
    std_dev, rlim, tot_iter, cost/array_n(netl));
    
    count++;
  }  
  t = 0;
  av_cost = 0.;
  success_sum = 0;
  sum_of_squares = 0.;

  for(inner_iter = 0; inner_iter < move_lim; inner_iter++) {
    if(Swap(*clbl, netl, nx, ny, &cost, t, rlim, clbs_occ, io_occ, io_rat,
       bbox_new, bedge_new, bboxes, bedges, net_move, anets) == 1) {
      sum_of_squares += cost * cost;
      success_sum++;
      av_cost += cost;
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
  (void) fclose(scost);
  (void) fclose(simp);
}

/**Function********************************************************************

  Synopsis           [Find an initial placement]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeIniPlaRand(
  netl_clbl_t **clbl,
  int nx,
  int ny,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  int io_rat)
{
int x, y, count, t, tsize;
struct xy { int x; int y; } *pos;
int *ior;
netl_clb_t *clb;

  tsize = MAX(nx*ny, 2*(nx + ny));
  pos = (struct xy *) ALLOC(struct xy, tsize);
  
  count = 0;
  for(x = 1;x <= nx;x++) {
    for(y = 1;y <= ny;y++) {
      pos[count].x = x;
      pos[count].y = y;
      count++;
    }
  }
  for(t = 0;t < array_n((*clbl)->clbs);t++) {    
    clb = array_fetch(netl_clb_t *, (*clbl)->clbs, t);
    x = util2_Irand(count - 1);
    clb->x = pos[x].x;
    clb->y = pos[x].y;
    clb->z = 0;
    clb->nio = -1;
    pos[x] = pos[count - 1];
    count--;
    clbs_occ[clb->x - 1][clb->y - 1] = clb;
  }
  
  count = 0;
  for(x = 1;x <= nx;x++) {
    pos[count].x = x;
    pos[count].y = 0;
    pos[count+1].x = x;
    pos[count+1].y = ny + 1;
    count += 2;
  }
  for(y = 1;y <= ny;y++) {
    pos[count].x = 0;
    pos[count].y = y;
    pos[count+1].x = nx + 1;
    pos[count+1].y = y;
    count += 2;
  }
  ior = (int *) ALLOC(int, count);
  for(x = 0;x < count;x++) ior[x] = 0;

  for(t = 0;t < array_n((*clbl)->inputs);t++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->inputs, t);
    x = util2_Irand(count - 1);
    clb->x = pos[x].x;
    clb->y = pos[x].y;
    clb->nio = ior[x];
    ior[x]++;
    if(ior[x] == io_rat) {
      pos[x] = pos[count - 1];
      ior[x] = ior[count - 1];
      count--;
    }
    if(clb->x == 0) {
      array_insert(netl_clb_t *, io_occ[0][clb->y - 1], clb->nio, clb);
    }
    else
      if(clb->y == 0) {
        array_insert(netl_clb_t *, io_occ[1][clb->x - 1], clb->nio, clb);
      }
      else
        if(clb->x == nx + 1) {
          array_insert(netl_clb_t *, io_occ[2][clb->y - 1], clb->nio, clb);
        }
        else
          if(clb->y == ny + 1) {
            array_insert(netl_clb_t *, io_occ[3][clb->x - 1], clb->nio, clb);
          }    
  }    
  for(t = 0;t < array_n((*clbl)->outputs);t++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->outputs, t);
    x = util2_Irand(count - 1);
    clb->x = pos[x].x;
    clb->y = pos[x].y;
    clb->nio = ior[x];
    ior[x]++;
    if(ior[x] == io_rat) {
      pos[x] = pos[count - 1];
      ior[x] = ior[count - 1];
      count--;
    }  
    if(clb->x == 0) {
      array_insert(netl_clb_t *, io_occ[0][clb->y - 1], clb->nio, clb);
    }
    else
      if(clb->y == 0) {
        array_insert(netl_clb_t *, io_occ[1][clb->x - 1], clb->nio, clb);
      }
      else
        if(clb->x == nx + 1) {
          array_insert(netl_clb_t *, io_occ[2][clb->y - 1], clb->nio, clb);
        }
        else
          if(clb->y == ny + 1) {
            array_insert(netl_clb_t *, io_occ[3][clb->x - 1], clb->nio, clb);
          }    
  }  
  FREE(pos);
  FREE(ior);
}

/**Function********************************************************************

  Synopsis           [Determine the initial temperature.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
float
placeStartingT(
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  float *cost, 
  int max_moves,
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
int num_accepted = 0, i, movelim;
double std_dev = 0, av = 0, sum_of_squares = 0;

  movelim = MIN(max_moves, array_n(clbl->clbs) + array_n(clbl->inputs) +
    array_n(clbl->outputs));

  for(i = 0;i < movelim;i++) {
    if(Swap(clbl, netl, nx, ny, cost, 1.e30, (float) nx, clbs_occ,
      io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges, net_move,
      anets) == 1) {
      num_accepted++;
      av += *cost;
      sum_of_squares += *cost * (*cost);      
    }
  }

  if(num_accepted != 0) {
    av /= num_accepted;
  }
  else
    av = 0;    
  std_dev = placeGetStdDev(num_accepted, sum_of_squares, av);

  return (20.*std_dev);
}

/**Function********************************************************************

  Synopsis           [Standard deviation calculator routine.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double 
placeGetStdDev(
  int n, 
  double sum_x_squared,
  double av_x)
{
double std_dev;
 
  if(n <= 1) {
    std_dev = 0.;
  }
  else {
    std_dev = (sum_x_squared - n * av_x * av_x)/(double) (n - 1);
  }

  if(std_dev > 0.) {
    std_dev = sqrt (std_dev);
  }
  else {
    std_dev = 0.;
  }
  return(std_dev);
}

/**Function********************************************************************

  Synopsis           [Select a position to be moved to.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void 
placeFindTo(
  int x_from,
  int y_from,
  int type,
  float rlim,
  int *x_to,
  int *y_to,
  int nx,
  int ny)
{
int x_rel, y_rel, iside, iplace, rlx, rly;

  rlx = MIN(nx,rlim);  
  rly = MIN(ny,rlim);  

  do {
    if(type == 0) {
      x_rel = util2_Irand(rlx*2);
      y_rel = util2_Irand(rly*2);
      *x_to = x_from - rlx + x_rel;
      *y_to = y_from - rly + y_rel;
      if(*x_to > nx) {
        *x_to = *x_to - nx; 
      }
      if(*x_to < 1) {
        *x_to = *x_to + nx;  
      }
      if(*y_to > ny) {
        *y_to = *y_to - ny; 
      }
      if(*y_to < 1) {
        *y_to = *y_to + ny;
      }
    }
    else {
      if(rlx >= nx) {
        iside = util2_Irand(3);
        switch(iside) {
          case 0: {
            iplace = util2_Irand(ny - 1) + 1;
            *x_to = 0;
            *y_to = iplace;
          } break;
          case 1: {
            iplace = util2_Irand(nx - 1) + 1;
            *x_to = iplace;
            *y_to = ny + 1;
          } break;
          case 2: {
            iplace = util2_Irand(ny - 1) + 1;
            *x_to = nx + 1;
            *y_to = iplace;
          } break;
          case 3: {
            iplace = util2_Irand(nx - 1) + 1;
            *x_to = iplace;
            *y_to = 0;
          } break;
          default: {
            printf("Error in find_to.  Unexpected io swap location.\n");
            exit (1);
          }
        }
     }
     else {   
       if(x_from == 0) {
         iplace = util2_Irand(2*rly);
         *y_to = y_from - rly + iplace;
         *x_to = x_from;
          if(*y_to > ny) {
            *y_to = ny + 1;
            *x_to = util2_Irand(rlx - 1) + 1;
          }
          else 
            if(*y_to < 1) {
              *y_to = 0;
              *x_to = util2_Irand(rlx - 1) + 1;
            }
        }
        else 
          if(x_from == nx + 1) {
            iplace = util2_Irand(2*rly);
            *y_to = y_from - rly + iplace;
            *x_to = x_from;
             if(*y_to > ny) {
               *y_to = ny + 1;
               *x_to = nx - util2_Irand(rlx - 1); 
             }
             else 
               if(*y_to < 1) {
                 *y_to = 0;
                 *x_to = nx - util2_Irand(rlx - 1);
               }
          }
          else 
            if(y_from == 0) {
              iplace = util2_Irand(2*rlx);
              *x_to = x_from - rlx + iplace;
              *y_to = y_from;
              if(*x_to > nx) {
                *x_to = nx + 1;
                *y_to = util2_Irand(rly - 1) + 1;
              }
              else 
                if(*x_to < 1) {
                  *x_to = 0;
                  *y_to = util2_Irand(rly -1) + 1;
                }
            }
            else {  
              iplace = util2_Irand (2*rlx);
              *x_to = x_from - rlx + iplace;
              *y_to = y_from;
              if(*x_to > nx) {
                *x_to = nx + 1;
                *y_to = ny - util2_Irand(rly - 1);
              }
              else 
                if(*x_to < 1) {
                  *x_to = 0;
                  *y_to = ny - util2_Irand(rly - 1);
                }
            }
         }    
      }    
  } while((x_from == *x_to) && (y_from == *y_to));
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
  float cost)
{
  if(t < (0.005 * cost/nnets)) {
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
int b_from, x_to, y_to, x_from, y_from, keep_switch, i, io;
float fnum, prob_fac;
float delta = 0.;
netl_clb_t *clb, *clb2 = NIL(netl_clb_t);
short isclb = 0;
int nc;

  b_from = util2_Irand(array_n(clbl->clbs) + array_n(clbl->inputs) +
    array_n(clbl->outputs) - 1);
    
  if(b_from < array_n(clbl->clbs)) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, b_from);
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
    isclb = 1;
  }
  else {
    if(b_from < (array_n(clbl->clbs) + array_n(clbl->inputs))) {
      b_from -= array_n(clbl->clbs);
      clb = array_fetch(netl_clb_t *, clbl->inputs, b_from);
    }
    else {
      b_from = b_from - array_n(clbl->inputs) - array_n(clbl->clbs);
      clb = array_fetch(netl_clb_t *, clbl->outputs, b_from);    
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
  }

/*  util2_StartTimer();*/
  nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
/*  util2_StopTimer();
  tempo += util2_GetRtime();*/
  
  for(i = 0;i < nc;i++) {
    if(array_n(anets[i]->snet) <= SMALL_NET) {
      placeGetNUbb(anets[i], &bbox_new[i], nx, ny);
    }
    else {      
      if(net_move[i] == FROM) {
        placeUpdateBb(&bbox_new[i], &bedge_new[i], bboxes[anets[i]->id], 
          bedges[anets[i]->id], anets[i], x_from, y_from, x_to, y_to, nx, ny);   
      }
      else {
        placeUpdateBb(&bbox_new[i], &bedge_new[i], bboxes[anets[i]->id],
          bedges[anets[i]->id], anets[i], x_to, y_to, x_from, y_from, nx, ny);
      }
    }    
    anets[i]->tempcost = placeNetCost(anets[i], bbox_new[i]);
    delta += anets[i]->tempcost - anets[i]->cost;
  }
  
  if(delta <= 0) {
    keep_switch = 1;    
  }
  else
    if(t > 0.) {
      fnum = util2_Frand();
      prob_fac = exp(-delta/t);
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

  if(keep_switch == 0) {
    if(clb2 != NIL(netl_clb_t)) {
      clb2->x = x_to;
      clb2->y = y_to;
    }
    clb->x = x_from;
    clb->y = y_from;
  }
  else {  
    if(isclb == 1) {
      if(clb2 != NIL(netl_clb_t)) {
        clbs_occ[x_from - 1][y_from - 1] = clb2;
      }
      else {
        clbs_occ[x_from - 1][y_from - 1] = NIL(netl_clb_t);
      }
      clbs_occ[x_to - 1][y_to - 1] = clb;    
    }
    else {
      if(clb2 != NIL(netl_clb_t)) {
        if(x_from == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_from - 1], clb->nio, clb2);
        }
        else 
          if(y_from == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_from - 1], clb->nio, clb2);
          }
          else
            if(x_from == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_from - 1], clb->nio, clb2);
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_from - 1], clb->nio, clb2);
            }
        clb2->nio = clb->nio;
      }
      else {
        if(x_from == 0) {
          array_insert(netl_clb_t *, io_occ[0][y_from - 1], clb->nio, 
            NIL(netl_clb_t));
        }
        else
          if(y_from == 0) {
            array_insert(netl_clb_t *, io_occ[1][x_from - 1], clb->nio,
              NIL(netl_clb_t));
          }
          else
            if(x_from == nx + 1) {
              array_insert(netl_clb_t *, io_occ[2][y_from - 1], clb->nio, 
                NIL(netl_clb_t));
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][x_from - 1], clb->nio, 
                NIL(netl_clb_t));
            }          
      } 
      if(x_to == 0) {
        array_insert(netl_clb_t *, io_occ[0][y_to - 1], io, clb);
      }
      else 
        if(y_to == 0) {
          array_insert(netl_clb_t *, io_occ[1][x_to - 1], io, clb);
        }
        else
          if(x_to == nx + 1) {
            array_insert(netl_clb_t *, io_occ[2][y_to - 1], io, clb);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][x_to - 1], io, clb);
          }
      clb->nio = io;
    }          
    
    for(i = 0;i < nc;i++) {
      anets[i]->cost = anets[i]->tempcost;
      if(array_n(anets[i]->snet) > SMALL_NET) {
        bedges[anets[i]->id]->xmax = bedge_new[i]->xmax;
        bedges[anets[i]->id]->ymax = bedge_new[i]->ymax;
        bedges[anets[i]->id]->xmin = bedge_new[i]->xmin;
        bedges[anets[i]->id]->ymin = bedge_new[i]->ymin;
      }
      bboxes[anets[i]->id]->xmax = bbox_new[i]->xmax;
      bboxes[anets[i]->id]->ymax = bbox_new[i]->ymax;
      bboxes[anets[i]->id]->xmin = bbox_new[i]->xmin;
      bboxes[anets[i]->id]->ymin = bbox_new[i]->ymin;
    }    
    (*cost) = (*cost) + delta;
  }  
  
  return(keep_switch);
}
