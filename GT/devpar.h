typedef struct Devpar
{
 float	m,cm;		// ��砫쭠� ���� ���ன�⢠, ⥪��� ���� ���ன�⢠
 float	hull,chull;	// ��砫쭮� � ⥪�饥 ����⢮ ���ன�⢠
 struct Lnk	*mysl;		// ���ᠭ�� ᮡ�⢥���� ᥪ樨
 struct Lnk	*dvsl;		// ���ᠭ�� ���஥���� ᥪ権 ���ன�⢠
 struct Lnk	*rqsl;		// �ॡ������ � ������ ��㣨� ���ன�� �� ��ࠡ��
 union
 {
  float	defp[7][2];	// ����� ��ࠬ���� ���ன�⢠
  float	atkp[7][2];	// ���०���騥 ��ࠬ���� ���ன�⢠
 };
 float	pwr[4];		// ᨫ� ���⭮�� ���� (����. �����, ⥪�騩 �����, ᪮���� ����⠭�������, ���ॡ�塞�� ��魮���)
 			// ᨫ� �����⥫�, ࠤ��, ���⮢��� ��������, ஡�⮢
} devpar;
