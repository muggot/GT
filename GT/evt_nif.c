#include "gt.h"
extern float NIF_STEP;

void evt_send_nif_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_send_nif;
 ne->t=GTIME+time;
 ne->l=l; // буфер отправки, после отправки нужно прибить
 ne->o=o;
 ne->e=e;
}

void evt_recv_nif_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_recv_nif;
 ne->t=GTIME+time;
 ne->o=o;
 ne->e=e;
 ne->l=l;
}

void evt_send_nif_act(obj *o,evt *e,lnk *l)
{
 const struct timeval tout={0,0};
 fd_set wfds, efds;
 int i;
 lnk *ls,*ln;

 del_evt(o,e); free_evt(o,e);

 FD_ZERO(&wfds); FD_ZERO(&efds);
 FD_SET(o->cs,&wfds); FD_SET(o->cs,&efds);
 i=select(0,NULL,&wfds,&efds,&tout);

 ls=o->nb.se;
 if(i==1 && FD_ISSET(o->cs,&efds)==0) 
  { i=send(o->cs,ls->b+ls->cp,ls->n-ls->cp,0); ls->cp+=i; }

 printf("SEND: %s\n",ls->b);

 FD_CLR(o->cs,&wfds); FD_CLR(o->cs,&efds);
 
 if(ls->cp==ls->n) // сообщение отправлено
  {
   ln=ls->prev; o->nb.se=ln; if(ln!=NULL) ln->next=NULL; else o->nb.sb=NULL;
   myfree(ls->b); free_lnk(NULL,ls); 
  }
 else ln=ls;

 if(ln!=NULL)
  { e=get_evt(); evt_send_nif_set(e,o,NULL,ln,NIF_STEP); ins_evt(o,e); }
}

void del_nif_obj(obj *o)
{
 int i;
 evt *e,*de;
 lnk *l,*dl;
 struct linger lin;

// удаляю очередь событий
 e=o->bevt; if(e!=NULL) e=e->next;
 while(e!=NULL) 
  { de=e->next; del_evt(o,e); free_evt(o,e); e=de; }
 e=o->bevt; if(e!=NULL) { del_evt(o,e); free_evt(o,e); }

// удаляю список ссылок на зависимые события
 for(i=0;i<ELST_NUM;i++)
  while(o->elst[i]!=NULL)
  {
   l=dl=o->elst[i]; o->elst[i]=l->next;
   if(dl->l==l) { if(l->e->e!=NULL) l->e->e->e=NULL; free_evt(o,l->e); evt_lnk_del(l); }
   else 
   {
    while(dl->l!=l) dl=dl->l;
    dl->l=l->l; if(l->e!=NULL) { dl->e=l->e; l->e->l=dl; evt_lnk_del(l); }
   }
  }
 
// удаляю буферы
 myfree(o->nb.trb);
 while(o->nb.sb!=NULL)
 {
  l=o->nb.sb; o->nb.sb=l->next; myfree(l->b); free_lnk(o,l);
 }
 while(o->nb.rb!=NULL)
 {
  l=o->nb.rb; o->nb.rb=l->next; myfree(l->b); free_lnk(o,l);
 }
 o->nb.se=NULL; o->nb.re=NULL;

// закрываю сокет
 printf("Закрываю соединение \n"); 
 closesocket(o->cs); 

 o->pl->nif=NULL; // удаляю устройство

// удаляю объект
  del_obj(o); free_obj(o);
}

void drop_recv_tag(obj *o)
{
 lnk *l,*ln;
 l=o->nb.rb; ln=l->next; o->nb.rb=ln;
 if(ln!=NULL) ln->prev=NULL; else o->nb.re=NULL;
 myfree(l->b); free_lnk(NULL,l);
}

void evt_recv_nif_act(obj *o,evt *e,lnk *l)
{
 const struct timeval tout={0,0};
 fd_set rfds,efds;
 int i,j,bp=0;
 lnk *ln;

 del_evt(o,e); free_evt(o,e);

 FD_ZERO(&rfds); FD_ZERO(&efds);
 FD_SET(o->cs,&rfds); FD_SET(o->cs,&efds);
 i=select(0,&rfds,NULL,&efds,&tout);
 
 if(i==1 && FD_ISSET(o->cs,&efds)==0) 
  { 
   i=recv(o->cs,o->nb.trb->b+o->nb.trb->nr,sizeof(o->nb.trb->b)-o->nb.trb->nr,0);
   if(i==0) { del_nif_obj(o); return; } // соединение закрыто клиентом
   for(j=o->nb.trb->nr;j<o->nb.trb->nr+i;j++)
    {
     if(o->nb.trb->b[j]=='<' && o->nb.trb->rf==0) { bp=j+1; o->nb.trb->rf=1; }
     else
     if(o->nb.trb->b[j]=='>' && o->nb.trb->rf==1)
      { // найден конец тэга сбросим содержимое тэга в цепочку
       o->nb.trb->rf=0; 
       if(bp!=j) // игнор пустые тэги
       {
        l=get_lnk(); l->n=j-bp+1; l->b=mymalloc(l->n);
        memcpy(l->b,&o->nb.trb->b[bp],l->n-1); l->b[l->n-1]=0;

        l->prev=NULL; l->next=o->nb.rb; 
        if(o->nb.rb!=NULL) { o->nb.rb->prev=l; l->cp=o->nb.rb->cp; }
        else { o->nb.re=l; l->cp=0; }
        o->nb.rb=l;
        // исследуем новый тэг на предмет его завершенности
	if(l->b[0]=='/') // чиста закрыващий тэг
         l->cp--;
        else if(l->b[l->n-2]!='/') l->cp++;
        if(l->cp==0) while(l->next!=NULL && l->next->cp!=0) { l=l->next; l->cp=0; }
        if(l->cp<0) drop_recv_tag(o); // удаляю лишние закрывающие тэги
       }
       bp=0;
      }
    }
   o->nb.trb->nr+=i;

   // сместим в начало фрагментированное содержимое буфера
   if(o->nb.trb->rf!=0 && bp!=0)
   { o->nb.trb->nr-=bp; memcpy(o->nb.trb->b,&o->nb.trb->b[bp],o->nb.trb->nr); }

   if(o->nb.trb->rf==0) o->nb.trb->nr=0; 
   o->rt=GTIME;
  }
 FD_CLR(o->cs,&rfds);  FD_CLR(o->cs,&efds);
 
 if(o->nb.trb->nr==sizeof(o->nb.trb->b) || GTIME-o->rt>40.0) // буфер переполнен или таймаут обрываю соединение
  { del_nif_obj(o); return; }

 e=get_evt(); evt_recv_nif_set(e,o,NULL,NULL,NIF_STEP); ins_evt(o,e);
 recv_cmds_proc(o);
}
