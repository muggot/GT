#include "gt.h"

void evt_UL_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_UL;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void UL_act_Wgo(obj *o)
{
 double dr;

 if(o->pl->tag!=T_WORLD) return;

 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.ct=GTIME; o->s.tx=o->s.ty=0.0;
 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm);
 o->mem->tt=GTIME;
}


// в штатном режиме улей летит в оптимальную точку для добычи ресов спарками
// найдем эту точку и отправим туда UL
void evt_UL_act(obj *o,evt *e,lnk *l)
{
 int i,j=0;
 double sx=0.0,sy=0.0,cx,cy,d,dt;
 evt *me;

 del_evt(o,e); free_evt(o,e);

 UL_act_Wgo(o); o->mem->hx=o->s.cx; o->mem->hy=o->s.cy;
 for(i=0;i<6;i++)
 {
  if(o->mem->p[i][0]!=0.0 || o->mem->p[i][1]!=0.0)
   { sx+=o->mem->p[i][0]; sy+=o->mem->p[i][1]; j++; }
 }

 if(j==0) dt=3600.0; // подождем часок
 else 
 { 
  sx/=j; sy/=j; cx=sx-o->s.cx; cy=sy-o->s.cy; d=cx*cx+cy*cy;
  if(d<=90000.0) dt=3600.0; // ближе не подлетаем
  else
  {
   d=sqrt(d); o->s.tx=cx/d; o->s.ty=cy/d; dt=(d-250.0)/o->s.v;
  }
 }
 me=get_evt(); evt_UL_set(me,o,NULL,NULL,dt); ins_evt(o,me);
}
