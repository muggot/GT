// обработка принятых команд от клиента
#include "gt.h"
#include "evt_btl.h"

void evt_mvs_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_mvs_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_mvf_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_mvf_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}

void evt_mvfd_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_mvfd_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}


void evt_prich_user_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
{
 ne->tag=E_prich_user;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
}
}


#define NET_CMD_NUM		15
const char *cname[NET_CMD_NUM] ={
 "/",
#define		C_END		0
 "AUTH",
#define		C_AUTH		1
 "MV",
#define		C_MV		2
 "PONG",
#define		C_PONG		3
 "CHAT",
#define		C_CHAT		4
 "BS",					// начало боя или вмешательство в существующий
#define		C_BS		5
 "T",					// план хода в бою
#define		C_BT		6
 "/T",					// конец плана хода в бою
#define		C_ET		7
 "BM",					// движение в бою
#define		C_BM		8
 "PRICH",				// причалить к объекту
#define		C_PRICH		9
 "OTCH",				// причалить к объекту
#define		C_OTCH		10
 "BUSE",				// стрелять
#define		C_BUSE		11
 "BEXIT",				// выйти из боя
#define		C_BEXIT		12
 "NEWP",				// создать нового перса
#define		C_NEWP		13
 "MNT"					// установить/размонтировать устройство
#define		C_MNT		14
};

#define NET_CMDP_NUM	14
const char *cpname[NET_CMDP_NUM] ={
 "/",
#define		CP_CXML		0
 "login",
#define		CP_LOGIN	1
 "pass",
#define		CP_PASS		2
 "tx",
#define		CP_TX		3
 "ty",
#define		CP_TY		4
 "t",
#define		CP_T		5
 "obj",
#define		CP_OBJ		6
 "tobj",
#define		CP_TOBJ		7
 "enemy",
#define		CP_ENEMY	8
 "x",
#define		CP_X		9
 "y",
#define		CP_Y		10
 "txt",
#define		CP_TXT		11
 "ta",
#define		CP_TA		12
 "slot"
#define		CP_SLOT		13
};
char *cpval[NET_CMDP_NUM]={0};

int cmds_react[NET_CMD_NUM]=
{
 -1,	-1,	0,	-1,	
 -1,	-1,	-1,	-1,
 -1,    -1,	-1,	-1,
 -1,	-1,	-1
};



// событие начала движения корабля
void evt_mvs_user_act(obj *o,evt *e,lnk *l)
{
 evt *me,*de;
 obj *co=o->nif;
 lnk *nl;

 del_evt(o,e); free_evt(o,e);

// if(co==NULL) return;
 
 // создаю два события: завершения движения и его антисобытие
 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm);
 me=get_evt(); evt_mvf_user_set(me,o,NULL,NULL,o->s.ct); ins_evt(o,me); o->s.me=me;
 de=get_evt(); evt_mvfd_user_set(de,o,NULL,NULL,0);
 me->e=de; de->e=me; nl=evt_lnk_ins(cmds_react[C_MV],co,de,NULL); de->l=nl; nl->l=nl;
 o->s.ct=GTIME;
 locs_send_mv(o);

 // нужно: расчитать время столкновения с границами локаций, вставить
 // события столкновения и их антисобытия, ссылки на антисобытия вставить
 // в список реакций на команду MV
 // всего может быть 4 события столкновения, можно вставить сразу 4 события,
 // а можно ограничиться ближайшим по времени, но тогда при его срабатывании
 // нужно снова искать ближайшее столкновение. Второй вариант легче в реализации.
 // его и делаю.
 loc_col(o,NULL);

 // нужно сохранить состояние объекта (позицию и параметры движения)
 obj_save_update(o);
 obj_save_cpoint();
}

lnk *rmi_lnk_mv(obj *co,evt *de)
{
 return evt_lnk_ins(cmds_react[C_MV],co,de,NULL);
}

// событие завершения движения корабля
void evt_mvf_user_act(obj *o,evt *e,lnk *l)
{
 double dr;
 lnk *ml,*dl,*tl;
// obj *co=o->nif;

 del_evt(o,e); free_evt(o,e);
 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.tx=0; o->s.ty=0; o->s.ct=0.0;

 o->s.me=NULL;

// if(co==NULL) return;
 locs_send_mv(o);
}

// событие прерывания движения корабля
void evt_mvfd_user_act(obj *o,evt *e,lnk *l)
{ 
 evt_mvf_user_act(o->pl,l->e->e,l); //:) 
}

void cmd_mv_proc(obj *o)
{
 evt *e; obj *po=o->pl;
 double r;

 return; // команда отключена, все движение через команду bmv

 if(po->pl->tag!=T_WORLD) return;
 po->s.tx=po->s.tx=0; po->s.ct=0.0;
 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL || cpval[CP_T]==NULL) return;
 sscanf(cpval[CP_TX],"%f",&po->s.tx);
 sscanf(cpval[CP_TY],"%f",&po->s.ty);
 sscanf(cpval[CP_T],"%Lf",&po->s.ct);
 r=po->s.tx*po->s.tx+po->s.ty*po->s.ty;
 if(r>1.001) { r=1.0/sqrt(r); po->s.tx*=r; po->s.ty*=r; }
 if(po->s.me==NULL)
 {
  e=get_evt(); evt_mvs_user_set(e,po,NULL,NULL,5.0); ins_evt(po,e); po->s.me=e;
 }
}

void cmd_auth_proc(obj *o)
{
 evt *e; obj *po;
 obj *co;

 if(cpval[CP_LOGIN]==NULL || cpval[CP_PASS]==NULL) return;
 po=find_user(cpval[CP_LOGIN]);
 if(po==NULL || strcmp(po->password,cpval[CP_PASS])!=0) { cmd_send_hui(o); return; } // не правильно заданы логин или пароль

 // проверка завершена, убиваем существующий сетевой интерфейс, если он есть 
 co=po->bo; while(co!=NULL && co->tag!=T_NIF) co=co->next;
 if(co!=NULL) del_nif_obj(co);

 obj_ch_parent(o,po);

 po->nif=o; // добавляю устройство

 // можно отправлять текущее состояние кораблика клиенту
 cmd_send_iam(po);
 // отправляю список объектов видимых локаций
 if(po->pl->tag==T_WORLD || po->pl->tag==T_BL)
 {
//  loc_obj_reg(po,0);
  locs_send_obj(po);
  if(po->pl->tag==T_BL) // объект в бою высылаю доп инфу
  cmd_send_bl_update(po);
 }
 else
  cmd_send_pobj_obj(po,po->pl);
}

// создание нового перса из шаблона с заданным именем и паролем
void cmd_newp_proc(obj *o)
{
 obj *po;

 if(cpval[CP_LOGIN]==NULL || cpval[CP_PASS]==NULL) return;
 po=find_user(cpval[CP_LOGIN]);
 if(po!=NULL) return; // логин уже существует

 // проверка завершена, создаем объект из шаблона и цепляемся к нему
 po=obj_create_PERS(cpval[CP_LOGIN],cpval[CP_PASS]);
 cmd_send_hello(o); // сообщаем, что пользователь создан
}

// пока просто останавливаю движение
void obj_btl_prep_PERS(obj *o)
{
 return; // отключено поскольку все движение работает через bmv
 if(o->s.me!=NULL && o->s.me->tag==E_mvs_user ) 
 {
  del_evt(o,o->s.me); free_evt(o,o->s.me);
  o->s.ct=0.0; o->s.tx=o->s.ty=0.0; o->s.me=NULL; // корабль в ожидании начала движения, был
 }
 else if(o->s.me!=NULL) // сдвигаем событие в начало очереди
  { o->s.me->t=GTIME; move_evt(o,o->s.me); } // нужно проверить работоспособность функции move_evt
// формируем реакцию на изменение E_bmv
 loc_col_bmv(o);
}

void obj_btl_prep_SPARK(obj *o)
{
 if(o->s.me!=NULL) // сдвигаем событие в начало очереди
  { o->s.me->t=GTIME; move_evt(o,o->s.me); }
 // при следующем срабатывании события спарк должен понять, что находтся в бою
}

void obj_btl_prep_AS(obj *o)
{
}

void obj_btl_prep_MUSOR(obj *o)
{
}

void obj_btl_prep(obj *o)
{
 if(o==NULL) return;
 if(o->tag==T_PERS) obj_btl_prep_PERS(o);
 if(o->tag==T_SP) obj_btl_prep_SPARK(o);
 if(o->tag==T_AS) obj_btl_prep_AS(o);
 if(o->tag==T_MUSOR) obj_btl_prep_MUSOR(o);
}

// получили команду перейти в пошаговый режим
// реакцией на команду должна быть остановка движения
// 1. obj="" или отсутствует
// 2. obj="id1"
// 3. tobj= "" или отсутствует
// 4. tobj="id2"

// 1,3 - переход в пошаговый режим
// 2,3 - вмешаться в бой id1 против всех
// 1,4 - напасть на id2
// 2,4 - вмешаться в бой id1 на стороне id2

// вместе с id нужно передавать и уникальное имя объекта, иначе возможны
// накладки при разных объектах с одним id (случай когда старый объект уже умер, а
// новый получил тот же id)

void cmd_bs_proc(obj *o)
{
 obj *bo,*to=NULL,*co;
 evt *me;
 char bn[ONAM_LEN];
 char tn[ONAM_LEN];
 int bside;

 float test;

 o=o->pl;

 if(o->pl->tag==T_BL) return; // уже в пошаговом режиме, потом проверку убрать за ненадобностью
 if(cpval[CP_OBJ]==NULL || cpval[CP_OBJ][0]==0) // новый бой
 {
  if(cpval[CP_TOBJ]!=NULL && cpval[CP_TOBJ][0]!=0) // напали на кого-то
  {
   sscanf(cpval[CP_TOBJ],"%x,%s",&to,tn);
   if(find_obj_ombase(to)!=0) return; // нету такого объекта
//   if(strcmp(to->dn,tn)!=0) return; // не тот объект, что нужен
//   if(is_obj_onmap(to)!=0) return; // нету на карте
   if(to->pl->tag==T_BL) return; // уже в бою
   test=get_obj_dist(o,to);
   if(get_obj_dist(o,to)>=1.0) return; // слишком далеко
  }
  bo=get_obj(o->pl,T_BL,""); if(bo==NULL) return; obj_new_name(bo);
  strcpy(bo->name,"┴ющ");
  obj_ch_parent(o,bo); obj_ch_parent(to,bo);
  if(to!=NULL) { bo->bside++; to->bside=bo->bside; } 
  bo->bside++; o->bside=bo->bside;
  
  BL_init_rt(bo);
  // остановим управляемые объекты, прервем торговлю и т.п.
  obj_btl_prep(o); obj_btl_prep(to);
 }
 else // вмешиваемся в текущий бой
 {
  sscanf(cpval[CP_OBJ],"%x",&bo); // считаем id боя
  if(find_obj_ombase(bo)!=0) return; // нету такого объекта
//  if(strcmp(bo->dn,tn)!=0) return; // не тот объект, что нужен
  if(bo->tag!=T_BL) return; // это не бой

// найдем расстояние до первого бойца в пределах дальности входа в бой
  co=bo->bo; 
  while(co!=NULL) 
  {
   if(co->bside!=0) // боец однака
    { if(get_obj_dist(o,co)<1.0) break; }// достаточно близко
   co=co->next;
  }
  if(co==NULL) return;

  bside=0;
  if(cpval[CP_TOBJ]!=NULL && cpval[CP_TOBJ][0]!=0) // присоединимся к указанной стороне
  {
   sscanf(cpval[CP_TOBJ],"%d",&bside); // считаем номер стороны
   if(bside>bo->bside || bside<0) bside=0;
  }
  if(bside==0) { bo->bside++; bside=bo->bside; }
  o->bside=bside;
  obj_ch_parent(o,bo);
  cmd_send_bl_begin1(o);
  cmd_send_bl_update(o);
  obj_btl_prep(o);
 }
}

int bt_open=0; // флаг начала плана хода
// получили план хода в бою
void cmd_bt_proc(obj *o)
{
 obj *po;
 evt *e;
 int turn;

 po=o->pl;
 if(po->tag!=T_BL) return; // уже не пошаговый режим
 if(cpval[CP_T]==NULL || cpval[CP_T][0]==0) return;
 sscanf(cpval[CP_T],"%d",&turn);
 if(po->turn!=turn || o->turn!=turn) return; // не тот ход, игнор.
 o->turn=-1; // больше планы не принимаем до начала следующего хода
 if(cpval[CP_CXML]==NULL) bt_open=1; // ход не пустой
 e=get_evt(); evt_func_set[E_bl_dummy](e,po,NULL,NULL,0.0); ins_evt(po,e);
}

// встретили конец плана
void cmd_et_proc(obj *o)
{
 bt_open=0;
}






int bt_cmd=0;
// получили команду двигаться в указанном направлении
// команда порождает одно или два события E_bm в зависимости
// от наличия параметра ta в команде
// к событию прикрепляется ссылка в которой есть информация о
// направлении движения
void cmd_bm_proc(obj *o)
{
 double r,tx,ty,t,ta=0.0;
 obj *np,*eo=NULL;
 evt *e;
 lnk *l;

 if(bt_cmd==0) return; // что-то не то пришло

 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL || 
    cpval[CP_T]==NULL) return;
 sscanf(cpval[CP_TX],"%Lf",&tx);
 sscanf(cpval[CP_TY],"%Lf",&ty);
 sscanf(cpval[CP_T],"%Lf",&t);
 if(cpval[CP_TA]!=NULL) sscanf(cpval[CP_TA],"%Lf",&ta);

 if(t<0.0 || t>BTURN_TIME) return; // время за пределами допустимого диапазона
 if(t==BTURN_TIME && (tx!=0.0 || ty!=0.0)) return; // конец хода, все останавливаются

 t*=RBTURN_TIME; ta*=RBTURN_TIME; r=tx*tx+ty*ty;
 if(r>1.001) { r=1.0/sqrt(r); tx*=r; ty*=r; }
 l=get_lnk(); l->tx=tx; l->ty=ty;
 e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,t); ins_evt(o,e);

 if(ta==0.0) return;
 t+=ta; if(t<0.0 || t>BTURN_TIME) return; // время за пределами допустимого диапазона
 l=get_lnk(); l->tx=0.0; l->ty=0.0;
 e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,t); ins_evt(o,e);
}

// получили команду двигаться в указанном направлении
// команда порождает одно или два события E_bmv в зависимости
// от наличия параметра t в команде
// к событию прикрепляется ссылка в которой есть информация о
// направлении движения
void cmd_bmv_proc(obj *o)
{
 double tx,ty;
 double dt,tmin;
 int n=0;
 evt *e;
 lnk *l;
 
 o=o->pl;
 if(o->dev.pwr[1]==0) return;

 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL) return;
 sscanf(cpval[CP_TX],"%Lf",&tx);
 sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_T]!=NULL) sscanf(cpval[CP_T],"%d",&n);

 dt=GTIME-o->s.ct;

// найдем время смены курса у двигателя
 tmin=1.0/o->dev.pwr[2];

 if(o->s.me!=NULL) // событие уже есть, посмотрим что оно будет делать
 {
  e=o->s.me; 
  if(e->l->tx==0.0 && e->l->ty==0.0) // ожидается останов, меняю параметры и время наступления
   { e->t=(dt>tmin)?GTIME:GTIME+(tmin-dt); e->l->tx=tx; e->l->ty=ty; e->l->n=n; move_evt(o,e); }
  else { e->l->tx=tx; e->l->ty=ty; e->l->n=n; } // просто устанавливаю новые параметры
 }
 else // события нет, создаю
 {
  l=get_lnk(); l->tx=tx; l->ty=ty; l->n=n;
  e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,(dt>tmin)?0.0:(tmin-dt)); ins_evt(o,e); o->s.me=e;
 }
}


void cmd_bmv_proc_v1(obj *o)
{
 double tx,ty;
 int n=0;
 evt *e;
 lnk *l;
 
 o=o->pl;

 if(cpval[CP_TX]==NULL || cpval[CP_TY]==NULL) return;
 sscanf(cpval[CP_TX],"%Lf",&tx);
 sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_T]!=NULL) sscanf(cpval[CP_T],"%d",&n);

 if(o->s.me!=NULL) // событие уже есть, посмотрим что оно будет делать
 {
  e=o->s.me;
  if(e->l->tx==0.0 && e->l->ty==0.0) // ожидается останов, меняю параметры и время наступления
   { e->t=GTIME+5.0; e->l->tx=tx; e->l->ty=ty; e->l->n=n; move_evt(o,e); }
  else { e->l->tx=tx; e->l->ty=ty; e->l->n=n; } // просто устанавливаю новые параметры
 }
 else // события нет, создаю
 {
  l=get_lnk(); l->tx=tx; l->ty=ty; l->n=n;
  e=get_evt(); evt_func_set[E_bmv](e,o,NULL,l,5.0); ins_evt(o,e); o->s.me=e;
 }
}


lnk *get_slot(obj *o,char st,int sc,int sn)
{
 lnk *l=o->dev.dvsl;
 while(l!=NULL)
 {
  if(l->st[0]==st && l->sc==sc && l->sn==sn) return l;
  l=l->next;
 }
 return NULL;
}

void cmd_buse_proc_tb(obj *o)
{
 double tx=0.0,ty=0.0,t;
 obj *np,*eo=NULL;
 lnk *l,*sl;
 char st; int sc,sn; 
 evt *e;

 if(bt_cmd==0) return; // что-то не то пришло

 if(cpval[CP_SLOT]==NULL || cpval[CP_T]==NULL) return; //обязательные поля
 if(cpval[CP_TX]!=NULL && cpval[CP_TY]!=NULL && cpval[CP_OBJ]!=NULL) return; // так не должно быть

 if(cpval[CP_TX]!=NULL) sscanf(cpval[CP_TX],"%Lf",&tx);
 if(cpval[CP_TY]!=NULL) sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_OBJ]!=NULL) 
 {
  sscanf(cpval[CP_OBJ],"%x",&eo);
  if(find_obj_ombase(eo)!=0) return; // нету такого объекта
  if(eo->pl!=o->pl) return; // хз где находится
 }
 sscanf(cpval[CP_T],"%Lf",&t); sscanf(cpval[CP_SLOT],"%c%d,%d",&st,&sc,&sn);

 sl=get_slot(o,st,sc,sn); if(sl==NULL) return;

 if(t<0.0 || t>BTURN_TIME) return; // время за пределами допустимого диапазона

 t*=RBTURN_TIME;

 l=get_lnk(); l->o=eo; l->tx=tx; l->ty=ty;
 e=get_evt(); evt_func_set[E_buse](e,sl->o,NULL,l,t); ins_evt(o,e);
}

void cmd_buse_proc_rt(obj *o)
{
 double tx=0.0,ty=0.0,t;
 obj *np,*eo=NULL;
 lnk *l,*sl;
 char st; int sc,sn,mode=0; 
 evt *e;

 o=o->pl;
 if(cpval[CP_SLOT]==NULL) return; //обязательные поля
 if(cpval[CP_TX]!=NULL && cpval[CP_TY]!=NULL && cpval[CP_OBJ]!=NULL) return; // так не должно быть

 if(cpval[CP_TX]!=NULL) sscanf(cpval[CP_TX],"%Lf",&tx);
 if(cpval[CP_TY]!=NULL) sscanf(cpval[CP_TY],"%Lf",&ty);
 if(cpval[CP_T]!=NULL) sscanf(cpval[CP_T],"%d",&mode);
 if(cpval[CP_OBJ]!=NULL) 
 {
  sscanf(cpval[CP_OBJ],"%x",&eo);
  if(find_obj_ombase(eo)!=0) return; // нету такого объекта
  if(eo->pl!=o->pl) return; // хз где находится
 }
 sscanf(cpval[CP_SLOT],"%c%d_%d",&st,&sc,&sn);

 sl=get_slot(o,st,sc,sn); if(sl==NULL) return;
 e=sl->o->s.me;

// проверим нет ли отмены выстрела
 if(cpval[CP_TX]==NULL && 
    cpval[CP_TY]==NULL && 
    cpval[CP_OBJ]==NULL &&
    cpval[CP_T]==NULL   ) 
 {
  if(e!=NULL) // отменить надобно
   { del_evt(sl->o,e); free_evt(sl->o,e); sl->o->s.me=NULL; }
  return;
 }

// проверим нет ли изменения режима стрельбы
 if(cpval[CP_TX]==NULL && 
    cpval[CP_TY]==NULL && 
    cpval[CP_OBJ]==NULL &&
    cpval[CP_T]!=NULL   ) 
 {
  if(e!=NULL) e->l->st[0]=mode;  return;
 }
 
 if(e!=NULL) // в очереди уже есть событие, просто меняю параметры
 {
  e->l->o=eo; e->l->tx=tx; e->l->ty=ty; e->l->st[0]=mode;
 }
 else
 {
  // события нет, но устройство может быть не готово
  l=get_lnk(); l->o=eo; l->tx=tx; l->ty=ty; l->st[0]=mode;
  e=get_evt(); evt_func_set[E_buse](e,sl->o,NULL,l,1.0); ins_evt(sl->o,e);
 }
}

void cmd_buse_proc(obj *o)
{
 if(o->bmode==0) cmd_buse_proc_rt(o);
 else if(o->bmode==1) cmd_buse_proc_tb(o);
}


void cmd_prich_proc(obj *no)
{
 double x,y,dr;
 obj *o,*co;
 lnk *l; evt *e;

 o=no->pl;
 if(o->pl->tag!=T_WORLD) return;
 if(cpval[CP_OBJ]==NULL || cpval[CP_OBJ][0]==0) return;
 sscanf(cpval[CP_OBJ],"%x",&co);
 if(find_obj_ombase(co)!=0) return; // нету такого объекта
 if(co->pl->tag!=T_WORLD) return;
// дальше нужно проверить есть ли у co признак объекта с которым можно стыковаться
// но пока для простоты не буду
 if(o->s.me!=NULL && o->s.me->tag==E_mvs_user ) 
 {
  del_evt(o,o->s.me); free_evt(o,o->s.me);
  o->s.ct=0.0; o->s.tx=o->s.ty=0.0; o->s.me=NULL; // корабль в ожидании начала движения, был
 }
 dr=(GTIME-o->s.ct)*o->s.v; x=o->s.cx+o->s.tx*dr; y=o->s.cy+o->s.ty*dr;
 if(co->s.me!=NULL && co->s.me->tag==E_mvs_user ) dr=0.0;
 else dr=(GTIME-co->s.ct)*co->s.v; 
 x-=(co->s.cx+co->s.tx*dr); y-=(co->s.cy+co->s.ty*dr);

 if(fabs(x)>co->s.xs || fabs(y)>co->s.ys) return; // тут нужно будет послать ответ, что объект слишком далеко
// теперь нужно вставить событие входа, а затем сдвинуть событие завершения движения
 l=get_lnk(); l->o=co;
 e=get_evt(); evt_func_set[E_prich_user](e,o,NULL,l,0.0); ins_evt(o,e);
 if(o->s.me!=NULL) // сдвигаем событие в начало очереди
  { o->s.me->t=GTIME; move_evt(o,o->s.me); } // нужно проверить работоспособность функции move_evt
}

void evt_prich_user_act(obj *o,evt *e,lnk *l)
{
 obj *co=l->o;

 free_lnk(o,l); e->l=NULL;
 del_evt(o,e); free_evt(o,e);
// теперь можно входить
 loc_obj_unreg(o); // убираю регистрацию объекта в локациях
 obj_ch_parent(o,co); // влетаю в co
// отправляю клиенту сообщение о входе и список видимых объектов, а объектам 
// сообщение об o
 cmd_send_prich(o,co); 
}

// получили команду отчалить, пока не будем создавать событие, а выполним сразу
// в дальнейшем событие понадобится, поскольку на него будут реакции
void cmd_otch_proc(obj *no)
{
 double x,y,dr;
 obj *o,*po;
 lnk *l; evt *e;

 o=no->pl; po=o->pl;
 if(po->tag==T_WORLD) return;
 cmd_send_pobj_dobj(o);
 
 dr=(GTIME-po->s.ct)*po->s.v; x=po->s.cx+po->s.tx*dr; y=po->s.cy+po->s.ty*dr;
 if(1.0*rand()/RAND_MAX>0.5) dr=-1.0; else dr=1.0;
 o->s.cx=x+po->s.xs*dr*(0.55+0.45*rand()/RAND_MAX); 
 if(1.0*rand()/RAND_MAX>0.5) dr=-1.0; else dr=1.0;
 o->s.cy=y+po->s.ys*dr*(0.55+0.45*rand()/RAND_MAX);
 o->s.ct=0.0; o->s.me=NULL;
 obj_ch_parent(o,po->pl);
 loc_obj_reg(o,0);
 cmd_send_iam(o);
 // отправляю список объектов видимых локаций
 locs_send_obj(o);
}

// получили команду выйти из боя, выполним сразу
// в дальнейшем событие понадобится, поскольку на него будут реакции
void cmd_bexit_proc(obj *no)
{
 double x,y,dr;
 obj *o,*po,*co;
 lnk *l,*ml; evt *e,*me;
 int i=0,n;
 char *b;

 o=no->pl; po=o->pl;
 if(po->tag!=T_BL) return;

// проверяю можно ли выйти
 co=po->bo;
 while(co!=NULL)
 {
  if(co->bside!=0 && co->bside!=o->bside) { i++; break; }
  co=co->next;
 }
 if(i!=0) return; // в бою еще остались враги

// останавливаю корабль
 if(o->s.me!=NULL) // событие уже есть
 {
  me=o->s.me; 
  me->t=GTIME; me->l->tx=0.0; me->l->ty=0.0; me->l->n=0; move_evt(o,me);
 }
 else // события нет, создаю
 {
  ml=get_lnk(); ml->tx=0.0; ml->ty=0.0; ml->n=0;
  me=get_evt(); evt_func_set[E_bmv](me,o,NULL,ml,0.0); ins_evt(o,me); o->s.me=me;
 }

 n=snprintf(NULL,0,"<BEXIT t=\"%f\" id=\"%x\" pid=\"%x\" />",GTIME,o,po->pl)+1; b=mymalloc(n); 
 snprintf(b,n,"<BEXIT t=\"%f\" id=\"%x\" pid=\"%x\" />",GTIME,o,po->pl);
 cmd_send_buse(po, b, n); myfree(b);
 
 o->bside=0; obj_ch_parent(o,po->pl);
 // рассылаю всем оповещение об изменении моего OBJ
 locs_send_myobj(o);
}

// получили чатовое сообщение, сразу парсю и пересылаю
void cmd_chat_proc(obj *no)
{
 char *bb,*b,*bn,*en;
 lnk *bl,*l; // буду использовать для построения списка адресатов
 obj *o;
 int nusr=0,dn;

 if(cpval[CP_TXT]==NULL || cpval[CP_TXT][0]==0) return; // нету ничего
 dn=strlen(no->pl->dn)+3;
 bb=mymalloc(strlen(cpval[CP_TXT])+1+dn); 
 bb[0]='['; strcpy(bb+1,no->pl->dn); bb[dn-2]=']'; bb[dn-1]=' ';
 strcpy(bb+dn,cpval[CP_TXT]); bl=l=NULL;
 b=bb+dn;

 while(*b!=0)
 {
  // проверим нет ли адреса "to ["
  while(*b==0x20 || *b==0x9) b++; // пропущу все пробелы вначале
  if(*b!='t' || b[1]!='o') break; // адреcа нету, посылаю всем кого нашел
  b+=2;
  while(*b==0x20 || *b==0x9) b++; // пропущу все пробелы между
  if(*b!='[') break; // адреcа нету, посылаю всем кого нашел
  bn=b+1;
  while(*b!=']' && *b!=0) b++; // ищу ]
  if(*b==0) break; // адреcа нету, посылаю всем кого нашел
  *b=0;
  if(bn>=b) break; // пустой адрес
  nusr++;
  o=find_user(bn);
  if(o!=NULL) // добавляю юзера в список
  {
   l=get_lnk(); l->o=o; l->next=bl; bl=l;
  }
  *b=']'; b++;
 }

// все заданные адреса не существуют, не отправляю никуда
 if(nusr!=0 && bl==NULL) { myfree(bb); return; }

// список юзеров построен, можно отправлять
 if(bl==NULL) // список пуст, отправляю широковещательное локациям
  { cmd_send_chat_broadcast(no->pl,bb); myfree(bb); return; }
 
 cmd_send_chat(no->pl,bb); // отправляю себе
 l=bl; while(l!=NULL) { if(l->o!=no->pl) cmd_send_chat(l->o,bb); l=l->next; }
 myfree(bb);
}


void cmd_mnt_proc(obj *no)
{
 obj *mo=NULL;
 char st; int sc,sn;
 lnk *sl;
 obj *o,*co;

 o=no->pl;
 if(cpval[CP_SLOT]==NULL) return; //обязательные поля
 sscanf(cpval[CP_SLOT],"%c%d_%d",&st,&sc,&sn);
 sl=get_slot(o,st,sc,sn); if(sl==NULL) return; // нету такого слота

 if(cpval[CP_OBJ]!=NULL)  // монтирование однака
 {
  if(sl->o!=NULL) return; // слот занят, не катит
  sscanf(cpval[CP_OBJ],"%x",&mo);
  if(find_obj_ombase(mo)!=0) return; // нету такого объекта
  if(mo->pl==NULL) return;
  if(mo->pl!=NULL && mo->pl->tag!=T_CRGO && mo->pl->pl!=o) return; // хз где находится
  // нужно проверить соответствует ли предмет слоту
  if(mo->dev.mysl==NULL) return; // устройство не слотовое
  if(mo->dev.mysl->sn!=0) return; // устройство уже установлено
  if(st!=mo->dev.mysl->st[0] || sc<mo->dev.mysl->sc) return; // несоответствие слоту

  obj_ch_parent(mo,o); // монтирую
  mo->dev.mysl->sn=sn; sl->o=mo; // вставляю в слот
  // включаю
  dev_mnt(mo);
  // посылаю инфу клиенту
  cmd_send_mnt(o,mo);
 }
 else // размонтирование
 {
  if(st=='C') return;
  if(sl->o==NULL) return; // нету нехуя
  mo=sl->o; if(mo->pl!=o) return; // хз где находится
  co=o->bo; while(co!=NULL) { if(co->tag==T_CRGO) break; co=co->next; }
  if(co==NULL) return;
  // выключаю
  dev_umnt(mo);
  mo->dev.mysl->sn=0; sl->o=NULL;
  obj_ch_parent(mo,co);
  cmd_send_umnt(o,mo);
 }

// e=sl->o->s.me;

}



void (*cmd_func_proc[NET_CMD_NUM])(obj *o)=
{
 NULL,			cmd_auth_proc,			cmd_mv_proc,
 NULL,			cmd_chat_proc,			cmd_bs_proc,
 cmd_bt_proc,		cmd_et_proc,			cmd_bmv_proc,
 cmd_prich_proc,	cmd_otch_proc,			cmd_buse_proc,
 cmd_bexit_proc,	cmd_newp_proc,			cmd_mnt_proc
};


long long *CMD_P_TAB=NULL;

void cmd_set_props(char *buf)
{
 int pn;
 char *tp;
 do
 {
  while(*buf<=0x20 && *buf!=0) buf++; if(*buf==0) break;
  tp=buf; // нашли начало имени параметра
  while(*buf>0x20 && *buf!='=') buf++;
  if(*tp=='/') cpval[CP_CXML]=1;
  if(*buf==0) break;
  *buf=0; buf++; // имя параметра нашли
  pn=fa_s(tp,CMD_P_TAB); if(pn<0) break;
  while(*buf!='"' && *buf!=0) buf++; // сканирую до начала значения
  if(*buf==0) break;
  buf++; tp=buf;
  while(*buf!='"' && *buf!=0) buf++; // сканирую до конца значения
  if(*buf==0) break;
  *buf=0; buf++;
  cpval[pn]=tp;
 }
 while(*buf!=0);
}

void cmd_clear_props(void)
{ int i;  for(i=0;i<NET_CMDP_NUM;i++) cpval[i]=NULL; }

long long *CMD_T_TAB=NULL; // таблица для поиска тэга объекта по имени

void recv_cmd_proc(obj *o, char *cmd)
{
 char *cmdp;
 int tagn;
 lnk *l;

 printf("RECV: %s\n",cmd);

 cmdp=cmd; while(*cmdp!=0 && *cmdp!=' ') cmdp++; if(*cmdp!=0) { *cmdp=0; cmdp++; }
 tagn=fa_s(cmd,CMD_T_TAB); if(tagn<0) return;
 if(bt_open!=0 && tagn!=C_ET) // команда пошагового режима, не обрабатываю
 {
  if(*(cmdp-1)==0) *(cmdp-1)=' '; // восстанавливаю прежний вид команды
  l=get_lnk(); l->next=o->pl->bo_turns; l->prev=NULL; o->pl->bo_turns=l;
  l->o=o; l->b=cmd; o->nif->nb.re->b=NULL;
  return;
 }
 if((tagn>=0 && o->pl->tag==T_PERS) || tagn==C_AUTH || tagn==C_NEWP)
 {
  //заполню таблицу свойств
  cmd_set_props(cmdp);
  // обработаю список реакций на команду
  evt_lnks_act(cmds_react[tagn],o);
  cmd_func_proc[tagn](o);
  //очищу таблицу свойств
  cmd_clear_props();
 }

}

// обработка команд в боевом режиме
void bl_cmd_proc(lnk *l)
{
 char *cmdp=l->b;
 int tagn;

 while(*cmdp!=0 && *cmdp!=' ') cmdp++; if(*cmdp!=0) { *cmdp=0; cmdp++; }
 tagn=fa_s(l->b,CMD_T_TAB); if(tagn<0) return;
 //заполню таблицу свойств
 cmd_set_props(cmdp);
 // обработаю список реакций на команду
 evt_lnks_act(cmds_react[tagn],l->o);
 cmd_func_proc[tagn](l->o);
 //очищу таблицу свойств
 cmd_clear_props();
}

void bl_cmds_proc(obj *o)
{
 lnk *l;
 if(CMD_T_TAB==NULL) CMD_T_TAB=fa_b(NET_CMD_NUM,cname); // строю таблицу
 if(CMD_T_TAB==NULL) return;
 if(CMD_P_TAB==NULL) CMD_P_TAB=fa_b(NET_CMDP_NUM,cpname); // строю таблицу
 if(CMD_P_TAB==NULL) return;
 bt_cmd=1; // разрешаю обработку боевых команд
 while(o->bo_turns!=NULL)
 {
  l=o->bo_turns;
  bl_cmd_proc(l);
  myfree(l->b); o->bo_turns=l->next; free_lnk(NULL,l);
 }
 bt_cmd=0;
}


void recv_cmds_proc(obj *o)
{
 lnk *cmd=o->nb.re;

 if(CMD_T_TAB==NULL) CMD_T_TAB=fa_b(NET_CMD_NUM,cname); // строю таблицу
 if(CMD_T_TAB==NULL) return;
 if(CMD_P_TAB==NULL) CMD_P_TAB=fa_b(NET_CMDP_NUM,cpname); // строю таблицу
 if(CMD_P_TAB==NULL) return;
 while(cmd!=NULL && cmd->cp==0)
  {
   recv_cmd_proc(o,cmd->b); cmd=cmd->prev;
   if(cmd==NULL) o->nb.rb=NULL; else cmd->next=NULL;
   myfree(o->nb.re->b); free_lnk(NULL,o->nb.re);
   o->nb.re=cmd;
  }
}
