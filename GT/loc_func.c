#include "gt.h"
#include "evt_rmi.h"


void loc_col_PERS(obj *o,evt *de,double tmin)
{
 evt *me;
 lnk *nl,*ml;
 obj *co;

 if(tmin<0.0) return;
 co=o->nif; if(co==NULL) return; // ��ꥪ� ��室���� � ���ࠢ�塞�� ०���
 // �६� ������襣� �⮫�������� �������
 // ᮧ��� ��� ᮡ���: �⮫�������� � ����樥� � ��� ���ᮡ�⨥
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

// ��⠢�塞 ����� �⮫�������� � ��।� � �ਢ�뢠�� ��� � ॠ�樨
// �� ��������� ��������
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

// ��楤�� �ମ�����, �� ����� ��⮬ �ம�⨬������
void loc_crossec(obj *o,int x1,int x2,int y1,int y2)
{
 int i,j,k,m,f,locn;
 lnk *l,*dl,*nl,*l1,*l2;
 obj *wo=o->pl;

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
// wo=o->pl; if(wo->tag!=T_WORLD) return; // �㤥� �����, �� �᫨ ��ꥪ� �� � ��ᬮ�, � �� �� �������� ॣ����樨
 while(wo->tag!=T_WORLD) wo=wo->pl;

 l=o->locl; 
 while(l!=NULL) 
 { 
  locn=l->ln; j=locn/wo->s.xs; i=locn-j*wo->s.xs; // :)
  dl=l; l=l->next;
  if(i>x1 || i<x2 || j>y1 || j<y2) // ��� ���� ��ꥪ� ������
  {
   // 㤠��� ��ꥪ� �� ����樨
   nl=dl->l; 
   if(nl->prev==NULL) wo->locs[locn].ol=nl->next; else nl->prev->next=nl->next;
   if(nl->next!=NULL) nl->next->prev=nl->prev;
   if(dl->prev==NULL) o->locl=dl->next; else dl->prev->next=dl->next;
   if(dl->next!=NULL) dl->next->prev=dl->prev;
   free_lnk(o,nl); free_lnk(o,dl);

   // ������ ����樨, ����� �⠫� �� �����
   for(k=-1;k<=1;k++) for(m=-1;m<=1;m++)
   {
    if(m+i>x1+1 || m+i<x2-1 || k+j>y1+1 || k+j<y2-1) // ���� ⥯��� �� �����
    {
     // �����⨬ �� �⮬ ������
     locn=(m+i)+(k+j)*wo->s.xs; nl=wo->locs[locn].ol;
     while(nl!=NULL) 
     {
      // �஢�ਬ �� �������� �� ��ꥪ� nl->o ���筮 � ������� ��������
      dl=nl->o->locl; f=0; 
      while(dl!=NULL) 
       { 
        locn=dl->ln; j=locn/wo->s.xs; i=locn-j*wo->s.xs; dl=dl->next;
        if(i<x1+1 && i>x2-1 && j<y1+1 && j>y2-1) { f=1; break; }
       }
      if(f==0) cmd_send_dobj(o,nl->o); // ������� �� 㤠����� ��ꥪ� ����� ���� ��᫠�� 3 ࠧ�, �� �� �� ���譮
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
   if(k==1) continue; // �� ���� 㦥 ��⥭�

   // ������ ��뫪� ��� �ਢ離� � ����樨 � ��ꥪ��
   l1=get_lnk(); l2=get_lnk();
   l1->o=o; l2->o=wo;
   l1->l=l2; l2->l=l1;

   // ��⠢��� ��뫪� �� ��ꥪ� � ������
   l2->e=(evt *)locn;
   l1->prev=NULL; l1->next=wo->locs[locn].ol;
   if(wo->locs[locn].ol!=NULL) wo->locs[locn].ol->prev=l1; 
   wo->locs[locn].ol=l1;

   // ��⠢��� ��뫪� �� ������ � ��ꥪ�
   l2->prev=NULL; l2->next=o->locl; 
   if(o->locl!=NULL) o->locl->prev=l2; 
   o->locl=l2;

   if(j==y2) // �����頥� i,j-1
   {
    locn=i+(j-1)*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
   }
   if(j==y1) // �����頥� i,j+1
   {
    locn=i+(j+1)*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
   }
   if(i==x2) // �����頥� i-1,j
   {
    locn=i-1+j*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
    if(j==y2) // �����頥� i-1,j-1
    {
     locn=i-1+(j-1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
    if(j==y1) // �����頥� i-1,j+1
    {
     locn=i-1+(j+1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
   }
   if(i==x1) // �����頥� i+1,j
   {
    locn=i+1+j*wo->s.xs; nl=wo->locs[locn].ol;
    cmd_send_obj_loc(o,nl);
    if(j==y2) // �����頥� i+1,j-1
    {
     locn=i+1+(j-1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
    if(j==y1) // �����頥� i+1,j+1
    {
     locn=i+1+(j+1)*wo->s.xs; nl=wo->locs[locn].ol;
     cmd_send_obj_loc(o,nl);
    }
   }
  }
}

// ���樠������ ᮡ��� �⮫�������� � �࠭�楩 ����樨
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

// ���ᥪ�� �࠭��� ����樨, ������ ������⢮ ����権, ����� �����㫨
// � ������⢮ ����権 � ����� ��ﯠ����
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
// wo=o->pl; 
// if(wo->tag!=T_WORLD)
// { 
//  del_evt(o,e); free_evt(o,e); return; // �㤥� �����, �� �᫨ ��ꥪ� �� � ��ᬮ�, � �� �� �������� ॣ����樨
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

 // ������ �६� ᫥���饣� ����祭��

 if(o->s.tx>0.0) { x1=1.0-x1; x2=1.0-x2; }
 if(o->s.ty>0.0) { y1=1.0-y1; y2=1.0-y2; }

 if(x1<x2) tx=x1; else tx=x2;
 if(y1<y2) ty=y1; else ty=y2;
 
 if(tx*vy<ty*vx) tmin=(tx*1000.0)/(vx*o->s.v);
 else tmin=(ty*1000.0)/(vy*o->s.v);
 tmin+=0.001; // ��� ������讣� �஭���������
/*
 if(o->pl->tag==T_BL && o->bside!=0)
  { del_evt(o,e); evt_col_loc_set(e,o,NULL,NULL,tmin); ins_evt(o,e); return; }
 
 if(o->tag==T_PERS) 
  { del_evt(o,e); free_evt(o,e); loc_col_PERS(o,get_evt(),tmin); return; }
*/

 if(o->tag==T_PERS) // ��饥 �������� �१ bmv 
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

// ��ꥪ� ��⠭������, �㦭� 㭨�⮦��� ᮡ�⨥ E_col_loc
// ᮡ�⨥ ��뢠���� �� ᯨ᪠ ॠ�権 �⥢��� ����䥩� T_NIF ��� �ࠢ�塞�� ��ꥪ⮢
// � �� ᯨ᪠ ॠ�樨 ᠬ��� ��ꥪ� ��� �ࡨ⠫��� ��ꥪ⮢ ⨯� T_AS
void evt_cold_loc_act(obj *o,evt *e,lnk *l)
{
 evt *de,*me;
 double tmin;

// if(o->pl->tag==T_BL && o->bside!=0)
  if(o->tag==T_PERS)
  { 
   tmin=get_loc_col_time(o); de=l->e;
   if(tmin<0.0) // �⮫�������� �� ���������
   {
    if(de->e!=NULL) // 㤠��� ᮡ�⨥ �⮫��������, ������ ॠ��� ��࠭�� �� ���饥
     { del_evt(o,de->e); free_evt(o,de->e); de->e=NULL; }
    return;
   }
   if(de->e!=NULL) // ᬥ�� ᮡ�⨥ �� tmin
    { de->e->t=GTIME+tmin; move_evt(o,de->e); }
   else // ᮧ��� ����� ᮡ�⨥ �⮫�������� � ��⠢��� � ��।�
    { me=get_evt(); evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me); de->e=me; }
   return;
  }
/* �ᯮ�짮������ �� �������� �१ ������� mv
 if(o->tag==T_NIF)
 {
  de=l->e->e;
  del_evt(o->pl,de); free_evt(o->pl,de);
  return;
 }
*/
 if(o->tag==T_AS || o->tag==T_MUSOR || o->tag==T_UL || o->tag==T_SP)
 {
// 㭨�⮦�� ᮡ�⨥ �⮫��������, �᫨ ��� �������
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 if(o->s.tx==0.0 && o->s.ty==0.0) return -1.0; // ��ꥪ� ����������
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
 tmin+=0.001; // ��� ������讣� �஭���������
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

// �㭪�� ��⠭�������� ᮡ�⨥ ����祭�� �࠭��� ����権 ��ꥪ⮬ 
// � ������ ०���, ���ᮡ�⨥ ������ ���� ॠ�樥� ᮡ��� bm
/*
void loc_col_bmv(obj *o, double tmin)
{
 evt *me,*de;
 lnk *nl,*ml;
 obj *co;

 if(tmin==0.0) tmin=get_loc_col_time(o);
 if(tmin<0.0) return;

 // �६� ������襣� �⮫�������� �������
 // ᮧ��� ��� ᮡ���: �⮫�������� � ����樥� � ��� ���ᮡ�⨥
 me=get_evt(); evt_col_loc_set(me,o,NULL,NULL,tmin); ins_evt(o,me);
 de=get_evt(); evt_cold_loc_set(de,o,NULL,NULL,0.0); me->e=de; de->e=me; 
 ml=evt_lnk_ins(lnks_tags[E_bmv],o,NULL,NULL); de->l=ml; ml->l=ml; ml->e=de;
}
*/

// �㭪�� ��뢠���� ���� ࠧ � ��砫� ����.
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 if(co==NULL) return; // ��ꥪ� ��室���� � ���ࠢ�塞�� ०���

 if(o->s.tx==0.0 && o->s.ty==0.0) return; // ��ꥪ� ����������
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
 tmin+=0.001; // ��� ������讣� �஭���������

 // �६� ������襣� �⮫�������� �������
 // ᮧ��� ��� ᮡ���: �⮫�������� � ����樥� � ��� ���ᮡ�⨥
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
 wo=o->pl; while(wo->tag!=T_WORLD) wo=wo->pl;

 // ��宦� ���न���� ����樨
 lx1=(o->s.cx-o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 ly1=(o->s.cy-o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 lx2=(o->s.cx+o->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 ly2=(o->s.cy+o->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 for(j=ly1;j<=ly2;j++)
 for(i=lx1;i<=lx2;i++)
 {
  // ������ ��뫪� ��� �ਢ離� � ����樨 � ��ꥪ��
  l1=get_lnk(); l2=get_lnk();
  l1->o=o; l2->o=wo;
  l1->l=l2; l2->l=l1;
  // ��⠢��� ��뫪� �� ��ꥪ� � ������
  locn=i+j*wo->s.xs; l2->e=(evt *)locn; // :)
  l1->prev=NULL; l1->next=wo->locs[locn].ol;
  if(wo->locs[locn].ol!=NULL) wo->locs[locn].ol->prev=l1; 
  wo->locs[locn].ol=l1;
  // ��⠢��� ��뫪� �� ������ � ��ꥪ�
  l2->prev=NULL; l2->next=o->locl; 
  if(o->locl!=NULL) o->locl->prev=l2; 
  o->locl=l2;
 }
 if(send_flag!=0) // ������� ����樨 � ������ ��ꥪ�
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD

 if(po->locl==NULL) return; // ��ꥪ� �� ��ॣ����஢�� � �������
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

// �����頥� ����樨 �� ���������� � ���� ��ࠬ����
void locs_send_myobj(obj *po)
{
 obj *wo;
 lnk *ol;
 int x1,x2,y1,y2;
 int i,j,locn;

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
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

 // ������ த�⥫�᪨� ��ꥪ� - WORLD
 if(po->locl==NULL) return;
 wo=po->locl->o;

 // ��।�� ��ࠬ���� �������� �ᥬ ��ࠡ����� �����

 x2=(po->s.cx+po->s.xs*0.5)*0.001+(wo->s.xs*0.5); 
 x1=(po->s.cx-po->s.xs*0.5)*0.001+(wo->s.xs*0.5);
 y2=(po->s.cy+po->s.ys*0.5)*0.001+(wo->s.ys*0.5); 
 y1=(po->s.cy-po->s.ys*0.5)*0.001+(wo->s.ys*0.5);
 x1-=1;  x2+=1;  y1-=1;  y2+=1;

 for(j=y1;j<=y2;j++)
  for(i=x1;i<=x2;i++)
   { locn=j*wo->s.xs+i; ol=wo->locs[locn].ol; cmd_send_mv_loc(po,ol); }
}
