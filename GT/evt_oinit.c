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
 loc_col_bmv(o); // ��������� ��� �⪠�� �� �०���� ०��� �������� �१ ������� mv
}

void obj_SP_init(obj *o,evt *e,lnk *l)
{
 evt *me;
 lnk *ml;
 double dr;
 if(o->pl->tag==T_WORLD) // �᫨ ᯠ� ����� ��㣮�� ��ꥪ�, � �� ���㤠 �� ���� 
 {
  dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
  o->s.tx=0; o->s.ty=0; o->s.ct=GTIME;
  obj_FF_init(o,e,l);
 }
 o->dev.cm=0; o->dev.pwr[0]=o->dev.pwr[1]=o->dev.pwr[2]=0;
 if(o->pl->tag!=T_SP) // �᫨ ᯠ� �� ��室��� � ����� � ����, � �� ������ �㬠��
 {
  me=get_evt(); evt_func_set[E_spark](me,o,NULL,NULL,1.0+(49.0*rand()/RAND_MAX)); ins_evt(o,me);
// ��᪮��� ᯠન ����� ����� � ��ᬮ�, � �㦭� ��ॣ����஢��� ॠ���
// �� ���४�� ��� 㭨�⮦���� ᮡ��� ����祭�� � �࠭�楩 ����樨
// � १���� ��᫥ ��ࢮ�� �맮�� E_spark, �㤥� �맢��� E_cold_loc,
// ���஥ �� ����室����� ᮧ���� E_col_loc � ��⠢�� ��� � ��।�.
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
 me=get_evt(); evt_func_set[E_bl_dummy](me,o,NULL,NULL,3.0); ins_evt(o,me); // 室 ���⮣� ��ꥪ� (���� �� ���������� 室��)
 me=get_evt(); evt_func_set[E_bl_eturn](me,o,NULL,NULL,40.0); ins_evt(o,me); // ���室 室� �� ⠩�����
 de=get_evt(); evt_func_set[E_bl_eturnd](de,o,NULL,NULL,0.0);
 me->e=de; de->e=me; nl=evt_lnk_ins(lnks_tags[E_bl_dummy],o,de,NULL); de->l=nl; nl->l=nl;
 o->turn=0; // ���� 室
 co=o->bo; o->nact=1;
 while(co!=NULL) 
 { 
  o->nact++; 
//  if(co->bitf&GOF_ISNIF!=0 && co->s.me!=NULL) // ��ꥪ� �ࠢ�塞� � ��������, ��⠭����� �������
//   evt_func_act[E_mvf_user](co,co->s.me,NULL);

  co->turn=0; // ���� 室

// ��� ���஥���� ��⮢ ᮧ���� ᮡ�⨥ �����஢���� 室� � ��⠢�塞 � ��।�
  if(co->bitf&GOF_ISNIF==0)
  {
   me=get_evt(); evt_func_set[E_bt_obj](me,co,NULL,NULL,3.0*rand()/RAND_MAX); ins_evt(co,me);
  }
  
  co=co->next;
 }
 o->rt=GTIME; // �६� ����砭�� �।��饣� 室�, ᮢ������ � �६���� ��砫� ⥪�饣�
 cmd_send_bl_begin(o); // ������⨬ ���⭨��� � ��砫� ���
}

void obj_BL_init_rt(obj *o,evt *e,lnk *l)
{
 obj *co;
 evt *me,*de;
 lnk *nl;

 co=o->bo;
 while(co!=NULL) 
 { 
// ��� ���஥���� ��⮢ ᮧ���� ᮡ�⨥ �����஢���� 室� � ��⠢�塞 � ��।�
  if(co->tag!=T_PERS)
  {
   me=get_evt(); evt_func_set[E_bobj](me,co,NULL,NULL,3.0*rand()/RAND_MAX); ins_evt(co,me);
  }
  co=co->next;
 }
 cmd_send_bl_begin(o); // ������⨬ ���⭨��� � ��砫� ���
}

void obj_SHP_init(obj *o,evt *e,lnk *l)
{
 if(o->dev.mysl->sn==0) return; // ���ன�⢮ �� ��⠭������
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

// ��� ����� ���ன�⢠
 o->dev.cm=o->dev.m;
 co=o->pl; while(co->tag!=T_WORLD) { co->dev.cm+=o->dev.m; co=co->pl; }

 if(o->dev.mysl==NULL || o->dev.mysl->sn==0) return; // ���ன�⢮ �� ��⠭������
 if(o->pl->dev.mysl==NULL) 
  {
   co=get_TAG_obj(o->pl->bo,T_SHP);
   if(co==NULL) return; // �訡�� � ������� ������, ����� ���������, � ��稭�� ����
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
 if(o->dev.mysl->sn==0) return; // ���ன�⢮ �� ��⠭������
 o->pl->dev.pwr[0]+=o->dev.pwr[0]; // �㬬���� ��魮��� �����⥫�� ��ࠡ��
 o->pl->dev.pwr[1]+=o->dev.pwr[1]; // �㬬���� ��魮��� �����⥫�� ��ࠡ��
 o->pl->dev.pwr[2]+=1.0/o->dev.pwr[2]; // �㬬���� �����७����� �����⥫�� ��ࠡ��
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

 if(o->tag==T_SHAB) return; // ���⥩��� 蠡�����, 蠡���� �� ���樠�����㥬.

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
