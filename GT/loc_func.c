#include "gt.h"
#include "evt_rmi.h"


void loc_col_PERS(obj *o,evt *de,double tmin)
{
 evt *me;
 lnk *nl,*ml;
 obj *co;

 if(tmin<0.0) return;
 co=o->nif; if(co==NULL) return; // объект находится в неуправляемом режиме
 // время ближайшего столкновения найдено
 // создаю два события: столкновения с локацией и его антисобытие
 me=get_evt(); evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me);
 evt_cold_loc_set(de,o,NULL,NULL,0.0);
 me->e=de; de->e=me; nl=rmi_lnk_mv(co,de); de->l=nl;
 ml=evt_lnk_ins(lnks_tags[E_mvf_user],o,NULL,NULL); nl->l=ml; ml->l=nl;
}


void loc_col_AS(obj *o,evt *de,double tmin)
{
 evt *me;
 lnk *nl,*ml;

 if(de->e!=NULL) { de->e->e=NULL; del_evt(o,de->e); }

// вставляем новое столкновение в очередь и привязываем его к реакции
// на изменение движения
 if(tmin>=0.0)
  { 
   me=de->e; if(me==NULL) me=get_evt();
   evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me); me->e=de;
  } 
 else { if(de->e!=NULL) free_evt(o,de->e); me=NULL; }
 de->e=me;
}

void loc_col_MUSOR(obj *o,evt *e,double tmin)
{
 evt *me;
 lnk *nl,*ml;

 if(e!=NULL) { del_evt(o,e); me=e; } else me=get_evt();
 if(tmin>=0.0)
  { 
   evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me);
  } 
 else { if(e!=NULL) free_evt(o,e); }
}

// процедура тормознутая, но может потом прооптимизирую
void loc_crossec(obj *o,int x1,int x2,int y1,int y2)
{
 int i,j,k,m,f,locn;
 lnk *l,*dl,*nl,*l1,*l2;
 obj *wo=o->pl;

 // найдем родительский объект - WORLD
// wo=o->pl; if(wo->tag!=T_WORLD) return; // будем считать, что если объект не в космосе, то он не подлежит регистрации
 while(wo->tag!=T_WORLD) wo=wo->pl;

 l=o->locl; 
 while(l!=NULL) 
 { 
  locn=l->ln; j=locn/wo->s.xs; i=locn-j*wo->s.xs; // :)
  dl=l; l=l->next;
  if(i>x1 || i<x2 || j>y1 || j<y2) // эту локу объект покинул
  {
   // удалим объект из локации
   nl=dl->l; 
   if(nl->prev==NULL) wo->locs[locn].ol=nl->next; else nl->prev->next=nl->next;
   if(nl->next!=NULL) nl->next->prev=nl->prev;
   if(dl->prev==NULL) o->locl=dl->next; else dl->prev->next=dl->next;
   if(dl->next!=NULL) dl->next->prev=dl->prev;
   free_lnk(o,nl); free_lnk(o,dl);

   // найдем локации, которые стали не видны
   for(k=-1;k<=1;k++) for(m=-1;m<=1;m++)
   {
    if(m+i>x1+1 || m+i<x2-1 || k+j>y1+1 || k+j<y2-1) // лока теперь не видна
    {
     // известим об этом клиент
     locn=(m+i)+(k+j)*wo->s.xs; nl=wo->locs[locn].ol;
     while(nl!=NULL) 
     {
      // проверим не попадает ли объект nl->o частично в область видимости
      dl=nl->o->locl; f=0; 
      while(dl!=NULL) 
       { 
        locn=dl->ln; j=locn/wo->s.xs; i=locn-j*wo->s.xs; dl=dl->next;
        if(i<x1+1 && i>x2-1 && j<y1+1 && j>y2-1) { f=1; break; }
       }
      if(f==0) cmd_send_dobj(o,nl->o); // команда на удаление объекта может быть послана 3 раза, но это не страшно
      nl=nl->next; 
     }
    }
   }
  }
 }

 for(j=y2;j<=y1;j++)
 for(i=x2;i<=x1;i++)
  {
   locn=i+j*wo->s.xs; k=0;
   l=o->locl; while(l!=NULL) { if(l->ln==locn) { k=1; break; } l=l->next; }
   if(k==1) continue; // эта лока уже учтена

   // получаю ссылки для привязки к локации и объекту
   l1=get_lnk(); l2=get_lnk();
   l1->o=o; l2->o=wo;
   l1->l=l2; l2->l=l1;

   // вставляю ссылку на объект в локацию
   l2->e=(evt *)locn;
   l1->prev=NULL; l1->next=wo->locs[locn].ol;
   if(wo->locs[locn].ol!=NULL) wo->locs[locn].ol->prev=l1; 
   wo->locs[locn].ol=l1;

   // вставляю ссылку на локацию в объект
   l2->prev=NULL; l2->next=o->locl; 
   if(o->locl!=NULL) o->locl->prev=l2; 
   o->locl=l2;

   if(j==y2) // оповещаем i,j-1
   {
    locn=i+(j-1)*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
   }
   if(j==y1) // оповещаем i,j+1
   {
    locn=i+(j+1)*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
   }
   if(i==x2) // оповещаем i-1,j
   {
    locn=i-1+j*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
    if(j==y2) // оповещаем i-1,j-1
    {
     locn=i-1+(j-1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
    if(j==y1) // оповещаем i-1,j+1
    {
     locn=i-1+(j+1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
   }
   if(i==x1) // оповещаем i+1,j
   {
    locn=i+1+j*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
    if(j==y2) // оповещаем i+1,j-1
    {
     locn=i+1+(j-1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
    if(j==y1) // оповещаем i+1,j+1
    {
     locn=i+1+(j+1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
   }
  }
}

// инициализация события столкновения с границей локации
void evt_col_loc_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_col_loc;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_cold_loc_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_cold_loc;
 ne->t=0.0;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

// пересекли границу локации, найдем множество локаций, которые покинули
// и множество локаций в которые вляпались
void evt_col_loc_act(obj *o,evt *e,lnk *l)
{
 double x1,x2,y1,y2;
 double tmin,tx,ty;
 float vx,vy;
 double cx,cy;
 int lx1,lx2,ly1,ly2;
 evt *me,*de;
 lnk *nl,*ml;
 obj *wo;

 // найдем родительский объект - WORLD
// wo=o->pl; 
// if(wo->tag!=T_WORLD)
// { 
//  del_evt(o,e); free_evt(o,e); return; // будем считать, что если объект не в космосе, то он не подлежит регистрации
// }
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 vx=fabs(o->s.tx); vy=fabs(o->s.ty);

 cx=o->s.cx+o->s.tx*o->s.v*(GTIME-o->s.ct);
 cy=o->s.cy+o->s.ty*o->s.v*(GTIME-o->s.ct);
 
 x1=(cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 x2=(cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y1=(cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 y2=(cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 lx1=x1;  lx2=x2;  ly1=y1;  ly2=y2;
 x1-=lx1; x2-=lx2; y1-=ly1; y2-=ly2;

 loc_crossec(o,lx1,lx2,ly1,ly2);

 // найдем время следующего пересечения

 if(o->s.tx>0.0) { x1=1.0-x1; x2=1.0-x2; }
 if(o->s.ty>0.0) { y1=1.0-y1; y2=1.0-y2; }

 if(x1<x2) tx=x1; else tx=x2;
 if(y1<y2) ty=y1; else ty=y2;
 
 if(tx*vy<ty*vx) tmin=(tx*1000.0)/(vx*o->s.v);
 else tmin=(ty*1000.0)/(vy*o->s.v);
 tmin+=0.001; // для небольшого проникновения
/*
 if(o->pl->tag==T_BL && o->bside!=0)
  { del_evt(o,e); evt_col_loc_set(e,o,NULL,NULL,tmin); ins_evt(o,e); return; }
 
 if(o->tag==T_PERS) 
  { del_evt(o,e); free_evt(o,e); loc_col_PERS(o,get_evt(),tmin); return; }
*/

 if(o->tag==T_PERS) // общее движение через bmv 
  { del_evt(o,e); evt_col_loc_set(e,o,NULL,NULL,tmin); ins_evt(o,e); return; }
 if(o->tag==T_AS || o->tag==T_UL || o->tag==T_SP) 
  { 
   loc_col_AS(o,e->e,tmin); return; 
  }
 if(o->tag==T_MUSOR) 
  { 
   loc_col_MUSOR(o,e,tmin); return; 
  }
 
}

// объект остановился, нужно уничтожить событие E_col_loc
// событие вызывается из списка реакций сетевого интерфейса T_NIF для управляемых объектов
// и из списка реакции самого объекта для орбитальных объектов типа T_AS
void evt_cold_loc_act(obj *o,evt *e,lnk *l)
{
 evt *de,*me;
 double tmin;

// if(o->pl->tag==T_BL && o->bside!=0)
  if(o->tag==T_PERS)
  { 
   tmin=get_loc_col_time(o); de=l->e;
   if(tmin<0.0) // столкновения не ожидается
   {
    if(de->e!=NULL) // удаляю событие столкновения, однако реакцию сохраняю на будущее
     { del_evt(o,de->e); free_evt(o,de->e); de->e=NULL; }
    return;
   }
   if(de->e!=NULL) // смещаю событие на tmin
    { de->e->t=GTIME+tmin; move_evt(o,de->e); }
   else // создаю новое событие столкновения и вставляю в очередь
    { me=get_evt(); evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me); de->e=me; }
   return;
  }
/* использовалось при движении через команду mv
 if(o->tag==T_NIF)
 {
  de=l->e->e;
  del_evt(o->pl,de); free_evt(o->pl,de);
  return;
 }
*/
 if(o->tag==T_AS || o->tag==T_MUSOR || o->tag==T_UL || o->tag==T_SP)
 {
// уничтожаю событие столкновения, если оно существует
  loc_col(o,l->e);
  locs_send_mv(o);
 }
}

double get_loc_col_time(obj *o)
{
 double x1,x2,y1,y2;
 double tmin,tx,ty;
 float vx,vy;
 obj *wo;

 // найдем родительский объект - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 if(o->s.tx==0.0 && o->s.ty==0.0) return -1.0; // объект неподвижен
 vx=fabs(o->s.tx); vy=fabs(o->s.ty);
 
 x1=(o->s.cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x2=(o->s.cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y1=(o->s.cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y2=(o->s.cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1=fmod(x1,1.0); x2=fmod(x2,1.0); y1=fmod(y1,1.0); y2=fmod(y2,1.0);

 if(o->s.tx>0.0) { x1=1.0-x1; x2=1.0-x2; }
 if(o->s.ty>0.0) { y1=1.0-y1; y2=1.0-y2; }

 if(x1<x2) tx=x1; else tx=x2;
 if(y1<y2) ty=y1; else ty=y2;
 
 if(tx*vy<ty*vx) tmin=(tx*1000.0)/(vx*o->s.v);
 else tmin=(ty*1000.0)/(vy*o->s.v);
 tmin+=0.001; // для небольшого проникновения
 return tmin;
}

void loc_col(obj *o,evt *de)
{
 double tmin=get_loc_col_time(o);
 if(o->tag==T_PERS) 
 { loc_col_PERS(o,get_evt(),tmin); return; }
 if(o->tag==T_AS || o->tag==T_UL || o->tag==T_SP)
 { loc_col_AS(o,de,tmin); return; }
 if(o->tag==T_MUSOR) 
  { 
   loc_col_MUSOR(o,de,tmin); return; 
  }
}

// функция устанавливает событие пересечения границы локаций объектом 
// в боевом режиме, антисобытие должно быть реакцией события bm
/*
void loc_col_bmv(obj *o, double tmin)
{
 evt *me,*de;
 lnk *nl,*ml;
 obj *co;

 if(tmin==0.0) tmin=get_loc_col_time(o);
 if(tmin<0.0) return;

 // время ближайшего столкновения найдено
 // создаю два события: столкновения с локацией и его антисобытие
 me=get_evt(); evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me);
 de=get_evt(); evt_cold_loc_set(de,o,NULL,NULL,0.0); me->e=de; de->e=me; 
 ml=evt_lnk_ins(lnks_tags[E_bmv],o,NULL,NULL); de->l=ml; ml->l=ml; ml->e=de;
}
*/

// функция вызывается один раз в начале игры.
void loc_col_bmv(obj *o)
{
 evt *de;
 lnk *ml;

 de=get_evt(); evt_cold_loc_set(de,o,NULL,NULL,0.0); 
 ml=evt_lnk_ins(lnks_tags[E_bmv],o,NULL,NULL); de->l=ml; ml->l=ml; ml->e=de;
}


/*
void loc_col_P(obj *o)
{
 double x1,x2,y1,y2;
 double tmin,tx,ty;
 float vx,vy;
 int i;

 evt *me,*de;
 obj *co=o->nif;
 lnk *nl,*ml;
 obj *wo;

 // найдем родительский объект - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 if(co==NULL) return; // объект находится в неуправляемом режиме

 if(o->s.tx==0.0 && o->s.ty==0.0) return; // объект неподвижен
 vx=fabs(o->s.tx); vy=fabs(o->s.ty);
 
 x1=(o->s.cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x2=(o->s.cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y1=(o->s.cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y2=(o->s.cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1=fmod(x1,1.0); x2=fmod(x2,1.0); y1=fmod(y1,1.0); y2=fmod(y2,1.0);

 if(o->s.tx>0.0) { x1=1.0-x1; x2=1.0-x2; }
 if(o->s.ty>0.0) { y1=1.0-y1; y2=1.0-y2; }

 if(x1<x2) tx=x1; else tx=x2;
 if(y1<y2) ty=y1; else ty=y2;
 
 if(tx*vy<ty*vx) tmin=(tx*1000.0)/(vx*o->s.v);
 else tmin=(ty*1000.0)/(vy*o->s.v);
 tmin+=0.001; // для небольшого проникновения

 // время ближайшего столкновения найдено
 // создаю два события: столкновения с локацией и его антисобытие
 me=get_evt(); evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me);
 de=get_evt(); evt_cold_loc_set(de,o,NULL,NULL,0.0);
 me->e=de; de->e=me; nl=rmi_lnk_mv(co,de); de->l=nl;
 ml=evt_lnk_ins(lnks_tags[E_mvf_user],o,NULL,NULL); nl->l=ml; ml->l=nl;
}
*/

void loc_obj_reg(obj *o,int send_flag)
{
 lnk *l1,*l2;
 int lx1,ly1,lx2,ly2,i,j,locn;
 obj *wo;

 // найдем родительский объект - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 // нахожу координаты локации
 lx1=(o->s.cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 ly1=(o->s.cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 lx2=(o->s.cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 ly2=(o->s.cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 for(j=ly1;j<=ly2;j++)
 for(i=lx1;i<=lx2;i++)
 {
  // получаю ссылки для привязки к локации и объекту
  l1=get_lnk(); l2=get_lnk();
  l1->o=o; l2->o=wo;
  l1->l=l2; l2->l=l1;
  // вставляю ссылку на объект в локацию
  locn=i+j*wo->s.xs; l2->e=(evt *)locn; // :)
  l1->prev=NULL; l1->next=wo->locs[locn].ol;
  if(wo->locs[locn].ol!=NULL) wo->locs[locn].ol->prev=l1; 
  wo->locs[locn].ol=l1;
  // вставляю ссылку на локацию в объект
  l2->prev=NULL; l2->next=o->locl; 
  if(o->locl!=NULL) o->locl->prev=l2; 
  o->locl=l2;
 }
 if(send_flag!=0) // оповещаю локации о появлении объекта
 {
  locs_send_obj(o);
 }
}

void loc_obj_unreg(obj *o)
{
 lnk *l,*dl,*nl;
 obj *wo;
 int locn;

 locs_send_dobj(o);

 // найдем родительский объект - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 dl=o->locl; 
 while(dl!=NULL) 
 { 
  locn=dl->ln; l=dl->next;
  nl=dl->l; 
  if(nl->prev==NULL) wo->locs[locn].ol=nl->next; else nl->prev->next=nl->next;
  if(nl->next!=NULL) nl->next->prev=nl->prev;
  if(dl->prev==NULL) o->locl=dl->next; else dl->prev->next=dl->next;
  if(dl->next!=NULL) dl->next->prev=dl->prev;
  free_lnk(o,nl); free_lnk(o,dl);
  dl=l;
 }
}

void locs_send_obj(obj *po)
{
 obj *wo;
 lnk *ol;
 int x1,x2,y1,y2;
 int i,j,locn;

 // найдем родительский объект - WORLD

 if(po->locl==NULL) return; // объект не зарегистрирован в локациях
 wo=po->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 x2=(po->s.cx+po->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(po->s.cx-po->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(po->s.cy+po->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(po->s.cy-po->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; cmd_send_obj_loc(po,ol); }
}

// оповещает локации об изменениях в моих параметрах
void locs_send_myobj(obj *po)
{
 obj *wo;
 lnk *ol;
 int x1,x2,y1,y2;
 int i,j,locn;

 // найдем родительский объект - WORLD
 wo=po->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 x2=(po->s.cx+po->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(po->s.cx-po->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(po->s.cy+po->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(po->s.cy-po->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; cmd_send_myobj_loc(po,ol); }
}


void locs_send_dobj(obj *po)
{
 obj *wo;
 lnk *ol;
 int x1,x2,y1,y2;
 int i,j,locn;

 // найдем родительский объект - WORLD
 if(po->locl==NULL) return;
 wo=po->locl->o;

 printf("I am\n");


 x2=(po->s.cx+po->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(po->s.cx-po->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(po->s.cy+po->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(po->s.cy-po->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; cmd_send_dobj_loc(po,ol); }
}

void locs_send_mv(obj *po)
{
 obj *wo;
 lnk *ol;
 int i,j,locn;
 int x1,x2,y1,y2;

 if(po->tag==T_PERS && po->nif!=NULL) cmd_send_mymv(po);

 // найдем родительский объект - WORLD
 if(po->locl==NULL) return;
 wo=po->locl->o;

 // передаю параметры движения всем корабликам вокруг

 x2=(po->s.cx+po->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(po->s.cx-po->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(po->s.cy+po->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(po->s.cy-po->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; cmd_send_mv_loc(po,ol); }
}
