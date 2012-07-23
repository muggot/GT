#include "gt.h"
#include "res_mx.h"
#define		M_PI	3.1415926535897932384626433832795

// ��������� ᯠમ�

void evt_spark_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_spark;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
 o->s.me=ne;
}

// ᯠ� ����� ����� �ਪ�� ��⪨
// 0 ࠡ���� � ��⭮� ०���
// 1 ����⥫�� ���������� � 㫥�

// ᯠ� ���������� ����� � �ਭ����� �襭��

void evt_spark_act(obj *o,evt *e,lnk *l)
{
// printf("evt_spark_act\n");
 del_evt(o,e); free_evt(o,e);

 if(o->pl->tag==T_BL) { spark_act_btl(o,e,l); return; } // ᯠ� � ���

 if(o->s.img==1) spark_matka_act(o);
 if(o->s.img==2) spark_rab_act(o);
 if(o->s.img==3) spark_spy_act(o);
 if(o->s.img==4) spark_fighter_act(o);
}


void spark_selfkill(obj *o) // �६� ����� ���稫���
{
 int i;
 evt *e,*de;
 lnk *l,*dl;
 obj *co;

 if(o->pl->tag==T_WORLD) loc_obj_unreg(o);

 co=o->bo; while(co!=NULL) { spark_selfkill(co); co=o->bo; }

// 㤠��� ��।� ᮡ�⨩
 e=o->bevt; if(e!=NULL) e=e->next;
 while(e!=NULL) 
  { de=e->next; del_evt(o,e); free_evt(o,e); e=de; }
 e=o->bevt; if(e!=NULL) { del_evt(o,e); free_evt(o,e); }

// 㤠��� ᯨ᮪ ��뫮� �� ����ᨬ� ᮡ���
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
 
// 㤠��� �����
 if(o->mem!=NULL) myfree(o->mem);

// 㤠��� ��ꥪ�
  del_obj(o); free_obj(o);
}

// ��� �ਭ��� �襭�� ᯠમ� �ᯮ������� ᫥���騥 ��ࠬ����
// ���⮯�������� 0 - � ��, 1 - � ����࠭�⢥, 2 - � ���ந��
// ����� ᨫ 0 - �����, 1 - ��ଠ���, 2 - ��� �� ��ଥ���, 3 - 㦥 �� ��������
// ����㧪� 0 - ���⮩, 1 - �� �����, 2 - �����

// �������� �襭�� ��� �᪠⥫� 
// 0. 00X - ࠧ��㧪� � ��室 �� ���� ����
// 1. 01X,02X,03X - ࠧ��㧪� � ��⠭��
// 2. 100,101,110,111 - �롮� ������� � �����
// 3. 102,112,12X - �����饭�� � 㫥�
// 4. 130 - ����� ᨣ���� ����⢨�
// 5. 131,132 - ��⠭�� �� १�ࢠ � ��� ���ꥤ����� �ᮢ
// 6. 200,201,210,211 - ������
// 7. 202,212,22X - �� ��室 � �㫥� � 㫥�
// 8. 23X - �஢�ઠ १�ࢠ � ������

unsigned char spy_act[3][4][3]=
{
 {{0,0,0},{1,1,1},{1,1,1},{1,1,1}},
 {{2,2,3},{2,2,3},{3,3,3},{4,5,5}},
 {{6,6,7},{6,6,7},{7,7,7},{8,8,8}}
};

int get_spark_massa(obj *o)
{
 obj *co=o->bo,*ro;
 while(co!=NULL)
 {
  if(co->tag==T_CRGO)
  {
   if(co->dev.cm==0) return 0;
   else if(co->dev.cm>=co->dev.pwr[1]*0.9) return 2;
   else return 1;
  }
  co=co->next;
 }
 return 0;
}

int get_spark_mesto(obj *o)
{
 if(o->pl->tag==T_UL) return 0;
 if(o->pl->tag==T_AS) return 2;
 return 1;
}

int get_spark_golod(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL;
 float v,dr,dx,dy,tr;
 co=o->bo;
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || eng_o==NULL) { printf("� ᯠઠ ���� FTN ��� ENG\n"); return 3; }

 if(ftn_o->dev.pwr[0]==ftn_o->dev.pwr[1]) return 0;

 if(o->mem->hname[0]!=NULL)
  { dx=o->mem->hx-o->s.cx; dy=o->mem->hy-o->s.cy; } // ����ﭨ� �� ����
 else // � ᯠઠ ��� UL, �㦭� ��室��� �� ����ﭨ� �� ������襣� �ꥤ������ AS
  { dx=o->mem->p[0][0]-o->s.cx; dy=o->mem->p[0][1]-o->s.cy; }

 tr=ftn_o->dev.pwr[1]/eng_o->dev.pwr[3]; // �� ����� �६� 墠�� ⮯����
 dr=dx*dx+dy*dy; tr=tr*tr*o->s.v*o->s.v/dr;
 if(tr<1.0) return 3;
 if(tr<4.0) return 2;
 return 1;
}

void spark_act_ULrazgruz(obj *o)
{
 obj *co,*to,*fo,*crgo_from=NULL,*crgo_to=NULL;

// printf("spark_act_ULrazgruz\n");
 co=o->bo;
 while(co!=NULL) { if(co->tag==T_CRGO) { crgo_from=co; break; } co=co->next; }
 co=o->pl->bo;
 while(co!=NULL) { if(co->tag==T_CRGO) { crgo_to=co; break; } co=co->next; }
 if(crgo_from==NULL || crgo_to==NULL) return;

 fo=crgo_from->bo;
 while(fo!=NULL) 
 {
  co=crgo_from; while(co->tag!=T_WORLD) { co->dev.cm-=fo->dev.cm; co=co->pl; }
  co=crgo_to; while(co->tag!=T_WORLD) { co->dev.cm+=fo->dev.cm; co=co->pl; } 
  to=crgo_to->bo;
  while(to!=NULL) 
  {
   if(fo->tag==to->tag && fo->s.img==to->s.img && fo->tag==T_RES) // ��� �ࠧ� ��ꥤ��塞
   {
    if(fo->s.img==6) o->pl->mem->d[3]+=fo->dev.cm; // ���ଠ�� ��� ��⪨
    to->dev.m+=fo->dev.m; to->dev.cm+=fo->dev.cm; fo->dev.m=0; fo->dev.cm=0; 
    break;
   }
   to=to->next; 
  }
  if(to==NULL) // ���� ��४���뢠�� �।���
   { co=fo->next; obj_ch_parent(fo,crgo_to); fo=co; }
  else fo=fo->next; 
 }
}

// �᪠⥫ �ᬠ�ਢ��� ������ ����樨 � ��� ������訩 ������� 
// �� ��᫥������� ���ந�, �᫨ ⠪���� ��������� � ��室�� ���ࠢ�����
// ��������, �६� �������� � ��稭��� � ���� ���������.
// �᫨ ���ந�� ��� �᪠⥫ ��砩�� �롨ࠥ� ���ࠢ����� � 
// ��室�� �६� �������� �� ��ࢮ�� ����祭�� � �࠭�楩 ����樨
// ��᫥ ����祭�� �࠭��� ᭮�� �ந������ ���� ���ந�� � �.�.
// �롮� ���ࠢ����� �������� ��।������ ��᫥ ����祭�� � �࠭�楩
// ��।������ ��砩�� ࠧ��ᮬ �⭮�⥫쭮 ���ࠢ����� �� ���,
// 㣮� ࠧ��� ���� �� 90 �ࠤ�ᮢ.
// �᫨ ����譨� UL �����, � �����६���� �饬 ���� UL

void spark_set_ASsearch(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 evt *me;
 int i,locn;
 obj *aso=NULL;
 double dr,fi,cx,cy,tx,ty;

// printf("spark_set_ASsearch\n");
 o->mem->ct=1; o->mem->tt=GTIME;

 // �ᬮ�ਬ ������ ����樨 �� �।��� ������ �������� ���ந���
 while(l!=NULL)
 {
  nlo=wo->locs[l->ln].ol;
  while(nlo!=NULL) // �ன����� �� ᯨ�� ��ꥪ⮢ � ������ ����樨
  {
   if(nlo->o->tag==T_AS && spark_act_AStest(o,nlo->o)==0) // ���, ���ந�, ��ᬮ�ਬ �� ���頫� �� �� ���
   {
    for(i=1;i<6 && o->mem->pid[i]!=(int)nlo->o;i++);
    if(i==6) // �� �� ������, ��, �業�� ���������� � �ਬ�� �襭�� ����� �� �
    {
     fi=(GTIME-nlo->o->s.ct)*nlo->o->s.v; 
     cx=nlo->o->s.cx+nlo->o->s.tx*fi-o->s.cx; 
     cy=nlo->o->s.cy+nlo->o->s.ty*fi-o->s.cy;
     if(aso==NULL || dr>cx*cx+cy*cy) { dr=cx*cx+cy*cy; aso=nlo->o; }
    }
   }
   if(nlo->o->tag==T_UL && o->mem->hname[0]==0) // ���, ��襫 ���� ���, ����� 
   {
    fi=(GTIME-nlo->o->s.ct)*nlo->o->s.v; 
    o->mem->hx=nlo->o->s.cx+nlo->o->s.tx*fi; 
    o->mem->hy=nlo->o->s.cy+nlo->o->s.ty*fi;
    strcpy(o->mem->hname,nlo->o->dn);
   }
   nlo=nlo->next;
  }
  l=l->next;
 }
 if(aso!=NULL) // ���। � 楫�
 {
// ��� ���ந� � ����⢥ 楫�
  fi=(GTIME-aso->s.ct)*aso->s.v; dr=sqrt(dr);
  o->mem->p[0][0]=aso->s.cx+aso->s.tx*fi; o->mem->p[0][1]=aso->s.cy+aso->s.ty*fi;
  o->mem->pid[0]=(int)aso;
  cx=o->mem->p[0][0]-o->s.cx; cy=o->mem->p[0][1]-o->s.cy;
  o->s.tx=cx/dr; o->s.ty=cy/dr;
// ��ࠬ���� �������� ��।�����, ��⠭�������� ᮡ�⨥
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,dr/o->s.v); ins_evt(o,me);
  return;
 }
// ��⨬ �� ����� ����樨
 if(o->s.cx==o->mem->hx && o->s.cy==o->mem->hy)
 {
  fi=(2.0*M_PI*rand())/RAND_MAX; o->s.tx=cos(fi); o->s.ty=sin(fi);
 }
 else
 {
  tx=o->s.cx-o->mem->hx; ty=o->s.cy-o->mem->hy;
  fi=1.0/sqrt(tx*tx+ty*ty); tx*=fi; ty*=fi;
  fi=(M_PI*rand())/RAND_MAX-0.5*M_PI;
  o->s.tx=tx*cos(fi)-ty*sin(fi); o->s.ty=tx*sin(fi)+ty*cos(fi); 
 }
// ��ࠬ���� �������� ��।�����, ��⠭�������� ᮡ�⨥
 dr=get_loc_col_time(o)+1.0;
// ��� �㦭� �ਪ����� ��� ������ ᯠ� ������ � 墠�� �� ��� ᨫ �� �����饭��
// ��宦� ��� �㦭� ���� �����⭮� �ࠢ�����, � ����
// ���⮬� ����� ��孥�
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dr); ins_evt(o,me);
}

void spark_set_ASexit(obj *o)
{
 double dr,cx,cy;
 evt *me;

// printf("spark_set_ASexit\n");
 dr=(GTIME-o->pl->s.ct)*o->pl->s.v; 
 cx=o->pl->s.cx+o->pl->s.tx*dr; cy=o->pl->s.cy+o->pl->s.ty*dr;
 o->mem->p[0][0]=cx; o->mem->p[0][1]=cy; o->mem->pid[0]=(int)o->pl;

 if(o->mem->hname[0]==0) // ���� ���, ��� �६���� ����� ����� AS
  { o->mem->hx=cx; o->mem->hy=cy; }

 obj_ch_parent(o,o->pl->pl); // ��宦� �� AS
 o->s.cx=cx; o->s.cy=cy; // ��ᢠ���� ���न���� �� த�⥫�᪮�� ��ꥪ�
 o->s.tx=o->s.ty=0.0; o->s.v=0; o->s.ct=GTIME;
 loc_obj_reg(o,1); // ॣ������� ᯠઠ � �������
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
 o->mem->ct=3; // ᯠ� ���� � 㫥�
}

void spark_set_ULexit(obj *o)
{
 double dr,cx,cy;
 evt *me;

// �஢�ਬ �� ��� �� 㬥���
 if(o->mem->te<GTIME) { spark_selfkill(o); return; } // ����� ᮡ��� �� ��뢠����

// printf("spark_set_ULexit\n");
 dr=(GTIME-o->pl->s.ct)*o->pl->s.v; 
 o->mem->hx=o->pl->s.cx+o->pl->s.tx*dr; o->mem->hy=o->pl->s.cy+o->pl->s.ty*dr;
 strcpy(o->mem->hname,o->pl->dn);
 o->mem->d[0]=o->pl->mem->d[0];

 obj_ch_parent(o,o->pl->pl); // ��宦� �� UL
 o->s.cx=o->mem->hx; o->s.cy=o->mem->hy; // ��ᢠ���� ���न���� �� த�⥫�᪮�� ��ꥪ�
 o->s.tx=o->s.ty=0.0; o->s.v=0; o->s.ct=GTIME;
 loc_obj_reg(o,1); // ॣ������� ᯠઠ � �������
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
 o->mem->ct=1; // ᯠ� ���� � ���ந��
}

// �㦭� ��।����� �६� ��⠭�� � ���� �ਪ��
void spark_set_ULpitanie(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL;
 evt *me;
 float dt;

// printf("spark_set_ULpitanie\n");
 co=o->bo;
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || eng_o==NULL) { printf("� ᯠઠ ���� FTN ��� ENG\n"); exit(0); }

 dt=(ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1])/ftn_o->dev.pwr[2]+1.0;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
 o->mem->ct=0; o->mem->tt=GTIME;
}

obj *get_UL_eda(obj *ulo)
{
 obj *co;
 co=ulo->bo; while(co!=NULL && co->tag!=T_CRGO) co=co->next;
 if(co==NULL) { printf("� �� ���� CRGO\n"); exit(0); }
 co=co->bo; while(co!=NULL && (co->tag!=T_RES || co->s.img!=6)) co=co->next;
 if(co==NULL) { printf("� �� ���� �� ���\n"); exit(0); }
 return co;
}

// �����⠥� १����� ��⠭��
void spark_act_ULpitanie(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL,*eda_o=NULL;
 float v,dr,dx,dy,tr;

 if(o->mem->ct!=0 || o->pl->tag!=T_UL) return;
// printf("spark_act_ULpitanie\n");

 co=o->bo;
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }

 if(ftn_o==NULL || eng_o==NULL) { printf("� ᯠઠ ���� FTN ��� ENG\n"); exit(0); }

 co=get_UL_eda(o->pl);

 dr=(GTIME-o->mem->tt)*ftn_o->dev.pwr[2];
 if(ftn_o->dev.pwr[1]+dr>ftn_o->dev.pwr[0]) dr=ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1];
 ftn_o->dev.pwr[1]+=dr;
 co->dev.m-=dr; co->dev.cm-=dr; co->pl->dev.cm-=dr; // 㬥���� ����� ��� �� ���ந��
 o->pl->mem->d[2]+=dr; // ���ଠ�� �� ��⪨
 o->mem->tt=GTIME;
}


void spark_act_Wgo1(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL;
 co=o->bo;

// printf("spark_act_Wgo1\n");
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || eng_o==NULL) { printf("� ᯠઠ ���� FTN ��� ENG\n"); exit(0); }

 ftn_o->dev.pwr[1]-=(GTIME-o->mem->tt)*eng_o->dev.pwr[3];
 if(ftn_o->dev.pwr[1]<0.0) ftn_o->dev.pwr[1]=0.0;
}

// �����⠥� १����� �����
void spark_act_Wgo(obj *o)
{
 double dr;

 if(o->pl->tag!=T_WORLD && o->pl->tag!=T_BL) return;
// printf("spark_act_Wgo\n");
// 㡠��� ���஢� ᯠઠ
 spark_act_Wgo1(o);

 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.ct=GTIME; o->s.tx=o->s.ty=0.0;
 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm);
 o->mem->tt=GTIME;
}

// �㦭� ������� �⮣� �������
void spark_act_ASkopanie(obj *o)
{
 obj *co=o->bo,*ro;
 double *rm,m,mm=0;
 int i;

 if(o->mem->ct!=2 || o->pl->tag!=T_AS) return;
// printf("spark_act_ASkopanie\n");

 while(co!=NULL)
 {
  if(co->tag==T_CRGO)
  {
// �� ������ 室�� ᯠ� ����� �������� ⮫쪮 ��� ��� ��⮭, �� �������� ���祭��� mem->d[0]
// �� ��襤襥 �६� �� ����� ��।����� ᪮�쪮 �ᮢ ��� ⨯�� �뫮 ��������
// � ������ ���㦭� � ��ᬮ�
   m=(GTIME-o->mem->tt)*co->dev.pwr[2]; // �ᥣ� ��� ��������
   rm=res_mx[0]; ro=o->pl->bo; 
   for(i=0;i<RES_NUM;i++) rm[i]=0.0;
   while(ro!=NULL) // �ன����� �� �ᠬ � ����⠥� ᪮�쪮 ������� ��������
   {
    if(ro->tag==T_RES) 
     { rm[ro->s.img]=ro->dev.cm; mm+=ro->dev.cm; }
    ro=ro->next;
   }
   m/=mm; if(m>1.0) m=1.0; for(i=0;i<RES_NUM;i++) rm[i]*=m;
// ⥯��� ��।���� ����� �� �㤥� ����室���� ���������, � ����� ��⠭����
   mm=0;
   for(i=0;i<RES_NUM;i++)
    {
     if(rm[i]==0.0 && res_mx[o->mem->d[0]][i]!=0.0) // �㦭��� �� ᮢᥬ ���, ���ந� ��ࠡ�⠭, ��� ⮯��� ���
     {
      return;
     }
     if(rm[i]!=0.0 && res_mx[o->mem->d[0]][i]/rm[i]>mm)
      mm=res_mx[o->mem->d[0]][i]/rm[i]; 
    }

// ����㦠� ���ࠡ�⠭�� ��, ����뢠� 諠�, 㡠���� ����� ���ந��
   ro=o->pl->bo; 
   while(ro!=NULL) // 㡠���� ����� ���ந��
   {
    if(ro->tag==T_RES) 
     { ro->dev.cm-=rm[ro->s.img]; ro->pl->dev.cm-=rm[ro->s.img]; }
    ro=ro->next;
   }

   mm=1.0/mm;
   ro=co->bo; 
   while(ro!=NULL) // �ன����� �� �ᠬ � ������ ����� �㦭� ����㧨��
   {
    if(ro->tag==T_RES && ro->s.img==o->mem->d[0]) // �� ������
    {
     m=mm*res_mx[o->mem->d[0]][RES_NUM]*res_mx[o->mem->d[0]][RES_NUM];
     co->dev.cm+=m;
     if(co->dev.cm>co->dev.pwr[1]) 
      { m-=(co->dev.pwr[1]-co->dev.cm); co->dev.cm=co->dev.pwr[1]; }
     ro->dev.cm+=m;
     co->pl->dev.cm+=m;
     break;
    }
    ro=ro->next;
   }

// ��� ����뢠��� ��譥�� �� �㦭� ᮧ���� ���� ��ꥪ� - T_MUSOR
   ro=obj_create_MUSOR(o->pl->pl,o->pl);
// �����⠥� ��⠢訩�� ��, ���쥬 ���� �ᠬ�
   for(i=0;i<RES_NUM;i++) 
   { 
    rm[i]-=mm*res_mx[o->mem->d[0]][i];
    if(rm[i]>0.0) obj_create_RES(ro,i,rm[i]);
   }
   o->mem->tt=GTIME;
   return;
  }
  co=co->next;
 }
}


// �㦭� �������� �६� ����室���� ��� ������� � ���� �ਪ��
// �� �� ॠ��������
void spark_set_ASkopanie(obj *o)
{
 obj *co=o->bo;
 double dt;
 evt *me;
 int i,pid;

// printf("spark_set_ASkopanie\n");
 while(co!=NULL && co->tag!=T_CRGO)  co=co->next;
 if(co==NULL) { printf("� ᯠઠ ���� CRGO\n"); exit(0); }

 if(spark_act_AStest(o,o->pl)!=0)
 {
// ��室�� �� AS � ��ࠥ� � ��� ����
  spark_set_ASexit(o);
  pid=o->mem->pid[0];
  memcpy(o->mem->p[0],o->mem->p[1],sizeof(float)*5*3);
  memcpy(&o->mem->pid[0],&o->mem->pid[1],sizeof(int)*5);
  o->mem->p[5][0]=0.0; o->mem->p[5][1]=0.0; o->mem->pid[5]=pid;
  o->mem->ct=2; o->mem->tt=GTIME;
  return;
 }

 o->mem->ct=2; o->mem->tt=GTIME;
 dt=(co->dev.pwr[1]-co->dev.cm)/co->dev.pwr[2]+1.0;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
}


// ��뢠���� ����� �� UL, �ਪ�� ᯠ�� ����� � UL
void spark_set_ULgo(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 obj *ulo=NULL;
 double dr,dx,dy,dt;
 evt *me;
 int golod,massa;

// printf("spark_set_ULgo\n");
 if(o->mem->hname[0]==0) // த���� ��� ����, ��뢠� ���� ASsearch, ���� ASgo
 {
  golod=get_spark_golod(o);
  massa=get_spark_massa(o);
  if(golod>1 && massa>0) { spark_set_Wpitanie(o); return; } // ��蠥�
  if(golod<2) { spark_set_ASsearch(o); return; } // � ���᪥
  if(golod==2) { spark_set_ASgo(o); return; } // �����頥��� �� ������訩 AS
 }

 dt=1.0/o->s.v;	// ���⭠� ᪮���� ᯠઠ
 dx=o->mem->hx-o->s.cx; dy=o->mem->hy-o->s.cy; dr=sqrt(dx*dx+dy*dy);
 o->s.ct=GTIME; dt*=dr; o->s.tx=dx/dr; o->s.ty=dy/dr;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
 o->mem->tt=GTIME;
 o->mem->ct=3;
}


// ��뢠���� ����� �� AS, �ਪ�� ᯠ�� ����� � AS
void spark_set_ASgo(obj *o)
{
 double dr,dx,dy,dt;
 evt *me;

// printf("spark_set_ASgo\n");
 if(o->mem->p[0][0]==0.0 && o->mem->p[0][1]==0.0)
  { spark_set_ASsearch(o); return; }
 dt=(1.0*o->dev.cm)/o->dev.pwr[1];	// ���⭠� ᪮���� ᯠઠ
 dx=o->mem->p[0][0]-o->s.cx; dy=o->mem->p[0][1]-o->s.cy; dr=sqrt(dx*dx+dy*dy);
 o->s.ct=GTIME; dt*=dr; o->s.tx=dx/dr; o->s.ty=dy/dr;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
 o->mem->tt=GTIME;
 o->mem->ct=1;
}

// �஢���� AS �� ����������
int spark_act_AStest(obj *o, obj *aso)
{
 obj *ro=aso->bo;
 double *rm;
 int i;

 rm=res_mx[0]; for(i=0;i<RES_NUM;i++) rm[i]=0.0;
 while(ro!=NULL) // �ன����� �� �ᠬ
 {
  if(ro->tag==T_RES) rm[ro->s.img]=ro->dev.cm;
  ro=ro->next;
 }
 for(i=0;i<RES_NUM;i++)
 {
  if(rm[i]==0.0 && res_mx[o->mem->d[0]][i]!=0.0) // �㦭��� �� ᮢᥬ ���, ���ந� ��ࠡ�⠭, ��� ⮯��� ���
  {
   return -1;
  }
 }
 return 0;
}

// ���⠥� � AS �� ����室�����
void spark_act_ASenter(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 obj *aso=NULL;
 double d,dr,cx,cy;
 int pid;

// printf("spark_act_ASenter\n");
 cx=o->mem->p[0][0]-o->s.cx; cy=o->mem->p[0][1]-o->s.cy; d=cx*cx+cy*cy;
 if(o->mem->ct!=1 || d>25.0) return;
 
 // �ᬮ�ਬ ������ ����樨 �� �।��� ������ �᪮���� ���ந��
 while(l!=NULL && aso==NULL)
 {
  nlo=wo->locs[l->ln].ol;
  while(nlo!=NULL && aso==NULL) // �ன����� �� ᯨ�� ��ꥪ⮢ � ������ ����樨
  {
   if(nlo->o->tag==T_AS && spark_act_AStest(o,nlo->o)==0) // ���, ���ந�
   {
    if(o->mem->pid[0]==0) // ���ﭨ� ��⠭� � ��᪠, pid �����, �஡�� ����⠭�����
    {
     d=(GTIME-nlo->o->s.ct)*nlo->o->s.v; 
     cx=nlo->o->s.cx+nlo->o->s.tx*d-o->mem->p[0][0]; 
     cy=nlo->o->s.cy+nlo->o->s.ty*d-o->mem->p[0][1];
     if(aso==NULL || dr>cx*cx+cy*cy) { dr=cx*cx+cy*cy; aso=nlo->o; }
    }
    else
    if(o->mem->pid[0]==(int)nlo->o) aso=nlo->o; // �᪮�� ���ந� ������
   }
   nlo=nlo->next;
  }
  l=l->next;
 }

 if(aso==NULL) // �㦭� ���ந� ��祧, �모��� 楫� �� �����
 {
  pid=o->mem->pid[0];
  memcpy(o->mem->p[0],o->mem->p[1],sizeof(float)*5*3);
  memcpy(&o->mem->pid[0],&o->mem->pid[1],sizeof(int)*5);
  o->mem->p[5][0]=0.0; o->mem->p[5][1]=0.0; o->mem->pid[5]=pid;
  return;
 }

 dr=(GTIME-aso->s.ct)*aso->s.v; 
 o->mem->p[0][0]=aso->s.cx+aso->s.tx*dr; o->mem->p[0][1]=aso->s.cy+aso->s.ty*dr;
 cx=o->mem->p[0][0]-o->s.cx; cy=o->mem->p[0][1]-o->s.cy;
 dr=cx*cx+cy*cy;

 if(0.25*(aso->s.xs*aso->s.xs+aso->s.ys*aso->s.ys)>dr) // 㦥 ��� ���ந���, ����� ��室���
  {
   loc_obj_unreg(o); // 㡨�� ॣ������ ��ꥪ� � �������
   obj_ch_parent(o,aso); // ����� � ���ந�
  }
}

// ���⠥� � UL �� ����室�����
void spark_act_ULenter(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 obj *ulo=NULL;
 double dr,cx,cy;
 int i,j;

// printf("spark_act_ULenter\n");
 if(o->mem->ct!=3) return;

// ⥪�饥 ����ﭨ� �� �।����������� ��������� ���
 cx=o->mem->hx-o->s.cx; cy=o->mem->hy-o->s.cy; dr=cx*cx+cy*cy; if(dr>25.0) return;

 // �ᬮ�ਬ ������ ����樨 �� �।��� ������ �쥢
 while(l!=NULL && ulo==NULL)
 {
  nlo=wo->locs[l->ln].ol;
  while(nlo!=NULL && ulo==NULL) // �ன����� �� ᯨ�� ��ꥪ⮢ � ������ ����樨
  {
   if(nlo->o->tag==T_UL) // ���, 㫥�, ��ᬮ�ਬ �� ��� ��
   {
    if(strcmp(o->mem->hname,nlo->o->dn)==0) ulo=nlo->o; // �᪮�� 㫥� ������
   }
   nlo=nlo->next;
  }
  l=l->next;
 }

 if(ulo==NULL) // த��� 㫥� �� ������, �모��� ���� �� �� �� �����
 {
  o->mem->hx=o->mem->p[0][0]; o->mem->hx=o->mem->p[0][1]; o->mem->hname[0]=0; return;
 }

 dr=(GTIME-ulo->s.ct)*ulo->s.v; 
 o->mem->hx=ulo->s.cx+ulo->s.tx*dr; o->mem->hy=ulo->s.cy+ulo->s.ty*dr;
 cx=o->mem->hx-o->s.cx; cy=o->mem->hy-o->s.cy;
 dr=cx*cx+cy*cy;

 if(0.25*(ulo->s.xs*ulo->s.xs+ulo->s.ys*ulo->s.ys)>dr) // 㦥 ��� �쥬, ����� ��室���
 {
  loc_obj_unreg(o); // 㡨�� ॣ������ ��ꥪ� � �������
  obj_ch_parent(o,ulo); // ����� � UL
// �����⥫쭮, ᯠ� ������ � UL, 
// ���न���� ��᫥����� AS ��室���� � o->mem->p[0], ᮮ�騬 � ��� UL, 
// �᫨ ����ﭨ� �� AS �������� �������� � ������
  for(i=0;i<6;i++)
  {
   if(o->mem->pid[i]!=0 && o->mem->p[i][0]==0.0 && o->mem->p[i][1]==0.0)
   {
    for(j=0;j<6;j++)
     if(o->mem->pid[i]==ulo->mem->pid[j])
     {
      if(j!=5)
      {
       memcpy(&ulo->mem->pid[j],&ulo->mem->pid[j+1],sizeof(int)*(5-j));
       memcpy(&ulo->mem->p[j],&ulo->mem->p[j+1],sizeof(float)*3*(5-j));
      }
      ulo->mem->pid[5]=0; ulo->mem->p[5][0]=0.0; ulo->mem->p[5][1]=0.0;
      break;
     }
   }
  }


  if(o->mem->pid[0]!=0)
  {
   for(i=0;i<6;i++) 
    if(o->mem->pid[0]==ulo->mem->pid[i])
    {
     if(i!=5)
     {
      memcpy(&ulo->mem->pid[i],&ulo->mem->pid[i+1],sizeof(int)*(5-i));
      memcpy(&ulo->mem->p[i],&ulo->mem->p[i+1],sizeof(float)*3*(5-i));
     }
     ulo->mem->pid[5]=0; ulo->mem->p[5][0]=0.0; ulo->mem->p[5][1]=0.0;
     break;
    }
  }

  cx=o->mem->p[0][0]-o->mem->hx; cy=o->mem->p[0][1]-o->mem->hy;
  dr=cx*cx+cy*cy;
  for(i=0;i<5;i++)
  {
   cx=ulo->mem->p[i][0]-o->mem->hx; cy=ulo->mem->p[i][1]-o->mem->hy;
   if(dr<cx*cx+cy*cy) // ���� ��, ������ � ᯨ᮪, �᢮����� ���� ��� ����� ���� � ��������
   {
    for(j=4;j>=i;j--) 
    { 
     ulo->mem->p[j+1][0]=ulo->mem->p[j][0]; ulo->mem->p[j+1][1]=ulo->mem->p[j][1];
     ulo->mem->pid[j+1]=ulo->mem->pid[j];
    }
    ulo->mem->p[i][0]=o->mem->p[0][0]; ulo->mem->p[i][1]=o->mem->p[0][1];
    ulo->mem->pid[i]=o->mem->pid[0];
    break;
   }
  }
 }
}

void spark_set_SOSsend(obj *o)
{
 printf("spark_set_SOSsend\n");
 o->s.me=NULL;
// ���� ���� 㬨��
}

void spark_set_Wpitanie(obj *o)
{
// ��ᬮ�ਬ �� ���� � �㧥
 obj *co=o->bo,*ro,*mo=NULL;
 obj *ftn_o=NULL;
 obj *crgo_o=NULL;
 float *rm,m=0.0;
 int i;
 evt *me;

// printf("spark_set_Wpitanie\n");
 while(co!=NULL)
 {
  if(co->tag==T_FTN) ftn_o=co;
  if(co->tag==T_CRGO) crgo_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || crgo_o==NULL) { printf("� ᯠઠ ��������� FTN ��� CRGO\n"); exit(0); }
 ro=crgo_o->bo;
 while(ro!=NULL)
 {
  if(ro->tag==T_RES) 
  {
   if(ro->s.img!=6 && ro->dev.cm>0.0) // ����뢠� ��
   {
// ����뢠� ��
    if(mo==NULL) mo=obj_create_MUSOR(o->pl,o);
    obj_create_RES(mo,ro->s.img,ro->dev.cm);
// 㬥���� ����� ᯠઠ
    ro->pl->dev.cm-=ro->dev.cm; ro->pl->pl->dev.cm-=ro->dev.cm; ro->dev.cm=0;
   }
   else // ������� ���, ��ॣ���� �� � ���஢� 1 � 1
   {
    m=ro->dev.cm; 
    if(ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1]<m) 
     m=ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1];
    ro->pl->dev.cm-=m; ro->pl->pl->dev.cm-=m; ro->dev.cm-=m;
   }
   ro=ro->next;
  }
  else // �� �� ��, ⮦� ����뢠�
  {
   if(mo==NULL) mo=obj_create_MUSOR(o->pl,o);
   co=ro->next; obj_ch_parent(ro,mo); ro=co;
  }
 }
 ftn_o->dev.pwr[1]+=m;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
}

void spark_set_ASpitanie(obj *o)
{
// ��ᬮ�ਬ �� ���� � �㧥
 obj *co=o->bo,*ro,*mo=NULL,*tmpo;
 obj *ftn_o=NULL,*crgo_o=NULL;
 float *rm,m=0.0;
 int i;
 evt *me;

// printf("spark_set_ASpitanie\n");
 while(co!=NULL)
 {
  if(co->tag==T_FTN) ftn_o=co;
  if(co->tag==T_CRGO) crgo_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || crgo_o==NULL) { printf("� ᯠઠ ��������� FTN ��� CRGO\n"); exit(0); }
 ro=crgo_o->bo;
 while(ro!=NULL)
 {
  if(ro->tag==T_RES) 
  {
   if(ro->s.img!=6 && ro->dev.cm>0.0) // ����뢠� ��
   {
// ����뢠� ��
    if(mo==NULL) mo=obj_create_MUSOR(o->pl->pl,o->pl);
    obj_create_RES(mo,ro->s.img,ro->dev.cm);
// 㬥���� ����� ᯠઠ
    ro->pl->dev.cm-=ro->dev.cm; ro->pl->pl->dev.cm-=ro->dev.cm; ro->dev.cm=0;
   }
   else // ������� ���, ��ॣ���� �c � ���஢� 1 � 1
   {
    m=ro->dev.cm; 
    if(ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1]<m) 
     m=ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1];
    ro->pl->dev.cm-=m; ro->pl->pl->dev.cm-=m; ro->dev.cm-=m;
   }
   ro=ro->next;
  }
  else // �� �� ��, ⮦� ����뢠�
  {
   if(mo==NULL) mo=obj_create_MUSOR(o->pl->pl,o->pl);
   tmpo=ro->next; obj_ch_parent(ro,mo); ro=tmpo;
  }
 }
 ftn_o->dev.pwr[1]+=m;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
}

void spark_spy_act(obj *o)
{
 int res,mesto,golod,massa;
 int i;

 spark_act_ASkopanie(o);
 spark_act_ULpitanie(o);
 spark_act_Wgo(o);

 spark_act_ASenter(o);
 spark_act_ULenter(o);

 mesto=get_spark_mesto(o);
 golod=get_spark_golod(o);
 massa=get_spark_massa(o);

 res=spy_act[mesto][golod][massa];
 if(res==0)
 {
  spark_act_ULrazgruz(o);
  spark_set_ULexit(o);	  // ��室�� � ���� �ਪ�� �᪠�� ���ந�
// ����⨬ ����� � ᠬ�� ��஬ ���ந�� �� ���������
// � �᢮��� ���� ��� ������ � ����樨 0
  for(i=4;i>=0;i--)
   { 
    o->mem->p[i+1][0]=o->mem->p[i][0]; o->mem->p[i+1][1]=o->mem->p[i][1]; 
    o->mem->pid[i+1]=o->mem->pid[i];
   }
  o->mem->p[0][0]=o->mem->p[0][1]=0.0; o->mem->pid[0]=0;
  return;
 }
 else if(res==1)
 {
  spark_act_ULrazgruz(o);
  spark_set_ULpitanie(o); // ����� �ਪ�� ������� �����
  return;
 }
 else if(res==2)
 {
  spark_set_ASsearch(o);  // ����� �ਪ�� �᪠�� ���ந� 
  return;
 }
 else if(res==3)
 {
  spark_set_ULgo(o);  // ����� �ਪ�� ����� � 㫥�
  return;
 }
 else if(res==4)
 {
  spark_set_SOSsend(o);  // ����� �ਪ�� ��᫠�� ᨣ��� SOS
  return;
 }
 else if(res==5)
 {
  spark_set_Wpitanie(o); // ��⠥��� � ��ᬮ� �� १�ࢮ�, ���뢠�� ���ꥤ���� ���
  return;
 }
 else if(res==6)
 {
  spark_set_ASkopanie(o); // ���� �ਪ�� ������ �����஥ �६�
  return;
 }
 else if(res==7)
 {
  spark_set_ASexit(o); // ��室�� � ���� �ਪ�� �� �����饭�� � UL
  return;
 }
 else if(res==8)
 {
  o->mem->d[0]=6; // �祭� ����� ����, ��� ������ ���
  o->mem->ct=1; // ᤥ��� ���, �� ⮫쪮-�� �ਫ�⥫ � �� �� �����, ���� �� ������� ������ 諠�
  o->mem->tt=GTIME;
  spark_set_ASpitanie(o); // ����뢠� ���ꥤ���� �� � ���� �ꥤ����
  return;
 }

}

// ����� ��⪨ ����ந�� 㫥� � �������� ������� ���, �� 
// ����ﭭ�� 㢥��祭�� �᫠ ᯠમ� � ��
// ��᫥ ���������� ��।�������� ������⢠ ���, �� ����稨 �����筮��
// ������⢠ ᯠમ�, ��⪠ ஦���� ����� ���� � ���� ᯠમ� ��������
// த��� 㫥� � ����ᮬ ��� � ��ந⥫쭮�� �� �⮡� ᮧ���� ���� 㫥�
// ��ࠬ���� �ᯮ��㥬� �� �ਭ�⨨ �襭��
// ����稥 ������ �⮫�������� �� ��襤訩 ��ਮ�: 0 �� �뫮, 1 �뫨
// ������⢮ ��� ���ॡ������ �� ��ਮ�
// ������⢮ ��� �ந��������� �� ��ਮ�
// ������⢮ ���
// ������⢮ ࠧ�������� ���ந���

// �㤥� ����� �� ����� ���
// ���� ᯠ� ஦������ �᫨ ����ᮢ ��� 墠⠥� 
// �� 10 �ᮢ �� ⥪�饬 ���ॡ����� � ������⢮ �������� ���ந��� ����� 1
// � ��⨢��� ��砥 ࠧ � ���� �ᮢ ��⪠ ஦���� ࠧ���稪�
// �᫨ ��室 ��� ����� 祬 � ��� ࠧ� �ॢ�蠥� ��室, � ��⪠ ஦���� �����
// � ��⨢��� ��砥 ����⥫�

void drop_UL_stat(obj *o)
{
 o->pl->mem->d[1]=o->pl->mem->d[2]=o->pl->mem->d[3]=0;
 o->mem->tt=GTIME;
}

void spark_matka_act(obj *o)
{
 evt *me;
 obj *co;
 int i;

 if(o->mem->tt==0.0) o->mem->tt=GTIME;
 if(GTIME-o->mem->tt<18.0) // ᫨誮� ࠭� �ਭ����� �襭��
  { me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return; }
 if(o->pl->mem->d[1]!=0) // ���� �����
 {
  if(o->pl->mem->d[2]<o->pl->mem->d[3]) obj_create_SP(o,4); 
  else obj_create_SP(o,2); 
  drop_UL_stat(o);
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return;
 }

 co=get_UL_eda(o->pl);
 for(i=0;i<6 && o->pl->mem->pid[i]!=0;i++);
 if((o->pl->mem->d[2]==0 || co->dev.cm/o->pl->mem->d[2]>20.0) && i>1)
 {
  if(o->pl->mem->d[2]*2<o->pl->mem->d[3]) obj_create_SP(o,2); 
  else obj_create_SP(o,2); 
  drop_UL_stat(o);
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return;
 }
 else if(1.0*rand()/RAND_MAX>0.8)
 {
  obj_create_SP(o,3); drop_UL_stat(o);
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return;
 }
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me);
}


// ���� ����६ ������ ���, �筥� �, �� ��ᠥ��� 楫��
void spark_act_ULgetmem(obj *o)
{
 int i;
 obj *ulo=o->pl;
 for(i=0;i<6;i++)
 {
  o->mem->p[i][0]=ulo->mem->p[i][0];  o->mem->p[i][1]=ulo->mem->p[i][1];
  o->mem->pid[i]=ulo->mem->pid[i];  
 }
  o->mem->d[0]=ulo->mem->d[0];
  o->mem->cp=ulo->mem->cp; o->mem->tp=GTIME; 
}

void spark_fighter_act(obj *o)
{
 
}

void spark_rab_act(obj *o)
{
 int res,mesto,golod,massa;

 spark_act_ASkopanie(o);
 spark_act_ULpitanie(o);
 spark_act_Wgo(o);

 spark_act_ASenter(o);
 spark_act_ULenter(o);

 mesto=get_spark_mesto(o);
 golod=get_spark_golod(o);
 massa=get_spark_massa(o);

 res=spy_act[mesto][golod][massa];
 if(res==0)
 {
  spark_act_ULrazgruz(o);
  spark_act_ULgetmem(o);
  spark_set_ULexit(o);	  // ��室�� � ���� �ਪ�� �᪠�� ���ந�
  return;
 }
 else if(res==1)
 {
  spark_act_ULrazgruz(o);
  spark_set_ULpitanie(o); // ����� �ਪ�� ������� �����
  return;
 }
 else if(res==2)
 {
  spark_set_ASgo(o);  // ����� �ਪ�� �᪠�� ���ந� 
  return;
 }
 else if(res==3)
 {
  spark_set_ULgo(o);  // ����� �ਪ�� ����� � 㫥�
  return;
 }
 else if(res==4)
 {
  spark_set_SOSsend(o);  // ����� �ਪ�� ��᫠�� ᨣ��� SOS
  return;
 }
 else if(res==5)
 {
  spark_set_Wpitanie(o); // ��⠥��� � ��ᬮ� �� १�ࢮ�, ���뢠�� ���ꥤ���� ���
  return;
 }
 else if(res==6)
 {
  spark_set_ASkopanie(o); // ���� �ਪ�� ������ �����஥ �६�
  return;
 }
 else if(res==7)
 {
  spark_set_ASexit(o); // ��室�� � ���� �ਪ�� �� �����饭�� � UL
  return;
 }
 else if(res==8)
 {
  o->mem->d[0]=6; // �祭� ����� ����, ��� ������ ���
  o->mem->ct=1; // ᤥ��� ���, �� ⮫쪮-�� �ਫ�⥫ � �� �� �����, ���� �� ������� ������ 諠�
  o->mem->tt=GTIME;
  spark_set_ASpitanie(o); // ����뢠� ���ꥤ���� �� � ���� �ꥤ����
  return;
 }
}

void spark_act_btl(obj *o,evt *e,lnk *l)
{
 obj *bo=o->pl;
 obj *co,*eo=NULL;
 obj *wo1=NULL,*wo2=NULL;
 evt *me;
 double minr=0,dr,dx,dy,cx,cy,r;
 double wt1,wt2; // ⥪�饥 �६� �����⮢�� ��㦨� � ����५�
 double dt,ct;
 int n; char *b;
 lnk *nl;

// del_evt(o,e); free_evt(o,e);

 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.ct=GTIME; o->s.tx=0.0; o->s.ty=0.0; cx=o->s.cx; cy=o->s.cy;
  
 co=bo->bo;
 while(co!=NULL)
 {
  if(co->bside!=0 && co->bside!=o->bside) // ��⨢��� ������
  {
   dr=(GTIME-co->s.ct)*co->s.v; dx=co->s.cx+co->s.tx*dr-cx; dy=co->s.cy+co->s.ty*dr-cy;
   dr=1.0/(dx*dx+dy*dy); if(dr>minr) { minr=dr; eo=co; }
  }
  co=co->next;
 }

 if(eo==NULL) // �ࠣ�� ����, ��宦� �� ���
  { 
   obj_ch_parent(o,o->pl->pl); 
   me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
   return; 
  }

 dr=(GTIME-eo->s.ct)*eo->s.v; minr=sqrt(minr); 
 dx=eo->s.cx+eo->s.tx*dr-cx; dy=eo->s.cy+eo->s.ty*dr-cy; 
 r=1.0/minr; dx*=minr; dy*=minr;

// ������ ��ࠬ���� ��㦨�
 co=o->bo;
 while(co!=NULL)
 {
  if(co->tag==T_WPN) if(co->s.img==1) wo1=co; else wo2=co;
  co=co->next;
 }

 wt1=GTIME-wo1->s.ct; wt2=GTIME-wo2->s.ct;

 if(wt1>=wo1->dev.pwr[2] && wo1->s.me==NULL) 
 {
// �ନ��� ������� ����५� �� 楫�
  nl=get_lnk(); nl->st[0]=0; nl->o=eo; nl->tx=0.0; nl->ty=0.0;
  me=get_evt(); evt_buse_set(me,wo1,NULL,nl,0.0); ins_evt(wo1,me);
  wt1=0.0;
 }

 wt1=wo1->dev.pwr[2]-wt1; // �६� �� ᫥���饣� ����५�

 if(r<=wo2->dev.pwr[0] && wt2>=wo2->dev.pwr[2] && wo2->s.me==NULL)
 {
// �ନ��� ������� ����५� �� 楫�
  nl=get_lnk(); nl->st[0]=0; nl->o=eo; nl->tx=0.0; nl->ty=0.0;
  me=get_evt(); evt_buse_set(me,wo2,NULL,nl,0.0); ins_evt(wo2,me);
  dt=wo2->dev.pwr[2];
 }
 else dt=wo2->dev.pwr[2]-wt2;

 if(r>wo2->dev.pwr[0]) // �����⠥� �६� ������
 {
  dt=(r-wo2->dev.pwr[0])/o->s.v;
// �ନ��� ������� ������ � 楫�
  o->s.ct=GTIME; o->s.tx=dx; o->s.ty=dy;
 }

 if(dt>wt1) dt=wt1;

 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt+1.0); ins_evt(o,me);
}
