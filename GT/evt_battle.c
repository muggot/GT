#include "gt.h"


void evt_bl_dummy_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bl_dummy;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_bl_eturn_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bl_eturn;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_bl_eturnd_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bl_eturnd;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_bt_obj_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_bt_obj;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}


// спарк в бою, нужно составить план хода и передать его в BL->bo_turns
// в принципе можно сразу вставлять строки в BL->bo_turns
void spark_bt_act(obj *o,evt *e,lnk *l)
{
 obj *bo=o->pl;
 obj *co,*eo=NULL;
 obj *wo1=NULL,*wo2=NULL;
 double minr=0,dr,dx,dy,r;
 double wt1,wt2; // текущее время подготовки оружия к выстрелу
 double dt,ct;
 int n; char *b;

// проверим простейшую наступательную логику, 
// подойти на расстояние эффективной дальности
// и бить плазмой, на подходе и по мере готовности быть болванками
// при подготовке к стрельбе маневрировать.

// на первом ходе будем считать, что все оружие спарка готово к использованию

// определяю расстояние до ближайшего противника
// поскольку среди противников могут быть объекты с орбитальными параметрами
// движения то вычисляю текущие координаты для всех 
  
 co=bo->bo;
 while(co!=NULL)
 {
  if(co->bside!=0 && co->bside!=o->bside) // противник найден
  {
   dr=(GTIME-co->s.ct)*co->s.v; 
   dx=co->s.cx+co->s.tx*dr-o->s.cx; 
   dy=co->s.cy+co->s.ty*dr-o->s.cy;
   dr=1.0/dx*dx+dy*dy;
   if(dr>minr) { minr=dr; eo=co; }
  }
  co=co->next;
 }

 if(eo==NULL) return; // врагов нету

 dr=(GTIME-eo->s.ct)*eo->s.v; minr=sqrt(minr); 
 dx=eo->s.cx+eo->s.tx*dr-o->s.cx; 
 dy=eo->s.cy+eo->s.ty*dr-o->s.cy; 
 r=1.0/minr; dx*=minr; dy*=minr;

// найдем параметры оружия
 co=o->bo;
 while(co!=NULL)
 {
  if(co->tag==T_WPN) if(co->s.img==1) wo1=co; else wo2=co;
  co=co->next;
 }

 if(wo1->rt==0.0) wt1=-wo1->dev.pwr[2]; else wt1=bo->rt-wo1->rt;
 if(wo2->rt==0.0) wt2=-wo1->dev.pwr[2]; else wt2=bo->rt-wo1->rt;

 ct=wo1->dev.pwr[2]-wt1; //  время первого выстрела
 while(ct<BTURN_TIME) 
 {
// формирую команду выстрела по цели
  n=snprintf(NULL,0,"<BUSE t=\"%f\" slot=\"%c%d,%d\" obj=\"%x\" />",ct,wo1->dev.mysl->st[0],wo1->dev.mysl->sc,wo1->dev.mysl->sn,eo)+1; b=mymalloc(n); 
  snprintf(b,n,"<BUSE t=\"%f\" slot=\"%c%d,%d\" obj=\"%x\" />",ct,wo1->dev.mysl->st[0],wo1->dev.mysl->sc,wo1->dev.mysl->sn,eo);
  l=get_lnk(); l->next=bo->bo_turns; l->prev=NULL; bo->bo_turns=l; l->o=o; l->b=b;
  ct+=wo1->dev.pwr[2];
 }

 ct=wo2->dev.pwr[2]-wt2; dt=0;
 if(r>wo2->dev.pwr[0]) // подсчитаем время подлета
 {
  dt=(r-wo2->dev.pwr[0])/eo->s.v;
// формирую команду подлета к цели
  if(dt>BTURN_TIME) dt=BTURN_TIME;
  n=snprintf(NULL,0,"<BM t=\"%f\" ta=\"%f\" tx=\"%f\" ty=\"%f\" />",0.0,dt,dx,dy)+1; b=mymalloc(n); 
  snprintf(b,n,"<BM t=\"%f\" ta=\"%f\" tx=\"%f\" ty=\"%f\" />",0.0,dt,dx,dy);
  l=get_lnk(); l->next=bo->bo_turns; l->prev=NULL; bo->bo_turns=l; l->o=o; l->b=b;
 }

 if(dt>ct) ct=dt; // сначала подлетим, а потом стреляем
 while(ct<BTURN_TIME) 
 {
// формирую команду выстрела по цели
  n=snprintf(NULL,0,"<BUSE t=\"%f\" slot=\"%c%d,%d\" obj=\"%x\" />",ct,wo1->dev.mysl->st[0],wo1->dev.mysl->sc,wo1->dev.mysl->sn,eo)+1; b=mymalloc(n); 
  snprintf(b,n,"<BUSE t=\"%f\" slot=\"%c%d,%d\" obj=\"%x\" />",ct,wo1->dev.mysl->st[0],wo1->dev.mysl->sc,wo1->dev.mysl->sn,eo);
  l=get_lnk(); l->next=bo->bo_turns; l->prev=NULL; bo->bo_turns=l; l->o=o; l->b=b;
  ct+=wo1->dev.pwr[2];
 }
}

void (*bt_obj_proc[OBJ_TAG_NUM])(obj *o,evt *e,lnk *l)=
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
 spark_bt_act, //T_SP
 NULL, //T_MUSOR
 NULL, //T_RES
 NULL //T_ARS
};



// событие планирования хода для встроенных автоматических ботов
void evt_bt_obj_act(obj *o,evt *e,lnk *l)
{
 evt *me;
// в зависимости от тэга объекта должна вызываться своя функция 
// функции сами вставляют план хода в BL->bo_turns
 if(bt_obj_proc[o->tag]!=NULL) bt_obj_proc[o->tag](o,e,l);
 me=get_evt(); evt_func_set[E_bl_dummy](me,o->pl,NULL,NULL,0.0); ins_evt(o,me); // ход пустого субъекта (защита от мгновенных ходов)
}


// завершение хода, создается после получения сервером сообщения от
// субъекта о завершении планирования хода
// уменьшаю счетчик субъектов сделавших ход
void evt_bl_dummy_act(obj *o,evt *e,lnk *l)
{
 del_evt(o,e); free_evt(o,e);
 o->nact--;
}

// событие завершения хода по окончании счетчика (реакция на E_bl_dummy)
void evt_bl_eturnd_act(obj *o,evt *e,lnk *l)
{
 if(o->nact>0) return; // еще не все сделали ход
 // сдвигаю событие eturn на GTIME
 e->e->t=GTIME; move_evt(o,e->e);
}

// событие завершения хода, время вышло или все сделали свой ход
// нужно проанализировать ходы всех субъектов, подвести итоги и
// разослать результат удаленным клиентам
char *bms="<BM t=\"BTURN_TIME\" tx=\"0\" ty=\"0\" />";
char *buse="<BUSE t=\"0.0\" />";

void evt_bl_eturn_act(obj *o,evt *e,lnk *l)
{
 obj *co;
 char *b;
 int n;
 lnk *sl,*nl;
 evt *me,*de;

 del_evt(o,e); free_evt(o,e);

// в o->bo_turns хранятся команды бойцов, их нужно
// передать на обработку для формирования событий
// но сначала для каждого бойца нужно вставить команду завершения
// движения в конце хода, а также запустить восстановительные работы

 co=o->bo;
 while(co!=NULL)
 {
  if(co->bside!=0) 
  {
   n=strlen(bms)+1; b=mymalloc(n); memcpy(b,bms,n);
   l=get_lnk(); l->next=o->bo_turns; l->prev=NULL; o->bo_turns=l; l->o=co; l->b=b;
// нужно пройтись по слотам и запустить необходимые устройства, например, ars и восстановление щитов
   sl=o->dev.dvsl;
   while(sl!=NULL)
   {
    if(sl->st[0]=='S' || (sl->st[0]=='U' && sl->o->tag==T_ARS))
    {
     n=strlen(buse)+1; b=mymalloc(n); memcpy(b,buse,n);
     l=get_lnk(); l->next=o->bo_turns; l->prev=NULL; o->bo_turns=l; l->o=co; l->b=b;
    }
    sl=sl->next;
   }
  }
  co=co->next;
 }

 bl_cmds_proc(o);
 o->s.ct=GTIME;

 co=o->bo;
 while(co!=NULL)
 {
  if(co->bitf&(GOF_ISNIF)!=0) 
  {
   //send
  }
  co=co->next;
 }

// начинаю новый ход
 me=get_evt(); evt_func_set[E_bl_dummy](me,o,NULL,NULL,3.0); ins_evt(o,me); // ход пустого субъекта (защита от мгновенных ходов)
 me=get_evt(); evt_func_set[E_bl_eturn](me,o,NULL,NULL,40.0); ins_evt(o,me); // переход хода по таймауту
 de=get_evt(); evt_func_set[E_bl_eturnd](de,o,NULL,NULL,0.0);
 me->e=de; de->e=me; nl=evt_lnk_ins(lnks_tags[E_bl_dummy],o,de,NULL); de->l=nl; nl->l=nl;
 co=o->bo; o->nact=1;
 while(co!=NULL) 
 { 
  o->nact++; 
//  if(co->bitf&GOF_ISNIF!=0 && co->s.me!=NULL) // объект управляемый и движется, остановить надобно
//   evt_func_act[E_mvf_user](co,co->s.me,NULL);

  co->turn++;

// для встроенных ботов создаем событие планирования хода и вставляем в очередь
  if(co->bitf&GOF_ISNIF==0)
  {
   me=get_evt(); evt_func_set[E_bt_obj](me,co,NULL,NULL,3.0*rand()/RAND_MAX); ins_evt(co,me);
  }
  
  co=co->next;
 }

}

