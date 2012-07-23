// ��ࠡ�⪠ �ਭ���� ������ �� ������
#include "gt.h"
#include "evt_btl.h"

void evt_mvs_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_mvs_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_mvf_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_mvf_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_mvfd_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_mvfd_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}


void evt_prich_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
{
 ne->tag=E_prich_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}
}


#define NET_CMD_NUM		15
const char *cname[NET_CMD_NUM] ={
 "/",
#define		C_END		0
 "AUTH",
#define		C_AUTH		1
 "MV",
#define		C_MV		2
 "PONG",
#define		C_PONG		3
 "CHAT",
#define		C_CHAT		4
 "BS",					// ��砫� ��� ��� ����⥫��⢮ � �������騩
#define		C_BS		5
 "T",					// ���� 室� � ���
#define		C_BT		6
 "/T",					// ����� ����� 室� � ���
#define		C_ET		7
 "BM",					// �������� � ���
#define		C_BM		8
 "PRICH",				// ��砫��� � ��ꥪ��
#define		C_PRICH		9
 "OTCH",				// ��砫��� � ��ꥪ��
#define		C_OTCH		10
 "BUSE",				// ��५���
#define		C_BUSE		11
 "BEXIT",				// ��� �� ���
#define		C_BEXIT		12
 "NEWP",				// ᮧ���� ������ ����
#define		C_NEWP		13
 "MNT"					// ��⠭�����/ࠧ����஢��� ���ன�⢮
#define		C_MNT		14
};

#define NET_CMDP_NUM	14
const char *cpname[NET_CMDP_NUM] ={
 "/",
#define		CP_CXML		0
 "login",
#define		CP_LOGIN	1
 "pass",
#define		CP_PASS		2
 "tx",
#define		CP_TX		3
 "ty",
#define		CP_TY		4
 "t",
#define		CP_T		5
 "obj",
#define		CP_OBJ		6
 "tobj",
#define		CP_TOBJ		7
 "enemy",
#define		CP_ENEMY	8
 "x",
#define		CP_X		9
 "y",
#define		CP_Y		10
 "txt",
#define		CP_TXT		11
 "ta",
#define		CP_TA		12
 "slot"
#define		CP_SLOT		13
};
char *cpval[NET_CMDP_NUM]={0};

int cmds_react[NET_CMD_NUM]=
{
 -1,	-1,	0,	-1,	
 -1,	-1,	-1,	-1,
 -1,    -1,	-1,	-1,
 -1,	-1,	-1
};



// ᮡ�⨥ ��砫� �������� ��ࠡ��
void evt_mvs_user_act(obj *o,evt *e,lnk *l)
{
 evt *me,*de;
 obj *co=o->nif;
 lnk *nl;

 del_evt(o,e); free_evt(o,e);

// if(co==NULL) return;
 
 // ᮧ��� ��� ᮡ���: �����襭�� �������� � ��� ���ᮡ�⨥
 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm);
 me=get_evt(); evt_mvf_user_set(me,o,NULL,NULL,o->s.ct); ins_evt(o,me); o->s.me=me;
 de=get_evt(); evt_mvfd_user_set(de,o,NULL,NULL,0);
 me->e=de; de->e=me; nl=evt_lnk_ins(cmds_react[C_MV],co,de,NULL); de->l=nl; nl->l=nl;
 o->s.ct=GTIME;
 locs_send_mv(o);

 // �㦭�: ������ �६� �⮫�������� � �࠭�栬� ����権, ��⠢���
 // ᮡ��� �⮫�������� � �� ���ᮡ���, ��뫪� �� ���ᮡ��� ��⠢���
 // � ᯨ᮪ ॠ�権 �� ������� MV
 // �ᥣ� ����� ���� 4 ᮡ��� �⮫��������, ����� ��⠢��� �ࠧ� 4 ᮡ���,
 // � ����� ��࠭������ ������訬 �� �६���, �� ⮣�� �� ��� �ࠡ��뢠���
 // �㦭� ᭮�� �᪠�� ������襥 �⮫��������. ��ன ��ਠ�� ���� � ॠ����樨.
 // ��� � �����.
 loc_col(o,NULL);

 // �㦭� ��࠭��� ���ﭨ� ��ꥪ� (������ � ��ࠬ���� ��������)
 obj_save_update(o);
 obj_save_cpoint();
}

lnk *rmi_lnk_mv(obj *co,evt *de)
{
 return evt_lnk_ins(cmds_react[C_MV],co,de,NULL);
}

// ᮡ�⨥ �����襭�� �������� ��ࠡ��
void evt_mvf_user_act(obj *o,evt *e,lnk *l)
{
 double dr;
 lnk *ml,*dl,*tl;
// obj *co=o->nif;

 del_evt(o,e); free_evt(o,e);
 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.tx=0; o->s.ty=0; o->s.ct=0.0;

 o->s.me=NULL;

// if(co==NULL) return;
 locs_send_mv(o);
}

// ᮡ�⨥ ���뢠��� �������� ��ࠡ��
void evt_mvfd_user_act(obj *o,evt *e,lnk *l)
{ 
 evt_mvf_user_act(o->pl,l->e->e,l); //:) 
}

void cmd_mv_proc(obj *o)
{
 evt *e; obj *po=o->pl;
 double r;

 return; // ������� �⪫�祭�, �� �������� �१ ������� bmv

 if(po->pl->tag!=T_WORLD) return;
 po->s.tx=po->s.tx=0; po->s.ct=0.0;
 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL || cpval[CP_T]==NULL) return;
 sscanf(cpval[CP_TX],"%f",&po->s.tx);
 sscanf(cpval[CP_TY],"%f",&po->s.ty);
 sscanf(cpval[CP_T],"%Lf",&po->s.ct);
 r=po->s.tx*po->s.tx+po->s.ty*po->s.ty;
 if(r>1.001) { r=1.0/sqrt(r); po->s.tx*=r; po->s.ty*=r; }
 if(po->s.me==NULL)
 {
  e=get_evt(); evt_mvs_user_set(e,po,NULL,NULL,5.0); ins_evt(po,e); po->s.me=e;
 }
}

void cmd_auth_proc(obj *o)
{
 evt *e; obj *po;
 obj *co;

 if(cpval[CP_LOGIN]==NULL || cpval[CP_PASS]==NULL) return;
 po=find_user(cpval[CP_LOGIN]);
 if(po==NULL || strcmp(po->password,cpval[CP_PASS])!=0) { cmd_send_hui(o); return; } // �� �ࠢ��쭮 ������ ����� ��� ��஫�

 // �஢�ઠ �����襭�, 㡨���� �������騩 �⥢�� ����䥩�, �᫨ �� ���� 
 co=po->bo; while(co!=NULL && co->tag!=T_NIF) co=co->next;
 if(co!=NULL) del_nif_obj(co);

 obj_ch_parent(o,po);

 po->nif=o; // �������� ���ன�⢮

 // ����� ��ࠢ���� ⥪�饥 ���ﭨ� ��ࠡ���� �������
 cmd_send_iam(po);
 // ��ࠢ��� ᯨ᮪ ��ꥪ⮢ ������� ����権
 if(po->pl->tag==T_WORLD || po->pl->tag==T_BL)
 {
//  loc_obj_reg(po,0);
  locs_send_obj(po);
  if(po->pl->tag==T_BL) // ��ꥪ� � ��� ���뫠� ��� ����
  cmd_send_bl_update(po);
 }
 else
  cmd_send_pobj_obj(po,po->pl);
}

// ᮧ����� ������ ���� �� 蠡���� � ������� ������ � ��஫��
void cmd_newp_proc(obj *o)
{
 obj *po;

 if(cpval[CP_LOGIN]==NULL || cpval[CP_PASS]==NULL) return;
 po=find_user(cpval[CP_LOGIN]);
 if(po!=NULL) return; // ����� 㦥 �������

 // �஢�ઠ �����襭�, ᮧ���� ��ꥪ� �� 蠡���� � 楯�塞�� � ����
 po=obj_create_PERS(cpval[CP_LOGIN],cpval[CP_PASS]);
 cmd_send_hello(o); // ᮮ�頥�, �� ���짮��⥫� ᮧ���
}

// ���� ���� ��⠭������� ��������
void obj_btl_prep_PERS(obj *o)
{
 return; // �⪫�祭� ��᪮��� �� �������� ࠡ�⠥� �१ bmv
 if(o->s.me!=NULL && o->s.me->tag==E_mvs_user ) 
 {
  del_evt(o,o->s.me); free_evt(o,o->s.me);
  o->s.ct=0.0; o->s.tx=o->s.ty=0.0; o->s.me=NULL; // ��ࠡ�� � �������� ��砫� ��������, ��
 }
 else if(o->s.me!=NULL) // ᤢ����� ᮡ�⨥ � ��砫� ��।�
  { o->s.me->t=GTIME; move_evt(o,o->s.me); } // �㦭� �஢���� ࠡ��ᯮᮡ����� �㭪樨 move_evt
// �ନ�㥬 ॠ��� �� ��������� E_bmv
 loc_col_bmv(o);
}

void obj_btl_prep_SPARK(obj *o)
{
 if(o->s.me!=NULL) // ᤢ����� ᮡ�⨥ � ��砫� ��।�
  { o->s.me->t=GTIME; move_evt(o,o->s.me); }
 // �� ᫥���饬 �ࠡ��뢠��� ᮡ��� ᯠ� ������ ������, �� ��室��� � ���
}

void obj_btl_prep_AS(obj *o)
{
}

void obj_btl_prep_MUSOR(obj *o)
{
}

void obj_btl_prep(obj *o)
{
 if(o==NULL) return;
 if(o->tag==T_PERS) obj_btl_prep_PERS(o);
 if(o->tag==T_SP) obj_btl_prep_SPARK(o);
 if(o->tag==T_AS) obj_btl_prep_AS(o);
 if(o->tag==T_MUSOR) obj_btl_prep_MUSOR(o);
}

// ����稫� ������� ��३� � ��蠣��� ०��
// ॠ�樥� �� ������� ������ ���� ��⠭���� ��������
// 1. obj="" ��� ���������
// 2. obj="id1"
// 3. tobj= "" ��� ���������
// 4. tobj="id2"

// 1,3 - ���室 � ��蠣��� ०��
// 2,3 - �������� � ��� id1 ��⨢ ���
// 1,4 - ������� �� id2
// 2,4 - �������� � ��� id1 �� ��஭� id2

// ����� � id �㦭� ��।����� � 㭨���쭮� ��� ��ꥪ�, ���� ��������
// �������� �� ࠧ��� ��ꥪ�� � ����� id (��砩 ����� ���� ��ꥪ� 㦥 㬥�, �
// ���� ����稫 �� �� id)

void cmd_bs_proc(obj *o)
{
 obj *bo,*to=NULL,*co;
 evt *me;
 char bn[ONAM_LEN];
 char tn[ONAM_LEN];
 int bside;

 float test;

 o=o->pl;

 if(o->pl->tag==T_BL) return; // 㦥 � ��蠣���� ०���, ��⮬ �஢��� ���� �� �������������
 if(cpval[CP_OBJ]==NULL || cpval[CP_OBJ][0]==0) // ���� ���
 {
  if(cpval[CP_TOBJ]!=NULL && cpval[CP_TOBJ][0]!=0) // ������ �� ����-�
  {
   sscanf(cpval[CP_TOBJ],"%x,%s",&to,tn);
   if(find_obj_ombase(to)!=0) return; // ���� ⠪��� ��ꥪ�
//   if(strcmp(to->dn,tn)!=0) return; // �� �� ��ꥪ�, �� �㦥�
//   if(is_obj_onmap(to)!=0) return; // ���� �� ����
   if(to->pl->tag==T_BL) return; // 㦥 � ���
   test=get_obj_dist(o,to);
   if(get_obj_dist(o,to)>=1.0) return; // ᫨誮� ������
  }
  bo=get_obj(o->pl,T_BL,""); if(bo==NULL) return; obj_new_name(bo);
  strcpy(bo->name,"���");
  obj_ch_parent(o,bo); obj_ch_parent(to,bo);
  if(to!=NULL) { bo->bside++; to->bside=bo->bside; } 
  bo->bside++; o->bside=bo->bside;
  
  BL_init_rt(bo);
  // ��⠭���� �ࠢ�塞� ��ꥪ��, ��ࢥ� �࣮��� � �.�.
  obj_btl_prep(o); obj_btl_prep(to);
 }
 else // ���訢����� � ⥪�騩 ���
 {
  sscanf(cpval[CP_OBJ],"%x",&bo); // ��⠥� id ���
  if(find_obj_ombase(bo)!=0) return; // ���� ⠪��� ��ꥪ�
//  if(strcmp(bo->dn,tn)!=0) return; // �� �� ��ꥪ�, �� �㦥�
  if(bo->tag!=T_BL) return; // �� �� ���

// ������ ����ﭨ� �� ��ࢮ�� ���� � �।���� ���쭮�� �室� � ���
  co=bo->bo; 
  while(co!=NULL) 
  {
   if(co->bside!=0) // ���� ������
    { if(get_obj_dist(o,co)<1.0) break; }// �����筮 ������
   co=co->next;
  }
  if(co==NULL) return;

  bside=0;
  if(cpval[CP_TOBJ]!=NULL && cpval[CP_TOBJ][0]!=0) // ��ᮥ������� � 㪠������ ��஭�
  {
   sscanf(cpval[CP_TOBJ],"%d",&bside); // ��⠥� ����� ��஭�
   if(bside>bo->bside || bside<0) bside=0;
  }
  if(bside==0) { bo->bside++; bside=bo->bside; }
  o->bside=bside;
  obj_ch_parent(o,bo);
  cmd_send_bl_begin1(o);
  cmd_send_bl_update(o);
  obj_btl_prep(o);
 }
}

int bt_open=0; // 䫠� ��砫� ����� 室�
// ����稫� ���� 室� � ���
void cmd_bt_proc(obj *o)
{
 obj *po;
 evt *e;
 int turn;

 po=o->pl;
 if(po->tag!=T_BL) return; // 㦥 �� ��蠣��� ०��
 if(cpval[CP_T]==NULL || cpval[CP_T][0]==0) return;
 sscanf(cpval[CP_T],"%d",&turn);
 if(po->turn!=turn || o->turn!=turn) return; // �� �� 室, �����.
 o->turn=-1; // ����� ����� �� �ਭ����� �� ��砫� ᫥���饣� 室�
 if(cpval[CP_CXML]==NULL) bt_open=1; // 室 �� ���⮩
 e=get_evt(); evt_func_set[E_bl_dummy](e,po,NULL,NULL,0.0); ins_evt(po,e);
}

// ����⨫� ����� �����
void cmd_et_proc(obj *o)
{
 bt_open=0;
}






int bt_cmd=0;
// ����稫� ������� ��������� � 㪠������ ���ࠢ�����
// ������� ��஦���� ���� ��� ��� ᮡ��� E_bm � ����ᨬ���
// �� ������ ��ࠬ��� ta � �������
// � ᮡ��� �ਪ९����� ��뫪� � ���ன ���� ���ଠ�� �
// ���ࠢ����� ��������
void cmd_bm_proc(obj *o)
{
 double r,tx,ty,t,ta=0.0;
 obj *np,*eo=NULL;
 evt *e;
 lnk *l;

 if(bt_cmd==0) return; // ��-� �� � ��諮

 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL || 
    cpval[CP_T]==NULL) return;
 sscanf(cpval[CP_TX],"%Lf",&tx);
 sscanf(cpval[CP_TY],"%Lf",&ty);
 sscanf(cpval[CP_T],"%Lf",&t);
 if(cpval[CP_TA]!=NULL) sscanf(cpval[CP_TA],"%Lf",&ta);

 if(t<0.0 || t>BTURN_TIME) return; // �६� �� �।����� �����⨬��� ���������
 if(t==BTURN_TIME && (tx!=0.0 || ty!=0.0)) return; // ����� 室�, �� ��⠭����������

 t*=RBTURN_TIME; ta*=RBTURN_TIME; r=tx*tx+ty*ty;
 if(r>1.001) { r=1.0/sqrt(r); tx*=r; ty*=r; }
 l=get_lnk(); l->tx=tx; l->ty=ty;
 e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,t); ins_evt(o,e);

 if(ta==0.0) return;
 t+=ta; if(t<0.0 || t>BTURN_TIME) return; // �६� �� �।����� �����⨬��� ���������
 l=get_lnk(); l->tx=0.0; l->ty=0.0;
 e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,t); ins_evt(o,e);
}

// ����稫� ������� ��������� � 㪠������ ���ࠢ�����
// ������� ��஦���� ���� ��� ��� ᮡ��� E_bmv � ����ᨬ���
// �� ������ ��ࠬ��� t � �������
// � ᮡ��� �ਪ९����� ��뫪� � ���ன ���� ���ଠ�� �
// ���ࠢ����� ��������
void cmd_bmv_proc(obj *o)
{
 double tx,ty;
 double dt,tmin;
 int n=0;
 evt *e;
 lnk *l;
 
 o=o->pl;
 if(o->dev.pwr[1]==0) return;

 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL) return;
 sscanf(cpval[CP_TX],"%Lf",&tx);
 sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_T]!=NULL) sscanf(cpval[CP_T],"%d",&n);

 dt=GTIME-o->s.ct;

// ������ �६� ᬥ�� ���� � �����⥫�
 tmin=1.0/o->dev.pwr[2];

 if(o->s.me!=NULL) // ᮡ�⨥ 㦥 ����, ��ᬮ�ਬ �� ��� �㤥� ������
 {
  e=o->s.me; 
  if(e->l->tx==0.0 && e->l->ty==0.0) // ��������� ��⠭��, ����� ��ࠬ���� � �६� ����㯫����
   { e->t=(dt>tmin)?GTIME:GTIME+(tmin-dt); e->l->tx=tx; e->l->ty=ty; e->l->n=n; move_evt(o,e); }
  else { e->l->tx=tx; e->l->ty=ty; e->l->n=n; } // ���� ��⠭������� ���� ��ࠬ����
 }
 else // ᮡ��� ���, ᮧ���
 {
  l=get_lnk(); l->tx=tx; l->ty=ty; l->n=n;
  e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,(dt>tmin)?0.0:(tmin-dt)); ins_evt(o,e); o->s.me=e;
 }
}


void cmd_bmv_proc_v1(obj *o)
{
 double tx,ty;
 int n=0;
 evt *e;
 lnk *l;
 
 o=o->pl;

 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL) return;
 sscanf(cpval[CP_TX],"%Lf",&tx);
 sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_T]!=NULL) sscanf(cpval[CP_T],"%d",&n);

 if(o->s.me!=NULL) // ᮡ�⨥ 㦥 ����, ��ᬮ�ਬ �� ��� �㤥� ������
 {
  e=o->s.me;
  if(e->l->tx==0.0 && e->l->ty==0.0) // ��������� ��⠭��, ����� ��ࠬ���� � �६� ����㯫����
   { e->t=GTIME+5.0; e->l->tx=tx; e->l->ty=ty; e->l->n=n; move_evt(o,e); }
  else { e->l->tx=tx; e->l->ty=ty; e->l->n=n; } // ���� ��⠭������� ���� ��ࠬ����
 }
 else // ᮡ��� ���, ᮧ���
 {
  l=get_lnk(); l->tx=tx; l->ty=ty; l->n=n;
  e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,5.0); ins_evt(o,e); o->s.me=e;
 }
}


lnk *get_slot(obj *o,char st,int sc,int sn)
{
 lnk *l=o->dev.dvsl;
 while(l!=NULL)
 {
  if(l->st[0]==st && l->sc==sc && l->sn==sn) return l;
  l=l->next;
 }
 return NULL;
}

void cmd_buse_proc_tb(obj *o)
{
 double tx=0.0,ty=0.0,t;
 obj *np,*eo=NULL;
 lnk *l,*sl;
 char st; int sc,sn; 
 evt *e;

 if(bt_cmd==0) return; // ��-� �� � ��諮

 if(cpval[CP_SLOT]==NULL || cpval[CP_T]==NULL) return; //��易⥫�� ����
 if(cpval[CP_TX]!=NULL && cpval[CP_TY]!=NULL && cpval[CP_OBJ]!=NULL) return; // ⠪ �� ������ ����

 if(cpval[CP_TX]!=NULL) sscanf(cpval[CP_TX],"%Lf",&tx);
 if(cpval[CP_TY]!=NULL) sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_OBJ]!=NULL) 
 {
  sscanf(cpval[CP_OBJ],"%x",&eo);
  if(find_obj_ombase(eo)!=0) return; // ���� ⠪��� ��ꥪ�
  if(eo->pl!=o->pl) return; // � ��� ��室����
 }
 sscanf(cpval[CP_T],"%Lf",&t); sscanf(cpval[CP_SLOT],"%c%d,%d",&st,&sc,&sn);

 sl=get_slot(o,st,sc,sn); if(sl==NULL) return;

 if(t<0.0 || t>BTURN_TIME) return; // �६� �� �।����� �����⨬��� ���������

 t*=RBTURN_TIME;

 l=get_lnk(); l->o=eo; l->tx=tx; l->ty=ty;
 e=get_evt(); evt_func_set[E_buse](e,sl->o,NULL,l,t); ins_evt(o,e);
}

void cmd_buse_proc_rt(obj *o)
{
 double tx=0.0,ty=0.0,t;
 obj *np,*eo=NULL;
 lnk *l,*sl;
 char st; int sc,sn,mode=0; 
 evt *e;

 o=o->pl;
 if(cpval[CP_SLOT]==NULL) return; //��易⥫�� ����
 if(cpval[CP_TX]!=NULL && cpval[CP_TY]!=NULL && cpval[CP_OBJ]!=NULL) return; // ⠪ �� ������ ����

 if(cpval[CP_TX]!=NULL) sscanf(cpval[CP_TX],"%Lf",&tx);
 if(cpval[CP_TY]!=NULL) sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_T]!=NULL) sscanf(cpval[CP_T],"%d",&mode);
 if(cpval[CP_OBJ]!=NULL) 
 {
  sscanf(cpval[CP_OBJ],"%x",&eo);
  if(find_obj_ombase(eo)!=0) return; // ���� ⠪��� ��ꥪ�
  if(eo->pl!=o->pl) return; // � ��� ��室����
 }
 sscanf(cpval[CP_SLOT],"%c%d_%d",&st,&sc,&sn);

 sl=get_slot(o,st,sc,sn); if(sl==NULL) return;
 e=sl->o->s.me;

// �஢�ਬ ��� �� �⬥�� ����५�
 if(cpval[CP_TX]==NULL && 
    cpval[CP_TY]==NULL && 
    cpval[CP_OBJ]==NULL &&
    cpval[CP_T]==NULL   ) 
 {
  if(e!=NULL) // �⬥���� �������
   { del_evt(sl->o,e); free_evt(sl->o,e); sl->o->s.me=NULL; }
  return;
 }

// �஢�ਬ ��� �� ��������� ०��� ��५��
 if(cpval[CP_TX]==NULL && 
    cpval[CP_TY]==NULL && 
    cpval[CP_OBJ]==NULL &&
    cpval[CP_T]!=NULL   ) 
 {
  if(e!=NULL) e->l->st[0]=mode;  return;
 }
 
 if(e!=NULL) // � ��।� 㦥 ���� ᮡ�⨥, ���� ����� ��ࠬ����
 {
  e->l->o=eo; e->l->tx=tx; e->l->ty=ty; e->l->st[0]=mode;
 }
 else
 {
  // ᮡ��� ���, �� ���ன�⢮ ����� ���� �� ��⮢�
  l=get_lnk(); l->o=eo; l->tx=tx; l->ty=ty; l->st[0]=mode;
  e=get_evt(); evt_func_set[E_buse](e,sl->o,NULL,l,1.0); ins_evt(sl->o,e);
 }
}

void cmd_buse_proc(obj *o)
{
 if(o->bmode==0) cmd_buse_proc_rt(o);
 else if(o->bmode==1) cmd_buse_proc_tb(o);
}


void cmd_prich_proc(obj *no)
{
 double x,y,dr;
 obj *o,*co;
 lnk *l; evt *e;

 o=no->pl;
 if(o->pl->tag!=T_WORLD) return;
 if(cpval[CP_OBJ]==NULL || cpval[CP_OBJ][0]==0) return;
 sscanf(cpval[CP_OBJ],"%x",&co);
 if(find_obj_ombase(co)!=0) return; // ���� ⠪��� ��ꥪ�
 if(co->pl->tag!=T_WORLD) return;
// ����� �㦭� �஢���� ���� �� � co �ਧ��� ��ꥪ� � ����� ����� ��몮������
// �� ���� ��� ������ �� ���
 if(o->s.me!=NULL && o->s.me->tag==E_mvs_user ) 
 {
  del_evt(o,o->s.me); free_evt(o,o->s.me);
  o->s.ct=0.0; o->s.tx=o->s.ty=0.0; o->s.me=NULL; // ��ࠡ�� � �������� ��砫� ��������, ��
 }
 dr=(GTIME-o->s.ct)*o->s.v; x=o->s.cx+o->s.tx*dr; y=o->s.cy+o->s.ty*dr;
 if(co->s.me!=NULL && co->s.me->tag==E_mvs_user ) dr=0.0;
 else dr=(GTIME-co->s.ct)*co->s.v; 
 x-=(co->s.cx+co->s.tx*dr); y-=(co->s.cy+co->s.ty*dr);

 if(fabs(x)>co->s.xs || fabs(y)>co->s.ys) return; // ��� �㦭� �㤥� ��᫠�� �⢥�, �� ��ꥪ� ᫨誮� ������
// ⥯��� �㦭� ��⠢��� ᮡ�⨥ �室�, � ��⥬ ᤢ����� ᮡ�⨥ �����襭�� ��������
 l=get_lnk(); l->o=co;
 e=get_evt(); evt_func_set[E_prich_user](e,o,NULL,l,0.0); ins_evt(o,e);
 if(o->s.me!=NULL) // ᤢ����� ᮡ�⨥ � ��砫� ��।�
  { o->s.me->t=GTIME; move_evt(o,o->s.me); } // �㦭� �஢���� ࠡ��ᯮᮡ����� �㭪樨 move_evt
}

void evt_prich_user_act(obj *o,evt *e,lnk *l)
{
 obj *co=l->o;

 free_lnk(o,l); e->l=NULL;
 del_evt(o,e); free_evt(o,e);
// ⥯��� ����� �室���
 loc_obj_unreg(o); // 㡨�� ॣ������ ��ꥪ� � �������
 obj_ch_parent(o,co); // ����� � co
// ��ࠢ��� ������� ᮮ�饭�� � �室� � ᯨ᮪ ������� ��ꥪ⮢, � ��ꥪ⠬ 
// ᮮ�饭�� �� o
 cmd_send_prich(o,co); 
}

// ����稫� ������� ��砫���, ���� �� �㤥� ᮧ������ ᮡ�⨥, � �믮���� �ࠧ�
// � ���쭥�襬 ᮡ�⨥ �����������, ��᪮��� �� ���� ���� ॠ�樨
void cmd_otch_proc(obj *no)
{
 double x,y,dr;
 obj *o,*po;
 lnk *l; evt *e;

 o=no->pl; po=o->pl;
 if(po->tag==T_WORLD) return;
 cmd_send_pobj_dobj(o);
 
 dr=(GTIME-po->s.ct)*po->s.v; x=po->s.cx+po->s.tx*dr; y=po->s.cy+po->s.ty*dr;
 if(1.0*rand()/RAND_MAX>0.5) dr=-1.0; else dr=1.0;
 o->s.cx=x+po->s.xs*dr*(0.55+0.45*rand()/RAND_MAX); 
 if(1.0*rand()/RAND_MAX>0.5) dr=-1.0; else dr=1.0;
 o->s.cy=y+po->s.ys*dr*(0.55+0.45*rand()/RAND_MAX);
 o->s.ct=0.0; o->s.me=NULL;
 obj_ch_parent(o,po->pl);
 loc_obj_reg(o,0);
 cmd_send_iam(o);
 // ��ࠢ��� ᯨ᮪ ��ꥪ⮢ ������� ����権
 locs_send_obj(o);
}

// ����稫� ������� ��� �� ���, �믮���� �ࠧ�
// � ���쭥�襬 ᮡ�⨥ �����������, ��᪮��� �� ���� ���� ॠ�樨
void cmd_bexit_proc(obj *no)
{
 double x,y,dr;
 obj *o,*po,*co;
 lnk *l,*ml; evt *e,*me;
 int i=0,n;
 char *b;

 o=no->pl; po=o->pl;
 if(po->tag!=T_BL) return;

// �஢���� ����� �� ���
 co=po->bo;
 while(co!=NULL)
 {
  if(co->bside!=0 && co->bside!=o->bside) { i++; break; }
  co=co->next;
 }
 if(i!=0) return; // � ��� �� ��⠫��� �ࠣ�

// ��⠭������� ��ࠡ��
 if(o->s.me!=NULL) // ᮡ�⨥ 㦥 ����
 {
  me=o->s.me; 
  me->t=GTIME; me->l->tx=0.0; me->l->ty=0.0; me->l->n=0; move_evt(o,me);
 }
 else // ᮡ��� ���, ᮧ���
 {
  ml=get_lnk(); ml->tx=0.0; ml->ty=0.0; ml->n=0;
  me=get_evt(); evt_func_set[E_bmv](me,o,NULL,ml,0.0); ins_evt(o,me); o->s.me=me;
 }

 n=snprintf(NULL,0,"<BEXIT t=\"%f\" id=\"%x\" pid=\"%x\" />",GTIME,o,po->pl)+1; b=mymalloc(n); 
 snprintf(b,n,"<BEXIT t=\"%f\" id=\"%x\" pid=\"%x\" />",GTIME,o,po->pl);
 cmd_send_buse(po, b, n); myfree(b);
 
 o->bside=0; obj_ch_parent(o,po->pl);
 // ���뫠� �ᥬ �����饭�� �� ��������� ����� OBJ
 locs_send_myobj(o);
}

// ����稫� �⮢�� ᮮ�饭��, �ࠧ� ����� � ����뫠�
void cmd_chat_proc(obj *no)
{
 char *bb,*b,*bn,*en;
 lnk *bl,*l; // ��� �ᯮ�짮���� ��� ����஥��� ᯨ᪠ ����⮢
 obj *o;
 int nusr=0,dn;

 if(cpval[CP_TXT]==NULL || cpval[CP_TXT][0]==0) return; // ���� ��祣�
 dn=strlen(no->pl->dn)+3;
 bb=mymalloc(strlen(cpval[CP_TXT])+1+dn); 
 bb[0]='['; strcpy(bb+1,no->pl->dn); bb[dn-2]=']'; bb[dn-1]=' ';
 strcpy(bb+dn,cpval[CP_TXT]); bl=l=NULL;
 b=bb+dn;

 while(*b!=0)
 {
  // �஢�ਬ ��� �� ���� "to ["
  while(*b==0x20 || *b==0x9) b++; // �ய��� �� �஡��� ���砫�
  if(*b!='t' || b[1]!='o') break; // ���c� ����, ���뫠� �ᥬ ���� ��襫
  b+=2;
  while(*b==0x20 || *b==0x9) b++; // �ய��� �� �஡��� �����
  if(*b!='[') break; // ���c� ����, ���뫠� �ᥬ ���� ��襫
  bn=b+1;
  while(*b!=']' && *b!=0) b++; // ��� ]
  if(*b==0) break; // ���c� ����, ���뫠� �ᥬ ���� ��襫
  *b=0;
  if(bn>=b) break; // ���⮩ ����
  nusr++;
  o=find_user(bn);
  if(o!=NULL) // �������� � � ᯨ᮪
  {
   l=get_lnk(); l->o=o; l->next=bl; bl=l;
  }
  *b=']'; b++;
 }

// �� ������� ���� �� ��������, �� ��ࠢ��� ���㤠
 if(nusr!=0 && bl==NULL) { myfree(bb); return; }

// ᯨ᮪ ஢ ����஥�, ����� ��ࠢ����
 if(bl==NULL) // ᯨ᮪ ����, ��ࠢ��� �ப����⥫쭮� ������
  { cmd_send_chat_broadcast(no->pl,bb); myfree(bb); return; }
 
 cmd_send_chat(no->pl,bb); // ��ࠢ��� ᥡ�
 l=bl; while(l!=NULL) { if(l->o!=no->pl) cmd_send_chat(l->o,bb); l=l->next; }
 myfree(bb);
}


void cmd_mnt_proc(obj *no)
{
 obj *mo=NULL;
 char st; int sc,sn;
 lnk *sl;
 obj *o,*co;

 o=no->pl;
 if(cpval[CP_SLOT]==NULL) return; //��易⥫�� ����
 sscanf(cpval[CP_SLOT],"%c%d_%d",&st,&sc,&sn);
 sl=get_slot(o,st,sc,sn); if(sl==NULL) return; // ���� ⠪��� ᫮�

 if(cpval[CP_OBJ]!=NULL)  // ����஢���� ������
 {
  if(sl->o!=NULL) return; // ᫮� �����, �� ����
  sscanf(cpval[CP_OBJ],"%x",&mo);
  if(find_obj_ombase(mo)!=0) return; // ���� ⠪��� ��ꥪ�
  if(mo->pl==NULL) return;
  if(mo->pl!=NULL && mo->pl->tag!=T_CRGO && mo->pl->pl!=o) return; // � ��� ��室����
  // �㦭� �஢���� ᮮ⢥����� �� �।��� ᫮��
  if(mo->dev.mysl==NULL) return; // ���ன�⢮ �� ᫮⮢��
  if(mo->dev.mysl->sn!=0) return; // ���ன�⢮ 㦥 ��⠭������
  if(st!=mo->dev.mysl->st[0] || sc<mo->dev.mysl->sc) return; // ��ᮮ⢥��⢨� ᫮��

  obj_ch_parent(mo,o); // �������
  mo->dev.mysl->sn=sn; sl->o=mo; // ��⠢��� � ᫮�
  // ������
  dev_mnt(mo);
  // ���뫠� ���� �������
  cmd_send_mnt(o,mo);
 }
 else // ࠧ����஢����
 {
  if(st=='C') return;
  if(sl->o==NULL) return; // ���� �����
  mo=sl->o; if(mo->pl!=o) return; // � ��� ��室����
  co=o->bo; while(co!=NULL) { if(co->tag==T_CRGO) break; co=co->next; }
  if(co==NULL) return;
  // �몫���
  dev_umnt(mo);
  mo->dev.mysl->sn=0; sl->o=NULL;
  obj_ch_parent(mo,co);
  cmd_send_umnt(o,mo);
 }

// e=sl->o->s.me;

}



void (*cmd_func_proc[NET_CMD_NUM])(obj *o)=
{
 NULL,			cmd_auth_proc,			cmd_mv_proc,
 NULL,			cmd_chat_proc,			cmd_bs_proc,
 cmd_bt_proc,		cmd_et_proc,			cmd_bmv_proc,
 cmd_prich_proc,	cmd_otch_proc,			cmd_buse_proc,
 cmd_bexit_proc,	cmd_newp_proc,			cmd_mnt_proc
};


long long *CMD_P_TAB=NULL;

void cmd_set_props(char *buf)
{
 int pn;
 char *tp;
 do
 {
  while(*buf<=0x20 && *buf!=0) buf++; if(*buf==0) break;
  tp=buf; // ��諨 ��砫� ����� ��ࠬ���
  while(*buf>0x20 && *buf!='=') buf++;
  if(*tp=='/') cpval[CP_CXML]=1;
  if(*buf==0) break;
  *buf=0; buf++; // ��� ��ࠬ��� ��諨
  pn=fa_s(tp,CMD_P_TAB); if(pn<0) break;
  while(*buf!='"' && *buf!=0) buf++; // ᪠����� �� ��砫� ���祭��
  if(*buf==0) break;
  buf++; tp=buf;
  while(*buf!='"' && *buf!=0) buf++; // ᪠����� �� ���� ���祭��
  if(*buf==0) break;
  *buf=0; buf++;
  cpval[pn]=tp;
 }
 while(*buf!=0);
}

void cmd_clear_props(void)
{ int i;  for(i=0;i<NET_CMDP_NUM;i++) cpval[i]=NULL; }

long long *CMD_T_TAB=NULL; // ⠡��� ��� ���᪠ �� ��ꥪ� �� �����

void recv_cmd_proc(obj *o, char *cmd)
{
 char *cmdp;
 int tagn;
 lnk *l;

 printf("RECV: %s\n",cmd);

 cmdp=cmd; while(*cmdp!=0 && *cmdp!=' ') cmdp++; if(*cmdp!=0) { *cmdp=0; cmdp++; }
 tagn=fa_s(cmd,CMD_T_TAB); if(tagn<0) return;
 if(bt_open!=0 && tagn!=C_ET) // ������� ��蠣����� ०���, �� ��ࠡ��뢠�
 {
  if(*(cmdp-1)==0) *(cmdp-1)=' '; // ����⠭������� �०��� ��� �������
  l=get_lnk(); l->next=o->pl->bo_turns; l->prev=NULL; o->pl->bo_turns=l;
  l->o=o; l->b=cmd; o->nif->nb.re->b=NULL;
  return;
 }
 if((tagn>=0 && o->pl->tag==T_PERS) || tagn==C_AUTH || tagn==C_NEWP)
 {
  //������� ⠡���� ᢮���
  cmd_set_props(cmdp);
  // ��ࠡ��� ᯨ᮪ ॠ�権 �� �������
  evt_lnks_act(cmds_react[tagn],o);
  cmd_func_proc[tagn](o);
  //���� ⠡���� ᢮���
  cmd_clear_props();
 }

}

// ��ࠡ�⪠ ������ � ������ ०���
void bl_cmd_proc(lnk *l)
{
 char *cmdp=l->b;
 int tagn;

 while(*cmdp!=0 && *cmdp!=' ') cmdp++; if(*cmdp!=0) { *cmdp=0; cmdp++; }
 tagn=fa_s(l->b,CMD_T_TAB); if(tagn<0) return;
 //������� ⠡���� ᢮���
 cmd_set_props(cmdp);
 // ��ࠡ��� ᯨ᮪ ॠ�権 �� �������
 evt_lnks_act(cmds_react[tagn],l->o);
 cmd_func_proc[tagn](l->o);
 //���� ⠡���� ᢮���
 cmd_clear_props();
}

void bl_cmds_proc(obj *o)
{
 lnk *l;
 if(CMD_T_TAB==NULL) CMD_T_TAB=fa_b(NET_CMD_NUM,cname); // ���� ⠡����
 if(CMD_T_TAB==NULL) return;
 if(CMD_P_TAB==NULL) CMD_P_TAB=fa_b(NET_CMDP_NUM,cpname); // ���� ⠡����
 if(CMD_P_TAB==NULL) return;
 bt_cmd=1; // ࠧ��� ��ࠡ��� ������ ������
 while(o->bo_turns!=NULL)
 {
  l=o->bo_turns;
  bl_cmd_proc(l);
  myfree(l->b); o->bo_turns=l->next; free_lnk(NULL,l);
 }
 bt_cmd=0;
}


void recv_cmds_proc(obj *o)
{
 lnk *cmd=o->nb.re;

 if(CMD_T_TAB==NULL) CMD_T_TAB=fa_b(NET_CMD_NUM,cname); // ���� ⠡����
 if(CMD_T_TAB==NULL) return;
 if(CMD_P_TAB==NULL) CMD_P_TAB=fa_b(NET_CMDP_NUM,cpname); // ���� ⠡����
 if(CMD_P_TAB==NULL) return;
 while(cmd!=NULL && cmd->cp==0)
  {
   recv_cmd_proc(o,cmd->b); cmd=cmd->prev;
   if(cmd==NULL) o->nb.rb=NULL; else cmd->next=NULL;
   myfree(o->nb.re->b); free_lnk(NULL,o->nb.re);
   o->nb.re=cmd;
  }
}
