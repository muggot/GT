#include "gt.h"
#include "prep.h"
#include <dos.h>

#define TR_UPDATE	1
#define TR_CREATE	2
#define TR_DELETE	3
#define TR_RENAME	4
#define TR_CPOINT	5

extern struct conf cfg;

typedef struct Tr_list
{
 struct Tr_list *next;	// следующая транзакция в списке
 long long num;       	// номер транзакции, совпадает с именем файла транзакции
 int tr_size;		// размер транзакции
 char *tr;            	// собственно транзакция
} tr_list;

#define TR_LIST_MAX	5

tr_list dummy={NULL,-1,0,NULL},*tr_beg=&dummy,*tr_end=&dummy; // первая и последняя транзакция в списке

// поток для сохранения накопленных транзакций из буфера на диск
DWORD obj_save_thread(void)
{
 long long i;
 char ftn[300];
 FILE *tr;
 tr_list *dtr;

 while(1)
  {
   while(tr_end->num-dummy.num<=TR_LIST_MAX) { /* printf("trnum=%d\n",tr_end->num); */ sleep(1);} // в буфере маловато транзакций
   for(i=1;i<=TR_LIST_MAX;i++)
   {
    sprintf(ftn,"%s\\%018Ld",cfg.log_path,dummy.num+i);
    tr=fopen(ftn,"wb"); fwrite(dummy.next->tr,dummy.next->tr_size,1,tr); fclose(tr);
    dtr=dummy.next; dummy.next=dummy.next->next; myfree(dtr->tr); myfree(dtr);
   }
   dummy.num+=TR_LIST_MAX;
  }
 return 0;
}

void obj_save_thread_init(void)
{
 HANDLE th;
 DWORD id;
 th=CreateThread(0,0,(LPTHREAD_START_ROUTINE) obj_save_thread,0,0,&id);
 if(th==NULL) { printf("Ошибка запуска потока сохранения состояния"); exit(1); }
}

void obj_save_cpoint(void)
{
 tr_list *dtr=mymalloc(sizeof(tr_list));
 int ts,ds;
 char *b;
 int *bi;

 tr_end->next=dtr; dtr->next=NULL; dtr->num=tr_end->num+1;

 ts=4; // размер типа транзакции
 ts+=cfg.dpl+1;
 ts+=sizeof("dynamic.dat"); // имя файла для сохранения глобальных данных
 ds=snprintf(NULL,0,"<W gt=\"%f\" />.",GTIME)+1;
 b=mymalloc(ts+ds); bi=(int *)b; bi[0]=TR_CPOINT; dtr->tr_size=ts+ds; dtr->tr=b;
 memcpy(b+4,cfg.data_path,cfg.dpl); b[4+cfg.dpl]='\\';
 memcpy(b+5+cfg.dpl,"dynamic.dat",sizeof("dynamic.dat"));
 snprintf(b+ts,ds,"<W gt=\"%f\" />.",GTIME)+1; 

 tr_end=dtr;
}

void obj_save_delete(obj *o)
{
 tr_list *dtr=mymalloc(sizeof(tr_list));
 int ts,ds;
 char *b;
 int *bi;
 obj *co;

 tr_end->next=dtr; dtr->next=NULL; dtr->num=tr_end->num+1;

 ts=4; // размер типа транзакции
 ts+=cfg.dpl+1;
 co=o; while(co->tag!=T_WORLD) { ts+=strlen(co->dn)+1; co=co->pl; }
 ts+=1;
 b=mymalloc(ts); bi=(int *)b; bi[0]=TR_DELETE; dtr->tr_size=ts; dtr->tr=b;
 ts--; b[ts]='.'; ts--; b[ts]=0;
 co=o; 
 while(co->tag!=T_WORLD) 
  { ds=strlen(co->dn); ts-=ds; memcpy(b+ts,co->dn,ds); ts--; b[ts]='\\'; co=co->pl; }
 ts-=cfg.dpl; memcpy(b+ts,cfg.data_path,cfg.dpl);
 tr_end=dtr;
}

void obj_save_rename(obj *o, obj *po)
{
 tr_list *dtr=mymalloc(sizeof(tr_list));
 int ts,ds;
 char *b;
 int *bi;
 obj *co;

 tr_end->next=dtr; dtr->next=NULL; dtr->num=tr_end->num+1;

 ts=4; // размер типа транзакции
 ts+=cfg.dpl+1;
 co=o; while(co->tag!=T_WORLD) { ts+=strlen(co->dn)+1; co=co->pl; }

 ts=strlen(o->dn)+1;
 ts+=cfg.dpl+1;
 co=po; while(co->tag!=T_WORLD) { ts+=strlen(co->dn)+1; co=co->pl; }
 ts+=1;

 b=mymalloc(ts); bi=(int *)b; bi[0]=TR_RENAME; dtr->tr_size=ts; dtr->tr=b;
 ts--; b[ts]='.'; ts--; b[ts]=0;
 ds=strlen(o->dn); ts-=ds; memcpy(b+ts,o->dn,ds); ts--; b[ts]='\\';
 co=po; 
 while(co->tag!=T_WORLD) 
  { ds=strlen(co->dn); ts-=ds; memcpy(b+ts,co->dn,ds); ts--; b[ts]='\\'; co=co->pl; }
 ts-=cfg.dpl; memcpy(b+ts,cfg.data_path,cfg.dpl);
 ts--; b[ts]=0;

 co=o; 
 while(co->tag!=T_WORLD) 
  { ds=strlen(co->dn); ts-=ds; memcpy(b+ts,co->dn,ds); ts--; b[ts]='\\'; co=co->pl; }
 ts-=cfg.dpl; memcpy(b+ts,cfg.data_path,cfg.dpl);

 tr_end=dtr;
}





int obj_save_P(obj *o,int create_flag,char *buf,int len)
{
 int ds;
 if(create_flag==1)
  ds=snprintf(buf,len,"<P password=\"%s\" xs=\"%f\" ys=\"%f\" v=\"%f\" img=\"%d\" />.",o->password,o->s.xs,o->s.ys,o->s.v,o->s.img)+1; 
 else 
  ds=snprintf(buf,len,"<P ct=\"%f\" tx=\"%f\" ty=\"%f\" xp=\"%f\" yp=\"%f\" />.",o->s.ct,o->s.tx,o->s.ty,o->s.cx,o->s.cy)+1; 
 return ds; 
}

int (*obj_save_dat[OBJ_TAG_NUM])(obj *o,int create_flag,char *buf,int len)=
{
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 obj_save_P,
 NULL,
 NULL,
 NULL
};


void obj_save(obj *o,int create_flag)
{
 tr_list *dtr=mymalloc(sizeof(tr_list));
 int ts,ds,cs,ms;
 char *b;
 int *bi;
 obj *co;

 if(create_flag==1) // создание объекта, создаем папку и сохраняем статику
 {
  tr_end->next=dtr; dtr->next=NULL; dtr->num=tr_end->num+1;

  ts=4; // размер типа транзакции
  ts+=(cfg.dpl+1)*2;
  co=o; while(co->tag!=T_WORLD) { ts+=(strlen(co->dn)+1)*2; co=co->pl; }
  ts+=sizeof("object.dat");
  ds=obj_save_dat[o->tag](o,1,NULL,0); // найдем длину сохраняемых данных объекта
 
  b=mymalloc(ts+ds); bi=(int *)b; bi[0]=TR_CREATE; dtr->tr_size=ts+ds; dtr->tr=b;

  cs=ts; ms=sizeof("object.dat"); cs-=ms; memcpy(b+cs,"object.dat",ms); cs--; b[cs]='\\';
  co=o; 
  while(co->tag!=T_WORLD) 
   { ms=strlen(co->dn); cs-=ms; memcpy(b+cs,co->dn,ms); cs--; b[cs]='\\'; co=co->pl; }
  cs-=cfg.dpl; memcpy(b+cs,cfg.data_path,cfg.dpl);
  cs--; b[cs]=0;
  co=o; 
  while(co->tag!=T_WORLD) 
   { ms=strlen(co->dn); cs-=ms; memcpy(b+cs,co->dn,ms); cs--; b[cs]='\\'; co=co->pl; }
  cs-=cfg.dpl; memcpy(b+cs,cfg.data_path,cfg.dpl);

  obj_save_dat[o->tag](o,1,b+ts,ds);

  tr_end=dtr;
  dtr=mymalloc(sizeof(tr_list));
 }
 // сохраняем динамические параметры
  tr_end->next=dtr; dtr->next=NULL; dtr->num=tr_end->num+1;

  ts=4; // размер типа транзакции
  ts+=cfg.dpl+1;
  co=o; while(co->tag!=T_WORLD) { ts+=strlen(co->dn)+1; co=co->pl; }
  ts+=sizeof("dynamic.dat");
  ds=obj_save_dat[o->tag](o,0,NULL,0); // найдем длину сохраняемых данных объекта
 
  b=mymalloc(ts+ds); bi=(int *)b; bi[0]=TR_UPDATE; dtr->tr_size=ts+ds; dtr->tr=b;

  cs=ts; ms=sizeof("dynamic.dat"); cs-=ms; memcpy(b+cs,"dynamic.dat",ms); cs--; b[cs]='\\';
  co=o; 
  while(co->tag!=T_WORLD) 
   { ms=strlen(co->dn); cs-=ms; memcpy(b+cs,co->dn,ms); cs--; b[cs]='\\'; co=co->pl; }
  cs-=cfg.dpl; memcpy(b+cs,cfg.data_path,cfg.dpl);

  obj_save_dat[o->tag](o,0,b+ts,ds);

  tr_end=dtr;
}

void obj_save_create(obj *o)
{
 obj_save(o,1);
}

void obj_save_update(obj *o)
{
 obj_save(o,0);
}
