typedef struct Lnk
{
 union
 {
  int	     tag;	// �� ॠ�樨 ��� ��뫮� ࠧ��頥��� � ᯨ᪠� ॠ�権
  char st[4];		// ⨯ ᫮�
 };
 struct Lnk  *prev;
 struct Lnk  *next;
 union
 {
  struct Obj  *o;	// 㪠�뢠�� �� ��ꥪ� � ���஬� �ਪ९���� ��뫪�
  int n;		// ����� ��
 };
 union
 {
  struct Evt  *e;	// 㪠�뢠�� �� ᮡ�⨥ ���஥ ��뢠���� ��� NULL �᫨ ᮡ�⨥ 㪠���� � ��㣮� ��뫪� 楯�窨
  int cp;		// ⥪�騩 ���稪 ��ࠢ������ ����, � ����� �����襭����� �ਭ�⮣� ��
  int sc;		// ����� ᫮�
  int ln;		// ����� ����樨
  float tx;
 };
 union
 {
  struct Lnk  *l;	// 㪠�뢠�� �� ᫥������ � 楯�窥 �痢� ��뫪�, ���� ᠬ� ᥡ�
  char *b; 		// ᮤ�ন��� �� �ਥ�� ��� ��ࠢ��
  int sn;		// ����� ᫮�
  float ty;
 };
} lnk;
