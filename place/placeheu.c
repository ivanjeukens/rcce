/**CFile***********************************************************************

  FileName    [placeHeu.c]

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

#define SEQUENTIAL 1
#define RANDOM 2

#define CLBS 1
#define INPUTS 2
#define OUTPUTS 3

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void DoIo(short order, float *bestcost, float *cost, array_t *data, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);
static void DoClb(short order, float *bestcost, float *cost, array_t *data, array_t *netl, int nx, int ny, netl_clb_t ***clbs_occ, array_t ***io_occ, int io_rat, bb_t **bbox_new, bb_t **bedge_new, bb_t **bboxes, bb_t **bedges, int *net_move, netl_net_t **anets);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Place the clb list based on a sifting like heuristic.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeSift(
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
float bestcost, cost;
char buf[100];
greedyconf_t *conf;
FILE *fp;

  fp = fopen("greedy.conf","r");
  if(fp == NIL(FILE)) {
    fprintf(stderr,"Could not open file greedy.conf\n");
    exit(1);
  }
  conf = (greedyconf_t *) ALLOC(greedyconf_t, 1);
  conf->first = INPUTS;
  conf->second = OUTPUTS;
  conf->third = CLBS;
  conf->order = SEQUENTIAL;
  (void) fscanf(fp, "%s", buf);
  while(!feof(fp)) {
    if(strcmp(buf,"first:") == 0) {
      (void) fscanf(fp, "%s", buf);
      if(strcmp(buf,"clbs") == 0) {
        conf->first = CLBS;
      }
      else
      if(strcmp(buf,"inputs") == 0) {
        conf->first = INPUTS;
      }
      else
      if(strcmp(buf,"outputs") == 0) {
        conf->first = OUTPUTS;
      }
    }
    else
    if(strcmp(buf,"second:") == 0) {
      (void) fscanf(fp, "%s", buf);
      if(strcmp(buf,"clbs") == 0) {
        conf->second = CLBS; 
      }
      else
      if(strcmp(buf,"inputs") == 0) {
        conf->second = INPUTS;
      }
      else
      if(strcmp(buf,"outputs") == 0) {
        conf->second = OUTPUTS;
      }    
    }
    else
    if(strcmp(buf,"third:") == 0) {
      (void) fscanf(fp, "%s", buf);
      if(strcmp(buf,"clbs") == 0) {
        conf->third = CLBS;      
      }
      else
      if(strcmp(buf,"inputs") == 0) {
        conf->third = INPUTS;
      }
      else
      if(strcmp(buf,"outputs") == 0) {
        conf->third = OUTPUTS;
      }    
    }
    else
    if(strcmp(buf,"order:") == 0) {
      (void) fscanf(fp,"%s",buf);
      if(strcmp(buf,"sequential") == 0) {
        conf->order = SEQUENTIAL;
      }
      else
      if(strcmp(buf,"random") == 0) {
        conf->order = RANDOM;
      }
    }  
    (void) fscanf(fp, "%s", buf);
  }
  (void) fclose(fp);

  cost = *hfc;
  bestcost = cost;  

  switch(conf->first) {
    case CLBS:
      DoClb(conf->order, &bestcost, &cost, (*clbl)->clbs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;
    case INPUTS:
      DoIo(conf->order, &bestcost, &cost, (*clbl)->inputs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;
    case OUTPUTS:
      DoIo(conf->order, &bestcost, &cost, (*clbl)->outputs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;    
  }

  switch(conf->second) {
    case CLBS:
      DoClb(conf->order, &bestcost, &cost, (*clbl)->clbs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;
    case INPUTS:
      DoIo(conf->order, &bestcost, &cost, (*clbl)->inputs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;
    case OUTPUTS:
      DoIo(conf->order, &bestcost, &cost, (*clbl)->outputs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;    
  }

  switch(conf->third) {
    case CLBS:
      DoClb(conf->order, &bestcost, &cost, (*clbl)->clbs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;
    case INPUTS:
      DoIo(conf->order, &bestcost, &cost, (*clbl)->inputs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;
    case OUTPUTS:
      DoIo(conf->order, &bestcost, &cost, (*clbl)->outputs, netl, nx, ny,
        clbs_occ, io_occ, io_rat, bbox_new, bedge_new, bboxes, bedges,
        net_move, anets);
      break;    
  }
    
  *hfc = cost;
}

/**Function********************************************************************

  Synopsis           [A local search heuristic based on swaping a clb.]

  Description        [RNA]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeSwap(
  int maxnoimp,
  float rlim,
  float *cost,
  netl_clbl_t **clbl,
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
int j, k, nc, noimp = 0;
netl_clb_t *clb, *clb2;
float delta;
int x_from, y_from, x_to, y_to, io;

  while(noimp < maxnoimp) {
    j = util2_Irand(2);
    if(j == 0) {
      j = util2_Irand(array_n((*clbl)->clbs) - 1);
      clb = array_fetch(netl_clb_t *, (*clbl)->clbs, j);
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
      delta = 0.;
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
        delta += anets[k]->tempcost - anets[k]->cost;       
      }

      if(delta <= 0) {
        if(clb2 != NIL(netl_clb_t)) {
          clbs_occ[x_from - 1][y_from - 1] = clb2;
        }
        else {
          clbs_occ[x_from - 1][y_from - 1] = NIL(netl_clb_t);
        }
        clbs_occ[x_to - 1][y_to - 1] = clb;

        for(k = 0;k < nc;k++) {
          anets[k]->cost = anets[k]->tempcost;
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
        }    
        (*cost) = (*cost) + delta;
        if(delta == 0) {
          noimp++;
        }
        else {
          noimp = 0;
        }
      }
      else {
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = x_to;
          clb2->y = y_to;
        }
        clb->x = x_from;
        clb->y = y_from;
        noimp++;
      }
    }
    else {
      if(j == 1) {
        j = util2_Irand(array_n((*clbl)->inputs) - 1);
        clb = array_fetch(netl_clb_t *, (*clbl)->inputs, j);
      }
      else {
        j = util2_Irand(array_n((*clbl)->outputs) - 1);
        clb = array_fetch(netl_clb_t *, (*clbl)->outputs, j);
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
      delta = 0.;      
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
        delta += anets[k]->tempcost - anets[k]->cost;       
      }          

      if(delta <= 0) {
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
        
        for(k = 0;k < nc;k++) {
          anets[k]->cost = anets[k]->tempcost;
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
        }
        (*cost) = (*cost) + delta;
        if(delta == 0) {
          noimp++;
        }
        else {
          noimp = 0;
        }
      }
      else {
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = x_to;
          clb2->y = y_to;
        }
        clb->x = x_from;
        clb->y = y_from;
        noimp++;
      }   
    }
  } 
  printf("Final RNA cost %4.5f\n", *cost);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [A local search heuristic based on swaping a clb]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
DoIo(
  short order,
  float *bestcost,
  float *cost,
  array_t *data,
  array_t *netl,
  int nx,
  int ny,
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
float delta;
int i, n, origx, origy, bestx, besty, io, bestio, j, nc, count;
netl_clb_t *clb, *clb2 = NIL(netl_clb_t);
int *pos;

  if(order == RANDOM) {
    pos = (int *) ALLOC(int, array_n(data));
    count = array_n(data);
    for(i = 0;i < count;i++) {
      pos[i] = i;
    }  
  }

  for(i = 0;i < array_n(data);i++) {
    if(order == SEQUENTIAL) {
      clb = array_fetch(netl_clb_t *, data, i);
    }
    else
    if(order == RANDOM) {
      n = util2_Irand(count - 1);
      clb = array_fetch(netl_clb_t *, data, pos[n]);
      pos[n] = pos[count - 1];
      count--;    
    }
    if(clb->noswap == 1) continue;
    origx = clb->x;
    origy = clb->y;
    bestx = clb->x;
    besty = clb->y;
    bestio = clb->nio;
    for(n = 1;n <= ny;n++) {
      io = util2_Irand(io_rat - 1);
      clb2 = array_fetch(netl_clb_t *, io_occ[0][n - 1], io);
      if((origx == 0) && (origy == n)) continue;
      if((clb2 != NIL(netl_clb_t)) && (clb2->noswap == 1)) continue;
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
      }  
      clb->x = 0;
      clb->y = n;
      delta = 0.;
      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      for(j = 0;j < nc;j++) {
        if(array_n(anets[j]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
        }
        else {
          if(net_move[j] == FROM) {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], origx, origy, 0, n, nx, ny);   
          }
          else {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], 0, n, origx, origy, nx, ny);
          }
        }
        delta += placeNetCost(anets[j], bbox_new[j]) - anets[j]->cost;
      }
      if(((*cost) + delta) < (*bestcost)) {
        (*bestcost) = (*cost) + delta;
        bestx = 0;
        besty = n;
        bestio = io;
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = 0;
        clb2->y = n;
      }    
    }
    for(n = 1;n <= nx;n++) {
      io = util2_Irand(io_rat - 1);
      clb2 = array_fetch(netl_clb_t *, io_occ[1][n - 1], io);
      if((origx == n) && (origy == 0)) continue;
      if((clb2 != NIL(netl_clb_t)) && (clb2->noswap == 1)) continue;
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
      }  
      clb->x = n;
      clb->y = 0;
      delta = 0.;
      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      for(j = 0;j < nc;j++) {
        if(array_n(anets[j]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
        }
        else {
          if(net_move[j] == FROM) {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], origx, origy, n, 0, nx, ny);   
          }
          else {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], n, 0, origx, origy, nx, ny);
          }
        }
        delta += placeNetCost(anets[j], bbox_new[j]) - anets[j]->cost;
      }
      if(((*cost) + delta) < (*bestcost)) {
        (*bestcost) = (*cost) + delta;
        bestx = n;
        besty = 0;
        bestio = io;
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = n;
        clb2->y = 0;        
      }    
    }
    for(n = 1;n <= ny;n++) {
      io = util2_Irand(io_rat - 1);
      clb2 = array_fetch(netl_clb_t *, io_occ[2][n - 1], io);
      if((origx == nx+1) && (origy == n)) continue;
      if((clb2 != NIL(netl_clb_t)) && (clb2->noswap == 1)) continue;
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
      }  
      clb->x = nx+1;
      clb->y = n;
      delta = 0.;
      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      for(j = 0;j < nc;j++) {
        if(array_n(anets[j]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
        }
        else {
          if(net_move[j] == FROM) {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], origx, origy, nx+1, n, nx, ny);   
          }
          else {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], nx+1, n, origx, origy, nx, ny);
          }                
        }
        delta += placeNetCost(anets[j], bbox_new[j]) - anets[j]->cost;
      }
      if(((*cost) + delta) < (*bestcost)) {
        (*bestcost) = (*cost) + delta;
        bestx = nx+1;
        besty = n;
        bestio = io;
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = nx+1;
        clb2->y = n;        
      }    
    }
    for(n = 1;n <= nx;n++) {
      io = util2_Irand(io_rat - 1);
      clb2 = array_fetch(netl_clb_t *, io_occ[3][n - 1], io);
      if((origx == n) && (origy == ny+1)) continue;
      if((clb2 != NIL(netl_clb_t)) && (clb2->noswap == 1)) continue;
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
      }  
      clb->x = n;
      clb->y = ny+1;
      delta = 0.;
      nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
      for(j = 0;j < nc;j++) {
        if(array_n(anets[j]->snet) <= SMALL_NET) {
          placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
        }
        else {
          if(net_move[j] == FROM) {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], origx, origy, n, ny+1, nx, ny);   
          }
          else {
            placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
              bedges[anets[j]->id], anets[j], n, ny+1, origx, origy, nx, ny);
          }                
        }
        delta += placeNetCost(anets[j], bbox_new[j]) - anets[j]->cost;
      }
      if(((*cost) + delta) < (*bestcost)) {
        (*bestcost) = (*cost) + delta;
        bestx = n;
        besty = ny+1;
        bestio = io;
      }
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = n;
        clb2->y = ny+1;
      }    
    }
    if((*bestcost) < (*cost)) {
      (*cost) = (*bestcost);
      if(bestx == 0) {
        clb2 = array_fetch(netl_clb_t *, io_occ[0][besty - 1], bestio);
      }
      else
        if(besty == 0) {
          clb2 = array_fetch(netl_clb_t *, io_occ[1][bestx - 1], bestio);
        }
        else
          if(bestx == nx+1) {
            clb2 = array_fetch(netl_clb_t *, io_occ[2][besty - 1], bestio);
          }
          else {
            clb2 = array_fetch(netl_clb_t *, io_occ[3][bestx - 1], bestio); 
          }        
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
        clb2->nio = clb->nio;
        if(origx == 0) {
          array_insert(netl_clb_t *, io_occ[0][origy - 1], clb2->nio, clb2);
        }
        else
          if(origy == 0) {
            array_insert(netl_clb_t *, io_occ[1][origx - 1], clb2->nio, clb2);
          }
          else
            if(origx == nx+1) {
              array_insert(netl_clb_t *, io_occ[2][origy - 1], clb2->nio, clb2);
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][origx - 1], clb2->nio, clb2);
            }                   
      }
      else {
        if(origx == 0) {
          array_insert(netl_clb_t *, io_occ[0][origy - 1], clb->nio, 
            NIL(netl_clb_t));
        }
        else
          if(origy == 0) {
            array_insert(netl_clb_t *, io_occ[1][origx - 1], clb->nio,
              NIL(netl_clb_t));
          }
          else
            if(origx == nx+1) {
              array_insert(netl_clb_t *, io_occ[2][origy - 1], clb->nio, 
                NIL(netl_clb_t));
            }
            else {
              array_insert(netl_clb_t *, io_occ[3][origx - 1], clb->nio,
                NIL(netl_clb_t));
            }                   
      }
      clb->x = bestx;
      clb->y = besty;
      clb->nio = bestio;
      if(bestx == 0) {
        array_insert(netl_clb_t *, io_occ[0][besty - 1], bestio, clb);
      }
      else
        if(besty == 0) {
          array_insert(netl_clb_t *, io_occ[1][bestx - 1], bestio, clb);
        }
        else
          if(bestx == nx+1) {
            array_insert(netl_clb_t *, io_occ[2][besty - 1], bestio, clb);
          }
          else {
            array_insert(netl_clb_t *, io_occ[3][bestx - 1], bestio, clb);
          }
       
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
      clb->x = origx;
      clb->y = origy;     
    }
  }
  if(order == RANDOM) {
    FREE(pos);
  }
}

/**Function********************************************************************

  Synopsis           [A local search heuristic based on swaping a clb]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
DoClb(
  short order,
  float *bestcost,
  float *cost,
  array_t *data,
  array_t *netl,
  int nx,
  int ny,
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
float delta;
int i, n, m, origx, origy, bestx, besty, j, nc, count;
netl_clb_t *clb, *clb2 = NIL(netl_clb_t);
int *pos;

  if(order == RANDOM) {
    pos = (int *) ALLOC(int, array_n(data));
    count = array_n(data);
    for(i = 0;i < count;i++) {
      pos[i] = i;
    }  
  }
  for(i = 0;i < array_n(data);i++) {
    if(order == SEQUENTIAL) {
      clb = array_fetch(netl_clb_t *, data, i);
    }
    else
    if(order == RANDOM) {
      n = util2_Irand(count - 1);
      clb = array_fetch(netl_clb_t *, data, pos[n]);
      pos[n] = pos[count - 1];
      count--;    
    }
    if(clb->noswap == 1) continue;
    origx = clb->x;
    origy = clb->y;
    bestx = clb->x;
    besty = clb->y;
    for(m = 1;m <= nx;m++) {
      for(n = 1;n <= ny;n++) {
        if((m == origx) && (n == origy)) continue;
        clb2 = clbs_occ[m - 1][n - 1];
        if((clb2 != NIL(netl_clb_t)) && (clb2->noswap == 1)) continue;
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = origx;
          clb2->y = origy;
        }
        clb->x = m;
        clb->y = n;
        delta = 0.;
        nc = placeAffectedNets(&anets, netl, clb, clb2, &net_move);
        for(j = 0;j < nc;j++) {
          if(array_n(anets[j]->snet) <= SMALL_NET) {
            placeGetNUbb(anets[j], &bbox_new[j], nx, ny);
          }
          else {
            if(net_move[j] == FROM) {
              placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
                bedges[anets[j]->id], anets[j], origx, origy, m, n, nx, ny);   
            }
            else {
              placeUpdateBb(&bbox_new[j], &bedge_new[j], bboxes[anets[j]->id],
                bedges[anets[j]->id], anets[j], m, n, origx, origy, nx, ny);
            }                
          }
          delta += placeNetCost(anets[j], bbox_new[j]) - anets[j]->cost;
        }
        if(((*cost) + delta) < (*bestcost)) {
          (*bestcost) = (*cost) + delta;
          bestx = m;
          besty = n;
        }
        if(clb2 != NIL(netl_clb_t)) {
          clb2->x = m;
          clb2->y = n;        
        }
      }
    }
    if((*bestcost) < (*cost)) {
      (*cost) = (*bestcost);
      clb2 = clbs_occ[bestx - 1][besty - 1];   
      if(clb2 != NIL(netl_clb_t)) {
        clb2->x = origx;
        clb2->y = origy;
        clbs_occ[origx - 1][origy - 1] = clb2;
      }
      else {
        clbs_occ[origx - 1][origy - 1] = NIL(netl_clb_t);
      }
      clb->x = bestx;
      clb->y = besty;
      clbs_occ[bestx - 1][besty - 1] = clb;     

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
      clb->x = origx;
      clb->y = origy;    
    }
  }

  if(order == RANDOM) {
    FREE(pos);
  }
}
