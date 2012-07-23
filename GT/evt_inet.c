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
  // новое соединение открыто, нужно либо получить логин и пароль, либо
  // закрыть его по таймауту
  // создадим на уровне WORLD новый объект - сетевой интерфейс
  // сформируем 2 события: отправки приглашения и закрытия соединения по
  // таймауту. Поставим связь между событием успешной авторизации и завершением
  // соединения.
  ioctl_opt=1;
  ioctlsocket(consock,FIONBIO,&ioctl_opt);
  o=get_obj(wo,T_NIF,""); if(o==NULL) break;
  o->cs=consock;
  o->nb.trb=mymalloc(sizeof(struct Rbuf)); o->nb.trb->nr=0; o->nb.trb->rf=0;

  cmd_send_hello(o);

  // событие закрытия соединения
//  e=get_evt(); evt_func_act[E_close_nif](e,o,NULL,NULL,10.0); ins_evt(o,e);
  // событие отмены закрытия соединения
//  e=get_evt(); evt_func_act[E_!close_nif](e,o,NULL,NULL,10.0);
  // остановим обработку очереди событий
  EVT_STOP_FLAG=1;
  while(EVT_STOP_FLAG!=2) Sleep(0);
  o->rt=GTIME; // время рождения объекта, в дальнейшем время последнего отклика от клиента
  // запускаю событие приема сообщений от клиента
  e=get_evt(); evt_func_set[E_recv_nif](e,o,NULL,NULL,0.1); ins_evt(o,e);
  EVT_STOP_FLAG=0;
 }

 return s;
}

// нужно проинициализировать сеть
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
 if(err) { printf("Ошибка инициализации сокетов"); exit(1); }

 if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=0) 
  { WSACleanup(); printf("Необходимая версия сокетов не поддерживается",""); exit(1); }

 th=CreateThread(0,0,(LPTHREAD_START_ROUTINE) mylisten,0,0,&id);
 if(th==NULL) { printf("Ошибка запуска потока приема соединений"); exit(1); }

// создаю событие синхронизации и цепляю его на уровень объекта WORLD

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
// printf("Время %d, шаг %f\n",LTIME,NIF_STEP);
}
