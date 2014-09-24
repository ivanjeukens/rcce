/**CFile***********************************************************************

  FileName    [util2Util.c]

  PackageName [util2]

  Synopsis    [Various generic functions]

  Description []

  SeeAlso     [util]

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include <sys/types.h>
#include <sys/resource.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "util2.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static struct timeval  time_start, time_stop;
static struct rusage   ru_start, ru_stop;

static double start, stop, seconds;

EXTERN FILE *msgerr;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Find out if a string represents an integer.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
short		/* is_int */
util2_IsInt(char *number)
{
int i;

  for(i = 0;i < (int)strlen(number);i++)
    if(isdigit(number[i]) == 0) break;

  if(i < (int) strlen(number))
    return 0;
    
  return 1;  
}

/**Function********************************************************************

  Synopsis           [Find out if a string represents a float.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
short		/* is_float */
util2_IsFloat(char *number)
{
int i;

  for(i = 0;i < (int)strlen(number);i++)
    if(isdigit(number[i]) == 0) break;

  if(i == (int) strlen(number)) return 1;
  
  if(number[i] != '.') return 0;
  
  for(;i < (int)strlen(number);i++)
    if(isdigit(number[i]) == 0) break;

  if(i < (int) strlen(number))
    return 0;

  return 1;
}

/**Function********************************************************************

  Synopsis           [Return a random integer.]

  Description        [This function was extracted from the VPR tool.]

  SideEffects        []
  
  SeeAlso	     [http://www.eecg.toronto.edu/~vaughn]
  
*****************************************************************************/
int	/* my_irand */
util2_Irand(int imax)
{
float fval;
int ival;

  fval = ((float) random ()) / ((float) UTIL2_MYRANDOM_MAX);
  ival = (int) (fval*(imax+1)-0.001);

  return ival;
} 

/**Function********************************************************************

  Synopsis           [Return a random float.]

  Description        [This function was extracted from the VPR tool.]

  SideEffects        []
  
  SeeAlso	     [http://www.eecg.toronto.edu/~vaughn]
  
*****************************************************************************/
float 
util2_Frand()
{
float fval;

  fval = ((float) random ()) / ((float) UTIL2_MYRANDOM_MAX);

  return(fval);
}

/**Function********************************************************************

  Synopsis           [See if a string represents a valid identifier.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
*****************************************************************************/
short
util2_ValidId(char *s)
{
int i;

  for(i = 0;i < strlen(s);i++)
    if(isalnum(s[i]) == 0) return 0;

  return 1;  
}

/**Function********************************************************************

  Synopsis           [Start the timer.]

  Description        []

  SideEffects        [Modify the timer_start variable and ru_start variable.]
  
  SeeAlso	     []
  
*****************************************************************************/
void
util2_StartTimer()
{
  if(gettimeofday(&time_start, (struct timezone *) 0) < 0) 
    fprintf(msgerr, "util_timer_start: gettimeofday() error!\n");
    
  if(getrusage(RUSAGE_SELF, &ru_start) < 0)
    fprintf(msgerr, "util_timer_start: getrusage() error!\n");    
}

/**Function********************************************************************

  Synopsis           [Start the timer.]

  Description        []

  SideEffects        [Modify the timer_stop variable and ru_stop variable.]
  
  SeeAlso	     []
  
*****************************************************************************/
void
util2_StopTimer()
{
  if(getrusage(RUSAGE_SELF, &ru_stop) < 0)
    fprintf(stderr, "util_timer_stop: getrusage() error!\n");    

  if(gettimeofday(&time_stop, (struct timezone *) 0) < 0) 
    fprintf(stderr, "util_timer_stop: gettimeofday() error!\n");
}

/**Function********************************************************************

  Synopsis           [Return the user time in seconds.]

  Description        []

  SideEffects        [Modify the start, stop and seconds variables.]
  
  SeeAlso	     []
  
*****************************************************************************/
double
util2_GetUtime()
{
  start = ((double) ru_start.ru_utime.tv_sec) * 1000000.0 +
    ru_start.ru_utime.tv_usec;
  stop = ((double) ru_stop.ru_utime.tv_sec) * 1000000.0 +
    ru_stop.ru_utime.tv_usec;
  seconds = (stop - start) / 1000000.0;
  
  return seconds;
}

/**Function********************************************************************

  Synopsis           [Return the system time in seconds.]

  Description        []

  SideEffects        [Modify the start, stop and seconds variables.]
  
  SeeAlso	     []
  
*****************************************************************************/
double
util2_GetStime()
{
  start = ((double) ru_start.ru_stime.tv_sec) * 1000000.0 +
    ru_start.ru_stime.tv_usec;
  stop = ((double) ru_stop.ru_stime.tv_sec) * 1000000.0 +
    ru_stop.ru_stime.tv_usec;
  seconds = (stop - start) / 1000000.0;

  return seconds;
}

/**Function********************************************************************

  Synopsis           [Return the total time in seconds.]

  Description        []

  SideEffects        [Modify the start, stop and seconds variables.]
  
  SeeAlso	     []
  
*****************************************************************************/
double
util2_GetRtime()
{

  start = ((double) time_start.tv_sec) * 1000000.0 +
    time_start.tv_usec;
  stop = ((double) time_stop.tv_sec) * 1000000.0 +
    time_stop.tv_usec;
  seconds = (stop - start) / 1000000.0;

  return seconds;
}

/**Function********************************************************************

  Synopsis           [Find out if a given point belongs to a line.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
*****************************************************************************/
short
util2_InLine(
  int x,
  int y,
  int x1,
  int y1,
  int x2,
  int y2)
{
int t;

  if(((x2 > x1) && (y1 < y2)) || ((x2 > x1) && (y2 > y1))) {
    t = x1;
    x1 = x2;
    x2 = t;
    t = y1;
    y1 = y2;
    y2 = t;
  }
    
  if((x1 < x2) && (y1 > y2)) {
    if( (x >= x1) && (x < x2) && (y >= y2) && (y < y1) && ((x - x1) != 0) &&
      ((x2 - x) != 0) && 
      ( ( (float) ((y1 - y)/(x - x1)) <= (float) ((y - y2)/(x2 - x)) + 0.5 ) ||
        ( (float) ((y1 - y)/(x - x1)) >= (float) ((y - y2)/(x2 - x)) - 0.5 ) ))
      return 1;
  }
  else
    if(y1 == y2) {
      if( (((x >= x1) && (x < x2)) || ((x >= x2) && (x < x1))) && (y == y1) )
       return 1;
    }
    else
    if((x1 > x2) && (y1 > y2)) {
      if( (x >= x2) && (y >= y2) && (x < x1) && (y < y1) && ((x - x2) != 0) &&
          ((x1 - x) != 0) &&
          ( ( (float) ((y - y2)/(x - x2)) <= (float) ((y1 - y)/(x1 - x)) + 5 ) ||
            ( (float) ((y - y2)/(x - x2)) >= (float) ((y1 - y)/(x1 - x)) - 5 ) ))
        return 1;    
    }
    if(x1 == x2) {
      if( (((y >= y1) && (y < y2)) || ((y >= y2) && (y < y1))) && (x == x1) )
        return 1;
    }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Find out if a given point is inside a rectangle.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
*****************************************************************************/
short
util2_InRect(
  int x,
  int y,
  int x1,
  int y1,
  int width,
  int height)
{
  if((width > 0) && (height > 0)) {
    if((x >= x1) && (y >= y1) && (x < x1 + width) && (y < y1 + height)) {
      return 1;  
    }
    else {
      return 0;
    }
  }
  else 
    if(width < 0) {
      if((x >= x1 + width) && (y >= y1) && (x < x1) && (y < y1 + height)) {
        return 1;
      }
      else {
        return 0;
      }
    }
    else
      if(height < 0) {
        if((x >= x1) && (y >= y1 + height) && (x < x1 + width) && 
          (y < y1)) {
          return 1;
        }
        else {
          return 0; 
        }
      }  
  return 0;
}
