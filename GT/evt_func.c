//////////////////////////////////////////////////////////////////
// �㭪樨 �ࠢ����� ��।ﬨ ᮡ�⨩ � �᪮���騬� ��뫪���
// ��।� ᮡ�⨩ �।�⠢��� ᮡ�� �����吝� ᯨ᮪ �������
// ᮡ�⨩ �����஢����� �� �६���. ���� ᮡ�⨥� ᯨ᪠
// ���� ������襥 �� �६��� ᮡ�⨥. ������� �᪮�����
// ��뫮� �।�⠢��� ᮡ�� �����吝� ᯨ᮪ ������� ��뫮�
// �� sqrt(N) ������� ᮡ�⨩, ��� N - ⥪�饥 �᫮ ᮡ�⨩ �
// ��।� ᮡ�⨩. ���᮪ ��뫮� ⠪�� 㯮�冷祭 �� �६���
// ᮮ⢥������� ᮡ�⨩. ���᮪ ��뫮� �ᯮ������ ���
// ����ன ��⠢�� ����� ᮡ�⨩, � ⠪�� ����� ���� �ᯮ�짮���
// ��� ���᪠ ᮡ�⨩ �⭮������ � ��।�������� ������� �६���.
// ��㤮������� ��⠢�� � ���᪠ � �।��� ࠢ�� sqrt(N).
//
// ��� ����஫� ����室����� 㢥��祭�� ��� 㬥��襭�� �᫠
// ��뫮� �ᯮ������� ���祭�� ⥪�饩 ����� ��।� N1 � ⥪�饩
// ����� ᯨ᪠ ��뫮� N2.
//
// ���⢥��⢨� ���� ᯨ᪮� ����� ��� N2=sqrt(N1). �㭪�� �����祭��
// ���� ��������� ���⮬� �ᯮ������ ��������� ���᪠ ���祭�� N2 ��
// ⠡��� � �㭪樮���쭮�� �襭�� �� �ॢ�襭�� ����� ��।�
// ��।�������� ���祭�� N1m=65536.
//#include <stdlib.h>
#include <math.h>
//#include <windows.h>
#include "gt_sqrt_tab.h" // ⠡��� ���祭�� N2
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


// ��������!!! ���������� ������� �����
double GTIME=0;

// ��������!!! ���������� ��������� ���� ����������� ������� ��������
// ���������������� ������� �������� ��� ������� �������
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

// ����砥� ᢮������ ᮡ�⨥ ��� ���쭥�襣� �ᯮ�짮�����
evt *get_evt(void)
{
 evt	*ne;
 memlst **ptml,*tml;
 int	i;

 if(GP.fe==NULL) /*ᯨ᮪ �� ᮧ��� ��� �����稫��� ᢮����� ��������*/
  {
   ptml=&(GP.ml); while(*ptml!=NULL) ptml=&((*ptml)->next);
   tml=*ptml=(memlst *)calloc(1,sizeof(memlst)); 
    if(tml==NULL) { printf("������ ���稫���\n"); return NULL; }
   GP.fe=tml->m.emem;
   for(i=0;i<EVT_IN_MEMLST-1;i++) tml->m.emem[i].next=&(tml->m.emem[i+1]);
   printf("�뤥��� ���� ᮡ�⨩\n");
  }
 ne=GP.fe; GP.fe=ne->next; ne->next=NULL;
 ne->tag=E_NORMAL;
 return ne;
}

// ����砥� ᢮������ ��뫪� ��� ���쭥�襣� �ᯮ�짮�����
lnk *get_lnk(void)
{
 lnk	*nl;
 memlst **ptml,*tml;
 int	i;

 if(GP.fl==NULL) /*ᯨ᮪ �� ᮧ��� ��� �����稫��� ᢮����� ��������*/
  {
   ptml=&(GP.ml); while(*ptml!=NULL) ptml=&((*ptml)->next);
   tml=*ptml=(memlst *)calloc(1,sizeof(memlst)); 
    if(tml==NULL) { printf("������ ���稫���\n"); return NULL; }
   GP.fl=tml->m.lmem;
   for(i=0;i<LNK_IN_MEMLST-1;i++) tml->m.lmem[i].next=&(tml->m.lmem[i+1]);
   printf("�뤥��� ���� ��뫮�\n");
  }
 nl=GP.fl; GP.fl=nl->next; 
 nl->tag=0; nl->prev=nl->next=NULL; nl->o=NULL; nl->e=NULL; nl->l=NULL;
 
 return nl;
}

// ��⠢��� ᮡ�⨥-��뫪� � ��।� த�⥫�᪮�� ��ꥪ�.
void ins_par_evt(obj *o,evt *se)
{
 evt *e;

 if(se==NULL) return;
 e=get_evt(); if(e==NULL) exit(0); 
 e->t=se->t; e->o=o; e->e=NULL; e->l=NULL; e->tag=E_LINK; // ������ ��㣨� ��ࠬ���� ᮡ���-��뫪�
 ins_evt(o->pl,e); se->pe=e;
}

// 㤠��� ᮡ���-��뫪� �� �������� ᮡ�⨥ �� ��।�� த�⥫�᪨� ��ꥪ⮢.
// �ᯮ������ �� ᬥ�� த�⥫�᪮�� ��ꥪ� � ��࠭����� ᮡ�⢥���� ��।�.
void del_par_evt(obj *o)
{
 evt *e=o->bevt;
 if(e!=NULL && e->pe!=NULL) 
  { 
   e->pe->o=NULL; del_evt(o->pl,e->pe); free_evt(o->pl,e->pe); // 㤠�塞 ᮡ��� ��뫪� � த�⥫�᪨� ��ꥪ��
   e->pe=NULL;
  }
}


// ��⠢��� ᮡ�⨥ � ��।�.
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
   if(ol==NULL) // ��।� �����, ᮧ���� ��
    {
     o->bevt=e; e->prev=e->next=NULL; o->evtn=1; //evtn �ᥣ�� ���樠���������
     l=get_lnk(); l->e=e;
     o->blnk=l; l->prev=l->next=NULL; o->lnkn=1; //lnkn �ᥣ�� ���樠���������
     e->ql=l;
    }
   else		// ����� ᯨ᪠ ��뫮� ���ࠢ����� ���쭥�襣� ���᪠ ��אַ�
    {
     ce=ol->e;
     do
      { oe=ce; ce=ce->next; }
     while(ce!=NULL && ce->t<e->t);
     // ��⠢�塞 ��᫥ oe
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
   if(ol==NULL) ce=o->bevt; // cl-��ࢠ� ��뫪� ᯨ᪠
   else 	ce=ol->e;
   while(ce!=NULL && ce->t<e->t)
    { oe=ce; ce=ce->next; }
   // ��⠢�塞 ��᫥ oe
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
   ins_par_evt(o,e); //���� ४����
  }
}

// 㤠��� ��뫪� �� ᯨ᪠ ��।�
void del_lnk(obj *o,lnk *l)
{
 if(l->next!=NULL) l->next->prev=l->prev;
 if(l->prev!=NULL) l->prev->next=l->next;
 else o->blnk=l->next;
 o->lnkn--;
}

// �᢮������� ��뫪�
void free_lnk(obj *o,lnk *l)
{
 l->prev=NULL; l->next=GP.fl; GP.fl=l;
}

// �᢮������� ᮡ�⨥
void free_evt(obj *o,evt *e)
{
 e->prev=NULL; e->pe=NULL; e->next=GP.fe; e->tag=E_EMPTY; GP.fe=e;
}

// 㤠��� ᮡ�⨥ �� ��।�
void del_evt(obj *o,evt *e)
{
 int sq;
 evt *ce;
 lnk *l,*dl,*tl;

 ce=e->e; 
 if(ce!=NULL) // ���� ᮡ�⨥ 㭨�⮦����
 {
  // 㤠��� ��뫪� ॠ�権
  l=ce->l; dl=l->l; while(dl!=l){ tl=dl->l; evt_lnk_del(dl); dl=tl; }
  evt_lnk_del(dl);  ce->l=NULL;
  // 㤠��� ᮡ�⨥ 㭨�⮦����
  free_evt(NULL,ce);
 }

 if(o->bevt==e && e->pe!=NULL) 
  { 
   e->pe->o=NULL; del_evt(o->pl,e->pe); free_evt(o->pl,e->pe); // 㤠�塞 ᮡ��� ��뫪� � த�⥫�᪨� ��ꥪ��
   if(e->next!=NULL) ins_par_evt(o,e->next); // ��⠢�塞 ᮡ�⨥-��뫪� � த�⥫�᪨� ��ꥪ�� � 㪠������ �� ���� ��ࢮ� ᮡ�⨥
  }

 if(e->next!=NULL) e->next->prev=e->prev;
 if(e->prev!=NULL) e->prev->next=e->next;
 else o->bevt=e->next;
 o->evtn--;

 if(o->evtn<65536) sq=sqtab[o->evtn]; else sq=sqrt(o->evtn);
 if(sq<o->lnkn) // ����� 㡠���� ������⢮ ��뫮�
  {
   if(e->ql!=NULL) { del_lnk(o,e->ql); free_lnk(o,e->ql); } // ���� �᢮������� ��뫪�
  }
 else
  {
   if(e->ql==NULL) return; 
   // ��뫪� �㦭� ��७��ࠢ��� �� ��㣮� ᮡ�⨥
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

// ������� ��������� ᮡ��� � ��।� � ᮮ⢥��⢨� � ��⠭������� �६����
void move_evt(obj *o,evt *e) 
{ 
 evt *de=e->e;
 e->e=NULL; del_evt(o,e); ins_evt(o,e); e->e=de;
}

static void F_set(obj *o,evt *e)
{
 F_O=o; memcpy(&F_E,e,sizeof(evt));
}

// ��ࠡ��稪 �����饣� ᮡ���
static void act_evt(obj *o,evt *e)
{
 int tag=lnks_tags[e->tag];

 F_set(o,e);
 evt_func_act[e->tag](o,e,e->l);
 evt_lnks_act(tag,o);
}

// �।���⥫�� ��ࠡ��稪 ᮡ���
static void proc_evt(obj *o,evt *e)
{
 if(e->tag==E_LINK) proc_evt(e->o,e->o->bevt);
 else act_evt(o,e);
}

// �㫥��� 横� ������஢����
void proc_evts(obj *o)
{
 while(o->bevt!=NULL) // ���� ������쭠� ��।� �� ���� ���� �� �����.
  {
   GTIME=o->bevt->t; proc_evt(o,o->bevt);
   if(EVT_STOP_FLAG==1) { EVT_STOP_FLAG=2; while(EVT_STOP_FLAG==2) Sleep(0); }
  }
}
