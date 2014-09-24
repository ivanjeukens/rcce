/**CFile***********************************************************************

  FileName    [placeGA.c]

  PackageName [place]

  Synopsis    [Genetic algorithm.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

******************************************************************************/

#include "placeInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define ROULETTE 1
#define RANDOM 2

#define INVERSE 1
#define PROPORTIONAL 2

#define DIRECT 1
#define BEST 2
#define COMBINED 3
#define PRESELECTION 4

int nmut;
int ncross;
int nalt;
int dch;

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

static void Generational(gaconf_t *conf, population_t *pop, netl_clbl_t **clbl, array_t *netl, float *cost, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, int *net_move, netl_net_t **anets, xy_t **neigh);
static void SteadyState(gaconf_t *conf, population_t *pop, netl_clbl_t **clbl, array_t *netl, float *cost, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, int *net_move, netl_net_t **anets, xy_t **neigh);
static void Replace(population_t *pop, cromossome_t *ch1, cromossome_t *ch2, cromossome_t *p1, int p1pos, cromossome_t *p2, int p2pos, gaconf_t *conf);
static void Scale(population_t *pop, float coef, int popsize);
static cromossome_t * Best(population_t *pop, int popsize);
static int Crossover(gaconf_t *conf, cromossome_t *parent1, cromossome_t *parent2, cromossome_t **child1, cromossome_t **child2, int nx, int ny, int io_rat, int npt, int nclb);
static int Mutate(allele_t **al, int nx, int ny, int io_rat, float prob);
static cromossome_t * RouletteSelect(population_t *pop, int *pos, int invpos, int popsize);
static cromossome_t * RandomSelect(population_t *pop, int *pos, int invpos, int popsize);
static population_t * PopInit(gaconf_t *conf, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, xy_t **neigh);
static void PopFree(population_t *pop, int popsize);
static void CromoFree(cromossome_t *cromo);
static cromossome_t * CromoCopy(cromossome_t *cromo);
static float CromoCost(cromossome_t *cromo, netl_clbl_t *clbl, array_t *netl, int nx, int ny);
static void Decode(netl_clbl_t **clbl, cromossome_t *cromo, netl_clb_t ***clbs_occ, array_t ***io_occ, int nx, int ny);
static void Encode(cromossome_t *cromo, netl_clbl_t *clbl);
static void PopPrint(population_t *pop, int popsize);
static cromossome_t * Local(cromossome_t *cromo, int nclb, int nio, netl_clbl_t **clbl, array_t *netl, int nx, int ny, int io_rat, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges, bb_t **bbox_new, bb_t **bedge_new, netl_net_t **anets, int *net_move, xy_t **neigh);
static void BugPop(population_t *pop, int nx, int ny, int io_rat, int nc, int popsize);
static short Bug(cromossome_t *cromo, int nx, int ny, int io_rat, int nc);
static void PopFit(gaconf_t *conf, population_t *pop, netl_clbl_t *clbl, array_t *netl, netl_clb_t ***clbs_occ, array_t ***io_occ, bb_t **bboxes, bb_t **bedges);
static void Verify(cromossome_t *cromo, int nc, int nio, int nx, int ny, int io_rat);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initialize the population.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeGA(
  netl_clbl_t **clbl,
  array_t *netl,
  float *cost,
  int nx, 
  int ny,
  int io_rat,
  short generational,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  int *net_move,
  netl_net_t **anets)
{
population_t *pop;
int ns, i;
xy_t **neigh;
gaconf_t *conf;
FILE *fp;
char buf[100];

  conf = (gaconf_t *) ALLOC(gaconf_t, 1);
  fp = fopen("gene.conf","r");
  if(fp == NIL(FILE)) {
    fprintf(stderr,"couldn't open file gene.conf\n");
    return;
  }
  
  conf->scale = 0;
  conf->select1 = ROULETTE;
  conf->select2 = RANDOM;
  conf->popsize = 40;
  conf->probcross = 0.8;
  conf->probmut = 0.001;
  conf->nsite = 1;
  conf->niter = 50;
  conf->scalecoef = 2.0;
  conf->local = 0;
  conf->localstart = 0;
  conf->loclb = 0.5;
  conf->loio = 0.75;
  conf->fitness = INVERSE;
  conf->replacement = DIRECT;
  
  (void) fscanf(fp, "%s", buf);
  while(!feof(fp)) {
    if(strcmp(buf, "scale:") == 0) {
      fscanf(fp,"%s", buf);
      conf->scale = atoi(buf);
    }
    else
    if(strcmp(buf,"select1:") == 0) {
      fscanf(fp,"%s", buf);
      if(strcmp(buf,"roulette") == 0) {
        conf->select1 = ROULETTE;
      }
      else
      if(strcmp(buf,"random") == 0) {
        conf->select1 = RANDOM;
      }
    }
    else
    if(strcmp(buf,"select2:") == 0) {
      fscanf(fp,"%s", buf);
      if(strcmp(buf,"roulette") == 0) {
        conf->select2 = ROULETTE;
      }
      else
      if(strcmp(buf,"random") == 0) {
        conf->select2 = RANDOM;
      }        
    }
    else
    if(strcmp(buf,"popsize:") == 0) {
      fscanf(fp,"%s", buf);
      conf->popsize = atoi(buf);         
    }
    else
    if(strcmp(buf,"probcross:") == 0) {
      fscanf(fp,"%s", buf);
      conf->probcross = atof(buf);            
    }
    else
    if(strcmp(buf,"nsite:") == 0) {
      fscanf(fp,"%s", buf);
      conf->nsite = atoi(buf);              
    }
    else
    if(strcmp(buf,"probmut:") == 0) {
      fscanf(fp,"%s", buf);
      conf->probmut = atof(buf);                
    }
    else
    if(strcmp(buf,"niter:") == 0) {
      fscanf(fp,"%s", buf);
      conf->niter = atoi(buf);                  
    }
    else
    if(strcmp(buf,"scalecoef:") == 0) {
      fscanf(fp,"%s", buf);
      conf->scalecoef = atof(buf);                    
    }
    else
    if(strcmp(buf,"local:") == 0) {
      fscanf(fp,"%s", buf);
      conf->local = atoi(buf);                      
    }
    else
    if(strcmp(buf,"localstart:") == 0) {
      fscanf(fp,"%s", buf);
      conf->localstart = atoi(buf);
    }
    else
    if(strcmp(buf,"loclb:") == 0) {
      fscanf(fp,"%s", buf);
      conf->loclb = atof(buf);                          
    }
    else
    if(strcmp(buf,"loio:") == 0) {
      fscanf(fp,"%s", buf);
      conf->loio = atof(buf);                            
    }
    else
    if(strcmp(buf,"fitness:") == 0) {
      fscanf(fp,"%s", buf);
      if(strcmp(buf,"inverse") == 0) {
        conf->fitness = INVERSE;
      }                         
      else
      if(strcmp(buf,"proportional") == 0) {
        conf->fitness = PROPORTIONAL;
      }
    }
    else
    if(strcmp(buf,"replacement:") == 0) {
      fscanf(fp,"%s", buf);
      if(strcmp(buf,"direct") == 0) {
        conf->replacement = DIRECT;
      }
      else
      if(strcmp(buf,"best") == 0) {
        conf->replacement = BEST;
      }
      else
      if(strcmp(buf,"combined") == 0) {
        conf->replacement = COMBINED;
      }
      else
      if(strcmp(buf,"preselection") == 0) {
        conf->replacement = PRESELECTION;
      }      
    }
    fscanf(fp,"%s", buf);
  }
  (void) fclose(fp);
  
  ns = MAX(nx*ny, 2*(nx + ny)*io_rat);
  neigh = (xy_t **) ALLOC(xy_t *, ns);
  for(i = 0;i < ns;i++) {
    neigh[i] = (xy_t *) ALLOC(xy_t, 1);
  }
  
  pop = PopInit(conf, *clbl, netl, nx, ny, io_rat, clbs_occ, io_occ,
    bboxes, bedges, bbox_new, bedge_new, anets, net_move, neigh);
  
  if(generational == GENERATIONAL) {
    Generational(conf, pop, clbl, netl, cost, nx, ny, io_rat, clbs_occ, io_occ,
      bboxes, bedges, bbox_new, bedge_new, net_move, anets, neigh);
  }
  else {
    SteadyState(conf, pop, clbl, netl, cost, nx, ny, io_rat, clbs_occ, io_occ,
      bboxes, bedges, bbox_new, bedge_new, net_move, anets, neigh);
  }

  for(i = 0;i < ns;i++) {
    FREE(neigh[i]);
  }
  FREE(neigh);
  FREE(conf);
}

/**Function********************************************************************

  Synopsis           [Mutation Operator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
placeEncodePos(
  int x, 
  int y,
  int nio,
  int nx, 
  int ny,
  int io_rat)
{
  if(nio == -1) {
    return(y*ny + x - 1);
  }
  else {
    if(y == 0) {
      return (x*io_rat + nio);
    }
    else
      if(x == nx+1)  {
        return (io_rat*nx + y*io_rat + nio);
      }
      else
        if(y == ny+1) {
          return (io_rat*nx + io_rat*ny + x*io_rat + nio);
        }
        else {
          return(2*io_rat*nx + io_rat*ny + y*io_rat + nio);
        }        
  }
}

/**Function********************************************************************

  Synopsis           [Mutation Operator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeDecodePos(
  int pos,
  int *x, 
  int *y,
  int *nio,
  int nx, 
  int ny,
  int io_rat)
{
  if(*nio == -1) {
    *y = (int) pos/ny;
    *x = (int) pos%ny + 1;
    *nio = -1;
  }
  else {
    if(pos < (nx*io_rat + io_rat)) {
      *y = 0;
      *x = (int) pos/io_rat;
      *nio = (int) pos%io_rat;
    }
    else
      if(pos < (ny*io_rat + nx*io_rat + io_rat)) {
        *x = nx+1;
        *y = (int) (pos - nx*io_rat)/io_rat;
        *nio = (int) (pos - nx*io_rat)%io_rat;
      }
      else
        if(pos < (2*nx*io_rat + ny*io_rat + io_rat)) {
          *y = ny+1; 
          *x = (int) (pos - nx*io_rat - ny*io_rat)/io_rat;
          *nio = (int) (pos - nx*io_rat - ny*io_rat)%io_rat;
        }
        else {
          *x = 0;
          *y = (int) (pos - 2*nx*io_rat - ny*io_rat)/io_rat;
          *nio = (int) (pos - 2*nx*io_rat - ny*io_rat)%io_rat;
        }
  }
}
/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [A generational genetic algorithm.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Generational(
  gaconf_t *conf,
  population_t *pop,
  netl_clbl_t **clbl,  
  array_t *netl,
  float *cost,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  int *net_move,
  netl_net_t **anets,
  xy_t **neigh)
{  
population_t *newpop;
cromossome_t *p1, *p2, *ch1, *ch2, *alltime;
int i = 0, j, p1pos, p2pos, tot_gen = 0, ret;
float sumonline = 0., sumoffline = 0.;
FILE *fon, *foff, *fmax, *fmin, *fdch, *falt, *fstd;

  fon = fopen("online.dat","w");
  foff = fopen("offline.dat","w");
  fmax = fopen("maxfit.dat","w");
  fmin = fopen("minfit.dat","w");
  fdch = fopen("dch.dat","w");
  falt = fopen("falt.dat","w");
  fstd = fopen("stddev.dat","w");
  
  p1 = Best(pop, conf->popsize);
  alltime = CromoCopy(p1);

  if(conf->scale == 1) {
    Scale(pop, conf->scalecoef, conf->popsize);
  }
  printf("%5d  %4.7f  %4.7f  %4.7f\n", 0, pop->maxfit, pop->minfit,
    alltime->cost);  

  for(i = 1;i <= conf->niter;i++) {
    newpop = (population_t *) ALLOC(population_t, 1);
    newpop->cromo = array_alloc(cromossome_t *, conf->popsize);
    newpop->nx = nx;
    newpop->ny = ny;
    newpop->io_rat = io_rat;
    newpop->maxcost = -1;
    newpop->mincost = INFINITY;
    newpop->avgcost = 0.;
    newpop->squared_sumcost = 0.;
    newpop->sumfit = 0.;
    newpop->maxfit = -1;
    newpop->minfit = INFINITY;
    nmut = 0;
    ncross = 0;
    nalt = 0;
    dch = 0;
    for(j = 0;j < conf->popsize;j += 2) {
      if(conf->select1 == ROULETTE) {
        p1 = RouletteSelect(pop, &p1pos, -1, conf->popsize);
      }
      else
      if(conf->select1 == RANDOM) {
        p1 = RandomSelect(pop, &p1pos, -1, conf->popsize);
      }

      if(conf->select2 == ROULETTE) {
        p2 = RouletteSelect(pop, &p2pos, p1pos, conf->popsize);
      }
      else
      if(conf->select2 == RANDOM) {
        p2 = RandomSelect(pop, &p2pos, p1pos, conf->popsize);
      }
            
      ret = Crossover(conf, p1, p2, &ch1, &ch2, nx, ny, io_rat, conf->nsite,
        array_n((*clbl)->clbs));
      
      if(ret == 1) {
        ch1->cost = CromoCost(ch1, *clbl, netl, pop->nx, pop->ny);
        if(conf->local == 1) {
          Local(ch1, nx*ny*conf->loclb, 2*(nx + ny)*conf->loio, clbl, netl,
            nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new,
            bedge_new, anets, net_move, neigh);
        }
        ch2->cost = CromoCost(ch2, *clbl, netl, pop->nx, pop->ny);
        if(conf->local == 1) {
          Local(ch2, nx*ny*conf->loclb, 2*(nx + ny)*conf->loio, clbl, netl,
            nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new,
            bedge_new, anets, net_move, neigh);      
        }        

        if(conf->replacement == BEST) {       
          if(ch1->cost > p1->cost) {
            CromoFree(ch1);
            ch1 = CromoCopy(p1);
            dch++;
          }
          if(ch2->cost > p2->cost) {
            CromoFree(ch2);
            ch2 = CromoCopy(p2);
            dch++;          
          }
        }
      }
      else {
        ch1 = CromoCopy(p1);
        ch2 = CromoCopy(p2);
      }
      
      if(ch1->cost < newpop->mincost) {
        newpop->mincost = ch1->cost;
      }
      if(ch1->cost > newpop->maxcost) {
        newpop->maxcost = ch1->cost;
      }
      if(ch2->cost < newpop->mincost) {
        newpop->mincost = ch2->cost;
      }
      if(ch2->cost > newpop->maxcost) {
        newpop->maxcost = ch2->cost;
      }
      sumonline += ch1->cost + ch2->cost;
      tot_gen += 2;

      newpop->squared_sumcost += ch1->cost*ch1->cost + ch2->cost*ch2->cost;
      newpop->avgcost += ch1->cost + ch2->cost;
      
      array_insert(cromossome_t *, newpop->cromo, j, ch1);
      array_insert(cromossome_t *, newpop->cromo, j+1, ch2);
    }    
    newpop->avgcost = newpop->avgcost/conf->popsize;
    newpop->stddev = placeGetStdDev(conf->popsize, newpop->squared_sumcost,
      newpop->avgcost);

    for(j = 0;j < conf->popsize;j++) {
      p1 = array_fetch(cromossome_t *, newpop->cromo, j);
      if(conf->fitness == PROPORTIONAL) {
        p1->fitness = (newpop->maxcost - p1->cost) - 
         (newpop->maxcost - newpop->mincost)/3;
      }
      else
      if(conf->fitness == INVERSE) {
        p1->fitness = 1/p1->cost;
      }
      
      if(p1->fitness < newpop->minfit) {
        newpop->minfit = p1->fitness;
      }
      if(p1->fitness > newpop->maxfit) {
        newpop->maxfit = p1->fitness;
      }
      newpop->sumfit += p1->fitness;
    }
    newpop->avgfit = newpop->sumfit/conf->popsize;   

    if(conf->scale == 1) {
      Scale(pop, conf->scalecoef, conf->popsize);
    }
    PopFree(pop, conf->popsize);
    pop = newpop;
    
    p1 = Best(pop, conf->popsize);
    if(p1->cost < alltime->cost) {
      CromoFree(alltime);
      alltime = CromoCopy(p1);
    }    
    printf("%5d  %4.7f  %4.7f  %4.7f %4.7f %3.2f %3.2f\n", i, pop->maxfit,
      pop->minfit, alltime->cost, newpop->stddev, 
      (float) 100*dch/(conf->popsize*2), (float) 100 * nalt/(conf->popsize*
      2*(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) + 
      array_n((*clbl)->outputs))));

    fprintf(fstd,"%d %f\n", i, pop->stddev);
    fprintf(fmax,"%d %f\n", i, pop->maxfit);
    fprintf(fmin,"%d %f\n", i, pop->minfit);
    fprintf(falt,"%d %f\n", i, (float) nalt/(conf->popsize*2*(
      array_n((*clbl)->clbs) + array_n((*clbl)->inputs) + 
      array_n((*clbl)->outputs))));
    fprintf(fdch,"%d %f\n", i, (float) dch/(conf->popsize*2));

    fprintf(fon,"%d %f\n", i, (float) sumonline/tot_gen);
    sumoffline += alltime->cost;
    fprintf(foff,"%d %f\n", i, (float) sumoffline/i);
  }  
  Decode(clbl, alltime, clbs_occ, io_occ, nx, ny);

  PopFree(pop, conf->popsize);
  CromoFree(alltime);
  
  (void) fclose(fon);
  (void) fclose(foff);
  (void) fclose(fmax);
  (void) fclose(fmin);
  (void) fclose(fstd);
  (void) fclose(falt);
  (void) fclose(fdch);
}

/**Function********************************************************************

  Synopsis           [A steady genetic algorithm.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
SteadyState(
  gaconf_t *conf,
  population_t *pop,
  netl_clbl_t **clbl,  
  array_t *netl,
  float *cost,
  int nx,
  int ny,
  int io_rat,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges,
  bb_t **bbox_new,
  bb_t **bedge_new,
  int *net_move,
  netl_net_t **anets,
  xy_t **neigh)
{  
cromossome_t *p1, *p2, *ch1, *ch2, *alltime;
int i = 0, p1pos, p2pos, tot_gen = 0, ret;
float sumonline = 0., sumoffline = 0.;
FILE *fon, *foff, *fmax, *fmin, *fdch, *falt;

  fon = fopen("online.dat","w");
  foff = fopen("offline.dat","w");
  fmax = fopen("maxfit.dat","w");
  fmin = fopen("minfit.dat","w");
  fdch = fopen("dch.dat","w");
  falt = fopen("falt.dat","w");
  
  p1 = Best(pop, conf->popsize);
  alltime = CromoCopy(p1);

  if(conf->scale == 1) {
    Scale(pop, conf->scalecoef, conf->popsize);
  }
  printf("%5d  %4.7f  %4.7f  %4.7f\n", 0, pop->maxfit, pop->minfit,
    alltime->cost);  

  nmut = 0;
  ncross = 0;

  for(i = 1;i <= conf->niter;i++) {
    nalt = 0;
    dch = 0;
    if(conf->select1 == ROULETTE) {
      p1 = RouletteSelect(pop, &p1pos, -1, conf->popsize);
    }
    else
    if(conf->select1 == RANDOM) {
      p1 = RandomSelect(pop, &p1pos, -1, conf->popsize);
    }

    if(conf->select2 == ROULETTE) {
      p2 = RouletteSelect(pop, &p2pos, p1pos, conf->popsize);
    }
    else
    if(conf->select2 == RANDOM) {
      p2 = RandomSelect(pop, &p2pos, p1pos, conf->popsize);
    }

    ret = Crossover(conf, p1, p2, &ch1, &ch2, nx, ny, io_rat, conf->nsite,
      array_n((*clbl)->clbs));
      
    if(ret == 1) {
      ch1->cost = CromoCost(ch1, *clbl, netl, pop->nx, pop->ny);
      if(conf->local == 1) {
        Local(ch1, nx*ny*conf->loclb, 2*(nx + ny)*conf->loio, clbl, netl,
          nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new,
          bedge_new, anets, net_move, neigh);
      }
      ch2->cost = CromoCost(ch2, *clbl, netl, pop->nx, pop->ny);
      if(conf->local == 1) {
        Local(ch2, nx*ny*conf->loclb, 2*(nx + ny)*conf->loio, clbl, netl,
          nx, ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new,
          bedge_new, anets, net_move, neigh);      
      }
      
      Replace(pop, ch1, ch2, p1, p1pos, p2, p2pos, conf);

      sumonline += ch1->cost + ch2->cost;
      tot_gen += 2;      

      if(conf->scale == 1) {
        Scale(pop, conf->scalecoef, conf->popsize);
      }
      if(ch1->cost < alltime->cost) {
        CromoFree(alltime);
        alltime = CromoCopy(ch1);        
      }
      if(ch2->cost < alltime->cost) {
        CromoFree(alltime);
        alltime = CromoCopy(ch2);
      }
    }
    printf("%5d  %4.7f  %4.7f  %4.7f %3.2f %3.2f\n", i, pop->maxfit,
      pop->minfit, alltime->cost,  
      (float) 100*dch/(conf->popsize*2), (float) 100 * nalt/(conf->popsize*
      2*(array_n((*clbl)->clbs) + array_n((*clbl)->inputs) + 
      array_n((*clbl)->outputs))));

    fprintf(fmax,"%d %f\n", i, pop->maxfit);
    fprintf(fmin,"%d %f\n", i, pop->minfit);
    fprintf(falt,"%d %f\n", i, (float) nalt/(conf->popsize*2*(
      array_n((*clbl)->clbs) + array_n((*clbl)->inputs) + 
      array_n((*clbl)->outputs))));
    fprintf(fdch,"%d %f\n", i, (float) dch/(conf->popsize*2));

    fprintf(fon,"%d %f\n", i, (float) sumonline/tot_gen);
    sumoffline += alltime->cost;
    fprintf(foff,"%d %f\n", i, (float) sumoffline/i);
  }    
    
  Decode(clbl, alltime, clbs_occ, io_occ, nx, ny);

  PopFree(pop, conf->popsize);
  CromoFree(alltime);
  
  (void) fclose(fon);
  (void) fclose(foff);
  (void) fclose(fmax);
  (void) fclose(fmin);
  (void) fclose(falt);
  (void) fclose(fdch);
}

/**Function********************************************************************

  Synopsis           [Scale the fitness of a population.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Replace(
  population_t *pop,
  cromossome_t *ch1,
  cromossome_t *ch2,
  cromossome_t *p1,
  int p1pos,
  cromossome_t *p2,
  int p2pos,
  gaconf_t *conf)
{
float p2f, p1f;
cromossome_t *cromo;
int pos;

  switch(conf->replacement) {
    case COMBINED: {
    
    } break;
    
    case DIRECT: {
      p1f = p1->fitness;
      CromoFree(p1);
      if(ch1->cost < pop->mincost) {
        pop->mincost = ch1->cost;
        pop->minpos = p1pos;
      }
      if(ch1->cost > pop->maxcost) {
        pop->maxcost = ch1->cost;
        pop->maxpos = p1pos;
      }
      if(conf->fitness == PROPORTIONAL) {
        ch1->fitness = (pop->maxcost - ch1->cost) - 
         (pop->maxcost - pop->mincost)/3;
      }
      else
        if(conf->fitness == INVERSE) {
          ch1->fitness = 1/ch1->cost;
        }
      if(ch1->fitness < pop->minfit) {
        pop->minfit = ch1->fitness;
      }
      if(p1->fitness > pop->maxfit) {
        pop->maxfit = ch1->fitness;
      }
      pop->sumfit = pop->sumfit - p1f + ch1->fitness;
      array_insert(cromossome_t *, pop->cromo, p1pos, ch1);      

      p2f = p2->fitness;
      CromoFree(p2);
      if(ch2->cost < pop->mincost) {
        pop->mincost = ch2->cost;
        pop->minpos = p2pos;
      }
      if(ch2->cost > pop->maxcost) {
        pop->maxcost = ch2->cost;
        pop->maxpos = p2pos;
      }
      if(conf->fitness == PROPORTIONAL) {
        ch2->fitness = (pop->maxcost - ch2->cost) - 
         (pop->maxcost - pop->mincost)/3;
      }
      else
        if(conf->fitness == INVERSE) {
          ch2->fitness = 1/ch2->cost;
        }      
      if(ch2->fitness < pop->minfit) {
        pop->minfit = ch2->fitness;
      }
      if(p2->fitness > pop->maxfit) {
        pop->maxfit = ch2->fitness;
      }
      pop->sumfit = pop->sumfit - p2f + ch2->fitness;
      array_insert(cromossome_t *, pop->cromo, p2pos, ch2);
            
      pop->avgfit = pop->sumfit/conf->popsize;
    } break;
    
    case BEST: {
    
    } break;
    
    case PRESELECTION: {
      cromo = array_fetch(cromossome_t *, pop->cromo, pop->maxpos);
      pos = pop->maxpos;
      p1f = cromo->fitness;
      CromoFree(cromo);
      if(ch1->cost < pop->mincost) {
        pop->mincost = ch1->cost;
        pop->minpos = pos;
      }
      if(ch1->cost > pop->maxcost) {
        pop->maxcost = ch1->cost;
        pop->maxpos = pos;
      }
      if(conf->fitness == PROPORTIONAL) {
        ch1->fitness = (pop->maxcost - ch1->cost) - 
         (pop->maxcost - pop->mincost)/3;
      }
      else
        if(conf->fitness == INVERSE) {
          ch1->fitness = 1/ch1->cost;
        }
      if(ch1->fitness < pop->minfit) {
        pop->minfit = ch1->fitness;
      }
      if(p1->fitness > pop->maxfit) {
        pop->maxfit = ch1->fitness;
      }
      pop->sumfit = pop->sumfit - p1f + ch1->fitness;
      array_insert(cromossome_t *, pop->cromo, pos, ch1);

      cromo = array_fetch(cromossome_t *, pop->cromo, pop->maxpos);
      pos = pop->maxpos;
      p2f = cromo->fitness;
      CromoFree(cromo);
      if(ch2->cost < pop->mincost) {
        pop->mincost = ch2->cost;
        pop->minpos = pos;
      }
      if(ch2->cost > pop->maxcost) {
        pop->maxcost = ch2->cost;
        pop->maxpos = pos;
      }
      if(conf->fitness == PROPORTIONAL) {
        ch2->fitness = (pop->maxcost - ch2->cost) - 
         (pop->maxcost - pop->mincost)/3;
      }
      else
        if(conf->fitness == INVERSE) {
          ch2->fitness = 1/ch2->cost;
        }      
      if(ch2->fitness < pop->minfit) {
        pop->minfit = ch2->fitness;
      }
      if(p2->fitness > pop->maxfit) {
        pop->maxfit = ch2->fitness;
      }
      pop->sumfit = pop->sumfit - p2f + ch2->fitness;
      array_insert(cromossome_t *, pop->cromo, pos, ch2);
            
      pop->avgfit = pop->sumfit/conf->popsize;    
    } break;
  }


}

/**Function********************************************************************

  Synopsis           [Scale the fitness of a population.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Scale(
  population_t *pop,
  float coef,
  int popsize)
{
float a, b;
int i;
cromossome_t *cromo;

  if(pop->minfit > (coef * pop->avgfit - pop->maxfit)) {
    a = (coef - 1.0) * pop->avgfit/(pop->maxfit - pop->avgfit);
    b = pop->avgfit*(pop->maxfit - coef*pop->avgfit)/
      (pop->maxfit - pop->avgfit);  
  }
  else {
    a = pop->avgfit/(pop->avgfit - pop->minfit);
    b = -pop->minfit*pop->avgfit/(pop->avgfit - pop->minfit);  
  }

  pop->sumfit = 0.;
  for(i = 0;i < popsize;i++) {
    cromo = array_fetch(cromossome_t *, pop->cromo, i);
    cromo->fitness = a*cromo->fitness + b;
    pop->sumfit += cromo->fitness;
  }
}

/**Function********************************************************************

  Synopsis           [Verify the cromossome for a valid solution.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static cromossome_t *
Best(
  population_t *pop,
  int popsize)
{
int i;
cromossome_t *cromo, *cm;
float mincost = INFINITY;

  for(i = 0;i < popsize;i++) {
    cromo = array_fetch(cromossome_t *, pop->cromo, i);
    if(cromo->cost < mincost) {
      mincost = cromo->cost;
      cm = cromo;
    }
  }  
  return cm;
}

/**Function********************************************************************

  Synopsis           [Crossover operator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
Crossover(
  gaconf_t *conf,
  cromossome_t *parent1,
  cromossome_t *parent2,
  cromossome_t **child1,
  cromossome_t **child2,
  int nx,
  int ny,
  int io_rat,
  int npt,
  int nclb)
{
float n;
int *site, i, *pos, j, mut;
allele_t *al, *an;
st_table *t1,*t2;
char *value;
int p, cr1 = 0, *correct1 = NIL(int), cr2 = 0, *correct2 = NIL(int),
tot1, tot2, *posf, tmp;

  n = util2_Frand();
  if(n < conf->probcross) {
    site = (int *) ALLOC(int, npt + 1);
    p = array_n(parent1->al) - 2;
    pos = (int *) ALLOC(int, p);
    for(i = 0;i < p;i++)  {
      pos[i] = i+1;
    }
    for(i = 0;i < npt;i++) {
      j = util2_Irand(p - 1);
      site[i] = pos[j];
      pos[j] = pos[p - 1];
      p--;
    }
    site[npt] = array_n(parent1->al);
    FREE(pos);
    ncross++;
  }
  else {
    return 0;
  }
  (*child1) = (cromossome_t *) ALLOC(cromossome_t, 1);
  (*child2) = (cromossome_t *) ALLOC(cromossome_t, 1);
  (*child1)->al = array_alloc(allele_t *, 0);
  (*child2)->al = array_alloc(allele_t *, 0);

  t1 = st_init_table(st_ptrcmp, st_ptrhash);
  t2 = st_init_table(st_ptrcmp, st_ptrhash);

  tot1 = nx*ny;
  tot2 = tot1;

  i = 0;  
  for(j = 0;j <= npt;j++) {
    if(j%2 == 0) {
      while(i < site[j]) {
        al = array_fetch(allele_t *, parent1->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t1, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child1)->al, an);          
          st_insert(t1, (char *) p, (char *) 0);
          tot1--;
        }
        else {
          cr1++;
          correct1 = (int *) REALLOC(int, correct1, cr1);
          correct1[cr1 - 1] = i;
          array_insert_last(allele_t *, (*child1)->al, an);          
        }

        al = array_fetch(allele_t *, parent2->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);          
        }
        if(st_lookup(t2, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child2)->al, an);
          st_insert(t2, (char *) p, (char *) 0);
          tot2--;
        }
        else {
          cr2++;
          correct2 = (int *) REALLOC(int, correct2, cr2);
          correct2[cr2 - 1] = i;
          array_insert_last(allele_t *, (*child2)->al, an);          
        }        
        i++;      
        if(i == nclb) break;
      } 
    }
    else {
      while(i < site[j]) {
        al = array_fetch(allele_t *, parent2->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t1, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child1)->al, an);          
          st_insert(t1, (char *) p, (char *) 0);
          tot1--;
        }
        else {
          cr1++;
          correct1 = (int *) REALLOC(int, correct1, cr1);
          correct1[cr1 - 1] = i;
          array_insert_last(allele_t *, (*child1)->al, an);
        }

        al = array_fetch(allele_t *, parent1->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t2, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child2)->al, an);
          st_insert(t2, (char *) p, (char *) 0);
          tot2--;
        }
        else {
          cr2++;
          correct2 = (int *) REALLOC(int, correct2, cr2);
          correct2[cr2 - 1] = i;
          array_insert_last(allele_t *, (*child2)->al, an);
        }        
        i++;
        if(i == nclb) break;
      }
    }
    if(i == nclb) break;
  }
  tmp = j;

  if(cr1 > 0) {
    posf = (int *) ALLOC(int, tot1);
    p = 0;
    for(i = 0;i < nx;i++) {
      for(j = 0;j < ny;j++) {
        mut = (j+1)*ny + i;     
        if(st_lookup(t1, (char *) mut, &value) == 0) {
          posf[p] = mut;
          p++;
        }
      }
    }
    for(i = 0;i < cr1;i++) {
      al = array_fetch(allele_t *, (*child1)->al, correct1[i]);
      j = util2_Irand(p - 1);
      placeDecodePos(posf[j], &al->x, &al->y, &al->nio, nx, ny, io_rat);
      posf[j] = posf[p - 1];
      p--;
    }
    FREE(posf);
    FREE(correct1);
    nalt += cr2;    
  }
  p = 0;
  if(cr2 > 0) {
    posf = (int *) ALLOC(int, tot2);
    for(i = 0;i < nx;i++) {
      for(j = 0;j < ny;j++) {
        mut = (j+1)*ny + i;
        if(st_lookup(t2, (char *) mut, &value) == 0) {
          posf[p] = mut;
          p++;
        }
      }
    }
    for(i = 0;i < cr2;i++) {
      al = array_fetch(allele_t *, (*child2)->al, correct2[i]);
      j = util2_Irand(p - 1);
      placeDecodePos(posf[j], &al->x, &al->y, &al->nio, nx, ny, io_rat);
      posf[j] = posf[p - 1];
      p--;    
    }
    FREE(posf);
    FREE(correct2);
    nalt += cr2;
  }
  st_free_table(t1);
  st_free_table(t2);
  cr1 = 0;
  cr2 = 0;
  correct1 = NIL(int);
  correct2 = NIL(int);

  t1 = st_init_table(st_ptrcmp, st_ptrhash);
  t2 = st_init_table(st_ptrcmp, st_ptrhash);
  
  tot1 = 2*(nx*io_rat + ny*io_rat);
  tot2 = tot1;

  i = nclb;
  j = tmp;
  for(;j <= npt;j++) {
    if(j%2 == 0) {
      while(i < site[j]) {
        al = array_fetch(allele_t *, parent1->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t1, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child1)->al, an);          
          st_insert(t1, (char *) p, (char *) 0);          
          tot1--;
        }
        else {
          cr1++;
          correct1 = (int *) REALLOC(int, correct1, cr1);
          correct1[cr1 - 1] = i;
          array_insert_last(allele_t *, (*child1)->al, an);          
        }
      
        al = array_fetch(allele_t *, parent2->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);        
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t2, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child2)->al, an);          
          st_insert(t2, (char *) p, (char *) 0);          
          tot2--;
        }
        else {
          cr2++;
          correct2 = (int *) REALLOC(int, correct2, cr2);
          correct2[cr2 - 1] = i;
          array_insert_last(allele_t *, (*child2)->al, an);
        }
        i++;
      } 
    }
    else {
      while(i < site[j]) {
        al = array_fetch(allele_t *, parent2->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t1, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child1)->al, an);          
          st_insert(t1, (char *) p, (char *) 0);          
          tot1--;
        }
        else {
          cr1++;
          correct1 = (int *) REALLOC(int, correct1, cr1);
          correct1[cr1 - 1] = i;
          array_insert_last(allele_t *, (*child1)->al, an);          
        }

        al = array_fetch(allele_t *, parent1->al, i);
        p = placeEncodePos(al->x, al->y, al->nio, nx, ny, io_rat);        
        an = (allele_t *) ALLOC(allele_t, 1);
        an->x = al->x;
        an->y = al->y;
        an->nio = al->nio;
        mut = Mutate(&an, nx, ny, io_rat, conf->probmut);
        if(mut == 1) {
          p = placeEncodePos(an->x, an->y, an->nio, nx, ny, io_rat);
        }
        if(st_lookup(t2, (char *) p, &value) == 0) {
          array_insert_last(allele_t *, (*child2)->al, an);          
          st_insert(t2, (char *) p, (char *) 0);          
          tot2--;
        }
        else {
          cr2++;
          correct2 = (int *) REALLOC(int, correct2, cr2);
          correct2[cr2 - 1] = i;
          array_insert_last(allele_t *, (*child2)->al, an);
        }
        i++;
      } 
    }
  }

  if(cr1 > 0) {
    posf = (int *) ALLOC(int, tot1);
    p = 0;
    for(i = 1;i <= nx;i++)
      for(j = 0;j < io_rat;j++) {
        tmp = i*io_rat + j;
        if(st_lookup(t1, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 1;i <= ny;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = io_rat*nx + i*io_rat + j;
        if(st_lookup(t1, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 1;i <= nx;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = io_rat*nx + io_rat*ny + i*io_rat + j;
        if(st_lookup(t1, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 1;i <= ny;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = 2*io_rat*nx + io_rat*ny + i*io_rat + j;
        if(st_lookup(t1, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 0;i < cr1;i++) {
      al = array_fetch(allele_t *, (*child1)->al, correct1[i]);
      j = util2_Irand(p - 1);      
      placeDecodePos(posf[j], &al->x, &al->y, &al->nio, nx, ny, io_rat);
      posf[j] = posf[p - 1];
      p--;
    }
    FREE(posf);
    FREE(correct1);
    nalt += cr1;
  }

  p = 0;
  if(cr2 > 0) {
    posf = (int *) ALLOC(int, tot2);
    for(i = 1;i <= nx;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = i*io_rat + j;
        if(st_lookup(t2, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 1;i <= ny;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = io_rat*nx + i*io_rat + j;
        if(st_lookup(t2, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 1;i <= nx;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = io_rat*nx + io_rat*ny + i*io_rat + j;
        if(st_lookup(t2, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 1;i <= ny;i++) 
      for(j = 0;j < io_rat;j++) {
        tmp = 2*io_rat*nx + io_rat*ny + i*io_rat + j;
        if(st_lookup(t2, (char *) tmp, &value) == 0) {
          posf[p] = tmp;
          p++;
        }
      }
    for(i = 0;i < cr2;i++) {
      al = array_fetch(allele_t *, (*child2)->al, correct2[i]);
      j = util2_Irand(p - 1);
      placeDecodePos(posf[j], &al->x, &al->y, &al->nio, nx, ny, io_rat);
      posf[j] = posf[p - 1];
      p--;    
    }
    FREE(posf);
    FREE(correct2);
    nalt += cr2;
  }
  st_free_table(t1);
  st_free_table(t2);

  FREE(site);
  return 1;
}

/**Function********************************************************************

  Synopsis           [Mutation Operator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
Mutate(
  allele_t **al,
  int nx, 
  int ny,
  int io_rat,
  float prob)
{
float n;
int x, y, side;

  n = util2_Frand();
  if(n < prob) {
    nmut++;
    if((*al)->nio == -1) {
      (*al)->x = util2_Irand(nx - 1) + 1;
      (*al)->y = util2_Irand(ny - 1) + 1;
    }
    else {
      side = util2_Irand(3);
      switch(side) {
        case 0: {
          y = util2_Irand(ny);
          (*al)->x = 0;
          (*al)->y = MAX(MIN(y, ny), 1);
        } break;
        case 1: {
          x = util2_Irand(nx);
          (*al)->x = MAX(MIN(x, nx), 1);
          (*al)->y = 0;        
        } break;
        case 2: {
          y = util2_Irand(ny);
          (*al)->x = nx + 1;
          (*al)->y = MAX(MIN(y, ny), 1);
        } break;
        case 3: {
          x = util2_Irand(nx);
          (*al)->x = MAX(MIN(x, nx), 1);
          (*al)->y = ny + 1;
        } break;
        (*al)->nio = util2_Irand(io_rat - 1);
      }
    }  
  }
  else {
    return 0;
  }

  return 1;
}

/**Function********************************************************************

  Synopsis           [Select operator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static cromossome_t *
RouletteSelect(
  population_t *pop,
  int *pos,
  int invpos,
  int popsize)
{
cromossome_t *ret;
int i = 0;
float sum = 0., n;

  n = util2_Frand() * pop->sumfit;
  while((sum < n) && (i < popsize)) {
    ret = array_fetch(cromossome_t *, pop->cromo, i);
    sum += ret->fitness;
    i++;
  }      
  *pos = i - 1;
  
  if(*pos == invpos) {
    if(invpos + 1 == popsize) {
      (*pos) = invpos - 1;
    }
    else {
      (*pos) = invpos + 1;
    }
    ret = array_fetch(cromossome_t *, pop->cromo, *pos);
  }
  
  return ret;  
}

/**Function********************************************************************

  Synopsis           [Select operator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static cromossome_t *
RandomSelect(
  population_t *pop,
  int *pos,
  int invpos,
  int popsize)
{
cromossome_t *ret;
  
  *pos = util2_Irand(popsize - 1);
  
  if(*pos == invpos) {
    if((invpos + 1) == popsize) {
      *pos = invpos - 1;
    }
    else {
      *pos = invpos + 1;
    }
    ret = array_fetch(cromossome_t *, pop->cromo, *pos);
  }  
  else {
    ret = array_fetch(cromossome_t *, pop->cromo, *pos);
  }
  
  return ret;  
}

/**Function********************************************************************

  Synopsis           [Initialize the population.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static population_t *
PopInit(
  gaconf_t *conf,
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
  xy_t **neigh)
{
population_t *ret;
cromossome_t *new;
allele_t *alnew;
int i;
int x, y, count, t, tsize;
struct xy { int x; int y; } *pos;
int *ior;
  
  ret = (population_t *) ALLOC(population_t, 1);
  ret->nx = nx;
  ret->ny = ny;
  ret->io_rat = io_rat;
  ret->maxfit = -1;
  ret->sumfit = 0.;
  ret->minfit = INFINITY;
  ret->maxcost = -1;
  ret->mincost = INFINITY;
  ret->squared_sumcost = 0.;
  ret->avgcost = 0.;  
  ret->cromo = array_alloc(cromossome_t *, 0);
  for(i = 0;i < conf->popsize;i++) {
    new = (cromossome_t *) ALLOC(cromossome_t, 1);
    new->fitness = 0.;
        
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
    new->al = array_alloc(allele_t *, 0);    
    for(t = 0;t < array_n(clbl->clbs);t++) {
      alnew = (allele_t *) ALLOC(allele_t, 1);
      x = util2_Irand(count - 1);
      alnew->x = pos[x].x;
      alnew->y = pos[x].y;
      alnew->nio = -1;
      pos[x] = pos[count - 1];
      count--;
      array_insert_last(allele_t *, new->al, alnew);
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

    for(t = 0;t < array_n(clbl->inputs);t++) {
      alnew = (allele_t *) ALLOC(allele_t, 1);      
      x = util2_Irand(count - 1);
      alnew->x = pos[x].x;
      alnew->y = pos[x].y;
      alnew->nio = ior[x];
      ior[x]++;
      if(ior[x] == io_rat) {
        pos[x] = pos[count - 1];
        ior[x] = ior[count - 1];
        count--;
      }
      array_insert_last(allele_t *, new->al, alnew);
    }    

    for(t = 0;t < array_n(clbl->outputs);t++) {
      alnew = (allele_t *) ALLOC(allele_t, 1);
      x = util2_Irand(count - 1);
      alnew->x = pos[x].x;
      alnew->y = pos[x].y;
      alnew->nio = ior[x];
      ior[x]++;
      if(ior[x] == io_rat) {
        pos[x] = pos[count - 1];
        ior[x] = ior[count - 1];
        count--;
      }  
      array_insert_last(allele_t *, new->al, alnew);      
    }  
    FREE(pos);
    FREE(ior);

    new->cost = CromoCost(new, clbl, netl, nx, ny);

    if(conf->localstart == 1) {
      Local(new, nx*ny*conf->loclb, 2*(nx + ny)*conf->loio, &clbl, netl, nx,
        ny, io_rat, clbs_occ, io_occ, bboxes, bedges, bbox_new, bedge_new,
        anets, net_move, neigh);
    }
    array_insert_last(cromossome_t *, ret->cromo, new);
    if(new->cost < ret->mincost) {
      ret->mincost = new->cost;
      ret->minpos = array_n(ret->cromo) - 1;
    }
    if(new->cost > ret->maxcost) {
      ret->maxcost = new->cost;
      ret->maxpos = array_n(ret->cromo) - 1;
    }
    ret->squared_sumcost += new->cost*new->cost;    
    ret->avgcost += new->cost;
  }
  ret->avgcost = ret->avgcost/conf->popsize;
  ret->stddev = placeGetStdDev(conf->popsize, ret->squared_sumcost,
    ret->avgcost);
  
  for(i = 0;i < conf->popsize;i++) {
    new = array_fetch(cromossome_t *, ret->cromo, i);
    if(conf->fitness == PROPORTIONAL) {
      new->fitness = (ret->maxcost - new->cost) -
        (ret->maxcost - ret->mincost)/3;    
    }
    else
    if(conf->fitness == INVERSE) {
      new->fitness = 1/new->cost;
    }
    if(new->fitness < ret->minfit) {
      ret->minfit = new->fitness;
    }
    if(new->fitness > ret->maxfit) {
      ret->maxfit = new->fitness;
    }
    ret->sumfit += new->fitness;
  }
  ret->avgfit = ret->sumfit/conf->popsize;
  
  return ret;
}

/**Function********************************************************************

  Synopsis           [Destroy the population]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
PopFree(
  population_t *pop,
  int popsize)
{
int i;
cromossome_t *cromo;

  if(pop == NIL(population_t)) return;
  
  for(i = 0;i < popsize;i++) {
    cromo = array_fetch(cromossome_t *, pop->cromo, i);
    CromoFree(cromo);
  }
  array_free(pop->cromo);
  FREE(pop);
}

/**Function********************************************************************

  Synopsis           [Destroy a cromossome]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
CromoFree(cromossome_t *cromo)
{
int i;
allele_t *al;

  if(cromo == NIL(cromossome_t)) return;
  
  for(i = 0;i < array_n(cromo->al);i++) {
    al = array_fetch(allele_t *, cromo->al, i);
    FREE(al);
  }
  array_free(cromo->al);
  FREE(cromo);
}

/**Function********************************************************************

  Synopsis           [Destroy a cromossome]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static cromossome_t *
CromoCopy(cromossome_t *cromo)
{
int i;
allele_t *al, *alnew;
cromossome_t *ret;

  if(cromo == NIL(cromossome_t)) return NIL(cromossome_t);
  
  ret = (cromossome_t *) ALLOC(cromossome_t, 1);
  ret->al = array_alloc(allele_t *, 0);
  
  for(i = 0;i < array_n(cromo->al);i++) {
    al = array_fetch(allele_t *, cromo->al, i);
    alnew = (allele_t *) ALLOC(allele_t, 1);
    alnew->x = al->x;
    alnew->y = al->y;
    alnew->nio = al->nio;
    array_insert_last(allele_t *, ret->al, alnew);   
  }
  ret->fitness = cromo->fitness;
  ret->cost = cromo->cost;

  return ret;
}

/**Function********************************************************************

  Synopsis           [Compute the fitness a cromossome.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static float 
CromoCost(
  cromossome_t *cromo,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny)
{
float fitness = 0., tmp;
netl_net_t *net;
int xmax, xmin, ymax, ymin, j, i;
netl_signet_t *sn;
float cross;
netl_clb_t *clb;
allele_t *al;

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    al = array_fetch(allele_t *, cromo->al, i);
    clb->x = al->x;
    clb->y = al->y;
    clb->nio = -1;
  }
  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    al = array_fetch(allele_t *, cromo->al, i + array_n(clbl->clbs));
    clb->x = al->x;
    clb->y = al->y;
    clb->nio = al->nio;
  }
  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    al = array_fetch(allele_t *, cromo->al, i + array_n(clbl->clbs) +
      array_n(clbl->inputs));
    clb->x = al->x;
    clb->y = al->y;
    clb->nio = al->nio;
  }

  for(i = 0;i < array_n(netl);i++) {
    net = array_fetch(netl_net_t *, netl, i);
    sn = array_fetch(netl_signet_t *, net->snet, 0);
    xmax = sn->clb->x;
    xmin = sn->clb->x;
    ymin = sn->clb->y;
    ymax = sn->clb->y;
    for(j = 1;j < array_n(net->snet);j++) {
      sn = array_fetch(netl_signet_t *, net->snet, j);      
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
    xmax = MAX(MIN(xmax, nx), 1);
    ymax = MAX(MIN(ymax, ny), 1);
    xmin = MAX(MIN(xmin, nx), 1);
    ymin = MAX(MIN(ymin, ny), 1);    
  
    if(array_n(net->snet) > 50) {
      cross = 2.7933 + 0.02616 * (array_n(net->snet) - 50); 
    }
    else {
      cross = cross_count[array_n(net->snet) - 1];
    }    
    tmp = (xmax - xmin + 1) * cross * chanx_place_cost_fac[ymax][ymin - 1];
    tmp += (ymax - ymin + 1) * cross * chany_place_cost_fac[xmax][xmin - 1];    
    net->cost = tmp;    
    fitness += tmp;
  }

  return fitness;
}

/**Function********************************************************************

  Synopsis           [Decode the cromossome]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Decode(
  netl_clbl_t **clbl,
  cromossome_t *cromo,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  int nx,
  int ny)
{
int i, j;
allele_t *al;
netl_clb_t *clb;

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


  for(i = 0;i < array_n((*clbl)->clbs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->clbs, i);
    al = array_fetch(allele_t *, cromo->al, i);
    clb->x = al->x;
    clb->y = al->y;
    clb->nio = -1;
    clbs_occ[clb->x - 1][clb->y - 1] = clb;
  }
  for(i = 0;i < array_n((*clbl)->inputs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->inputs, i);
    al = array_fetch(allele_t *, cromo->al, i + array_n((*clbl)->clbs));
    clb->x = al->x;
    clb->y = al->y;
    clb->nio = al->nio;
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
  for(i = 0;i < array_n((*clbl)->outputs);i++) {
    clb = array_fetch(netl_clb_t *, (*clbl)->outputs, i);
    al = array_fetch(allele_t *, cromo->al, i + array_n((*clbl)->clbs) +
      array_n((*clbl)->inputs));
    clb->x = al->x;
    clb->y = al->y;
    clb->nio = al->nio;
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
}

/**Function********************************************************************

  Synopsis           [Decode the cromossome]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Encode(
  cromossome_t *cromo,
  netl_clbl_t *clbl)
{
int i;
allele_t *al;
netl_clb_t *clb;

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    al = array_fetch(allele_t *, cromo->al, i);
    al->x = clb->x;
    al->y = clb->y;
    al->nio = -1;
  }
  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    al = array_fetch(allele_t *, cromo->al, i + array_n(clbl->clbs));
    al->x = clb->x;
    al->y = clb->y;
    al->nio = clb->nio;
  }
  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    al = array_fetch(allele_t *, cromo->al, i + array_n(clbl->clbs) +
      array_n(clbl->inputs));
    al->x = clb->x;
    al->y = clb->y;
    al->nio = clb->nio;
  }
}

/**Function********************************************************************

  Synopsis           [Initialize the population.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
PopPrint(
  population_t *pop,
  int popsize)
{
cromossome_t *cromo;
int i;

  printf("Max. Fitness = %f\tAvg. Fitness = %f\tMin. Fitness = %f\tSum = %f\n",
    pop->maxfit, pop->avgfit, pop->minfit, pop->sumfit);
  printf("Population size: %d\n\n", array_n(pop->cromo));
  
  for(i = 0;i < popsize;i++) {
    cromo = array_fetch(cromossome_t *, pop->cromo, i);
    printf("Cromossome %d\tFitness = %f\n", i, cromo->fitness);
  } 
}

/**Function********************************************************************

  Synopsis           [Make a local improvement of the cromossome.]

  Description        [This procedure calls a local optimization procedure,
  based on neighborrod search. It is assumed that the cromossome fitness,
  carries the cost of the associated solution. This cost will be automatically
  modified.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static cromossome_t *
Local(
  cromossome_t *cromo,
  int nclb,
  int nio,
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
  int *net_move,
  xy_t **neigh)
{
float cost;

  Decode(clbl, cromo, clbs_occ, io_occ, nx, ny);
  cost = placeCompCost(*clbl, netl, bboxes, bedges, nx, ny);  
/*
  placeSwap(&cost, nclb, nio, clbl, netl, nx, ny, io_rat, clbs_occ, 
    io_occ, bboxes, bedges, bbox_new, bedge_new, anets, net_move, neigh);
*/
  Encode(cromo, *clbl);
  cromo->cost = CromoCost(cromo, *clbl, netl, nx, ny);  
  
  return cromo;
}

/**Function********************************************************************

  Synopsis           [Verify the cromossome for a valid solution.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
BugPop(
  population_t *pop,
  int nx,
  int ny,
  int io_rat,
  int nc,
  int popsize)
{
int i, error;
cromossome_t *cromo;

  for(i = 0;i < popsize;i++) {
    cromo = array_fetch(cromossome_t *, pop->cromo, i);
    error = Bug(cromo, nx, ny, io_rat, nc);
    switch(error) {
      case 1: {
        printf("ERROR NO CROMOSSOMO ALLELE NULL!!\n");
        printf("Cromossome %d da populacao\n", i);
      } break;
      case 2: {
        printf("ERROR NO CROMOSSOME X !\n");
        printf("Cromossome %d da populacao\n", i);
      } break;        
      case 3: {
        printf("ERROR NO CROMOSSOME Y !\n");        
        printf("Cromossome %d da populacao\n", i);
      } break;
      case 4: {
        printf("ERROR NO CROMOSSOME NIO! \n");          
        printf("Cromossome %d da populacao\n", i);
      } break;    
      case 5: {
        printf("NAO TEM -1 NO NIO!!\n");
        printf("Cromossome %d da populacao\n", i);
      } break;    
    }
  }
}

/**Function********************************************************************

  Synopsis           [Verify the cromossome for a valid solution.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static short
Bug(
  cromossome_t *cromo,
  int nx,
  int ny,  
  int io_rat,
  int nc)
{
int i;
allele_t *al;

  for(i = 0;i < nc;i++) {
    al = array_fetch(allele_t *, cromo->al, i);
    if(al == NIL(allele_t)) {
      return 1;
    }
    if((al->x > nx) || (al->x < 1)) {
      printf("%d ",al->x);
      return 2;
    }
    if((al->y > ny) || (al->y < 1)) {
      printf("%d ",al->y);
      return 3;
    }
    if(al->nio != -1) {
      return 5;
    }
  }    
  for(;i < array_n(cromo->al);i++) {
    al = array_fetch(allele_t *, cromo->al, i);
    if(al == NIL(allele_t)) {
      return 1;
    }
    if(al->nio > io_rat) {
      return 4;
    } 
  }  
  return 0;
}
/**Function********************************************************************

  Synopsis           [Compute the fitness of all individues of a population.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
PopFit(
  gaconf_t *conf,
  population_t *pop,
  netl_clbl_t *clbl,
  array_t *netl,
  netl_clb_t ***clbs_occ,
  array_t ***io_occ,
  bb_t **bboxes,
  bb_t **bedges)
{
float maxfit = -1, minfit = INFINITY, avgfit = 0.;
cromossome_t *cromo;
int i;

  for(i = 0;i < conf->popsize;i++) {
    cromo = array_fetch(cromossome_t *, pop->cromo, i);
    cromo->fitness = CromoCost(cromo, clbl, netl, pop->nx, pop->ny);
    if(cromo->fitness < pop->mincost) {
      pop->mincost = cromo->fitness;
    }
    if(cromo->fitness > pop->maxcost) {
      pop->maxcost = cromo->fitness;
    }
  }

  if(conf->fitness == PROPORTIONAL) {
    for(i = 0;i < conf->popsize;i++) {
      cromo = array_fetch(cromossome_t *, pop->cromo, i);
      cromo->fitness = (pop->maxcost - cromo->fitness) +
        (pop->maxcost - pop->mincost)/3;
      avgfit += cromo->fitness;
      if(cromo->fitness < minfit) {
        minfit = cromo->fitness;
      }
      if(cromo->fitness > maxfit) {
        maxfit = cromo->fitness;
      }
    }  
  }  
  else {
    for(i = 0;i < conf->popsize;i++) {
      cromo = array_fetch(cromossome_t *, pop->cromo, i);
      cromo->fitness = 1/cromo->fitness;
      avgfit += cromo->fitness;
      if(cromo->fitness < minfit) {
        minfit = cromo->fitness;
      }
      if(cromo->fitness > maxfit) {
        maxfit = cromo->fitness;
      }
    }  
  }
  
  pop->maxfit = maxfit;
  pop->minfit = minfit;
  pop->sumfit = avgfit;
  pop->avgfit = avgfit/array_n(pop->cromo);
}

/**Function********************************************************************

  Synopsis           [Verify the cromossome for a valid solution.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Verify(
  cromossome_t *cromo,
  int nc,
  int nio,
  int nx, 
  int ny,
  int io_rat)
{
st_table *computed;
char *value, *key;
st_generator *stgen;
allele_t *al;
int i, pos, j, a;
int *clbpos = NIL(int), count = 0;
array_t *ca;

  computed = st_init_table(st_ptrcmp, st_ptrhash);  
  for(i = 0;i < nc;i++) {
    al = array_fetch(allele_t *, cromo->al, i);
    pos = al->y * ny + al->x - 1;
    if(st_lookup(computed, (char *) pos, &value) == 1) {
      ca = (array_t *) value;
      array_insert_last(allele_t *, ca, al);
      st_insert(computed, (char *) pos, (char *) ca);
    }
    else {
      ca = array_alloc(allele_t *, 0);
      array_insert_last(allele_t *, ca, al);
      st_insert(computed, (char *) pos, (char *) ca);
    }
  }  
  for(i = 0;i < nx*ny;i++) {
    pos = i + ny;
    if(st_lookup(computed, (char *) pos, &value) == 0) {
      count++;
      clbpos = (int *) REALLOC(int, clbpos, count);
      clbpos[count - 1] = pos;
    }  
  }  
  st_foreach_item(computed, stgen, &key, &value) {
    ca = (array_t *) value;
    if(array_n(ca) > 1) {
      for(j = 1;j < array_n(ca);j++) {
        nalt++;
        al = array_fetch(allele_t *, ca, j);
        a = util2_Irand(count - 1);
        pos = clbpos[a];
        clbpos[a] = clbpos[count - 1];
        count--;
        al->y = (int) pos/ny;
        al->x = (int) pos%ny + 1;       
      }    
    }  
  }      
  free(clbpos);
  clbpos = NULL;
  count = 0;


  st_foreach_item(computed, stgen, &key, &value) {
    ca = (array_t *) value;
    array_free(ca);
  }
  st_free_table(computed);

  computed = st_init_table(st_ptrcmp, st_ptrhash);  
  for(i = nc;i < nc+nio;i++) {
    al = array_fetch(allele_t *, cromo->al, i);
    if(al->y == 0) {
      pos = al->x*io_rat + al->nio;
    }
    else
      if(al->x == nx+1)  {
        pos = io_rat*nx + al->y*io_rat + al->nio;
      }
      else
        if(al->y == ny+1) {
          pos = io_rat*nx + io_rat*ny + al->x*io_rat + al->nio;
        }
        else {
          pos = 2*io_rat*nx + io_rat*ny + al->y*io_rat + al->nio;
        }        
    if(st_lookup(computed, (char *) pos, &value) == 1) {
      ca = (array_t *) value;
      array_insert_last(allele_t *, ca, al);
      st_insert(computed, (char *) pos, (char *) ca);
    }
    else {
      ca = array_alloc(allele_t *, 0);
      array_insert_last(allele_t *, ca, al);
      st_insert(computed, (char *) pos, (char *) ca);
    }
  }  

  for(i = 1;i <= nx;i++) {
    for(j = 0;j < io_rat;j++) {
      pos = i*io_rat + j;
      if(st_lookup(computed, (char *) pos, &value) == 0) {
        count++;
        clbpos = (int *) REALLOC(int, clbpos, count);
        clbpos[count - 1] = pos;
      }  
    }     
  }
  for(i = 1;i <= ny;i++) {
    for(j = 0;j < io_rat;j++) {
      pos = io_rat*nx + i*io_rat + j;
      if(st_lookup(computed, (char *) pos, &value) == 0) {
        count++;
        clbpos = (int *) REALLOC(int, clbpos, count);
        clbpos[count - 1] = pos;
      }  
    }     
  }
  for(i = 1;i <= nx;i++) {
    for(j = 0;j < io_rat;j++) {
      pos = io_rat*nx + io_rat*ny + i*io_rat;
      if(st_lookup(computed, (char *) pos, &value) == 0) {
        count++;
        clbpos = (int *) REALLOC(int, clbpos, count);
        clbpos[count - 1] = pos;
      }  
    }     
  }
  for(i = 1;i <= ny;i++) {
    for(j = 0;j < io_rat;j++) {
      pos = 2*io_rat*nx + io_rat*ny + i*io_rat + j;
      if(st_lookup(computed, (char *) pos, &value) == 0) {
        count++;
        clbpos = (int *) REALLOC(int, clbpos, count);
        clbpos[count - 1] = pos;
      }  
    }     
  }    
  st_foreach_item(computed, stgen, &key, &value) {
    ca = (array_t *) value;
    if(array_n(ca) > 1) {
      for(j = 1;j < array_n(ca);j++) {
        nalt++;
        al = array_fetch(allele_t *, ca, j);
        a = util2_Irand(count - 1);
        pos = clbpos[a];
        clbpos[a] = clbpos[count - 1];
        count--;
        if(pos < (nx*io_rat + io_rat)) {
          al->y = 0;
          al->x = (int) pos/io_rat;
          al->nio = (int) pos%io_rat;
        }
        else
          if(pos < (ny*io_rat + nx*io_rat + io_rat)) {
            al->x = nx+1;
            al->y = (int) (pos - nx*io_rat)/io_rat;
            al->nio = (int) (pos - nx*io_rat)%io_rat;
          }
          else
            if(pos < (2*nx*io_rat + ny*io_rat + io_rat)) {
              al->y = ny+1; 
              al->x = (int) (pos - nx*io_rat - ny*io_rat)/io_rat;
              al->nio = (int) (pos - nx*io_rat - ny*io_rat)%io_rat;
            }
            else {
              al->x = 0;
              al->y = (int) (pos - 2*nx*io_rat - ny*io_rat)/io_rat;
              al->nio = (int) (pos - 2*nx*io_rat - ny*io_rat)%io_rat;
            }
      }    
    }  
  }      
  free(clbpos);
  
  st_foreach_item(computed, stgen, &key, &value) {
    ca = (array_t *) value;
    array_free(ca);
  }
  st_free_table(computed);
  
}
