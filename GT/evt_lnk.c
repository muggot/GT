#include "gt.h"

// �뤥��� ��뫪� �� ����ᨬ�� ᮡ�⨥ � �������� �� � ᯨ᮪
// ����ᨬ�� ᮡ�⨩
lnk *evt_lnk_ins(int tag,obj *o,evt *e,lnk *l2)
{
 lnk *l1;

 if(tag>=ELST_NUM) return NULL;
 l1=get_lnk(); l1->tag=tag;
 l1->o=o; l1->e=e; l1->l=l2;
 l1->prev=NULL; l1->next=o->elst[tag];
 if(o->elst[tag]!=NULL) o->elst[tag]->prev=l1; 
 o->elst[tag]=l1;
 return l1;
}

// 㤠��� ��뫪� �� ����ᨬ�� ᮡ�⨥ �� ᯨ᪠ ����ᨬ�� ᮡ�⨩ �
// �᢮������� ��
void evt_lnk_del(lnk *l)
{
 if(l->tag>=ELST_NUM) return;
 if(l->prev!=NULL) l->prev->next=l->next;
 else              l->o->elst[l->tag]=l->next;
 if(l->next!=NULL) l->next->prev=l->prev;
 free_lnk(l->o,l);
}

// ��뢠�� ��ࠡ��� ����ᨬ�� ᮡ�⨩ �� ᯨ᪠.
void evt_lnks_act(int tag,obj *o)
{
 lnk *l, *lnext;

 if(tag<0) return;

 l=o->elst[tag];

 if(tag>=ELST_NUM) return;
 while(l!=NULL)
  {
   lnext=l->next;
   while(l->e==NULL) l=l->l;
   evt_lnk_act(l->o,l->e,l);
   l=lnext;
  }

/*
 if(tag>=ELST_NUM) return;
 while(l!=NULL)
  {
   lnext=l->next;
   if(l->o!=o) evt_lnk_act(l->o,l->e,l->l);
   else        evt_lnk_act(o,l->e,l);
   l=lnext;
  }
*/
}

// ��뢠�� ��ࠡ��� �⤥�쭮�� ����ᨬ��� ᮡ���.
void evt_lnk_act(obj *o,evt *e,lnk *l)
{
 if(evt_func_act[e->tag]!=NULL) evt_func_act[e->tag](o,NULL,l);
}
