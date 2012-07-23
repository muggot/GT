#include "gt.h"
#include "prep.h"

extern struct conf cfg;

void evt_init_obj_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_init_obj;
 ne->t=GTIME+time;
 ne->o=o;
 ne->e=e;
 ne->l=l;
}

void obj_W_init(obj *o,evt *e,lnk *l)
{
 o->cnt=1;
 o->locs=calloc(o->s.xs*o->s.ys,sizeof(loc));
}

void obj_FF_init(obj *o,evt *e,lnk *l)
{
 if(o->pl->tag==T_WORLD) loc_obj_reg(o,0);
}

void obj_S_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_JG_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_BS_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_UL_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_SY_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_SHOP_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_FP_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_P_init(obj *o,evt *e,lnk *l)
{
 double dr;
 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.tx=0; o->s.ty=0; o->s.ct=GTIME;
 o->dev.cm=0.0; o->dev.pwr[0]=o->dev.pwr[1]=o->dev.pwr[2]=0;
 if(o->port!=NULL) o->port=find_user((char *)&o->port);
 obj_FF_init(o,e,l);
 loc_col_bmv(o); // добавлено для отказа от прежнего режима движения через команду mv
}

void obj_SP_init(obj *o,evt *e,lnk *l)
{
 evt *me;
 lnk *ml;
 double dr;
 if(o->pl->tag==T_WORLD) // если спарк внутри другого объекта, то он никуда не летит 
 {
  dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
  o->s.tx=0; o->s.ty=0; o->s.ct=GTIME;
  obj_FF_init(o,e,l);
 }
 o->dev.cm=0; o->dev.pwr[0]=o->dev.pwr[1]=o->dev.pwr[2]=0;
 if(o->pl->tag!=T_SP) // если спарк не находтся в животе у мамы, то он должен думать
 {
  me=get_evt(); evt_func_set[E_spark](me,o,NULL,NULL,1.0+(49.0*rand()/RAND_MAX)); ins_evt(o,me);
// поскольку спарки могут летать в космосе, то нужно зарегистрировать реакцию
// на коррекцию или уничтожение события пересечения с границей локации
// в результате после первого вызова E_spark, будет вызвано E_cold_loc,
// которое при необходимости создаст E_col_loc и вставит его в очередь.
  me=get_evt(); evt_func_set[E_cold_loc](me,o,NULL,NULL,0.0);
  ml=evt_lnk_ins(lnks_tags[E_spark],o,me,NULL); ml->l=ml; me->l=ml;
 }
}

void obj_AS_init(obj *o,evt *e,lnk *l)
{
 evt *me;
 me=get_evt(); evt_func_set[E_orb](me,o,NULL,NULL,0.0); ins_evt(o,me);
}

void obj_MUSOR_init(obj *o,evt *e,lnk *l)
{
 obj_FF_init(o,e,l);
}

void obj_RES_init(obj *o,evt *e,lnk *l)
{
 obj *co;
 o->dev.cm=o->dev.m;
 if(o->pl->tag==T_CRGO)
 {
  co=o->pl; while(co->tag!=T_WORLD) { co->dev.cm+=o->dev.m; co=co->pl; }
 }
}

void obj_BL_init(obj *o,evt *e,lnk *l)
{
 if(o->bmode==0) obj_BL_init_rt(o,e,l);
 else obj_BL_init_tb(o,e,l);
}

void obj_BL_init_tb(obj *o,evt *e,lnk *l)
{
 obj *co;
 evt *me,*de;
 lnk *nl;
 me=get_evt(); evt_func_set[E_bl_dummy](me,o,NULL,NULL,3.0); ins_evt(o,me); // ход пустого субъекта (защита от мгновенных ходов)
 me=get_evt(); evt_func_set[E_bl_eturn](me,o,NULL,NULL,40.0); ins_evt(o,me); // переход хода по таймауту
 de=get_evt(); evt_func_set[E_bl_eturnd](de,o,NULL,NULL,0.0);
 me->e=de; de->e=me; nl=evt_lnk_ins(lnks_tags[E_bl_dummy],o,de,NULL); de->l=nl; nl->l=nl;
 o->turn=0; // первый ход
 co=o->bo; o->nact=1;
 while(co!=NULL) 
 { 
  o->nact++; 
//  if(co->bitf&GOF_ISNIF!=0 && co->s.me!=NULL) // объект управляемый и движется, остановить надобно
//   evt_func_act[E_mvf_user](co,co->s.me,NULL);

  co->turn=0; // первый ход

// для встроенных ботов создаем событие планирования хода и вставляем в очередь
  if(co->bitf&GOF_ISNIF==0)
  {
   me=get_evt(); evt_func_set[E_bt_obj](me,co,NULL,NULL,3.0*rand()/RAND_MAX); ins_evt(co,me);
  }
  
  co=co->next;
 }
 o->rt=GTIME; // время окончания предыдущего хода, совпадает со временем начала текущего
 cmd_send_bl_begin(o); // оповестим участников о начале боя
}

void obj_BL_init_rt(obj *o,evt *e,lnk *l)
{
 obj *co;
 evt *me,*de;
 lnk *nl;

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

void obj_SHP_init(obj *o,evt *e,lnk *l)
{
 if(o->dev.mysl->sn==0) return; // устройство не установлено
 o->pl->dev.mysl=o->dev.mysl; o->dev.mysl->o=o;
 o->pl->dev.dvsl=o->dev.dvsl;
}

obj *get_TAG_obj(obj *co, int tag)
{
 while(co!=NULL && co->tag!=tag) co=co->next;
 return co;
}

void obj_CRGO_init(obj *o,evt *e,lnk *l)
{
 obj *co;
 lnk *cl;

// учет массы устройства
 o->dev.cm=o->dev.m;
 co=o->pl; while(co->tag!=T_WORLD) { co->dev.cm+=o->dev.m; co=co->pl; }

 if(o->dev.mysl==NULL || o->dev.mysl->sn==0) return; // устройство не установлено
 if(o->pl->dev.mysl==NULL) 
  {
   co=get_TAG_obj(o->pl->bo,T_SHP);
   if(co==NULL) return; // ошибка в структуре данных, корпус отсутствует, а начинка есть
   o->pl->dev.mysl=co->dev.mysl; o->pl->dev.dvsl=co->dev.dvsl;
  }
 cl=o->pl->dev.dvsl;
 while(cl!=NULL)
 {
  if(cl->st[0]==o->dev.mysl->st[0] && cl->sc>=o->dev.mysl->sc && cl->sn==o->dev.mysl->sn)
  {
   cl->o=o; break;
  }
  cl=cl->next;
 }
}

void obj_ENG_init(obj *o,evt *e,lnk *l)
{
 obj_CRGO_init(o,e,l);
 if(o->dev.mysl->sn==0) return; // устройство не установлено
 o->pl->dev.pwr[0]+=o->dev.pwr[0]; // суммирую мощность двигателей корабля
 o->pl->dev.pwr[1]+=o->dev.pwr[1]; // суммирую мощность двигателей корабля
 o->pl->dev.pwr[2]+=1.0/o->dev.pwr[2]; // суммирую маневренность двигателей корабля
}

void obj_FLD_init(obj *o,evt *e,lnk *l)
{
 obj_CRGO_init(o,e,l);
 e=get_evt(); evt_func_set[E_buse](e,o,NULL,NULL,0.0); ins_evt(o,e); o->s.me=e;
}

void obj_ARS_init(obj *o,evt *e,lnk *l)
{
 obj_CRGO_init(o,e,l);
 e=get_evt(); evt_func_set[E_buse](e,o,NULL,NULL,0.0); ins_evt(o,e); o->s.me=e;
}

void obj_FTN_init(obj *o,evt *e,lnk *l)
{
 obj_CRGO_init(o,e,l);
}

void obj_HULL_init(obj *o,evt *e,lnk *l)
{
 obj_CRGO_init(o,e,l);
}

void obj_WPN_init(obj *o,evt *e,lnk *l)
{
 obj_CRGO_init(o,e,l);
}

void obj_SHAB_init(obj *o,evt *e,lnk *l)
{
 int ds=0; obj *co=o;

 set_ghab(o);
 while(co->tag!=T_WORLD) { ds+=strlen(co->dn)+1; co=co->pl; }
 ds+=strlen(cfg.data_path)+1;
 o->fpath=malloc(ds+1); o->fpath[ds]=0;
 co=o;
 while(co->tag!=T_WORLD)
  { o->fpath[ds-1]='\\'; ds=ds-strlen(co->dn)-1; memcpy(o->fpath+ds,co->dn,strlen(co->dn)); co=co->pl; }
 o->fpath[ds-1]='\\';
 memcpy(o->fpath,cfg.data_path,strlen(cfg.data_path));
}

void obj_DUMMY_init(obj *o,evt *e,lnk *l)
{
}

void (*obj_func_init[OBJ_TAG_NUM])(obj *o,evt *e,lnk *l)=
{
 obj_DUMMY_init,
 obj_W_init,
 obj_FF_init,
 obj_S_init,
 obj_JG_init,
 obj_SY_init,
 obj_FP_init,
 obj_P_init,
 obj_DUMMY_init,
 obj_AS_init,
 obj_BL_init,
 obj_SHP_init,
 obj_CRGO_init,
 obj_ENG_init,
 obj_FLD_init,
 obj_FTN_init,
 obj_HULL_init,
 obj_WPN_init,
 obj_BS_init,
 obj_UL_init,
 obj_SP_init,
 obj_MUSOR_init,
 obj_RES_init,
 obj_ARS_init,
 obj_SHAB_init,
 obj_DUMMY_init,
 obj_SHOP_init
};

void evt_init_obj_act(obj *o,evt *e,lnk *l)
{
 obj *co,*no;

 obj_func_init[o->tag](o,NULL,NULL);
 del_evt(o,e); free_evt(o,e);

 no=o->next;
 if(no!=NULL) { e=get_evt(); evt_init_obj_set(e,no,NULL,NULL,0.0); ins_evt(no,e); }

 if(o->tag==T_SHAB) return; // контейнер шаблонов, шаблоны не инициализируем.

 co=o->bo;
 if(co!=NULL) { e=get_evt(); evt_init_obj_set(e,co,NULL,NULL,0.0); ins_evt(co,e); }
}

void evt_init_newobj(obj *o)
{
 obj *co;
 evt *e;

 obj_func_init[o->tag](o,NULL,NULL);

 co=o->bo;
 if(co!=NULL) { e=get_evt(); evt_init_obj_set(e,co,NULL,NULL,0.0); ins_evt(co,e); }
}
