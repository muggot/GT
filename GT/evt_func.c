//////////////////////////////////////////////////////////////////
// Функции управления очередями событий и ускоряющими ссылками
// очередь событий представляет собой двухсвязный список структур
// событий отсортированных по времени. Первым событием списка
// является ближайшее по времени событие. Структура ускоряющих
// ссылок представляет собой двухсвязный список структур ссылок
// на sqrt(N) структур событий, где N - текущее число событий в
// очереди событий. Список ссылок также упорядочен по времени
// соответствующих событий. Список ссылок используется для
// быстрой вставки новых событий, а также может быть использован
// для поиска событий относящихся к определенному моменту времени.
// Трудоемкость вставки и поиска в среднем равна sqrt(N).
//
// Для контроля необходимости увеличения или уменьшения числа
// ссылок используются значения текущей длины очереди N1 и текущей
// длины списка ссылок N2.
//
// Соответствие длин списков имеет вид N2=sqrt(N1). Функция извлечения
// корня медленная поэтому используется комбинация поиска значений N2 по
// таблице и функционального решения при превышении длины очереди
// определенного значения N1m=65536.
//#include <stdlib.h>
#include <math.h>
//#include <windows.h>
#include "gt_sqrt_tab.h" // таблица значений N2
#include "gt.h"
#include "evt_oinit.h"
#include "evt_oload.h"
#include "evt_inet.h"
#include "evt_nif.h"
#include "evt_rmi.h"
#include "evt_orbit.h"
#include "evt_spark.h"
#include "evt_ul.h"
#include "evt_btl.h"
#include "evt_battle.h"


// ВНИМАНИЕ!!! ГЛОБАЛЬНОЕ ИГРОВОЕ ВРЕМЯ
double GTIME=0;

// ВНИМАНИЕ!!! ГЛОБАЛЬНЫЙ ГЕНЕРАТОР ИМЕН СОЗДАВАЕМЫХ ТИПОВЫХ ОБЪЕКТОВ
// ИНИЦИАЛИЗИРУЕТСЯ ТЕКУЩИМ ВРЕМЕНЕМ ПРИ ЗАПУСКЕ СЕРВЕРА
long long OGEN;

// static 
blockpool GP;

int EVT_STOP_FLAG=0;

evt  F_E;
obj *F_O;

void (*evt_func_set[E_NUMBER])(evt *ne,obj *o,evt *e,lnk *l,double time)=
{NULL,               NULL,      	     NULL,      evt_fload_obj_set,
 evt_init_obj_set, evt_init_net_set,  evt_sync_net_set, evt_send_nif_set,
 evt_recv_nif_set, evt_mvs_user_set,  evt_mvf_user_set, evt_mvfd_user_set,
 evt_col_loc_set,  evt_cold_loc_set,  evt_orb_set,      evt_spark_set, 
 evt_UL_set, 	   evt_prich_user_set,evt_bl_dummy_set, evt_bl_eturn_set,
 evt_bl_eturnd_set,evt_bt_obj_set,    evt_bmv_set,	evt_buse_set,
 evt_bobj_set,	   evt_bkill_set};

void (*evt_func_act[E_NUMBER])(obj *o,evt *e,lnk *l)=
{NULL,             NULL,      	     NULL,		evt_fload_obj_act,
 evt_init_obj_act, evt_init_net_act, evt_sync_net_act,	evt_send_nif_act,
 evt_recv_nif_act, evt_mvs_user_act, evt_mvf_user_act,	evt_mvfd_user_act,
 evt_col_loc_act,  evt_cold_loc_act, evt_orb_act,       evt_spark_act, 
 evt_UL_act,       evt_prich_user_act,evt_bl_dummy_act, evt_bl_eturn_act,
 evt_bl_eturnd_act,evt_bt_obj_act,    evt_bmv_act,	evt_buse_act,
 evt_bobj_act,	   evt_bkill_act};

int lnks_tags[E_NUMBER]=
{-1,               -1,               -1,		-1,
 -1,		   -1,		     -1,		-1,
 -1,		   -1,		      0,		-1,
 -1,		   -1,		      0,		 0,
  0,		   -1,		     -1,	        -1,
 -1,		   -1,		      1,		-1,
 -1,		   -1
 };

// получает свободное событие для дальнейшего использования
evt *get_evt(void)
{
 evt	*ne;
 memlst **ptml,*tml;
 int	i;

 if(GP.fe==NULL) /*список не создан или закончились свободные структуры*/
  {
   ptml=&(GP.ml); while(*ptml!=NULL) ptml=&((*ptml)->next);
   tml=*ptml=(memlst *)calloc(1,sizeof(memlst)); 
    if(tml==NULL) { printf("Память кончилась\n"); return NULL; }
   GP.fe=tml->m.emem;
   for(i=0;i<EVT_IN_MEMLST-1;i++) tml->m.emem[i].next=&(tml->m.emem[i+1]);
   printf("Выделен блок событий\n");
  }
 ne=GP.fe; GP.fe=ne->next; ne->next=NULL;
 ne->tag=E_NORMAL;
 return ne;
}

// получает свободную ссылку для дальнейшего использования
lnk *get_lnk(void)
{
 lnk	*nl;
 memlst **ptml,*tml;
 int	i;

 if(GP.fl==NULL) /*список не создан или закончились свободные структуры*/
  {
   ptml=&(GP.ml); while(*ptml!=NULL) ptml=&((*ptml)->next);
   tml=*ptml=(memlst *)calloc(1,sizeof(memlst)); 
    if(tml==NULL) { printf("Память кончилась\n"); return NULL; }
   GP.fl=tml->m.lmem;
   for(i=0;i<LNK_IN_MEMLST-1;i++) tml->m.lmem[i].next=&(tml->m.lmem[i+1]);
   printf("Выделен блок ссылок\n");
  }
 nl=GP.fl; GP.fl=nl->next; 
 nl->tag=0; nl->prev=nl->next=NULL; nl->o=NULL; nl->e=NULL; nl->l=NULL;
 
 return nl;
}

// вставляет событие-ссылку в очередь родительского объекта.
void ins_par_evt(obj *o,evt *se)
{
 evt *e;

 if(se==NULL) return;
 e=get_evt(); if(e==NULL) exit(0); 
 e->t=se->t; e->o=o; e->e=NULL; e->l=NULL; e->tag=E_LINK; // задать другие параметры события-ссылки
 ins_evt(o->pl,e); se->pe=e;
}

// удаляет события-ссылки на заданное событие из очередей родительских объектов.
// используется при смене родительского объекта с сохранением собственной очереди.
void del_par_evt(obj *o)
{
 evt *e=o->bevt;
 if(e!=NULL && e->pe!=NULL) 
  { 
   e->pe->o=NULL; del_evt(o->pl,e->pe); free_evt(o->pl,e->pe); // удаляем события ссылки в родительских объектах
   e->pe=NULL;
  }
}


// вставляет событие в очередь.
void ins_evt(obj *o,evt *e)
{
 int sq;
 evt *ce,*oe=NULL;
 lnk *l,*cl,*ol=NULL;

 cl=o->blnk; o->evtn++; e->ql=NULL;
 while(cl!=NULL && cl->e->t<e->t)
  {
   ol=cl; cl=cl->next;
  }
 if(cl==NULL)
  {
   if(ol==NULL) // очередь пустая, создаем ее
    {
     o->bevt=e; e->prev=e->next=NULL; o->evtn=1; //evtn всегда инициализируется
     l=get_lnk(); l->e=e;
     o->blnk=l; l->prev=l->next=NULL; o->lnkn=1; //lnkn всегда инициализируется
     e->ql=l;
    }
   else		// конец списка ссылок направление дальнейшего поиска прямое
    {
     ce=ol->e;
     do
      { oe=ce; ce=ce->next; }
     while(ce!=NULL && ce->t<e->t);
     // вставляем после oe
     e->next=oe->next; e->prev=oe; oe->next=e; if(ce!=NULL) ce->prev=e;

     if(o->evtn<65536) sq=sqtab[o->evtn]; else sq=sqrt(o->evtn);
     if(sq>o->lnkn)
      { 
       l=get_lnk(); if(l==NULL) exit(0); l->e=e;
       ol->next=l; l->prev=ol; l->next=NULL; o->lnkn++; e->ql=l; 
      }
    }
  }
 else
  {
   if(ol==NULL) ce=o->bevt; // cl-первая ссылка списка
   else 	ce=ol->e;
   while(ce!=NULL && ce->t<e->t)
    { oe=ce; ce=ce->next; }
   // вставляем после oe
   e->next=ce; e->prev=oe; ce->prev=e;
   if(oe==NULL) o->bevt=e; else oe->next=e; 

   if(o->evtn<65536) sq=sqtab[o->evtn]; else sq=sqrt(o->evtn);
   if(sq>o->lnkn)
    { 
     l=get_lnk(); l->e=e;
     if(ol==NULL) o->blnk=l; else ol->next=l; 
     l->prev=ol; l->next=cl; cl->prev=l; o->lnkn++; e->ql=l;
    }
  }
 if(o->bevt==e && o->pl->tag!=T_GAME) 
  {
   if(e->next!=NULL) { del_evt(o->pl,e->next->pe); free_evt(o->pl,e->next->pe); }
   ins_par_evt(o,e); //неявная рекурсия
  }
}

// удаляет ссылку из списка очереди
void del_lnk(obj *o,lnk *l)
{
 if(l->next!=NULL) l->next->prev=l->prev;
 if(l->prev!=NULL) l->prev->next=l->next;
 else o->blnk=l->next;
 o->lnkn--;
}

// освобождает ссылку
void free_lnk(obj *o,lnk *l)
{
 l->prev=NULL; l->next=GP.fl; GP.fl=l;
}

// освобождает событие
void free_evt(obj *o,evt *e)
{
 e->prev=NULL; e->pe=NULL; e->next=GP.fe; e->tag=E_EMPTY; GP.fe=e;
}

// удаляет событие из очереди
void del_evt(obj *o,evt *e)
{
 int sq;
 evt *ce;
 lnk *l,*dl,*tl;

 ce=e->e; 
 if(ce!=NULL) // есть событие уничтожения
 {
  // удаляю ссылки реакций
  l=ce->l; dl=l->l; while(dl!=l){ tl=dl->l; evt_lnk_del(dl); dl=tl; }
  evt_lnk_del(dl);  ce->l=NULL;
  // удаляю событие уничтожения
  free_evt(NULL,ce);
 }

 if(o->bevt==e && e->pe!=NULL) 
  { 
   e->pe->o=NULL; del_evt(o->pl,e->pe); free_evt(o->pl,e->pe); // удаляем события ссылки в родительских объектах
   if(e->next!=NULL) ins_par_evt(o,e->next); // вставляем событие-ссылку в родительские объекты с указанием на будуще первое событие
  }

 if(e->next!=NULL) e->next->prev=e->prev;
 if(e->prev!=NULL) e->prev->next=e->next;
 else o->bevt=e->next;
 o->evtn--;

 if(o->evtn<65536) sq=sqtab[o->evtn]; else sq=sqrt(o->evtn);
 if(sq<o->lnkn) // можно убавить количество ссылок
  {
   if(e->ql!=NULL) { del_lnk(o,e->ql); free_lnk(o,e->ql); } // просто освобождаем ссылку
  }
 else
  {
   if(e->ql==NULL) return; 
   // ссылку нужно перенаправить на другое событие
   ce=e->next; 
   if(ce!=NULL) 
   {
    if(ce->ql!=NULL) { del_lnk(o,e->ql); free_lnk(o,e->ql); return; }
    ce->ql=e->ql; e->ql->e=ce; 
   }
   else { ce=e->prev; while(ce->ql!=NULL) ce=ce->prev; ce->ql=e->ql; e->ql->e=ce; }
  }
// if(e->tag==E_LINK && e->o!=NULL) { del_evt(e->o,e->o->bevt); free_evt(o,e); }
}

// изменяет положение события в очереди в соответствии с установленным временем
void move_evt(obj *o,evt *e) 
{ 
 evt *de=e->e;
 e->e=NULL; del_evt(o,e); ins_evt(o,e); e->e=de;
}

static void F_set(obj *o,evt *e)
{
 F_O=o; memcpy(&F_E,e,sizeof(evt));
}

// обработчик настоящего события
static void act_evt(obj *o,evt *e)
{
 int tag=lnks_tags[e->tag];

 F_set(o,e);
 evt_func_act[e->tag](o,e,e->l);
 evt_lnks_act(tag,o);
}

// предварительный обработчик события
static void proc_evt(obj *o,evt *e)
{
 if(e->tag==E_LINK) proc_evt(e->o,e->o->bevt);
 else act_evt(o,e);
}

// нулевой цикл моделирования
void proc_evts(obj *o)
{
 while(o->bevt!=NULL) // пока глобальная очередь не пуста есть что считать.
  {
   GTIME=o->bevt->t; proc_evt(o,o->bevt);
   if(EVT_STOP_FLAG==1) { EVT_STOP_FLAG=2; while(EVT_STOP_FLAG==2) Sleep(0); }
  }
}
