#include "gt.h"
extern int EVT_STOP_FLAG;
float NIF_STEP=0.1;
static float MIN_NIF_STEP=0.001;

void evt_init_net_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_init_net;
 ne->t=GTIME+time;
 ne->o=o;
 ne->e=e;
 ne->l=l;
}

void evt_sync_net_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_sync_net;
 ne->t=GTIME+time;
 ne->o=o;
 ne->e=e;
 ne->l=l;
}

static HANDLE th;
static long long LTIME;
static obj *wo;

#define DEFAULT_PORT 5001
#define DEFAULT_IF   "0.0.0.0"

DWORD mylisten(LPDWORD param)
{
 DWORD s;
 struct sockaddr_in local, from;
 int fromlen=sizeof(from), ioctl_opt=1;
 SOCKET listen_socket, consock;
 obj *o;
 evt *e;
 lnk *l;

 local.sin_family = AF_INET;
 local.sin_addr.s_addr = inet_addr(DEFAULT_IF); 
 local.sin_port = htons(DEFAULT_PORT);

 listen_socket = socket(AF_INET, SOCK_STREAM,0);
 bind(listen_socket,(struct sockaddr*)&local,sizeof(local));
 listen(listen_socket,5);
 

 while(1)
 {
  consock= accept(listen_socket,(struct sockaddr*)&from,&fromlen);
//  consock= accept(listen_socket,NULL,0);
  if (consock == INVALID_SOCKET) break;
  // ����� ᮥ������� �����, �㦭� ���� ������� ����� � ��஫�, ����
  // ������� ��� �� ⠩�����
  // ᮧ����� �� �஢�� WORLD ���� ��ꥪ� - �⥢�� ����䥩�
  // ��ନ�㥬 2 ᮡ���: ��ࠢ�� �ਣ��襭�� � ������� ᮥ������� ��
  // ⠩�����. ���⠢�� ��� ����� ᮡ�⨥� �ᯥ譮� ���ਧ�樨 � �����襭���
  // ᮥ�������.
  ioctl_opt=1;
  ioctlsocket(consock,FIONBIO,&ioctl_opt);
  o=get_obj(wo,T_NIF,""); if(o==NULL) break;
  o->cs=consock;
  o->nb.trb=mymalloc(sizeof(struct Rbuf)); o->nb.trb->nr=0; o->nb.trb->rf=0;

  cmd_send_hello(o);

  // ᮡ�⨥ ������� ᮥ�������
//  e=get_evt(); evt_func_act[E_close_nif](e,o,NULL,NULL,10.0); ins_evt(o,e);
  // ᮡ�⨥ �⬥�� ������� ᮥ�������
//  e=get_evt(); evt_func_act[E_!close_nif](e,o,NULL,NULL,10.0);
  // ��⠭���� ��ࠡ��� ��।� ᮡ�⨩
  EVT_STOP_FLAG=1;
  while(EVT_STOP_FLAG!=2) Sleep(0);
  o->rt=GTIME; // �६� ஦����� ��ꥪ�, � ���쭥�襬 �६� ��᫥����� �⪫��� �� ������
  // ����᪠� ᮡ�⨥ �ਥ�� ᮮ�饭�� �� ������
  e=get_evt(); evt_func_set[E_recv_nif](e,o,NULL,NULL,0.1); ins_evt(o,e);
  EVT_STOP_FLAG=0;
 }

 return s;
}

// �㦭� �ந��樠����஢��� ���
void evt_init_net_act(obj *o,evt *e,lnk *l)
{
 DWORD s,r,id;
 WORD wVersionRequested;
 WSADATA wsaData;
 int err;

 wo=o;
 del_evt(o,e); free_evt(o,e);

 wVersionRequested=MAKEWORD(2,0);
 err=WSAStartup(wVersionRequested, &wsaData); 
 if(err) { printf("�訡�� ���樠����樨 ᮪�⮢"); exit(1); }

 if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=0) 
  { WSACleanup(); printf("����室���� ����� ᮪�⮢ �� �����ন������",""); exit(1); }

 th=CreateThread(0,0,(LPTHREAD_START_ROUTINE) mylisten,0,0,&id);
 if(th==NULL) { printf("�訡�� ����᪠ ��⮪� �ਥ�� ᮥ�������"); exit(1); }

// ᮧ��� ᮡ�⨥ ᨭ�஭���樨 � 楯��� ��� �� �஢��� ��ꥪ� WORLD

 GetSystemTimeAsFileTime((LPFILETIME)&LTIME);

 e=get_evt(); evt_sync_net_set(e,o,NULL,NULL,1.0); ins_evt(o,e);
}

void evt_sync_net_act(obj *o,evt *e,lnk *l)
{
 long long ct,dt;

 del_evt(o,e); free_evt(o,e);
 e=get_evt(); evt_sync_net_set(e,o,NULL,NULL,0.1); ins_evt(o,e);
 GetSystemTimeAsFileTime((LPFILETIME)&ct); dt=ct-LTIME; 
 if(dt>=0 && dt<900000 && NIF_STEP>MIN_NIF_STEP) NIF_STEP*=0.9;
 else if(dt>900000) NIF_STEP*=1.1;
 if(dt>=0 && dt<1000000) Sleep(100-dt*0.0001);
 GetSystemTimeAsFileTime((LPFILETIME)&LTIME); 
// printf("Delta: %f\n",LTIME*0.001-GTIME);
// printf("�६� %d, 蠣 %f\n",LTIME,NIF_STEP);
}
