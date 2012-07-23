#include "gt.h"


void evt_bmv_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bmv;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_buse_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_buse;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
 o->s.me=ne;
}

void evt_bobj_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bobj;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_bkill_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bkill;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}


lnk *get_slot_bytype(lnk *dl,char st)
{
 lnk *l=dl;
 while(l!=NULL)
 {
  if(l->st[0]==st && l->o!=NULL) return l;
  l=l->next;
 }
 return NULL;
}


// событие начала движения объекта в бою в пошаговом режиме
void evt_bm_act(obj *o,evt *e,lnk *l)
{
 double dr,tx=l->tx,ty=l->ty;

 free_lnk(NULL,l); e->l=NULL; del_evt(o,e); free_evt(o,e);

 dr=(GTIME-o->s.ct)*o->s.v; 
 o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;

 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm)*BTURN_TIME; 
 o->s.tx=tx; o->s.ty=ty; o->s.ct=GTIME; 

// loc_col_bmv(o,0.0);
}

// событие начала движения объекта в бою в режиме реального времени
void evt_bmv_act(obj *o,evt *e,lnk *l)
{
 double dr,t=0.0,tx=l->tx,ty=l->ty;
 int mode=l->n;

 if(o->pl->bmode==1) { evt_bm_act(o,e,l); return; } // пошаговый режим

 free_lnk(NULL,l); e->l=NULL; del_evt(o,e); free_evt(o,e); o->s.me=NULL;

 dr=(GTIME-o->s.ct)*o->s.v; 
 o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;

 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm); 

 if(mode!=0 && (tx!=0.0 || ty!=0.0)) // движение в указанную точку
 {
  tx-=o->s.cx; ty-=o->s.cy;
  dr=sqrt(tx*tx+ty*ty); t=dr/o->s.v; dr=1.0/dr; tx*=dr; ty*=dr;
  l=get_lnk(); l->tx=0.0; l->ty=0.0; l->n=0;
  e=get_evt(); evt_bmv_set(e,o,NULL,l,t); ins_evt(o,e); o->s.me=e;
 }
 else
  { dr=tx*tx+ty*ty; if(dr>1.001) { dr=1.0/sqrt(dr); tx*=dr; ty*=dr; } }

 o->s.tx=tx; o->s.ty=ty; if(tx!=0.0 || ty!=0.0) o->s.ct=GTIME; 

 locs_send_mv(o);
}

// событие смерти объекта в бою
// объект выкидывается из боя
// если объект управляемый, то еще телепортируется в портал
// если не управляемый, то уничтожается
void bkill_PERS_rt(obj *o,evt *e,lnk *l)
{
 double x,y,dr;
 obj *po;
 lnk *ml; evt *me;

// останавливаю корабль
 if(o->s.me!=NULL) // событие уже есть
 {
  me=o->s.me; 
  me->t=GTIME; me->l->tx=0.0; me->l->ty=0.0; me->l->n=0; move_evt(o,me);
 }
 else // события нет, создаю
 {
  ml=get_lnk(); ml->tx=0.0; ml->ty=0.0; ml->n=0;
  me=get_evt(); evt_func_set[E_bmv](me,o,NULL,ml,0.0); ins_evt(o,me); o->s.me=me;
 }
 
// loc_obj_unreg(o); // убираю корабль с карты
 po=o->port;

 ml=get_lnk(); ml->o=po;
 me=get_evt(); evt_func_set[E_prich_user](me,o,NULL,ml,0.1); ins_evt(o,me);

/* 
 dr=(GTIME-po->s.ct)*po->s.v; x=po->s.cx+po->s.tx*dr; y=po->s.cy+po->s.ty*dr;
 if(1.0*rand()/RAND_MAX>0.5) dr=-1.0; else dr=1.0;
 o->s.cx=x+po->s.xs*dr*(0.55+0.45*rand()/RAND_MAX); 
 if(1.0*rand()/RAND_MAX>0.5) dr=-1.0; else dr=1.0;
 o->s.cy=y+po->s.ys*dr*(0.55+0.45*rand()/RAND_MAX);
 o->s.ct=GTIME; o->s.tx=0.0; o->s.ty=0.0; o->s.v=0.0;
 loc_obj_reg(o,0);
// cmd_send_iam(o);
 // отправляю список объектов видимых локаций
 locs_send_obj(o);
*/
}

// полное уничтожение объекта и всех дочерних объектов
void bkill_OBJ_rt(obj *o,evt *e,lnk *l)
{
 
}

void evt_bkill_act(obj *o,evt *e,lnk *l)
{
 obj *bo=o->pl;
 char *b; int n;

 del_evt(o,e); free_evt(o,e);

 n=snprintf(NULL,0,"<BKILL t=\"%f\" id=\"%x\" pid=\"%x\" />",GTIME,o,bo->pl)+1; b=mymalloc(n); 
 snprintf(b,n,"<BKILL t=\"%f\" id=\"%x\" pid=\"%x\" />",GTIME,o,bo->pl);
 cmd_send_buse(bo, b, n); myfree(b);

// выкинем объект из боя
 o->bside=0; obj_ch_parent(o,bo->pl);
 
 if(o->tag==T_PERS) bkill_PERS_rt(o,e,l);
 else               bkill_OBJ_rt(o,e,l);
}

double upd_FLD_tb(obj *o)
{
 double dp,dt;
 obj *bo=o->pl->pl;

 dp=o->dev.pwr[0]-o->dev.pwr[1];
 if(dp<=0.0) return o->dev.pwr[1];

 dt=bo->rt-o->rt;
 if(dt>=0.0)
 {
  o->dev.pwr[1]+=dt*o->dev.pwr[2]; 
  if(o->dev.pwr[1]>o->dev.pwr[0]) o->dev.pwr[1]=o->dev.pwr[0]; 
  dp=o->dev.pwr[0]-o->dev.pwr[1];
  o->rt=bo->s.ct;
 }
// вызов в течении хода, посчитаем как идет восстановление
 if(dp>0.0)
 {
  dt=GTIME-o->rt;
  o->dev.pwr[1]+=dt*o->dev.pwr[2]; 
  if(o->dev.pwr[1]>o->dev.pwr[0]) o->dev.pwr[1]=o->dev.pwr[0]; 
 }
 o->rt=GTIME;
 return o->dev.pwr[1];
}

void buse_FLD_tb(obj *o,evt *e,lnk *l)
{
 double dp,dt;
 obj *bo=o->pl->pl;
 evt *me;

 if(l!=NULL) free_lnk(NULL,l); e->l=NULL; del_evt(o,e); free_evt(o,e); o->s.me=NULL;

 dp=o->dev.pwr[0]-o->dev.pwr[1];
 dt=bo->rt-o->rt;
 if(dt>=0.0) // первый вызов за ход
 {
  if(dp>0.0)
  {
   o->dev.pwr[1]+=dt*o->dev.pwr[2]; 
   if(o->dev.pwr[1]>o->dev.pwr[0]) o->dev.pwr[1]=o->dev.pwr[0]; 
   dp=o->dev.pwr[0]-o->dev.pwr[1];
  }
  o->rt=bo->s.ct;
 }
// вызов в течении хода, посчитаем как идет восстановление
 if(dp>0.0)
 {
  dt=GTIME-o->rt;
  o->dev.pwr[1]+=dt*o->dev.pwr[2]; 
  if(o->dev.pwr[1]>o->dev.pwr[0]) o->dev.pwr[1]=o->dev.pwr[0]; 
  dp=o->dev.pwr[0]-o->dev.pwr[1];
 }

 if(GTIME==o->rt) // было повреждение поля, нужно сообщить клиентам состояние
 {
 }

 o->rt=GTIME;
//  посчитаем не нужно ли вставить еще одно событие
 if(dp<=0.0) return;
 if(o->dev.pwr[2]<=0.0) return;

 dt=dp/o->dev.pwr[2]*RBTURN_TIME;
 if(GTIME-bo->s.ct+dt>1.0) return; // за ход не укладываемся
 me=get_evt(); evt_buse_set(me,o,NULL,NULL,dt); ins_evt(o,me);
}


double upd_FLD_rt(obj *o)
{
 double dp,dt;

 dp=o->dev.pwr[0]-o->dev.pwr[1];
 if(dp<=0.0) return o->dev.pwr[1];

 dt=GTIME-o->s.ct;
 o->dev.pwr[1]+=dt*o->dev.pwr[2]; 
 if(o->dev.pwr[1]>o->dev.pwr[0]) o->dev.pwr[1]=o->dev.pwr[0]; 
 o->s.ct=GTIME;
 return o->dev.pwr[1];
}

void buse_FLD_rt(obj *o,evt *e,lnk *l)
{
 double dp,dt;
 obj *bo=o->pl->pl;
 evt *me;

 if(l!=NULL) free_lnk(NULL,l); e->l=NULL; del_evt(o,e); free_evt(o,e); o->s.me=NULL;

 dp=o->dev.pwr[0]-o->dev.pwr[1];
 if(dp>0.0)
 {
  dt=GTIME-o->s.ct;
  o->dev.pwr[1]+=dt*o->dev.pwr[2]; 
  if(o->dev.pwr[1]>o->dev.pwr[0]) o->dev.pwr[1]=o->dev.pwr[0]; 
  dp=o->dev.pwr[0]-o->dev.pwr[1];
 }

 if(GTIME==o->s.ct) // было повреждение поля, нужно сообщить клиентам состояние
 {
 }

 o->s.ct=GTIME;
//  посчитаем не нужно ли вставить еще одно событие
 if(dp<=0.0) return;
 if(o->dev.pwr[2]<=0.0) return;

 dt=dp/o->dev.pwr[2];
 me=get_evt(); evt_buse_set(me,o,NULL,NULL,dt); ins_evt(o,me); o->s.me=me;
}


// событие использования оружия в бою
void buse_WPN_tb(obj *o,evt *e,lnk *l)
{
 obj *eo=l->o,*co;
 obj *bo;
 obj *mo;
 double dr,r,rmin,d,ex=l->tx,ey=l->ty;
 double fi,rt,mx,my,tx,ty,fx,fy;
 float dmg[8][2],dmgsum=0.0;
 int i,n;
 char *b;
 lnk *ml;
 evt *me;


 free_lnk(NULL,l); e->l=NULL; del_evt(o,e); free_evt(o,e); o->s.me=NULL;

 mo=o->pl; while(mo->pl->tag!=T_BL) mo=mo->pl; bo=mo->pl;

// проверяю готовность устройства
 if(o->rt==0.0) rt=o->dev.pwr[2]; 
 else if(o->rt<bo->rt) rt=(bo->rt-o->rt)+(GTIME-bo->s.ct);
 else rt=GTIME-o->rt;

 if(rt<o->dev.pwr[2]) return; // объект не готов к использованию

 dr=(GTIME-mo->s.ct)*mo->s.v; mx=mo->s.cx+mo->s.tx*dr; my=mo->s.cy+mo->s.ty*dr; 
 if(eo!=NULL)
 { dr=(GTIME-eo->s.ct)*eo->s.v; ex=eo->s.cx+eo->s.tx*dr; ey=eo->s.cy+eo->s.ty*dr; }

// нужно стрельнуть и посмотреть куда попали
// найдем направление выстрела
 tx=ex-mx; ty=ey-my; 
 fi=(1.0-o->dev.pwr[1])/(0.636619772367581343+o->dev.pwr[1])*(1.0-2.0*rand()/RAND_MAX);
 fx=tx*cos(fi)-ty*sin(fi); fy=tx*sin(fi)+ty*cos(fi); // направление выстрела

 dr=fx*fx+fy*fy; dr=1.0/sqrt(dr); fx*=dr; fy*=dr;

 rmin=o->dev.pwr[1]*2.0; // дальность на которой повреждения 0
 co=bo->bo; eo=NULL;
 while(co!=NULL) // пройдемся по объектам и найдем в кого попали
 {
  dr=(GTIME-co->s.ct)*co->s.v; ex=co->s.cx+co->s.tx*dr-mx; ey=co->s.cy+co->s.ty*dr-my;
  r=fx*ex+fy*ey;
  if(r>0.0 && r<rmin && co!=mo) // не очень правильно, ну да пусть будет так
  {
   r=fx*ey-fy*ex; if(r<co->s.r1) { eo=co; rmin=r; }
  }
  co=co->next;
 }

 if(eo==NULL) //никуда не попали, создадим сообщение и закончим на этом
 {
  n=snprintf(NULL,0,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"0\" />",GTIME-bo->s.ct,mo,o,fx,fy)+1; b=mymalloc(n); 
  snprintf(b,n,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"0\" />",GTIME-bo->s.ct,mo,o,fx,fy);
  ml=get_lnk(); ml->next=bo->bo_turns; ml->prev=NULL; bo->bo_turns=ml; ml->o=mo; ml->b=b;
  o->rt=GTIME; return;
 }

// подсчитаем повреждения
 for(i=0;i<7;i++)
 {
  dmg[i][0]=o->dev.atkp[i][0]+(o->dev.atkp[i][1]-o->dev.atkp[i][0])*rand()/RAND_MAX;
 }

// найдем все генераторы поля
 dmg[7][0]=dmg[7][1]=0;
 ml=eo->dev.dvsl;
 while(ml!=NULL)
 {
  ml=get_slot_bytype(ml,'F');
  if(ml!=NULL)
  {
   co=ml->o;
   for(i=0;i<7;i++) 
   {
    dmg[i][1]=co->dev.defp[i][0]+(co->dev.defp[i][1]-co->dev.defp[i][0])*rand()/RAND_MAX;
    if(dmg[i][1]>dmg[i][0]) dmg[i][1]=dmg[i][0];
    dmg[i][0]-=dmg[i][1]; dmg[7][0]+=dmg[i][1];
   }
   dmg[7][1]=upd_FLD_tb(co); // состояние поля на текущий момент
   if(dmg[7][0]>dmg[7][1]) // заряда поля не достаточно, часть повреждений нужно вернуть
   {
    dr=(dmg[7][0]-dmg[7][1])/dmg[7][0];
    for(i=0;i<7;i++) dmg[i][0]+=dmg[i][1]*dr;
    dmg[7][0]=dmg[7][1];
   }
   dmgsum+=dmg[7][0];
   co->dev.pwr[1]-=dmg[7][0];
   if(dmg[7][0]>0.0) 
   {
    n=snprintf(NULL,0,"<BFLD t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0])+1; b=mymalloc(n); 
    snprintf(b,n,"<BFLD t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0]);
    ml=get_lnk(); ml->next=bo->bo_turns; ml->prev=NULL; bo->bo_turns=ml; ml->o=eo; ml->b=b;
    me=get_evt(); evt_buse_set(me,co,NULL,NULL,0.0); ins_evt(co,me); // ревизия состояния поля
   }
  }
 }

// найдем все слои брони, в отличие от поля, броня поглощает часть повреждений,
// а остальная часть идет на ее разрушение. Если состояние брони 0 то она
// пропускает повреждения дальше
 dmg[7][0]=dmg[7][1]=0;
 ml=eo->dev.dvsl;
 while(ml!=NULL)
 {
  ml=get_slot_bytype(ml,'H');
  if(ml!=NULL && ml->o->dev.chull>0.0)
  {
   co=ml->o;
   for(i=0;i<7;i++) 
   {
    dmg[i][1]=co->dev.defp[i][0]+(co->dev.defp[i][1]-co->dev.defp[i][0])*rand()/RAND_MAX;
    if(dmg[i][1]>dmg[i][0]) dmg[i][1]=dmg[i][0];
    dmg[i][0]-=dmg[i][1]; dmg[7][0]+=dmg[i][0];
   }
   dmg[7][1]=co->dev.chull; // состояние брони на текущий момент
   if(dmg[7][0]>dmg[7][1])
   {
    dr=(dmg[7][0]-dmg[7][1])/dmg[7][0];
    for(i=0;i<7;i++) dmg[i][0]*=dr;
    dmg[7][0]=dmg[7][1];
   }
   dmgsum+=dmg[7][0];
   co->dev.chull-=dmg[7][0];
   if(dmg[7][0]>0.0)
   {
    n=snprintf(NULL,0,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0])+1; b=mymalloc(n); 
    snprintf(b,n,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0]);
    ml=get_lnk(); ml->next=bo->bo_turns; ml->prev=NULL; bo->bo_turns=ml; ml->o=eo; ml->b=b;
   }
  }
 }

// найдем корпус корабля
 dmg[7][0]=dmg[7][1]=0;
 co=eo->dev.mysl->o;
 if(co->dev.chull>0.0)
 {
  for(i=0;i<7;i++) 
  {
   dmg[i][1]=co->dev.defp[i][0]+(co->dev.defp[i][1]-co->dev.defp[i][0])*rand()/RAND_MAX;
   if(dmg[i][1]>dmg[i][0]) dmg[i][1]=dmg[i][0];
   dmg[i][0]-=dmg[i][1]; dmg[7][0]+=dmg[i][0];
  }
  dmg[7][1]=co->dev.chull; // состояние брони на текущий момент
  if(dmg[7][0]>dmg[7][1])
  {
   dr=(dmg[7][0]-dmg[7][1])/dmg[7][0];
   for(i=0;i<7;i++) dmg[i][0]*=dr;
   dmg[7][0]=dmg[7][1];
  }
  dmgsum+=dmg[7][0];
  co->dev.chull-=dmg[7][0];
  if(dmg[7][0]>0.0)
  {
   n=snprintf(NULL,0,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0])+1; b=mymalloc(n); 
   snprintf(b,n,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0]);
   ml=get_lnk(); ml->next=bo->bo_turns; ml->prev=NULL; bo->bo_turns=ml; ml->o=eo; ml->b=b;
  }
 }

 for(i=0;i<7;i++) dmgsum+=dmg[i][0];

 n=snprintf(NULL,0,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"%f\" tid=\"%x\" />",GTIME-bo->s.ct,mo,o,fx,fy,dmgsum,eo)+1; b=mymalloc(n); 
 snprintf(b,n,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"%f\" tid=\"%x\" />",GTIME-bo->s.ct,mo,o,fx,fy,dmgsum,eo);
 ml=get_lnk(); ml->next=bo->bo_turns; ml->prev=NULL; bo->bo_turns=ml; ml->o=mo; ml->b=b;

// пока считаем что при состоянии корпуса корабля 0 объект умирает
 if(co->dev.chull<=0.0)
 {
  me=get_evt(); evt_bkill_set(me,eo,NULL,NULL,0.0); ins_evt(co,me); // убиваю объект
 }

 o->rt=GTIME;
}

// событие использования оружия в бою
void buse_WPN_rt(obj *o,evt *e,lnk *l)
{
 obj *eo=l->o,*co;
 obj *bo;
 obj *mo;
 double dr,r,rmin,d,ex=l->tx,ey=l->ty;
 double fi,rt,mx,my,tx,ty,fx,fy;
 float dmg[8][2],dmgsum=0.0;
 int i,n;
 char *b;
 lnk *ml,*nl;
 evt *me;


 e->l=NULL; del_evt(o,e); free_evt(o,e); 

 mo=o->pl; bo=mo->pl; if(bo->tag!=T_BL) { free_lnk(NULL,l); o->s.me=NULL; return; } // бой закончился

// проверяю готовность устройства
 rt=GTIME-o->s.ct; 
 if(rt<o->dev.pwr[2]) // объект не готов к использованию, сдвигаю событие
 {
  printf("Подозрительное место, делаю задержку с повторным вызовом\n");
  me=get_evt(); evt_buse_set(me,o,NULL,l,o->dev.pwr[2]-rt+0.001); ins_evt(o,me); o->s.me=me;
  return;
 }

 if(l->st[0]==0) { free_lnk(NULL,l); o->s.me=NULL; } // одиночный выстрел
 else 
 if(l->st[0]==1) 
 {
  if(eo==NULL || eo->pl==bo)
  { me=get_evt(); evt_buse_set(me,o,NULL,l,o->dev.pwr[2]); ins_evt(o,me); o->s.me=me; } // продолжаю очередь
  else { free_lnk(NULL,l); o->s.me=NULL; return; } // уже мертв, обрываю очередь
 }

 dr=(GTIME-mo->s.ct)*mo->s.v; mx=mo->s.cx+mo->s.tx*dr; my=mo->s.cy+mo->s.ty*dr; 
 if(eo!=NULL)
 { dr=(GTIME-eo->s.ct)*eo->s.v; ex=eo->s.cx+eo->s.tx*dr; ey=eo->s.cy+eo->s.ty*dr; }

// нужно стрельнуть и посмотреть куда попали
// найдем направление выстрела
 tx=ex-mx; ty=ey-my; 
 fi=(1.0-o->dev.pwr[1])/(0.636619772367581343+o->dev.pwr[1])*(1.0-2.0*rand()/RAND_MAX);
 fx=tx*cos(fi)-ty*sin(fi); fy=tx*sin(fi)+ty*cos(fi); // направление выстрела

 dr=fx*fx+fy*fy; dr=1.0/sqrt(dr); fx*=dr; fy*=dr;

 rmin=o->dev.pwr[0]*2.0; // дальность на которой повреждения 0
 co=bo->bo; eo=NULL;
 while(co!=NULL) // пройдемся по объектам и найдем в кого попали
 {
  dr=(GTIME-co->s.ct)*co->s.v; ex=co->s.cx+co->s.tx*dr-mx; ey=co->s.cy+co->s.ty*dr-my;
  r=fx*ex+fy*ey;
  if(r>0.0 && r<rmin && co!=mo) // не очень правильно, ну да пусть будет так
  {
   r=fabs(fx*ey-fy*ex); if(r<co->s.r1) { eo=co; rmin=r; }
  }
  co=co->next;
 }

 o->s.ct=GTIME;

 if(eo==NULL) //никуда не попали, создадим сообщение и закончим на этом
 {
  n=snprintf(NULL,0,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"0\" />",GTIME,mo,o,fx,fy)+1; b=mymalloc(n); 
  snprintf(b,n,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"0\" />",GTIME,mo,o,fx,fy);
  cmd_send_buse(bo, b, n); myfree(b);
  return;
 }

// подсчитаем повреждения
 for(i=0;i<7;i++)
 {
  dmg[i][0]=o->dev.atkp[i][0]+(o->dev.atkp[i][1]-o->dev.atkp[i][0])*rand()/RAND_MAX;
 }

// найдем все генераторы поля
 dmg[7][0]=dmg[7][1]=0;
 ml=eo->dev.dvsl;
 while(ml!=NULL)
 {
  ml=get_slot_bytype(ml,'F');
  if(ml!=NULL)
  {
   co=ml->o;
   for(i=0;i<7;i++) 
   {
    dmg[i][1]=co->dev.defp[i][0]+(co->dev.defp[i][1]-co->dev.defp[i][0])*rand()/RAND_MAX;
    if(dmg[i][1]>dmg[i][0]) dmg[i][1]=dmg[i][0];
    dmg[i][0]-=dmg[i][1]; dmg[7][0]+=dmg[i][1];
   }
   dmg[7][1]=upd_FLD_rt(co); // состояние поля на текущий момент
   if(dmg[7][0]>dmg[7][1]) // заряда поля не достаточно, часть повреждений нужно вернуть
   {
    dr=(dmg[7][0]-dmg[7][1])/dmg[7][0];
    for(i=0;i<7;i++) dmg[i][0]+=dmg[i][1]*dr;
    dmg[7][0]=dmg[7][1];
   }
   dmgsum+=dmg[7][0];
   co->dev.pwr[1]-=dmg[7][0];
   if(dmg[7][0]>0.0) 
   {
    n=snprintf(NULL,0,"<BFLD t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0])+1; b=mymalloc(n); 
    snprintf(b,n,"<BFLD t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0]);
    cmd_send_buse(bo, b, n); myfree(b);
    me=get_evt(); evt_buse_set(me,co,NULL,NULL,0.0); ins_evt(co,me); // ревизия состояния поля
   }
   ml=ml->next;
  }
 }

// найдем все слои брони, в отличие от поля, броня поглощает часть повреждений,
// а остальная часть идет на ее разрушение. Если состояние брони 0 то она
// пропускает повреждения дальше
 dmg[7][0]=dmg[7][1]=0;
 ml=eo->dev.dvsl;
 while(ml!=NULL)
 {
  ml=get_slot_bytype(ml,'H');
  if(ml!=NULL && ml->o->dev.chull>0.0)
  {
   co=ml->o;
   for(i=0;i<7;i++) 
   {
    dmg[i][1]=co->dev.defp[i][0]+(co->dev.defp[i][1]-co->dev.defp[i][0])*rand()/RAND_MAX;
    if(dmg[i][1]>dmg[i][0]) dmg[i][1]=dmg[i][0];
    dmg[i][0]-=dmg[i][1]; dmg[7][0]+=dmg[i][0];
   }
   dmg[7][1]=co->dev.chull; // состояние брони на текущий момент
   if(dmg[7][0]>dmg[7][1])
   {
    dr=(dmg[7][0]-dmg[7][1])/dmg[7][0];
    for(i=0;i<7;i++) dmg[i][0]*=dr;
    dmg[7][0]=dmg[7][1];
   }
   dmgsum+=dmg[7][0];
   co->dev.chull-=dmg[7][0];
   if(dmg[7][0]>0.0)
   {
    n=snprintf(NULL,0,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0])+1; b=mymalloc(n); 
    snprintf(b,n,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0]);
    cmd_send_buse(bo, b, n); myfree(b);
   }
  }
  if(ml!=NULL) ml=ml->next;
 }

// найдем корпус корабля
 if(eo->dev.mysl==NULL) // у него даже корпуса нет, сразу умирает
 {
  for(i=0;i<7;i++) dmgsum+=dmg[i][0];
  n=snprintf(NULL,0,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"%f\" tid=\"%x\" />",GTIME,mo,o,fx,fy,dmgsum,eo)+1; b=mymalloc(n); 
  snprintf(b,n,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"%f\" tid=\"%x\" />",GTIME,mo,o,fx,fy,dmgsum,eo);
  cmd_send_buse(bo, b, n); myfree(b);
  me=get_evt(); evt_bkill_set(me,eo,NULL,NULL,0.0); ins_evt(eo,me);
  return; // убиваю объект
 }

 dmg[7][0]=dmg[7][1]=0;
 co=eo->dev.mysl->o;
 if(co->dev.chull>0.0)
 {
  for(i=0;i<7;i++) 
  {
   dmg[i][1]=co->dev.defp[i][0]+(co->dev.defp[i][1]-co->dev.defp[i][0])*rand()/RAND_MAX;
   if(dmg[i][1]>dmg[i][0]) dmg[i][1]=dmg[i][0];
   dmg[i][0]-=dmg[i][1]; dmg[7][0]+=dmg[i][0];
  }
  dmg[7][1]=co->dev.chull; // состояние брони на текущий момент
  if(dmg[7][0]>dmg[7][1])
  {
   dr=(dmg[7][0]-dmg[7][1])/dmg[7][0];
   for(i=0;i<7;i++) dmg[i][0]*=dr;
   dmg[7][0]=dmg[7][1];
  }
  dmgsum+=dmg[7][0];
  co->dev.chull-=dmg[7][0];
  if(dmg[7][0]>0.0)
  {
   n=snprintf(NULL,0,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0])+1; b=mymalloc(n); 
   snprintf(b,n,"<BDMG t=\"%f\" id=\"%x\" obj=\"%x\" dmg=\"%f\" />",GTIME-bo->s.ct,eo,co,dmg[7][0]);
   cmd_send_buse(bo, b, n); myfree(b);
  }
 }

 for(i=0;i<7;i++) dmgsum+=dmg[i][0];

 n=snprintf(NULL,0,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"%f\" tid=\"%x\" />",GTIME,mo,o,fx,fy,dmgsum,eo)+1; b=mymalloc(n); 
 snprintf(b,n,"<BUSE t=\"%f\" id=\"%x\" obj=\"%x\" trg=\"%f,%f\" res=\"%f\" tid=\"%x\" />",GTIME,mo,o,fx,fy,dmgsum,eo);
 cmd_send_buse(bo, b, n); myfree(b);

// пока считаем что при состоянии корпуса корабля 0 объект умирает
 if(co->dev.chull<=0.0)
 {
  me=get_evt(); evt_bkill_set(me,eo,NULL,NULL,0.0); ins_evt(eo,me); // убиваю объект
 }
}

void evt_buse_act(obj *o,evt *e,lnk *l)
{
 if(o->tag==T_FLD) { buse_FLD_rt(o,e,l); return; }
 if(o->tag==T_WPN) { buse_WPN_rt(o,e,l); return; }
 if(l!=NULL) free_lnk(NULL,l); e->l=NULL; del_evt(o,e); free_evt(o,e); o->s.me=NULL;
}



void (*bobj_proc[OBJ_TAG_NUM])(obj *o,evt *e,lnk *l)=
{
 NULL, //T_GAME
 NULL, //T_WORLD
 NULL, //T_FF
 NULL, //T_STATION
 NULL, //T_JUMP
 NULL, //T_SY
 NULL, //T_FP
 NULL, //T_PERS
 NULL, //T_NIF
 NULL, //T_AS
 NULL, //T_BL
 NULL, //T_SHP
 NULL, //T_CRGO
 NULL, //T_ENG
 NULL, //T_FLD
 NULL, //T_FTN
 NULL, //T_HULL
 NULL, //T_WPN
 NULL, //T_BS
 NULL, //T_UL
 NULL, //T_SP
 NULL, //T_MUSOR
 NULL, //T_RES
 NULL //T_ARS
};

// событие планирования хода для встроенных автоматических ботов
void evt_bobj_act(obj *o,evt *e,lnk *l)
{
 if(bobj_proc[o->tag]!=NULL) bobj_proc[o->tag](o,e,l);
 else { del_evt(o,e); free_evt(o,e); }
}

void BL_init_rt(obj *o)
{
 obj *co;
 evt *me;

 co=o->bo;
 while(co!=NULL) 
 { 
// для встроенных ботов создаем событие планирования хода и вставляем в очередь
  if(co->tag!=T_PERS)
  {
   me=get_evt(); evt_func_set[E_bobj](me,co,NULL,NULL,3.0*rand()/RAND_MAX); ins_evt(co,me);
  }
  co=co->next;
 }
 cmd_send_bl_begin(o); // оповестим участников о начале боя
}
