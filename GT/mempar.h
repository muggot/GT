// ������� ����� ᯠમ�, ��堭�����, 䠣�� � ���� ������ ��⮢
typedef struct Mempar
{
 double hx,hy;	// ���न���� ����
 char hname[ONAM_LEN]; // ��� ����
 int ct,cp;	// ⥪��� ����� � ⥪�騩 �ਪ��
 double tt,tp;	// �६� ����㯫���� ����� � �ਪ��� ᮮ⢥��⢥���
 double tb,te;	// �६� ஦����� � ����砭�� ����� ��ꥪ�

 float p[6][3]; // ���न���� � ���������� ��᫥���� 6 �������� ���ந��� (��� ᯠમ�)
 int   pid[6];  // �����䨪���� -//-//-

 int d[8];	//  d[0] �� ࠧ�襭�� � �������
		//  d[1] ����稥 ������ �⮫�������� �� ��襤訩 ��ਮ�: 0 �� �뫮, 1 �뫨
		//  d[2] ������⢮ ��� ���ॡ������ �� ��ਮ�
		//  d[3] ������⢮ ��� �ந��������� �� ��ਮ�
 
} mempar;
