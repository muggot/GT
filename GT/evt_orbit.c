#include "gt.h"

// параметры движения орбитальных объектов
#define		PI	3.1415926535897932384626433832795
#define		DFI	0.01

double fi_elip(orbit *o,double t)
{
 return(2.0*PI*o->v*t);
}

double fi_hyper(orbit *o,double t)
{
 double fi=acos(-1.0/o->e)*(1-1/(1+fabs(o->v*t)));
 if(o->v*t<0.0) fi*=-1.0;
 return fi;
}

double t_elip(orbit *o,double fi)
{
 return(fi/(2.0*PI*o->v));
}

double t_hyper(orbit *o,double fi)
{
 return(fi/(o->v*(acos(-1.0/o->e)-fabs(fi))));
}

double r_fi(orbit *o,double fi)
{
 return(o->p/(1+o->e*cos(fi)));
}

//вычисляет время следующей контрольной точки для элиптической орбиты
//поскольку dfi/dt=const то следующую функцию можно еще больше упростить

double get_next_cp_elip(obj *o, double t, double *fr)
{
 double fi,r,xn,yn,tn,n;
 tn=t-o->orb.t; n=floor(tn*o->orb.v); tn-=n/o->orb.v;
 if(o->orb.v>0.0) fi=fi_elip(&o->orb,tn)+DFI; else fi=fi_elip(&o->orb,tn)-DFI;
 tn=t_elip(&o->orb,fi)+o->orb.t+n/o->orb.v;
 *fr=fi; return(tn);
}

/*
double get_next_cp_elip(orbit *o, double t)
{
 return(t+DFI/(2.0*PI*fabs(o->v)));
}
*/

//вычисляет время следующей контрольной точки для элиптической орбиты
double get_next_cp_hyper(obj *o, double t, double *fr)
{
 double fi,tn;
 tn=t-o->orb.t;
 if(o->orb.v>0.0) fi=fi_hyper(&o->orb,tn)+DFI; else fi=fi_hyper(&o->orb,tn)-DFI;
 if(fabs(fi)>acos(-1.0/o->orb.e)) tn=t+1000000.0;
 else tn=t_hyper(&o->orb,fi)+o->orb.t;
 *fr=fi; return(tn);
}

double get_next_cp(obj *o, double t, double *fr)
{
 if(o->orb.e>1) return(get_next_cp_hyper(o,t,fr)); 
 else return(get_next_cp_elip(o,t,fr));
}

void evt_orb_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_orb;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

// инициализация орбитального движения
void evt_orb_init(obj *o)
{
 double fi,r,xn,yn;
 evt *de;
 lnk *ml;

// нахожу текущее положение объекта
 if(o->orb.e>1.0) fi=fi_hyper(&o->orb,GTIME-o->orb.t); else fi=fi_elip(&o->orb,GTIME-o->orb.t);
 r=r_fi(&o->orb,fi); 

 if(o->orb.e<1.0) 
 { 
  xn=r*cos(fi)+2.0*o->orb.p*o->orb.e/(1-o->orb.e*o->orb.e); yn=r*sin(fi);
  o->s.cx=xn*cos(o->orb.ang)-yn*sin(o->orb.ang)+o->orb.ox; 
  o->s.cy=xn*sin(o->orb.ang)+yn*cos(o->orb.ang)+o->orb.oy; 
 }
 else
 {
  fi+=o->orb.ang; 
  o->s.cx=r*cos(fi)+o->orb.ox; o->s.cy=r*sin(fi)+o->orb.oy;
 }
 o->s.ct=GTIME; o->s.v=0; o->s.tx=o->s.ty=0;

 loc_obj_reg(o,0);
 de=get_evt(); evt_cold_loc_set(de,o,NULL,NULL,0.0);
 ml=evt_lnk_ins(lnks_tags[E_orb],o,de,NULL); ml->l=ml; de->l=ml;
}

void evt_orb_act(obj *o,evt *e,lnk *l)
{
 double tn,fi,r,xn,yn,xp,yp;
 evt *me;

 del_evt(o,e); me=e;
 if(o->s.ct==0.0) { evt_orb_init(o); }

 tn=GTIME-o->s.ct;
 o->s.cx+=tn*o->s.tx*o->s.v; o->s.cy+=tn*o->s.ty*o->s.v; o->s.ct=GTIME;

 tn=get_next_cp(o,GTIME,&fi);
 r=r_fi(&o->orb,fi); 

 if(o->orb.e<1.0) 
 { 
  xp=r*cos(fi)+2.0*o->orb.p*o->orb.e/(1-o->orb.e*o->orb.e); yp=r*sin(fi);
  xn=xp*cos(o->orb.ang)-yp*sin(o->orb.ang)+o->orb.ox; 
  yn=xp*sin(o->orb.ang)+yp*cos(o->orb.ang)+o->orb.oy; 
 }
 else
 {
  fi+=o->orb.ang;
  xn=r*cos(fi)+o->orb.ox; yn=r*sin(fi)+o->orb.oy;
 }

// теперь мы знаем где должен быть объект в момент времени tn
// найдем скорость движения
 xn-=o->s.cx; yn-=o->s.cy; r=sqrt(xn*xn+yn*yn);
 o->s.v=r/(tn-GTIME); o->s.tx=xn/r; o->s.ty=yn/r;

// параметры движения до следующей контрольной точки заданы
// создадим следующее событие evt_orb_act
 evt_orb_set(me,o,NULL,NULL,tn-GTIME); ins_evt(o,me); o->s.me=me;
}
