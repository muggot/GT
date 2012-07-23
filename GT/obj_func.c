#include <stdlib.h>
#include "gt.h"
#include "evt_oload.h"
#include "evt_oinit.h"

#define		M_PI	3.1415926535897932384626433832795

static memlst **ombase=NULL;
static int omn=0,omi=0;

// глобальный контейнер шаблонов объектов для автогенерации
static obj *GHAB;

void set_ghab(obj *o)
{
 GHAB=o;
}

void add_memlst_ombase(memlst *ml)
{
 int l=0, i, m, r=omi-1;
 if(ombase==NULL) { ombase=calloc(1,sizeof(memlst *)); omn=1; }
 if(omn==omi) { omn<<=1; ombase=realloc(ombase,sizeof(memlst *)*omn); }
 if(r<0) { ombase[0]=ml; omi++; return; }
 if(ml<ombase[l]) { for(i=omi-1;i>=0;i--) ombase[i+1]=ombase[i]; ombase[0]=ml; omi++; return; }
 if(ml>ombase[r]) { ombase[omi]=ml; omi++; return; }

 while(r-l>1) { m=(l+r)>>1; if(ml>ombase[m]) l=m; else r=m; }
 // вставляю в r
 for(i=omi-1;i>=r;i--) ombase[i+1]=ombase[i]; ombase[r]=ml; omi++;
}

// проверяю существует ли объект по указанному адресу
// -1 если нет, 0 - если да
int find_obj_ombase(obj *o)
{
 int l=0, m, r=omi-1;
 if(o<(obj *)ombase[0] || o>=(obj *)(ombase[omi-1]+1)) return -1;
 if((void *)o>(void *)ombase[r]) l=r;
 else 
  while(r-l>1) { m=(l+r)>>1; if((void *)o>(void *)ombase[m]) l=m; else r=m; }
 // l - предположительно искомый блок, проверим, что o точно объект
 m=(char *)o-(char *)&ombase[l]->m;
 if(m%sizeof(obj)!=0 || m/sizeof(obj)>OBJ_IN_MEMLST) return -1;
 return 0;
}

// выделяет объект и проводит его начальную инициализацию
obj *get_obj(obj *po, int tag, char *gn)
{
 obj	*gs,*no;
 memlst **ptml,*tml;
 int	i;

 if(GP.fo==NULL) /*контейнер не создан или закончились свободные структуры*/
  {
   ptml=&(GP.ml); while(*ptml!=NULL) ptml=&((*ptml)->next);
   tml=*ptml=(memlst *)calloc(1,sizeof(memlst)); 
    if(tml==NULL) { printf("Память кончилась\n"); return NULL; }
   GP.fo=tml->m.omem;
   for(i=0;i<OBJ_IN_MEMLST-1;i++) tml->m.omem[i].next=&(tml->m.omem[i+1]);
   // добавим новый блок в массив для быстрого поиска объектов по адресу
   add_memlst_ombase(tml);
  }
 gs=GP.fo; GP.fo=gs->next; memset(gs,0,sizeof(obj));
 gs->prev=NULL; no=gs->next=po->bo; po->bo=gs;
 if(no!=NULL) no->prev=po->bo;
 gs->tag=tag; strcpy(gs->dn,gn); gs->pl=po;
 gs->cnt=0; gs->bitf=GOF_EXIST;
// printf("Создан объект %s\n",gs->dn);
 return gs;
}

// удаляет объект из списка используемых, но не освобождает структуру
void del_obj(obj *o)
{
 if(o->prev!=NULL) o->prev->next=o->next;
 else o->pl->bo=o->next;
 if(o->next!=NULL) o->next->prev=o->prev;
}

// вставляет объект o в начало списка дочерних объектов объекта po
void ins_obj(obj *po, obj *o)
{
 o->prev=NULL; o->next=po->bo;
 if(po->bo!=NULL) po->bo->prev=o;
 po->bo=o;
 o->pl=po;
}

// освобождает структуру объекта
void free_obj(obj *o)
{
 o->prev=NULL; o->next=GP.fo; GP.fo=o;
}

// возвращает указатель на временный объект 
// и проводит его начальную инициализацию.
// Временный объект не извлекается из очереди 
// свободных объектов и не требует освобождения.
obj *temp_obj(obj *po, int tag, char *gn)
{
 obj	*gs,*no;
 memlst **ptml,*tml;
 int	i;

 if(GP.fo==NULL) /*контейнер не создан или закончились свободные структуры*/
  {
   ptml=&(GP.ml); while(*ptml!=NULL) ptml=&((*ptml)->next);
   tml=*ptml=(memlst *)calloc(1,sizeof(memlst)); 
    if(tml==NULL) { printf("Память кончилась\n"); return NULL; }
   GP.fo=tml->m.omem;
   for(i=0;i<OBJ_IN_MEMLST-1;i++) tml->m.omem[i].next=&(tml->m.omem[i+1]);
  }
 gs=GP.fo;
 gs->tag=tag; strcpy(gs->dn,gn); gs->pl=po;
 gs->cnt=0; gs->bitf=GOF_EXIST;
 return gs;
}

void temp2real_obj(obj *o)
{
 GP.fo=o->next; 
}

void obj_ch_parent(obj *o,obj *po)
{
 obj *co;

 if(o==NULL || po==NULL) return;
 del_par_evt(o); // отцепляем очередь событий от родительского объекта

 // уменьшаю массу родительских объектов
 co=o->pl; while(co!=NULL && co->tag!=T_WORLD) { co->dev.cm-=o->dev.cm; co=co->pl; }

 if(o->prev==NULL) o->pl->bo=o->next; else o->prev->next=o->next;
 if(o->next!=NULL) o->next->prev=o->prev;

 o->prev=NULL; o->next=po->bo; po->bo=o; if(o->next!=NULL) o->next->prev=o;
 o->pl=po;

 ins_par_evt(o,o->bevt); // прицепляем очередь событий к новому род. объекту

 // увеличиваю массу родительских объектов
 co=o->pl; while(co!=NULL && co->tag!=T_WORLD) { co->dev.cm+=o->dev.cm; co=co->pl; }
}

int is_obj_onmap(obj *o)
{
 if(o==NULL) return -1;
 if(o->tag==T_BL) return -1;
 if(o->bitf&(GOF_EXIST|GOF_ONMAP)==0) return -1;
 return 0;
}

float get_obj_dist(obj *o,obj *eo)
{
 double xd,yd,dr;

 if(o->s.me!=NULL && o->s.me->tag==E_mvs_user ) { xd=o->s.cx; yd=o->s.cy; }
 else
 {
  dr=(GTIME-o->s.ct)*o->s.v; xd=o->s.cx+o->s.tx*dr; yd=o->s.cy+o->s.ty*dr;
 }

 if(eo->s.me!=NULL && eo->s.me->tag==E_mvs_user ) dr=0.0;
 else dr=(GTIME-eo->s.ct)*eo->s.v; 

 xd=fabs(xd-eo->s.cx-eo->s.tx*dr)*0.001; yd=fabs(yd-eo->s.cy-eo->s.ty*dr)*0.001;
 if(xd<yd) return yd; else return xd;
}

void obj_new_name(obj *o)
{
 sprintf(o->dn,"@%.16Lx",OGEN); OGEN++;
}

// создает объект Т_MUSOR и инициализирует его, 
// po - родительский объект для создаваемого, 
// so - объект от которого берутся параметры движения нового объекта
obj *obj_create_MUSOR(obj *po,obj *so)
{
 obj *o,*shp_o;
 float d,fi;
 double dr;
 evt *me;

 o=get_obj(po,T_MUSOR,""); if(o==NULL) { printf("Не смог создать объект T_MUSOR"); exit(0); }
 obj_new_name(o);
// задаю только не нулевые значения, обнуление производится на этапе выделения объекта 
 prop_hull_load(o,"100.0,100.0");
// параметры движения наследуются от so, с небольшими вариациями.
 o->s.img=1; o->s.xs=20; o->s.ys=20;
 strcpy(o->name,"╠єёюЁ");
/*
 if(so->tag==T_AS) // орбитальный объект, возьмем его орбиту
 {
  memcpy(&o->orb,&so->orb,sizeof(orbit));
// проварьирую ang,p,e,v
// сначала скорректирую t
  if(o->orb.e<1.0) { d=floor((o->orb.t-GTIME)*o->orb.v); o->orb.t+=d/o->orb.v; }
  d=0.0001*rand()/RAND_MAX-0.00005; o->orb.ang+=d;
  d=0.99995+0.0001*rand()/RAND_MAX; o->orb.p*=d;
  d=0.99995+0.0001*rand()/RAND_MAX; o->orb.e*=d;
  d=0.9995+0.001*rand()/RAND_MAX; o->orb.v*=d;
  me=get_evt(); evt_func_set[E_orb](me,o,NULL,NULL,1.0); ins_evt(o,me);
 }
*/
// else // просто задаем прямолинейное движение со случайным направлением и скоростью
 {
  fi=(2.0*M_PI*rand())/RAND_MAX; o->s.tx=cos(fi); o->s.ty=sin(fi);
  o->s.v=0.1+(0.1*rand())/RAND_MAX;
  dr=(GTIME-so->s.ct)*so->s.v; o->s.ct=GTIME;
  o->s.cx=so->s.cx+so->s.tx*dr+(100.0*rand())/RAND_MAX*o->s.tx; 
  o->s.cy=so->s.cy+so->s.ty*dr+(100.0*rand())/RAND_MAX*o->s.ty;
  loc_obj_reg(o,1); // регистрирую объект в локациях и извещаю клиентов
  loc_col(o,NULL); // вставляю событие столкновения с границами локаций
 }
 return o;
}

obj *obj_create_RES(obj *po,int img,float mass)
{
 obj *o;
 o=get_obj(po,T_RES,""); if(o==NULL) { printf("Не смог создать объект T_RES"); exit(0); }
 obj_new_name(o); o->dev.cm=mass; o->s.img=img;
 return o;
}

// рекурсивно копирует структуры объектов
obj *obj_rec_clone(obj *po, obj *co)
{
 obj *o,*bo,*prev,*next;
 o=get_obj(po,0,""); prev=o->prev; next=o->next;
 memcpy(o,co,sizeof(obj)); obj_new_name(o); 
 o->pl=po; o->bo=NULL; o->prev=prev; o->next=next;
 bo=co->bo; while(bo!=NULL) { obj_rec_clone(o,bo); bo=bo->next; }
 return o;
}

obj *obj_create_SP(obj *po, int img)
{
 obj *o,*co,*bo;
 float d,fi;
 double dr;
 evt *me;

 co=po->bo; while(co!=NULL && (co->tag!=T_SP || co->s.img!=img)) co=co->next;
 if(co==NULL) { printf("Шаблон для T_SP не найден"); exit(0); }

 o=obj_rec_clone(po->pl,co); o->s.img=img;
 o->mem=mymalloc(sizeof(mempar)); memset(o->mem,0,sizeof(mempar));
 o->mem->tb=GTIME; o->mem->te=GTIME+15000.0;

 me=get_evt(); evt_func_set[E_init_obj](me,o,NULL,NULL,1.0); ins_evt(o,me);

 return o;
}

static char fpath[256];
static obj fo={0};

/* понадобится после завершения разработки
obj *obj_create_PERS(char *name, char *pass)
{
 obj *po,*o,*co,*bo;
 evt *me;

 co=GHAB->bo; while(co!=NULL && co->tag!=T_PERS) co=co->next;
 if(co==NULL) { printf("Шаблон для T_PERS не найден"); exit(0); }

 evt_fload_newobj(&fo,fpath); co=fo.bo;

 po=find_user((char *)&co->port); // находим порт приписки
 o=obj_rec_clone(po,co); add_user(name,o);
 sprintf(o->dn,"%s",name); sprintf(o->password,"%s",pass); 
 
 evt_init_newobj(o);

 return o;
}
*/

obj *obj_create_PERS(char *name, char *pass)
{
 obj *po,*o,*co,*bo;
 evt *me;

 sprintf(fpath,"%s%s",GHAB->fpath,oname[T_PERS]);
 evt_fload_newobj(&fo,fpath); o=fo.bo;

 po=find_user((char *)&o->port); // находим порт приписки
 obj_ch_parent(o,po); add_user(name,o);
 sprintf(o->dn,"%s",name); sprintf(o->password,"%s",pass); 

// evt_init_newobj(o);
 me=get_evt(); evt_func_set[E_init_obj](me,o,NULL,NULL,0.0001); ins_evt(o,me);

 return o;
}
