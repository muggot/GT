// формирование команд для отправки клиенту
#include "gt.h"

char nam[ONAM_LEN*2];

// подготовка команды для отправки клиенту
void cmd_send_prep(char *cmd, int len, obj *o)
{
 evt *e;
 lnk *l;

 if(o==NULL) { myfree(cmd); return; }

 if(o->nb.sb==NULL)
  { e=get_evt(); evt_func_set[E_send_nif](e,o,NULL,NULL,0.0); ins_evt(o,e); }

 l=get_lnk(); l->cp=0; l->n=len; l->b=cmd; l->prev=NULL; l->next=o->nb.sb; 
 if(o->nb.sb!=NULL) o->nb.sb->prev=l; else o->nb.se=l;
 o->nb.sb=l;
}

// команда HELLO - приглашение к аутотентификации
void cmd_send_hello(obj *o)
{
 int n;
 char *b;

 n=snprintf(NULL,0,"<HELLO tm=\"%f\" />",GTIME)+1; b=mymalloc(n); snprintf(b,n,"<HELLO tm=\"%f\" />",GTIME);
 cmd_send_prep(b,n,o);
}

// команда IAM - полное состояние персонажа клиента
void cmd_send_iam(obj *o)
{
 int n;
 char *b;
 obj *no=o->nif;
 obj *co;
 lnk *l;

 if(no==NULL) return;

 co=o->pl; while(co!=NULL) { cmd_send_obj(no,co); co=co->pl; }

 n=0; b=o->dn; while(b[n]!=0) { sprintf(&nam[n*2],"%.2x",b[n]); n++; } nam[n*2]=0;
 n=snprintf(NULL,0,"<IAM tag=\"%d\" id=\"%x\" nam=\"%s\" img=\"%d\" pid=\"%x\" bside=\"%d\" xs=\"%f\" ys=\"%f\" >",o->tag,o,nam,o->s.img,o->pl,o->bside,o->s.xs,o->s.ys)+1; b=mymalloc(n); 
 snprintf(b,n,"<IAM tag=\"%d\" id=\"%x\" nam=\"%s\" img=\"%d\" pid=\"%x\" bside=\"%d\" xs=\"%f\" ys=\"%f\" >",o->tag,o,nam,o->s.img,o->pl,o->bside,o->s.xs,o->s.ys);
 cmd_send_prep(b,n-1,no);
  n=snprintf(NULL,0,"<DEV m=\"%f,%f\" hull=\"%f,%f\" p=\"%f,%f,%f,%f\" />",o->dev.m,o->dev.cm,o->dev.hull,o->dev.chull,o->dev.pwr[0],o->dev.pwr[1],o->dev.pwr[2],o->dev.pwr[3],o->dev.pwr[4])+1; b=mymalloc(n); 
  snprintf(b,n,"<DEV m=\"%f,%f\" hull=\"%f,%f\" p=\"%f,%f,%f,%f\" />",o->dev.m,o->dev.cm,o->dev.hull,o->dev.chull,o->dev.pwr[0],o->dev.pwr[1],o->dev.pwr[2],o->dev.pwr[3],o->dev.pwr[4]);
  cmd_send_prep(b,n-1,no);
 n=snprintf(NULL,0,"</IAM>")+1; b=mymalloc(n); snprintf(b,n,"</IAM>");
 cmd_send_prep(b,n-1,no);
 n=snprintf(NULL,0,"<MV id=\"%x\" tm=\"%f\" t=\"%f\" x=\"%f\" y=\"%f\" v=\"%f\" tx=\"%f\" ty=\"%f\" />",o,GTIME,o->s.ct,o->s.cx,o->s.cy,o->dev.pwr[0]/o->dev.cm,o->s.tx,o->s.ty)+1; b=mymalloc(n); 
 snprintf(b,n,"<MV id=\"%x\" tm=\"%f\" t=\"%f\" x=\"%f\" y=\"%f\" v=\"%f\" tx=\"%f\" ty=\"%f\" />",o,GTIME,o->s.ct,o->s.cx,o->s.cy,o->dev.pwr[1]/o->dev.cm,o->s.tx,o->s.ty);
 cmd_send_prep(b,n-1,no);

 co=o->bo;
 cmd_send_cobj(no,co,1);
/*
while(co!=NULL)
 {
  n=snprintf(NULL,0,"<COBJ id=\"%x\" tag=\"%d\" pid=\"%x\" img=\"%d\" t=\"%d\" >",co,co->tag,co->pl,co->s.img,co->s.ct)+1; b=mymalloc(n); 
  snprintf(b,n,"<COBJ id=\"%x\" tag=\"%d\" pid=\"%x\" img=\"%d\" t=\"%d\" >",co,co->tag,co->pl,co->s.img,co->s.ct);
  cmd_send_prep(b,n-1,no);
   n=snprintf(NULL,0,"<DEV m=\"%f,%f\" hull=\"%f,%f\" p=\"%f,%f,%f,%f\" />",co->dev.m,co->dev.cm,co->dev.hull,co->dev.chull,co->dev.pwr[0],co->dev.pwr[1],co->dev.pwr[2],co->dev.pwr[3],co->dev.pwr[4])+1; b=mymalloc(n); 
   snprintf(b,n,"<DEV m=\"%f,%f\" hull=\"%f,%f\" p=\"%f,%f,%f,%f\" />",co->dev.m,co->dev.cm,co->dev.hull,co->dev.chull,co->dev.pwr[0],co->dev.pwr[1],co->dev.pwr[2],co->dev.pwr[3],co->dev.pwr[4]);
   cmd_send_prep(b,n-1,no);
   if(co->dev.mysl!=NULL)
   {
    n=snprintf(NULL,0,"<MYSL slot=\"%c%d_%d\" />",co->dev.mysl->st[0],co->dev.mysl->sc,co->dev.mysl->sn)+1; b=mymalloc(n); 
    snprintf(b,n,"<MYSL slot=\"%c%d_%d\" />",co->dev.mysl->st[0],co->dev.mysl->sc,co->dev.mysl->sn);
    cmd_send_prep(b,n-1,no);
   }
   l=co->dev.dvsl;
   while(l!=NULL)
   {
    n=snprintf(NULL,0,"<DVSL id=\"%c%d_%d\" obj=\"%x\" />",l->st[0],l->sc,l->sn,l->o)+1; b=mymalloc(n); 
    snprintf(b,n,"<DVSL id=\"%c%d_%d\" obj=\"%x\" />",l->st[0],l->sc,l->sn,l->o);
    cmd_send_prep(b,n-1,no);
    l=l->next;
   }
   l=co->dev.rqsl;
   while(l!=NULL)
   {
    n=snprintf(NULL,0,"<RQSL id=\"%c%d_%d\" />",l->st[0],l->sc,l->sn)+1; b=mymalloc(n); 
    snprintf(b,n,"<RQSL id=\"%c%d_%d\" />",l->st[0],l->sc,l->sn);
    cmd_send_prep(b,n-1,no);
    l=l->next;
   }
  n=snprintf(NULL,0,"</COBJ>")+1; b=mymalloc(n); snprintf(b,n,"</COBJ>");
  co=co->next; if(co!=NULL) n--;
  cmd_send_prep(b,n,no);
 }
*/
}

void cmd_send_cobj(obj *no,obj *co,int eflag)
{
 int n;
 char *b;
 lnk *l;

 while(co!=NULL)
 {
  n=0; if(co->tag!=T_PERS) b=co->name; else b=co->dn;
  while(b[n]!=0) { sprintf(&nam[n*2],"%.2x",b[n]); n++; } nam[n*2]=0;
  n=snprintf(NULL,0,"<COBJ id=\"%x\" tag=\"%d\" nam=\"%s\" pid=\"%x\" img=\"%d\" t=\"%d\" >",co,co->tag,nam,co->pl,co->s.img,co->s.ct)+1; b=mymalloc(n); 
  snprintf(b,n,"<COBJ id=\"%x\" tag=\"%d\" nam=\"%s\" pid=\"%x\" img=\"%d\" t=\"%d\" >",co,co->tag,nam,co->pl,co->s.img,co->s.ct);
  cmd_send_prep(b,n-1,no);
   n=snprintf(NULL,0,"<DEV m=\"%f,%f\" hull=\"%f,%f\" p=\"%f,%f,%f,%f\" />",co->dev.m,co->dev.cm,co->dev.hull,co->dev.chull,co->dev.pwr[0],co->dev.pwr[1],co->dev.pwr[2],co->dev.pwr[3],co->dev.pwr[4])+1; b=mymalloc(n); 
   snprintf(b,n,"<DEV m=\"%f,%f\" hull=\"%f,%f\" p=\"%f,%f,%f,%f\" />",co->dev.m,co->dev.cm,co->dev.hull,co->dev.chull,co->dev.pwr[0],co->dev.pwr[1],co->dev.pwr[2],co->dev.pwr[3],co->dev.pwr[4]);
   cmd_send_prep(b,n-1,no);
   if(co->dev.mysl!=NULL)
   {
    n=snprintf(NULL,0,"<MYSL slot=\"%c%d_%d\" />",co->dev.mysl->st[0],co->dev.mysl->sc,co->dev.mysl->sn)+1; b=mymalloc(n); 
    snprintf(b,n,"<MYSL slot=\"%c%d_%d\" />",co->dev.mysl->st[0],co->dev.mysl->sc,co->dev.mysl->sn);
    cmd_send_prep(b,n-1,no);
   }
   l=co->dev.dvsl;
   while(l!=NULL)
   {
    n=snprintf(NULL,0,"<DVSL id=\"%c%d_%d\" obj=\"%x\" />",l->st[0],l->sc,l->sn,l->o)+1; b=mymalloc(n); 
    snprintf(b,n,"<DVSL id=\"%c%d_%d\" obj=\"%x\" />",l->st[0],l->sc,l->sn,l->o);
    cmd_send_prep(b,n-1,no);
    l=l->next;
   }
   l=co->dev.rqsl;
   while(l!=NULL)
   {
    n=snprintf(NULL,0,"<RQSL id=\"%c%d_%d\" />",l->st[0],l->sc,l->sn)+1; b=mymalloc(n); 
    snprintf(b,n,"<RQSL id=\"%c%d_%d\" />",l->st[0],l->sc,l->sn);
    cmd_send_prep(b,n-1,no);
    l=l->next;
   }
  cmd_send_cobj(no,co->bo,0); // внимание!!! рекурсия
  n=snprintf(NULL,0,"</COBJ>")+1; b=mymalloc(n); snprintf(b,n,"</COBJ>");
  co=co->next; if(co!=NULL || eflag==0) n--;
  cmd_send_prep(b,n,no);
 }
}

void cmd_send_mnt(obj *o,obj *mo)
{
 int n;
 char *b;
 n=snprintf(NULL,0,"<MNT slot=\"%c%d_%d\" obj=\"%x\" >",mo->dev.mysl->st[0],mo->dev.mysl->sc,mo->dev.mysl->sn,mo)+1; b=mymalloc(n); 
 snprintf(b,n,"<MNT slot=\"%c%d_%d\" obj=\"%x\" >",mo->dev.mysl->st[0],mo->dev.mysl->sc,mo->dev.mysl->sn,mo);
 cmd_send_prep(b,n,o->nif);
}

void cmd_send_umnt(obj *o,obj *mo)
{
 int n;
 char *b;
 n=snprintf(NULL,0,"<UMNT obj=\"%x\" >",mo)+1; b=mymalloc(n); 
 snprintf(b,n,"<UMNT obj=\"%x\" >",mo);
 cmd_send_prep(b,n,o->nif);
}

// команда HUI - отказ в регистрации
void cmd_send_hui(obj *o)
{
 int n;
 char *b;

 n=snprintf(NULL,0,"<HUI />")+1; b=mymalloc(n); snprintf(b,n,"<HUI />");
 cmd_send_prep(b,n,o);
}

// команда LOC - список объектов локации
void cmd_send_loc(obj *lo,obj *o)
{

}

void cmd_send_obj(obj *no,obj *so)
{
 int n;
 char *b;

 n=0; if(so->tag!=T_PERS) b=so->name; else b=so->dn;
 while(b[n]!=0) { sprintf(&nam[n*2],"%.2x",b[n]); n++; } nam[n*2]=0;
 n=snprintf(NULL,0,"<OBJ tag=\"%d\" id=\"%x\" nam=\"%s\" pid=\"%x\" bside=\"%d\" img=\"%d\" xs=\"%f\" ys=\"%f\" />",so->tag,so,nam,so->pl,so->bside,so->s.img,so->s.xs,so->s.ys)+1; b=mymalloc(n); 
 snprintf(b,n,"<OBJ tag=\"%d\" id=\"%x\" nam=\"%s\" pid=\"%x\" bside=\"%d\" img=\"%d\" xs=\"%f\" ys=\"%f\" />",so->tag,so,nam,so->pl,so->bside,so->s.img,so->s.xs,so->s.ys);
 cmd_send_prep(b,n,no);
}

void cmd_send_dobj(obj *o,obj *so)
{
 int n;
 char *b;
 if(o->tag!=T_PERS || o->nif==NULL) return;
 n=snprintf(NULL,0,"<DOBJ id=\"%x\" />",so)+1; b=mymalloc(n); 
 snprintf(b,n,"<DOBJ id=\"%x\" />",so);
 cmd_send_prep(b,n,o->nif);
}

void cmd_send_mv(obj *no,obj *so)
{
 int n;
 char *b;
 n=snprintf(NULL,0,"<MV id=\"%x\" tm=\"%f\" t=\"%f\" x=\"%f\" y=\"%f\" v=\"%f\" tx=\"%f\" ty=\"%f\" />",so,GTIME,so->s.ct,so->s.cx,so->s.cy,so->s.v,so->s.tx,so->s.ty)+1; b=mymalloc(n); 
 snprintf(b,n,"<MV id=\"%x\" tm=\"%f\" t=\"%f\" x=\"%f\" y=\"%f\" v=\"%f\" tx=\"%f\" ty=\"%f\" />",so,GTIME,so->s.ct,so->s.cx,so->s.cy,so->s.v,so->s.tx,so->s.ty);
 cmd_send_prep(b,n,no);
}

void cmd_send_pobj_obj(obj *o,obj *so)
{
 int n;
 char *b;
 obj *co;
// информирую местных о прибытии и новичка об местных
 co=so->bo;
 while(co!=NULL)
 {
  if(co->tag==T_PERS && co->nif!=NULL) cmd_send_obj(co->nif,o);
  if(o->tag==T_PERS && o->nif!=NULL) cmd_send_obj(o->nif,co); 
  co=co->next;
 }
}

void cmd_send_prich(obj *o,obj *so)
{
 int n;
 char *b;
 obj *co;

 if(o->nif==NULL) return;

//о может быть далеко от so, нужно передать инфу по so
 cmd_send_obj(o->nif,so); 

// сначала пусть o увидит внутренности so
 n=snprintf(NULL,0,"<PRICH id=\"%x\" />",so)+1; b=mymalloc(n); 
 snprintf(b,n,"<PRICH id=\"%x\" />",so);
 cmd_send_prep(b,n,o->nif);

 cmd_send_pobj_obj(o,so);
}

void cmd_send_pobj_dobj(obj *o)
{
 int n;
 char *b;
 obj *co;

// информирую местных об убытии объекта
 co=o->pl->bo;
 while(co!=NULL)
 {
  if(co!=o && (co->tag==T_PERS && co->nif!=NULL)) cmd_send_dobj(co,o);
  co=co->next;
 }
}

void cmd_send_mymv(obj *so)
{
 int n;
 char *b;
 obj *no=so->nif;
 if(no==NULL) return;

 n=snprintf(NULL,0,"<MV id=\"%x\" tm=\"%f\" t=\"%f\" x=\"%f\" y=\"%f\" v=\"%f\" tx=\"%f\" ty=\"%f\" />",so,GTIME,so->s.ct,so->s.cx,so->s.cy,so->dev.pwr[0]/so->dev.cm,so->s.tx,so->s.ty)+1; b=mymalloc(n); 
 snprintf(b,n,"<MV id=\"%x\" tm=\"%f\" t=\"%f\" x=\"%f\" y=\"%f\" v=\"%f\" tx=\"%f\" ty=\"%f\" />",so,GTIME,so->s.ct,so->s.cx,so->s.cy,so->dev.pwr[0]/so->dev.cm,so->s.tx,so->s.ty);
 cmd_send_prep(b,n,no);
}

void cmd_send_obj_loc(obj *o,lnk *ol)
{
 obj *co=o->nif;
 obj *so;

 while(ol!=NULL)
 {
  so=ol->o; ol=ol->next; if(so==o) continue;
  if(o->tag==T_PERS && co!=NULL) { cmd_send_obj(co,so); cmd_send_mv(co,so); }

  if(so->tag!=T_PERS || so->nif==NULL) continue;
  cmd_send_obj(so->nif,o); cmd_send_mv(so->nif,o);
 }
}

void cmd_send_dobj_loc(obj *o,lnk *ol)
{
 obj *so;

 while(ol!=NULL)
 {
  so=ol->o; ol=ol->next; if(so==o) continue;
  if(so->tag!=T_PERS || so->nif==NULL) continue;
  cmd_send_dobj(so,o);
 }
}

void cmd_send_mv_loc(obj *o,lnk *ol)
{
 obj *co=o->nif;
 obj *so;

 while(ol!=NULL)
 {
  so=ol->o; ol=ol->next; if(so==o) continue;

  if(so->tag!=T_PERS || so->nif==NULL) continue;
  cmd_send_mv(so->nif,o);
 }
}

void cmd_send_myobj_loc(obj *o,lnk *ol)
{
 obj *co=o->nif;
 obj *so;

 while(ol!=NULL)
 {
  so=ol->o; ol=ol->next;

  if(so->tag!=T_PERS || so->nif==NULL) continue;
  cmd_send_obj(so->nif,o);
 }
}

void locs_send_msg(obj *o,char *msg,int len)
{
 obj *wo,*so;
 lnk *ol;
 int i,j,locn;
 int x1,x2,y1,y2;
 double dr,cx,cy;
 char *bc;

 if(o->nif!=NULL) { bc=mymalloc(len); memcpy(bc,msg,len); cmd_send_prep(bc,len,o->nif); }
 // найдем родительский объект - WORLD
 if(o->locl==NULL) return;
 wo=o->locl->o;

 dr=(GTIME-o->s.ct)*o->s.v; cx=o->s.cx+o->s.tx*dr; cy=o->s.cy+o->s.ty*dr;

 x2=(cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { 
    locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; 
    while(ol!=NULL) 
     {  
      so=ol->o; ol=ol->next;
      if(so->nif!=NULL && so->pl!=o->pl) 
       { bc=mymalloc(len); memcpy(bc,msg,len); cmd_send_prep(bc,len,so->nif); }
     }
   }
}

// результат использования оружия рассылается всем участникам боя
void cmd_send_buse(obj *bo, char *cmd, int len)
{
 obj *co=bo->bo;
 char *bc;

 while(co!=NULL) 
 {  
  if(co->nif!=NULL) 
   { bc=mymalloc(len); memcpy(bc,cmd,len); cmd_send_prep(bc,len,co->nif); }
  co=co->next;
 }
}

void cmd_send_bl_begin(obj *o)
{
 int n,s;
 char *b,*bc;
 lnk *lb,*l;
 obj *co,*no;
 lnk *nl;


 lb=l=get_lnk(); s=0;
// всем участникам боя и просто наблюдателям нужно отправить измененные pid и сторону объектов
 co=o->bo;
 while(co!=NULL)
 {
  n=snprintf(NULL,0,"<OBJ id=\"%x\" pid=\"%x\" bside=\"%d\" />",co,o,co->bside)+1; b=mymalloc(n); 
  snprintf(b,n,"<OBJ id=\"%x\" pid=\"%x\" bside=\"%d\" />",co,o,co->bside);
  l->next=get_lnk(); l=l->next; l->n=n-1; l->b=b; l->next=NULL; s+=n-1;
  co=co->next;
 }
 l->n=n; s++;

 b=mymalloc(s);
 l=lb->next; free_lnk(NULL,lb); n=0;
 while(l!=NULL) 
 {
  memcpy(b+n,l->b,l->n); n+=l->n; myfree(l->b); lb=l; l=l->next; free_lnk(NULL,lb);
 }

 co=o->bo;
 while(co!=NULL)
 {
  if(co->bside!=0) locs_send_msg(co,b,s);
  co=co->next;
 }
 myfree(b);

// всем участникам боя нужно разослать урезанные структуры слотов COBJ
 lb=l=get_lnk(); s=0;
 co=o->bo;
 while(co!=NULL)
 {
  if(co->bside==0) {co=co->next; continue;}
  
  nl=co->dev.dvsl;
  while(nl!=NULL)
  {
   if(nl->o!=NULL)
   {
    n=snprintf(NULL,0,"<COBJ id=\"%x\" pid=\"%x\" img=\"%x\" />",nl->o,co,nl->o->s.img)+1; b=mymalloc(n); 
    snprintf(b,n,"<COBJ id=\"%x\" pid=\"%x\" img=\"%x\" />",nl->o,co,nl->o->s.img);
    l->next=get_lnk(); l=l->next; l->n=n-1; l->b=b; l->next=NULL; s+=n-1;
   }
   nl=nl->next;
  }
  co=co->next;
 }
 n=snprintf(NULL,0,"<BB id=\"%x\" />",o)+1; b=mymalloc(n); snprintf(b,n,"<BB id=\"%x\" />",o);
 l->next=get_lnk(); l=l->next; l->n=n; l->b=b; l->next=NULL; s+=n;
// l->n=n; s++;

 b=mymalloc(s);
 l=lb->next; free_lnk(NULL,lb); n=0;
 while(l!=NULL) 
 {
  memcpy(b+n,l->b,l->n); n+=l->n; myfree(l->b); lb=l; l=l->next; free_lnk(NULL,lb);
 }

 cmd_send_buse(o,b,s); myfree(b);
}


// рассылает сообщение всем видимым объектам в космосе
void locs_send_msg1(obj *o,char *msg,int len)
{
 obj *wo,*so;
 lnk *ol;
 int i,j,locn;
 int x1,x2,y1,y2;
 double dr,cx,cy;
 char *bc;

 if(o->locl==NULL) return;
 wo=o->locl->o;

 dr=(GTIME-o->s.ct)*o->s.v; cx=o->s.cx+o->s.tx*dr; cy=o->s.cy+o->s.ty*dr;

 x2=(cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { 
    locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; 
    while(ol!=NULL) 
     {  
      so=ol->o; ol=ol->next;
      if(so->nif!=NULL) 
       { bc=mymalloc(len); memcpy(bc,msg,len); cmd_send_prep(bc,len,so->nif); }
     }
   }
}


void cmd_send_bl_begin1(obj *o)
{
 int n,s;
 char *b;
 lnk *lb,*l;
 lnk *nl;


 n=snprintf(NULL,0,"<OBJ id=\"%x\" pid=\"%x\" bside=\"%d\" />",o,o->pl,o->bside)+1; b=mymalloc(n); 
 snprintf(b,n,"<OBJ id=\"%x\" pid=\"%x\" bside=\"%d\" />",o,o->pl,o->bside);
 locs_send_msg1(o,b,n);

// всем участникам боя нужно разослать урезанные структуры слотов COBJ
 lb=l=get_lnk(); s=0;
 nl=o->dev.dvsl;
 while(nl!=NULL)
 {
  if(nl->o!=NULL)
  {
   n=snprintf(NULL,0,"<COBJ id=\"%x\" pid=\"%x\" img=\"%x\" />",nl->o,o,nl->o->s.img)+1; b=mymalloc(n); 
   snprintf(b,n,"<COBJ id=\"%x\" pid=\"%x\" img=\"%x\" />",nl->o,o,nl->o->s.img);
   l->next=get_lnk(); l=l->next; l->n=n-1; l->b=b; l->next=NULL; s+=n-1;
  }
  nl=nl->next;
 }
 l->n=n; s++;

 b=mymalloc(s);
 l=lb->next; free_lnk(NULL,lb); n=0;
 while(l!=NULL) 
 {
  memcpy(b+n,l->b,l->n); n+=l->n; myfree(l->b); lb=l; l=l->next; free_lnk(NULL,lb);
 }
 cmd_send_buse(o->pl,b,s); myfree(b);

 n=snprintf(NULL,0,"<BB id=\"%x\" />",o->pl)+1; b=mymalloc(n); snprintf(b,n,"<BB id=\"%x\" />",o->pl);
 cmd_send_prep(b,n,o->nif);
}


void cmd_send_bl_update(obj *fo)
{
 int n,s;
 char *b,*bc;
 lnk *lb,*l;
 obj *co,*o=fo->pl;
 lnk *nl;

// всем участникам боя нужно разослать урезанные структуры слотов COBJ
 lb=l=get_lnk(); s=0;
 co=o->bo;
 while(co!=NULL)
 {
  if(co->bside==0) {co=co->next; continue;}
  
  nl=co->dev.dvsl;
  while(nl!=NULL)
  {
   if(nl->o!=NULL)
   {
    n=snprintf(NULL,0,"<COBJ id=\"%x\" pid=\"%x\" img=\"%x\" />",nl->o,co,nl->o->s.img)+1; b=mymalloc(n); 
    snprintf(b,n,"<COBJ id=\"%x\" pid=\"%x\" img=\"%x\" />",nl->o,co,nl->o->s.img);
    l->next=get_lnk(); l=l->next; l->n=n-1; l->b=b; l->next=NULL; s+=n-1;
   }
   nl=nl->next;
  }
  co=co->next;
 }
 l->n=n; s++;

 b=mymalloc(s);
 l=lb->next; free_lnk(NULL,lb); n=0;
 while(l!=NULL) 
 {
  memcpy(b+n,l->b,l->n); n+=l->n; myfree(l->b); lb=l; l=l->next; free_lnk(NULL,lb);
 }

 cmd_send_prep(b,s,fo->nif);
}

void cmd_send_chat(obj *o,char *txt)
{
 int n;
 char *b;
 if(o->tag!=T_PERS || o->nif==NULL) return;
 n=snprintf(NULL,0,"<CHAT txt=\"%s\" />",txt)+1; b=mymalloc(n); 
 snprintf(b,n,"<CHAT txt=\"%s\" />",txt);
 cmd_send_prep(b,n,o->nif);
}

// посылает строку чата в локации, где стоит перс
void cmd_send_chat_broadcast(obj *o, char *txt)
{
 obj *so,*wo;
 lnk *ol;
 lnk *l=o->locl;

 wo=o->pl;

 if(wo->tag==T_WORLD)
 {
  while(l!=NULL)
  {
   ol=wo->locs[l->ln].ol;
   while(ol!=NULL) {  so=ol->o; ol=ol->next; cmd_send_chat(so,txt); }
   l=l->next;
  }
  return;
 }

 so=wo->bo;
 while(so!=NULL)
 {
  if(so->tag==T_PERS && so->nif!=NULL) cmd_send_chat(so,txt);
  so=so->next;
 }
}


