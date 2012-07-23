#define NIF_RBUF	250

typedef struct Rbuf
{
 int nr; // ⥪�騩 ����� ���� �ਥ��
 int rf; // ����� �ਭ�������� ��: 0 �� �� ��稭����, 1 ��砫� �� �ਭ��, 2 ����� �� ������.
 char b[NIF_RBUF]; //���� ᡮન ������
} rbuf;

typedef struct Nif
{
 struct Lnk *sb; // ��砫� 楯�窨 ��ࠢ�� ⥣�� (� ��砫� ��⠢������ ���� ��)
 struct Lnk *se; // ����� 楯�窨 ��� (��� ������� �� ��ࠡ�⪠)
 struct Lnk *rb; // ��砫� 楯�窨 �ਥ�� ⥣�� (� ��砫� ��⠢������ ���� ��)
 struct Lnk *re; // ����� 楯�窨 ��� (��� ������� �� ��ࠡ�⪠)
 rbuf *trb;
} nif;


