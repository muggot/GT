#include <stdlib.h>
#include <math.h>
#include <windows.h>
#define M_PROC
#include "gt.h"

#include "prep.h"
extern struct conf cfg;

obj GAME={0};

int obj_GAME_init(void)
{
 GAME.tag=T_GAME;
 GAME.cnt=1;
 return(0);
}

int main(int argn, char**argv)
{
 evt *e;
 int res;
 obj *wo; //���
 double gt;

 if(getcfg()!=0) { printf("2 �訡�� �� �⥭�� 䠩�� ����஥�\n"); return 1; }


// ���樠������ ������쭮�� ������� ���� ��ꥪ⮢
 GetSystemTimeAsFileTime((LPFILETIME)&OGEN);

 obj_save_thread_init();
 init_user_tab();

 obj_GAME_init();
 chdir(cfg.data_path);
 wo=fload_obj(&GAME,"WORLD_01"); if(!wo) return(1);

// ��୥�� ��ꥪ�� ����㦥��, �㦭� ��稭��� ����㧪� ��⠫���
// �ନ��� ᮡ�⨥ ���뢠��� � ���樠����樨 ���୨� ��ꥪ⮢ ���

 gt=GTIME; GTIME=0.0;
 e=get_evt(); evt_func_set[E_fl_obj](e,wo,NULL,NULL,0.0);  ins_evt(wo,e);
 e=get_evt(); evt_func_set[E_init_obj](e,wo,NULL,NULL,gt); ins_evt(wo,e);
 e=get_evt(); evt_func_set[E_init_net](e,wo,NULL,NULL,gt+1.0); ins_evt(wo,e);
 proc_evts(wo);
 return(0);
}
