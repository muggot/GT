#include "evt_type.h"

typedef struct Evt
{
 int	tag;
 struct Evt  *prev;
 struct Evt  *next;
 struct Evt  *pe; 	// �ᯮ������ ⮫쪮 �᫨ ���� ���� ᮡ�⨥�, 㪠�뢠�� �� ��������� ᮡ���-��뫪� � த�⥫�᪮� ��।�
 struct Lnk  *ql;	// �� 㪠��⥫� �� ��������� ��뫪� ��।�
 double t;		// �६�
 struct Obj  *o;	// ��ꥪ�, ���祭�� ������ �� ⨯� ᮡ���
 struct Evt  *e;	// ����ୠ⨢��� ᮡ�⨥, � �᭮���� �ᯮ������ � ᮡ���� 㭨�⮦���� ��� 㪠��⥫� �� 㭨�⮦����� ᮡ�⨥.
 struct Lnk  *l;        // 㪠��⥫� �� ��뫪� �� ᯨ᪠ ����ᨬ���, ⮫쪮 ��� ॠ�権
} evt;

extern double GTIME;
extern long long OGEN;
