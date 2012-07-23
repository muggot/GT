#include "gt.h"
#include <sys\stat.h>

void evt_fload_obj_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_fl_obj;
 ne->t=GTIME;
}

long long *OBJ_T_TAB=NULL; // ⠡��� ��� ���᪠ �� ��ꥪ� �� �����
long long *OBJ_P_TAB=NULL; // ⠡��� ��� ���᪠ ᢮��� ��ꥪ� �� �����

/* ��᫮ �ࠪ���⨪ ��ꥪ⮢ */
#define		OBJ_PROP_NUM	38

const char *pname[OBJ_PROP_NUM] ={
 "/>",
#define		P_END		0
 "xs",
#define		P_XS		1
 "ys",
#define		P_YS		2
 "name_ru",
#define		P_NRU		3
 "name_en",
#define		P_NEN		4
 "xp",
#define		P_XP		5
 "yp",
#define		P_YP		6
 "img",
#define		P_IMG		7
 "login",   
#define		P_LOGIN		8
 "password",
#define		P_PASSWORD	9
 "v",
#define		P_V		10

// ��ࠬ���� �ࡨ⠫쭮�� ��������
 "orb_ox",
#define		P_ORB_OX	11
 "orb_oy",
#define		P_ORB_OY	12
 "orb_ang",
#define		P_ORB_ANG	13
 "orb_p",
#define		P_ORB_P		14
 "orb_e",
#define		P_ORB_E		15
 "orb_v",
#define		P_ORB_V		16
 "orb_t",
#define		P_ORB_T		17

// ��ࠬ���� ��אַ��������� ��������
 "tx",
#define		P_TX		18
 "ty",
#define		P_TY		19
 "ct",
#define		P_CT		20
 "gt",
#define		P_GT		21

// ��ࠬ���� �᭮���� ���㫥� ��ࠡ��
// ����
 "mass",
#define		P_M		22
// ����⢮
 "hull",
#define		P_HULL		23
// ᫮� ���ன�⢠ 
 "mslot",
#define		P_MYSL		24
// ���� ᫮⮢ (ᥪ権, ��ᥪ��), ���� ᫮� � ᯨ᪥ ᮡ�⢥��� 
 "dslot",
#define		P_DVSL		25
// ����� ��ࠬ����
 "def",
#define		P_DEF		26
// �⠪��騥 ��ࠬ����
 "atk",
#define		P_ATK		27
// ��ࠬ���� ���⭮�� ����
 "pwr",
#define		P_PWR		28
// ᯨ᮪ �ॡ������ � ���ன�⢠� ��ࠡ�� ��� �ᯮ�짮����� ������� ���ன�⢠
 "reqsl",
#define		P_RQSL		29
 "mem_home",
#define		P_MHOME		30
 "cur_task",
#define		P_MCTASK	31
 "cur_prik",
#define		P_MCPRIK	32
 "obj_lifet",
#define		P_MLIFET	33
 "mem_p",
#define		P_MP		34
 "mem_dat",
#define		P_MDAT		35
 "r1",					// ��䥪⨢�� ࠤ��� ��������� � ��ꥪ�
#define		P_R1		36
 "port"					// ��� ���⠫� ��� T_PERS, �� ����� 3 ᨬ�����
#define		P_PORT		37

};

void prop_xs_load(obj *o,char *str) { sscanf(str,"%f",&o->s.xs); }
void prop_ys_load(obj *o,char *str) { sscanf(str,"%f",&o->s.ys); }
void prop_nru_load(obj *o,char *str)
{
 int len=strlen(str); 
 if(len>ONAM_LEN-1) len=ONAM_LEN-1;
 snprintf(o->name,len+1,"%s",str);
}
void prop_nen_load(obj *o,char *str) { }
void prop_xp_load(obj *o,char *str) { sscanf(str,"%Lf",&o->s.cx); }
void prop_yp_load(obj *o,char *str) { sscanf(str,"%Lf",&o->s.cy); }
void prop_r1_load(obj *o,char *str) { sscanf(str,"%f",&o->s.r1); }
void prop_port_load(obj *o,char *str) { sscanf(str,"%s",(char *)&o->port); }
void prop_v_load(obj *o,char *str) { sscanf(str,"%f",&o->s.v); }
void prop_img_load(obj *o,char *str) { sscanf(str,"%d",&o->s.img); }
void prop_login_load(obj *o,char *str) { }
void prop_password_load(obj *o,char *str) 
{
 int len=strlen(str); 
 if(len>ONAM_LEN-1) len=ONAM_LEN-1;
 snprintf(o->password,len+1,"%s",str);
}

void prop_orb_ox_load(obj *o,char *str) { sscanf(str,"%Lf",&o->orb.ox); }
void prop_orb_oy_load(obj *o,char *str) { sscanf(str,"%Lf",&o->orb.oy); }
void prop_orb_ang_load(obj *o,char *str) { sscanf(str,"%f",&o->orb.ang); }
void prop_orb_p_load(obj *o,char *str) { sscanf(str,"%f",&o->orb.p); }
void prop_orb_e_load(obj *o,char *str) { sscanf(str,"%f",&o->orb.e); }
void prop_orb_v_load(obj *o,char *str) { sscanf(str,"%f",&o->orb.v); }
void prop_orb_t_load(obj *o,char *str) { sscanf(str,"%Lf",&o->orb.t); }

void prop_mem_home_load(obj *o,char *str) { sscanf(str,"%Lf,%Lf,%s",&o->mem->hx,&o->mem->hy,o->mem->hname); }
void prop_mem_cur_task_load(obj *o,char *str) { sscanf(str,"%d,%Lf",&o->mem->ct,&o->mem->tt); }
void prop_mem_cur_prik_load(obj *o,char *str) { sscanf(str,"%d,%Lf",&o->mem->cp,&o->mem->tp); }
void prop_mem_obj_lifet_load(obj *o,char *str) { sscanf(str,"%Lf,%Lf",&o->mem->tb,&o->mem->te); }
void prop_mem_mem_p_load(obj *o,char *str) 
{ 
 sscanf(str,"%Lf,%Lf,%d;%Lf,%Lf,%d;%Lf,%Lf,%d;%Lf,%Lf,%d;%Lf,%Lf,%d;%Lf,%Lf,%d",
 &o->mem->p[0][0],&o->mem->p[0][1],&o->mem->pid[0],
 &o->mem->p[1][0],&o->mem->p[1][1],&o->mem->pid[1],
 &o->mem->p[2][0],&o->mem->p[2][1],&o->mem->pid[2],
 &o->mem->p[3][0],&o->mem->p[3][1],&o->mem->pid[3],
 &o->mem->p[4][0],&o->mem->p[4][1],&o->mem->pid[4],
 &o->mem->p[5][0],&o->mem->p[5][1],&o->mem->pid[5]); 
}
void prop_mem_dat_load(obj *o,char *str) { sscanf(str,"%d,%d",&o->mem->d[0],&o->mem->d[1]); }

void prop_tx_load(obj *o,char *str) { sscanf(str,"%f",&o->s.tx); }
void prop_ty_load(obj *o,char *str) { sscanf(str,"%f",&o->s.ty); }
void prop_ct_load(obj *o,char *str) { sscanf(str,"%Lf",&o->s.ct); }

void prop_gt_load(obj *o,char *str) { sscanf(str,"%Lf",&GTIME); }

void prop_mass_load(obj *o,char *str) { sscanf(str,"%f",&o->dev.m); }
void prop_hull_load(obj *o,char *str) { sscanf(str,"%f,%f",&o->dev.hull,&o->dev.chull); }
void prop_def_load(obj *o,char *str) 
{ 
 sscanf(str,"%f-%f,%f-%f,%f-%f,%f-%f,%f-%f,%f-%f,%f-%f",
 &o->dev.defp[0][0],&o->dev.defp[0][1],&o->dev.defp[1][0],&o->dev.defp[1][1],
 &o->dev.defp[2][0],&o->dev.defp[2][1],&o->dev.defp[3][0],&o->dev.defp[3][1],
 &o->dev.defp[4][0],&o->dev.defp[4][1],&o->dev.defp[5][0],&o->dev.defp[5][1],
 &o->dev.defp[6][0],&o->dev.defp[6][1] ); 
}

void prop_atk_load(obj *o,char *str) 
{ 
 sscanf(str,"%f-%f,%f-%f,%f-%f,%f-%f,%f-%f,%f-%f,%f-%f",
 &o->dev.atkp[0][0],&o->dev.atkp[0][1],&o->dev.atkp[1][0],&o->dev.atkp[1][1],
 &o->dev.atkp[2][0],&o->dev.atkp[2][1],&o->dev.atkp[3][0],&o->dev.atkp[3][1],
 &o->dev.atkp[4][0],&o->dev.atkp[4][1],&o->dev.atkp[5][0],&o->dev.atkp[5][1],
 &o->dev.atkp[6][0],&o->dev.atkp[6][1] ); 
}

void prop_mysl_load(obj *o,char *str) 
{
 lnk *l;
 l=get_lnk();
 sscanf(str,"%c%d,%d",&l->st[0],&l->sc,&l->sn);
 l->next=o->dev.mysl; l->prev=NULL; l->o=NULL; o->dev.mysl=l;
}

void prop_dvsl_load(obj *o,char *str) 
{
 lnk *l;
 l=get_lnk();
 sscanf(str,"%c%d,%d",&l->st[0],&l->sc,&l->sn);
 l->next=o->dev.dvsl; l->prev=NULL; l->o=NULL; o->dev.dvsl=l;
 if(o->tag==T_SHP)
 {
  while(l->sn>1)
  {
   l=get_lnk();
   l->st[0]=o->dev.dvsl->st[0]; l->sc=o->dev.dvsl->sc; l->o=NULL;
   l->sn=o->dev.dvsl->sn-1; l->next=o->dev.dvsl; l->prev=NULL; o->dev.dvsl=l;
  }
 }
}

void prop_pwr_load(obj *o,char *str) { sscanf(str,"%f,%f,%f,%f",&o->dev.pwr[0],&o->dev.pwr[1],&o->dev.pwr[2],&o->dev.pwr[3]); }

void prop_rqsl_load(obj *o,char *str) 
{
 lnk *l;
 l=get_lnk();
 sscanf(str,"%c%d,%d",&l->st[0],&l->sc,&l->sn);
 l->next=o->dev.rqsl; l->prev=NULL; l->o=NULL; o->dev.rqsl=l;
}


void (*prop_func_load[OBJ_PROP_NUM])(obj *o,char *str)=
{
 NULL,
 prop_xs_load,
 prop_ys_load,
 prop_nru_load,
 prop_nen_load,
 prop_xp_load,
 prop_yp_load,
 prop_img_load,
 prop_login_load,
 prop_password_load,
 prop_v_load,
 prop_orb_ox_load,
 prop_orb_oy_load,
 prop_orb_ang_load,
 prop_orb_p_load,
 prop_orb_e_load,
 prop_orb_v_load,
 prop_orb_t_load,
 prop_tx_load,
 prop_ty_load,
 prop_ct_load,
 prop_gt_load,
 prop_mass_load,
 prop_hull_load,
 prop_mysl_load,
 prop_dvsl_load,
 prop_def_load,
 prop_atk_load,
 prop_pwr_load,
 prop_rqsl_load,
 prop_mem_home_load,
 prop_mem_cur_task_load,
 prop_mem_cur_prik_load,
 prop_mem_obj_lifet_load,
 prop_mem_mem_p_load,
 prop_mem_dat_load,
 prop_r1_load,
 prop_port_load
};

int set_prop(obj *o,char *buf,int sz)
{
 int pn;
 char *tp;
 do
 {
  while(*buf<=0x20 && *buf!=0) buf++; if(*buf==0) break;
  tp=buf; // ��諨 ��砫� ����� ��ࠬ���
  while(*buf>0x20 && *buf!='=') buf++;
  if(*buf==0 && (tp[0]!='/' || tp[1]!='>')) return -1; //䠩� ��ꥪ� ���०���
  *buf=0; buf++; // ��� ��ࠬ��� ��諨
  pn=fa_s(tp,OBJ_P_TAB); if(pn==0) return 0; // ��諨 �� ���� 䠩��
  while(*buf!='"' && *buf!=0) buf++; // ᪠����� �� ��砫� ���祭��
  if(*buf==0) return -1; //䠩� ��ꥪ� ���०���
  buf++; tp=buf;
  while(*buf!='"' && *buf!=0) buf++; // ᪠����� �� ���� ���祭��
  if(*buf==0) return -1; //䠩� ��ꥪ� ���०���
  *buf=0; buf++;
  if(pn>0) prop_func_load[pn](o,tp);
 }
 while(*buf!=0);
 return -1;
}

obj *fload_obj(obj *po,char *name) // ���뢠�� ���ᠭ�� ��ꥪ� �� 䠩�� object.dat
{
 char tag[32], *buf;
 obj *o;
 struct stat fs;
 int sz,tagn;
 FILE *fp;

 while(po->tag==T_DIR) po=po->pl; // po �� ����� ����⨯��� ��ꥪ⮢, ����७��� ��ꥪ�� ������ 楯������ � த�⥫�᪮��

 if(OBJ_T_TAB==NULL) OBJ_T_TAB=fa_b(OBJ_TAG_NUM,oname); // ���� ⠡����
 if(OBJ_T_TAB==NULL) return(NULL);
 if(OBJ_P_TAB==NULL) OBJ_P_TAB=fa_b(OBJ_PROP_NUM,pname); // ���� ⠡����
 if(OBJ_P_TAB==NULL) return(NULL);

 fp=fopen("object.dat","rt"); if(fp==NULL) return(NULL);

 fscanf(fp,"%31s",tag); if(tag[0]!='<') { fclose(fp); return(NULL); }
 tagn=fa_s(tag+1,OBJ_T_TAB); if(tagn<0) { fclose(fp); return(NULL); }

 o=get_obj(po,tagn,name); if(o==NULL) { fclose(fp); return(NULL); }

 fstat(fp->_handle,&fs);
 buf=mymalloc(fs.st_size); sz=fread(buf,1,fs.st_size,fp); buf[sz]=0;

 sz=set_prop(o,buf,sz); if(sz<0) { del_obj(o); free_obj(o); o=NULL; }

// if(tagn==T_PERS) 
 if(name[0]!='@') add_user(name,o); // ॣ������� �� �� �������᪨� ��ꥪ��

 myfree(buf); fclose(fp); 

 fp=fopen("orbit.dat","rt"); 
 if(fp!=NULL)
 {
  fscanf(fp,"%31s",tag); 
  if(strcmp(tag,"<ORB")==0)
  {
   fstat(fp->_handle,&fs);
   buf=mymalloc(fs.st_size); sz=fread(buf,1,fs.st_size,fp); buf[sz]=0;
   sz=set_prop(o,buf,sz);
   myfree(buf); 
  }
  fclose(fp);
 }

 fp=fopen("mempar.dat","rt"); 
 if(fp!=NULL)
 {
  fscanf(fp,"%31s",tag); 
  if(strcmp(tag,"<MEMPAR")==0)
  {
   o->mem=mymalloc(sizeof(mempar)); memset(o->mem,0,sizeof(mempar));
   fstat(fp->_handle,&fs);
   buf=mymalloc(fs.st_size); sz=fread(buf,1,fs.st_size,fp); buf[sz]=0;
   sz=set_prop(o,buf,sz);
   myfree(buf); 
  }
  fclose(fp);
 }

 fp=fopen("dynamic.dat","rt"); 
 if(fp!=NULL)
 {
  fscanf(fp,"%31s",tag); 
   fstat(fp->_handle,&fs);
   buf=mymalloc(fs.st_size); sz=fread(buf,1,fs.st_size,fp); buf[sz]=0;
   sz=set_prop(o,buf,sz);
   myfree(buf); 
  fclose(fp);
 }

 return(o);
}

void evt_fload_obj_act(obj *o,evt *e,lnk *l)
{
 struct dirent *dep;
 obj *co;

 // 㤠��� ⥪�饥 ᮡ�⨥.
 del_evt(o,e); free_evt(o,e);

 if(o->dirp==NULL) // ����� �� �� �����, ��஥� ��
  { o->dirp=opendir("."); if(o->dirp==NULL) return; }
 // ����� ����� �⠥� ����� � ���
 do{
    dep=readdir(o->dirp);
    // �஢�ਬ �� ���稫��� �� �����
    if(dep==NULL) { closedir(o->dirp); chdir(".."); return; } // ���室�� � த�⥫�᪨� ��⠫��
   }
 while(!(dep->d_attr&_A_SUBDIR) || strcmp(dep->d_name,".")==0 || strcmp(dep->d_name,"..")==0);
 // � ����� �� ���� ��������, ��⠢�� � ��।� ����� ᮡ�⨥ ����㧪�
 // ��ꥪ⮢

 e=get_evt(); evt_fload_obj_set(e,o,NULL,NULL,0.0); ins_evt(o,e);
 
 // ������� ��������, ����室���: ��३� �㤠; ����㧨�� ��ꥪ� �� 䠩��;
 // ��ନ஢��� ����� ᮡ�⨥ ����㧪� ���୨� ��ꥪ⮢
 if(chdir(dep->d_name)!=0) return;
 co=fload_obj(o,dep->d_name); if(!co) { chdir(".."); return; } // 䠩� ��ꥪ� ���०���, �����.
 if(co->tag==T_SHAB) { chdir(".."); return; } // 蠡����� ��ꥪ�� ���� ���뢠���� �����।�⢥��� ��। �ᯮ�짮������

 e=get_evt(); evt_fload_obj_set(e,co,NULL,NULL,0.0); ins_evt(co,e);
}

void evt_fload_newobj(obj *o,char *d_name)
{
 struct dirent *dep;
 obj *co;
 evt *e;

 if(chdir(d_name)!=0) return;
 co=fload_obj(o,"@"); if(!co) { chdir(".."); return; } // 䠩� ��ꥪ� ���०���, �����.

 e=get_evt(); evt_fload_obj_set(e,co,NULL,NULL,0.0); ins_evt(co,e);
}
