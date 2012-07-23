#include "shape.h"
#include "orbit.h"
#include "devpar.h"

#define	GOF_EXIST	1	//�ਧ��� ����⢮�����
#define	GOF_ONMAP	2	//�ᯮ����� �� ����
#define	GOF_ISNIF	4	//����� 㤠������ �ࠢ�����

#define 	ONAM_LEN	32
#define		ELST_NUM	6

#include "mempar.h"

typedef struct Obj
{
 int		tag;
 struct Obj   	*prev;
 struct Obj	*next;

 union
 {
  char		name[ONAM_LEN];		//�⮡ࠦ����� ���
  char		password[ONAM_LEN]; 	// ��஫� ����
 };

 char		dn[ONAM_LEN];	//��� ����� ᮤ�ঠ饥 ��������, ��� ��ꥪ⮢ 
				// T_JUMP ����� ����� �� ������ �ॢ���� 3 ᨬ����
 union
 {
  DIR		*dirp;		//����� � ���ன �ᯮ����� ��ꥪ�
  SOCKET	cs;		//᮪�� ��ꥪ� T_NIF
 };
 

 struct Obj	*pl;	//��뫪� �� த�⥫�᪨� ��ꥪ�
 int		cnt;	//�᫮ ���譨� ��뫮� �� ��ꥪ�
 int		bitf;	//��⮢� 䫠�� ࠧ��筮�� �����祭��, � ⮬ �᫥ 䫠� ����⢮�����

 struct Obj	*bo;	//ᯨ᮪ �ᯮ��㥬�� ������� ��ꥪ⮢
 int		 lnkn;          //����� ᯨ᪠ ��뫮�
 struct Lnk	*blnk;		//ᯨ᮪ ��뫮� �� ����� sqrt(evtn) ����� ��।�
 int		 evtn;		//����� �����쭮� ��।� ᮡ�⨩
 struct Evt	*bevt;		//�����쭠� ��।� ᮡ�⨩

 struct Lnk	*elst[ELST_NUM];	//ᯨ᪨ ��뫮� �� ����ᨬ� ᮡ���

 union
 {
  shape		s;
  struct Nif	nb;		// ���� �ਥ�� � ��ࠢ�� ��ꥪ� T_NIF
  struct Lnk	*bo_turns;	// ����� 室�� ���⭨��� ��� � ����樨 T_BL
 };

 union
 {
  struct Lnk	*locl;	//ᯨ᮪ ����権 � ����� �������� ��ꥪ�.
  struct Loc	*locs;	//��㬥�� ���ᨢ ����権, ��� ��ꥪ� T_WORLD
 };

 int		gview;  //�ਧ��� ������쭮� �������� ��ꥪ�
 int		colc;	//����� ��⮢ ����� �⮫��������
 double		rt;	//�६� ��᫥���� �஢�ન ॠ�쭮�� ०���

 union
 {
  struct Obj	*nif;	// 㪠��⥫� �� ��ꥪ� - �⥢�� ����䥩� ��� ��ࠡ����
  int		nact;	//���稪 �����ꥪ⮢ T_BL �� �� ᤥ����� 室 + 1
  char 	*fpath;		// ���� ��� T_SHAB
 };

 orbit		orb;	// ��ࠬ���� �ࡨ�� ��� �ࡨ⠫��� ��ꥪ⮢
 devpar		dev;	// ��ࠬ���� ���ன�⢠ ��ࠡ��
 mempar		*mem;	// ������ ���⥩�� ��⮢

 int		bside;  // ��஭� �� ������ ���� ��ꥪ� � ��蠣���� ०���, ��� ��ꥪ� T_BL - �᫮ ��஭ � ��⢥
 int		bmode;  // ०�� ������ ����樨: 0 -ॠ�⠩�, 1 -��蠣���
 int		turn;	// ����� 室� ��� ��ꥪ� T_BL � ��ꥪ⮢ � �ਧ����� GOF_ONMAP

 struct Obj	*port;	// 㪠��⥫� �� ���⠫ ��� T_PERS

} obj;
