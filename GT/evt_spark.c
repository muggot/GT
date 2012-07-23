#include "gt.h"
#include "res_mx.h"
#define		M_PI	3.1415926535897932384626433832795

// поведение спарков

void evt_spark_set(evt *ne,obj *o,evt *e,lnk *l,double time)
{
 ne->tag=E_spark;
 ne->t=GTIME+time;
 ne->l=l;
 ne->o=o;
 ne->e=e;
 o->s.me=ne;
}

// спарк может иметь приказ матки
// 0 работать в штатном режиме
// 1 копателам возвратиться в улей

// спарк анализирует ситуацию и принимает решение

void evt_spark_act(obj *o,evt *e,lnk *l)
{
// printf("evt_spark_act\n");
 del_evt(o,e); free_evt(o,e);

 if(o->pl->tag==T_BL) { spark_act_btl(o,e,l); return; } // спарк в бою

 if(o->s.img==1) spark_matka_act(o);
 if(o->s.img==2) spark_rab_act(o);
 if(o->s.img==3) spark_spy_act(o);
 if(o->s.img==4) spark_fighter_act(o);
}


void spark_selfkill(obj *o) // время жизни кончилось
{
 int i;
 evt *e,*de;
 lnk *l,*dl;
 obj *co;

 if(o->pl->tag==T_WORLD) loc_obj_unreg(o);

 co=o->bo; while(co!=NULL) { spark_selfkill(co); co=o->bo; }

// удаляю очередь событий
 e=o->bevt; if(e!=NULL) e=e->next;
 while(e!=NULL) 
  { de=e->next; del_evt(o,e); free_evt(o,e); e=de; }
 e=o->bevt; if(e!=NULL) { del_evt(o,e); free_evt(o,e); }

// удаляю список ссылок на зависимые события
 for(i=0;i<ELST_NUM;i++)
  while(o->elst[i]!=NULL)
  {
   l=dl=o->elst[i]; o->elst[i]=l->next;
   if(dl->l==l) { if(l->e->e!=NULL) l->e->e->e=NULL; free_evt(o,l->e); evt_lnk_del(l); }
   else 
   {
    while(dl->l!=l) dl=dl->l;
    dl->l=l->l; if(l->e!=NULL) { dl->e=l->e; l->e->l=dl; evt_lnk_del(l); }
   }
  }
 
// удаляю буферы
 if(o->mem!=NULL) myfree(o->mem);

// удаляю объект
  del_obj(o); free_obj(o);
}

// для принятия решения спарком используются следующие параметры
// местоположение 0 - в улье, 1 - в пространстве, 2 - в астероиде
// запас сил 0 - полный, 1 - нормальный, 2 - пора на кормежку, 3 - уже не вернуться
// загрузка 0 - пустой, 1 - не полный, 2 - полный

// возможные решения для искатела 
// 0. 00X - разгрузка и выход на новый поиск
// 1. 01X,02X,03X - разгрузка и питание
// 2. 100,101,110,111 - выбор маршрута и полет
// 3. 102,112,12X - возвращение в улей
// 4. 130 - подача сигнала бедствия
// 5. 131,132 - питание из резерва и сброс несъедобных ресов
// 6. 200,201,210,211 - копать
// 7. 202,212,22X - на выход и пулей в улей
// 8. 23X - проверка резерва и копать

unsigned char spy_act[3][4][3]=
{
 {{0,0,0},{1,1,1},{1,1,1},{1,1,1}},
 {{2,2,3},{2,2,3},{3,3,3},{4,5,5}},
 {{6,6,7},{6,6,7},{7,7,7},{8,8,8}}
};

int get_spark_massa(obj *o)
{
 obj *co=o->bo,*ro;
 while(co!=NULL)
 {
  if(co->tag==T_CRGO)
  {
   if(co->dev.cm==0) return 0;
   else if(co->dev.cm>=co->dev.pwr[1]*0.9) return 2;
   else return 1;
  }
  co=co->next;
 }
 return 0;
}

int get_spark_mesto(obj *o)
{
 if(o->pl->tag==T_UL) return 0;
 if(o->pl->tag==T_AS) return 2;
 return 1;
}

int get_spark_golod(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL;
 float v,dr,dx,dy,tr;
 co=o->bo;
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || eng_o==NULL) { printf("У спарка нету FTN или ENG\n"); return 3; }

 if(ftn_o->dev.pwr[0]==ftn_o->dev.pwr[1]) return 0;

 if(o->mem->hname[0]!=NULL)
  { dx=o->mem->hx-o->s.cx; dy=o->mem->hy-o->s.cy; } // расстояние до дома
 else // у спарка нет UL, нужно исходить из расстояния до ближайшего съедобного AS
  { dx=o->mem->p[0][0]-o->s.cx; dy=o->mem->p[0][1]-o->s.cy; }

 tr=ftn_o->dev.pwr[1]/eng_o->dev.pwr[3]; // на какое время хватит топлива
 dr=dx*dx+dy*dy; tr=tr*tr*o->s.v*o->s.v/dr;
 if(tr<1.0) return 3;
 if(tr<4.0) return 2;
 return 1;
}

void spark_act_ULrazgruz(obj *o)
{
 obj *co,*to,*fo,*crgo_from=NULL,*crgo_to=NULL;

// printf("spark_act_ULrazgruz\n");
 co=o->bo;
 while(co!=NULL) { if(co->tag==T_CRGO) { crgo_from=co; break; } co=co->next; }
 co=o->pl->bo;
 while(co!=NULL) { if(co->tag==T_CRGO) { crgo_to=co; break; } co=co->next; }
 if(crgo_from==NULL || crgo_to==NULL) return;

 fo=crgo_from->bo;
 while(fo!=NULL) 
 {
  co=crgo_from; while(co->tag!=T_WORLD) { co->dev.cm-=fo->dev.cm; co=co->pl; }
  co=crgo_to; while(co->tag!=T_WORLD) { co->dev.cm+=fo->dev.cm; co=co->pl; } 
  to=crgo_to->bo;
  while(to!=NULL) 
  {
   if(fo->tag==to->tag && fo->s.img==to->s.img && fo->tag==T_RES) // ресы сразу объединяем
   {
    if(fo->s.img==6) o->pl->mem->d[3]+=fo->dev.cm; // информация для матки
    to->dev.m+=fo->dev.m; to->dev.cm+=fo->dev.cm; fo->dev.m=0; fo->dev.cm=0; 
    break;
   }
   to=to->next; 
  }
  if(to==NULL) // просто перекладываем предмет
   { co=fo->next; obj_ch_parent(fo,crgo_to); fo=co; }
  else fo=fo->next; 
 }
}

// искател осматривает видимые локации и ищет ближайший полезный 
// не исследованный астероид, если таковой присутствует то находит направление
// движения, время движения и начинает к нему двигаться.
// если астероида нет искател случайно выбирает направление и 
// находит время движения до первого пересечения с границей локации
// после пересечения границы снова производит поиск астероида и т.д.
// выбор направления движения определяется после пересечения с границей
// определяется случайным разбросом относительно направления от улья,
// угол разброса пока до 90 градусов.
// если домашний UL потерян, то одновременно ищем новый UL

void spark_set_ASsearch(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 evt *me;
 int i,locn;
 obj *aso=NULL;
 double dr,fi,cx,cy,tx,ty;

// printf("spark_set_ASsearch\n");
 o->mem->ct=1; o->mem->tt=GTIME;

 // осмотрим видимые локации на предмет наличия полезных астероидов
 while(l!=NULL)
 {
  nlo=wo->locs[l->ln].ol;
  while(nlo!=NULL) // пройдемся по списку объектов в данной локации
  {
   if(nlo->o->tag==T_AS && spark_act_AStest(o,nlo->o)==0) // опа, астероид, посмотрим не посещали ли мы его
   {
    for(i=1;i<6 && o->mem->pid[i]!=(int)nlo->o;i++);
    if(i==6) // еще не копали, ура, оценим полезность и примем решение лететь ли к
    {
     fi=(GTIME-nlo->o->s.ct)*nlo->o->s.v; 
     cx=nlo->o->s.cx+nlo->o->s.tx*fi-o->s.cx; 
     cy=nlo->o->s.cy+nlo->o->s.ty*fi-o->s.cy;
     if(aso==NULL || dr>cx*cx+cy*cy) { dr=cx*cx+cy*cy; aso=nlo->o; }
    }
   }
   if(nlo->o->tag==T_UL && o->mem->hname[0]==0) // опа, нашел новый дом, живем 
   {
    fi=(GTIME-nlo->o->s.ct)*nlo->o->s.v; 
    o->mem->hx=nlo->o->s.cx+nlo->o->s.tx*fi; 
    o->mem->hy=nlo->o->s.cy+nlo->o->s.ty*fi;
    strcpy(o->mem->hname,nlo->o->dn);
   }
   nlo=nlo->next;
  }
  l=l->next;
 }
 if(aso!=NULL) // вперед к цели
 {
// заявим астероид в качестве цели
  fi=(GTIME-aso->s.ct)*aso->s.v; dr=sqrt(dr);
  o->mem->p[0][0]=aso->s.cx+aso->s.tx*fi; o->mem->p[0][1]=aso->s.cy+aso->s.ty*fi;
  o->mem->pid[0]=(int)aso;
  cx=o->mem->p[0][0]-o->s.cx; cy=o->mem->p[0][1]-o->s.cy;
  o->s.tx=cx/dr; o->s.ty=cy/dr;
// параметры движения определены, устанавливаем событие
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,dr/o->s.v); ins_evt(o,me);
  return;
 }
// летим до новой локации
 if(o->s.cx==o->mem->hx && o->s.cy==o->mem->hy)
 {
  fi=(2.0*M_PI*rand())/RAND_MAX; o->s.tx=cos(fi); o->s.ty=sin(fi);
 }
 else
 {
  tx=o->s.cx-o->mem->hx; ty=o->s.cy-o->mem->hy;
  fi=1.0/sqrt(tx*tx+ty*ty); tx*=fi; ty*=fi;
  fi=(M_PI*rand())/RAND_MAX-0.5*M_PI;
  o->s.tx=tx*cos(fi)-ty*sin(fi); o->s.ty=tx*sin(fi)+ty*cos(fi); 
 }
// параметры движения определены, устанавливаем событие
 dr=get_loc_col_time(o)+1.0;
// тут нужно прикинуть как далеко спарк залетит и хватит ли ему сил на возвращение
// похоже тут нужно решать квадратное уравнение, а лень
// поэтому пусть дохнет
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dr); ins_evt(o,me);
}

void spark_set_ASexit(obj *o)
{
 double dr,cx,cy;
 evt *me;

// printf("spark_set_ASexit\n");
 dr=(GTIME-o->pl->s.ct)*o->pl->s.v; 
 cx=o->pl->s.cx+o->pl->s.tx*dr; cy=o->pl->s.cy+o->pl->s.ty*dr;
 o->mem->p[0][0]=cx; o->mem->p[0][1]=cy; o->mem->pid[0]=(int)o->pl;

 if(o->mem->hname[0]==0) // дома нет, буду временно домом считать AS
  { o->mem->hx=cx; o->mem->hy=cy; }

 obj_ch_parent(o,o->pl->pl); // выхожу из AS
 o->s.cx=cx; o->s.cy=cy; // присваиваю координаты от родительского объекта
 o->s.tx=o->s.ty=0.0; o->s.v=0; o->s.ct=GTIME;
 loc_obj_reg(o,1); // регистрирую спарка в локациях
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
 o->mem->ct=3; // спарк летит в улей
}

void spark_set_ULexit(obj *o)
{
 double dr,cx,cy;
 evt *me;

// проверим не пора ли умереть
 if(o->mem->te<GTIME) { spark_selfkill(o); return; } // больше события не вызываются

// printf("spark_set_ULexit\n");
 dr=(GTIME-o->pl->s.ct)*o->pl->s.v; 
 o->mem->hx=o->pl->s.cx+o->pl->s.tx*dr; o->mem->hy=o->pl->s.cy+o->pl->s.ty*dr;
 strcpy(o->mem->hname,o->pl->dn);
 o->mem->d[0]=o->pl->mem->d[0];

 obj_ch_parent(o,o->pl->pl); // выхожу из UL
 o->s.cx=o->mem->hx; o->s.cy=o->mem->hy; // присваиваю координаты от родительского объекта
 o->s.tx=o->s.ty=0.0; o->s.v=0; o->s.ct=GTIME;
 loc_obj_reg(o,1); // регистрирую спарка в локациях
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
 o->mem->ct=1; // спарк летит к астероиду
}

// нужно определить время питания и дать приказ
void spark_set_ULpitanie(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL;
 evt *me;
 float dt;

// printf("spark_set_ULpitanie\n");
 co=o->bo;
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || eng_o==NULL) { printf("У спарка нету FTN или ENG\n"); exit(0); }

 dt=(ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1])/ftn_o->dev.pwr[2]+1.0;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
 o->mem->ct=0; o->mem->tt=GTIME;
}

obj *get_UL_eda(obj *ulo)
{
 obj *co;
 co=ulo->bo; while(co!=NULL && co->tag!=T_CRGO) co=co->next;
 if(co==NULL) { printf("В улье нету CRGO\n"); exit(0); }
 co=co->bo; while(co!=NULL && (co->tag!=T_RES || co->s.img!=6)) co=co->next;
 if(co==NULL) { printf("В улье нету реса еда\n"); exit(0); }
 return co;
}

// подсчитаем результаты питания
void spark_act_ULpitanie(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL,*eda_o=NULL;
 float v,dr,dx,dy,tr;

 if(o->mem->ct!=0 || o->pl->tag!=T_UL) return;
// printf("spark_act_ULpitanie\n");

 co=o->bo;
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }

 if(ftn_o==NULL || eng_o==NULL) { printf("У спарка нету FTN или ENG\n"); exit(0); }

 co=get_UL_eda(o->pl);

 dr=(GTIME-o->mem->tt)*ftn_o->dev.pwr[2];
 if(ftn_o->dev.pwr[1]+dr>ftn_o->dev.pwr[0]) dr=ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1];
 ftn_o->dev.pwr[1]+=dr;
 co->dev.m-=dr; co->dev.cm-=dr; co->pl->dev.cm-=dr; // уменьшаю запас еды на астероиде
 o->pl->mem->d[2]+=dr; // информация дл матки
 o->mem->tt=GTIME;
}


void spark_act_Wgo1(obj *o)
{
 obj *co,*ftn_o=NULL,*eng_o=NULL;
 co=o->bo;

// printf("spark_act_Wgo1\n");
 while(co!=NULL)
 {
       if(co->tag==T_FTN) ftn_o=co;
  else if(co->tag==T_ENG) eng_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || eng_o==NULL) { printf("У спарка нету FTN или ENG\n"); exit(0); }

 ftn_o->dev.pwr[1]-=(GTIME-o->mem->tt)*eng_o->dev.pwr[3];
 if(ftn_o->dev.pwr[1]<0.0) ftn_o->dev.pwr[1]=0.0;
}

// подсчитаем результаты полета
void spark_act_Wgo(obj *o)
{
 double dr;

 if(o->pl->tag!=T_WORLD && o->pl->tag!=T_BL) return;
// printf("spark_act_Wgo\n");
// убавим здоровье спарка
 spark_act_Wgo1(o);

 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.ct=GTIME; o->s.tx=o->s.ty=0.0;
 o->s.v=o->dev.pwr[1]/(1.0*o->dev.cm);
 o->mem->tt=GTIME;
}

// нужно подвести итоги копания
void spark_act_ASkopanie(obj *o)
{
 obj *co=o->bo,*ro;
 double *rm,m,mm=0;
 int i;

 if(o->mem->ct!=2 || o->pl->tag!=T_AS) return;
// printf("spark_act_ASkopanie\n");

 while(co!=NULL)
 {
  if(co->tag==T_CRGO)
  {
// за каждую ходку спарк может накопать только мед или бетон, это задается значением mem->d[0]
// за прошедшее время мы можем определить сколько ресов всех типов было накопано
// и выбросить ненужные в космос
   m=(GTIME-o->mem->tt)*co->dev.pwr[2]; // всего мог накопать
   rm=res_mx[0]; ro=o->pl->bo; 
   for(i=0;i<RES_NUM;i++) rm[i]=0.0;
   while(ro!=NULL) // пройдемся по ресам и посчитаем сколько каждого накопали
   {
    if(ro->tag==T_RES) 
     { rm[ro->s.img]=ro->dev.cm; mm+=ro->dev.cm; }
    ro=ro->next;
   }
   m/=mm; if(m>1.0) m=1.0; for(i=0;i<RES_NUM;i++) rm[i]*=m;
// теперь определим какой рес будет израсходован полностью, а какой останется
   mm=0;
   for(i=0;i<RES_NUM;i++)
    {
     if(rm[i]==0.0 && res_mx[o->mem->d[0]][i]!=0.0) // нужного реса совсем нет, астероид выработан, пора топать отсюда
     {
      return;
     }
     if(rm[i]!=0.0 && res_mx[o->mem->d[0]][i]/rm[i]>mm)
      mm=res_mx[o->mem->d[0]][i]/rm[i]; 
    }

// загружаю переработанный рес, выбрасываю шлак, убавляю запас астероида
   ro=o->pl->bo; 
   while(ro!=NULL) // убавляю запас астероида
   {
    if(ro->tag==T_RES) 
     { ro->dev.cm-=rm[ro->s.img]; ro->pl->dev.cm-=rm[ro->s.img]; }
    ro=ro->next;
   }

   mm=1.0/mm;
   ro=co->bo; 
   while(ro!=NULL) // пройдемся по ресам и найдем который нужно загрузить
   {
    if(ro->tag==T_RES && ro->s.img==o->mem->d[0]) // рес найден
    {
     m=mm*res_mx[o->mem->d[0]][RES_NUM]*res_mx[o->mem->d[0]][RES_NUM];
     co->dev.cm+=m;
     if(co->dev.cm>co->dev.pwr[1]) 
      { m-=(co->dev.pwr[1]-co->dev.cm); co->dev.cm=co->dev.pwr[1]; }
     ro->dev.cm+=m;
     co->pl->dev.cm+=m;
     break;
    }
    ro=ro->next;
   }

// для выбрасывания лишнего реса нужно создать новый объект - T_MUSOR
   ro=obj_create_MUSOR(o->pl->pl,o->pl);
// подсчитаем оставшийся рес, забьем мусор ресами
   for(i=0;i<RES_NUM;i++) 
   { 
    rm[i]-=mm*res_mx[o->mem->d[0]][i];
    if(rm[i]>0.0) obj_create_RES(ro,i,rm[i]);
   }
   o->mem->tt=GTIME;
   return;
  }
  co=co->next;
 }
}


// нужно подсчитать время необходимое для копания и дать приказ
// еще не реализована
void spark_set_ASkopanie(obj *o)
{
 obj *co=o->bo;
 double dt;
 evt *me;
 int i,pid;

// printf("spark_set_ASkopanie\n");
 while(co!=NULL && co->tag!=T_CRGO)  co=co->next;
 if(co==NULL) { printf("У спарка нету CRGO\n"); exit(0); }

 if(spark_act_AStest(o,o->pl)!=0)
 {
// выходим из AS и стираем о нем инфу
  spark_set_ASexit(o);
  pid=o->mem->pid[0];
  memcpy(o->mem->p[0],o->mem->p[1],sizeof(float)*5*3);
  memcpy(&o->mem->pid[0],&o->mem->pid[1],sizeof(int)*5);
  o->mem->p[5][0]=0.0; o->mem->p[5][1]=0.0; o->mem->pid[5]=pid;
  o->mem->ct=2; o->mem->tt=GTIME;
  return;
 }

 o->mem->ct=2; o->mem->tt=GTIME;
 dt=(co->dev.pwr[1]-co->dev.cm)/co->dev.pwr[2]+1.0;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
}


// вызывается вдали от UL, приказ спарку лететь к UL
void spark_set_ULgo(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 obj *ulo=NULL;
 double dr,dx,dy,dt;
 evt *me;
 int golod,massa;

// printf("spark_set_ULgo\n");
 if(o->mem->hname[0]==0) // родного улья нету, вызываю либо ASsearch, либо ASgo
 {
  golod=get_spark_golod(o);
  massa=get_spark_massa(o);
  if(golod>1 && massa>0) { spark_set_Wpitanie(o); return; } // кушаем
  if(golod<2) { spark_set_ASsearch(o); return; } // в поиске
  if(golod==2) { spark_set_ASgo(o); return; } // возвращаемся на ближайший AS
 }

 dt=1.0/o->s.v;	// обратная скорость спарка
 dx=o->mem->hx-o->s.cx; dy=o->mem->hy-o->s.cy; dr=sqrt(dx*dx+dy*dy);
 o->s.ct=GTIME; dt*=dr; o->s.tx=dx/dr; o->s.ty=dy/dr;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
 o->mem->tt=GTIME;
 o->mem->ct=3;
}


// вызывается вдали от AS, приказ спарку лететь к AS
void spark_set_ASgo(obj *o)
{
 double dr,dx,dy,dt;
 evt *me;

// printf("spark_set_ASgo\n");
 if(o->mem->p[0][0]==0.0 && o->mem->p[0][1]==0.0)
  { spark_set_ASsearch(o); return; }
 dt=(1.0*o->dev.cm)/o->dev.pwr[1];	// обратная скорость спарка
 dx=o->mem->p[0][0]-o->s.cx; dy=o->mem->p[0][1]-o->s.cy; dr=sqrt(dx*dx+dy*dy);
 o->s.ct=GTIME; dt*=dr; o->s.tx=dx/dr; o->s.ty=dy/dr;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt); ins_evt(o,me);
 o->mem->tt=GTIME;
 o->mem->ct=1;
}

// проверяет AS на полезность
int spark_act_AStest(obj *o, obj *aso)
{
 obj *ro=aso->bo;
 double *rm;
 int i;

 rm=res_mx[0]; for(i=0;i<RES_NUM;i++) rm[i]=0.0;
 while(ro!=NULL) // пройдемся по ресам
 {
  if(ro->tag==T_RES) rm[ro->s.img]=ro->dev.cm;
  ro=ro->next;
 }
 for(i=0;i<RES_NUM;i++)
 {
  if(rm[i]==0.0 && res_mx[o->mem->d[0]][i]!=0.0) // нужного реса совсем нет, астероид выработан, пора топать отсюда
  {
   return -1;
  }
 }
 return 0;
}

// влетаем в AS при необходимости
void spark_act_ASenter(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 obj *aso=NULL;
 double d,dr,cx,cy;
 int pid;

// printf("spark_act_ASenter\n");
 cx=o->mem->p[0][0]-o->s.cx; cy=o->mem->p[0][1]-o->s.cy; d=cx*cx+cy*cy;
 if(o->mem->ct!=1 || d>25.0) return;
 
 // осмотрим видимые локации на предмет наличия искомого астероида
 while(l!=NULL && aso==NULL)
 {
  nlo=wo->locs[l->ln].ol;
  while(nlo!=NULL && aso==NULL) // пройдемся по списку объектов в данной локации
  {
   if(nlo->o->tag==T_AS && spark_act_AStest(o,nlo->o)==0) // опа, астероид
   {
    if(o->mem->pid[0]==0) // состояние считано с диска, pid утеряны, пробую восстановить
    {
     d=(GTIME-nlo->o->s.ct)*nlo->o->s.v; 
     cx=nlo->o->s.cx+nlo->o->s.tx*d-o->mem->p[0][0]; 
     cy=nlo->o->s.cy+nlo->o->s.ty*d-o->mem->p[0][1];
     if(aso==NULL || dr>cx*cx+cy*cy) { dr=cx*cx+cy*cy; aso=nlo->o; }
    }
    else
    if(o->mem->pid[0]==(int)nlo->o) aso=nlo->o; // искомый астероид найден
   }
   nlo=nlo->next;
  }
  l=l->next;
 }

 if(aso==NULL) // нужный астероид исчез, выкинем цель из памяти
 {
  pid=o->mem->pid[0];
  memcpy(o->mem->p[0],o->mem->p[1],sizeof(float)*5*3);
  memcpy(&o->mem->pid[0],&o->mem->pid[1],sizeof(int)*5);
  o->mem->p[5][0]=0.0; o->mem->p[5][1]=0.0; o->mem->pid[5]=pid;
  return;
 }

 dr=(GTIME-aso->s.ct)*aso->s.v; 
 o->mem->p[0][0]=aso->s.cx+aso->s.tx*dr; o->mem->p[0][1]=aso->s.cy+aso->s.ty*dr;
 cx=o->mem->p[0][0]-o->s.cx; cy=o->mem->p[0][1]-o->s.cy;
 dr=cx*cx+cy*cy;

 if(0.25*(aso->s.xs*aso->s.xs+aso->s.ys*aso->s.ys)>dr) // уже над астероидом, можно заходить
  {
   loc_obj_unreg(o); // убираю регистрацию объекта в локациях
   obj_ch_parent(o,aso); // влетаю в астероид
  }
}

// влетаем в UL при необходимости
void spark_act_ULenter(obj *o)
{
 obj *wo=o->pl;
 lnk *l=o->locl;
 lnk *nlo;
 obj *ulo=NULL;
 double dr,cx,cy;
 int i,j;

// printf("spark_act_ULenter\n");
 if(o->mem->ct!=3) return;

// текущее расстояние до предпологаемого положения улья
 cx=o->mem->hx-o->s.cx; cy=o->mem->hy-o->s.cy; dr=cx*cx+cy*cy; if(dr>25.0) return;

 // осмотрим видимые локации на предмет наличия ульев
 while(l!=NULL && ulo==NULL)
 {
  nlo=wo->locs[l->ln].ol;
  while(nlo!=NULL && ulo==NULL) // пройдемся по списку объектов в данной локации
  {
   if(nlo->o->tag==T_UL) // опа, улей, посмотрим не наш ли
   {
    if(strcmp(o->mem->hname,nlo->o->dn)==0) ulo=nlo->o; // искомый улей найден
   }
   nlo=nlo->next;
  }
  l=l->next;
 }

 if(ulo==NULL) // родной улей не найден, выкинем инфу об улье из памяти
 {
  o->mem->hx=o->mem->p[0][0]; o->mem->hx=o->mem->p[0][1]; o->mem->hname[0]=0; return;
 }

 dr=(GTIME-ulo->s.ct)*ulo->s.v; 
 o->mem->hx=ulo->s.cx+ulo->s.tx*dr; o->mem->hy=ulo->s.cy+ulo->s.ty*dr;
 cx=o->mem->hx-o->s.cx; cy=o->mem->hy-o->s.cy;
 dr=cx*cx+cy*cy;

 if(0.25*(ulo->s.xs*ulo->s.xs+ulo->s.ys*ulo->s.ys)>dr) // уже над ульем, можно заходить
 {
  loc_obj_unreg(o); // убираю регистрацию объекта в локациях
  obj_ch_parent(o,ulo); // влетаю в UL
// замечательно, спарк вернулся в UL, 
// координаты последнего AS находятся в o->mem->p[0], сообщим о них UL, 
// если расстояние до AS позволяет вписаться в память
  for(i=0;i<6;i++)
  {
   if(o->mem->pid[i]!=0 && o->mem->p[i][0]==0.0 && o->mem->p[i][1]==0.0)
   {
    for(j=0;j<6;j++)
     if(o->mem->pid[i]==ulo->mem->pid[j])
     {
      if(j!=5)
      {
       memcpy(&ulo->mem->pid[j],&ulo->mem->pid[j+1],sizeof(int)*(5-j));
       memcpy(&ulo->mem->p[j],&ulo->mem->p[j+1],sizeof(float)*3*(5-j));
      }
      ulo->mem->pid[5]=0; ulo->mem->p[5][0]=0.0; ulo->mem->p[5][1]=0.0;
      break;
     }
   }
  }


  if(o->mem->pid[0]!=0)
  {
   for(i=0;i<6;i++) 
    if(o->mem->pid[0]==ulo->mem->pid[i])
    {
     if(i!=5)
     {
      memcpy(&ulo->mem->pid[i],&ulo->mem->pid[i+1],sizeof(int)*(5-i));
      memcpy(&ulo->mem->p[i],&ulo->mem->p[i+1],sizeof(float)*3*(5-i));
     }
     ulo->mem->pid[5]=0; ulo->mem->p[5][0]=0.0; ulo->mem->p[5][1]=0.0;
     break;
    }
  }

  cx=o->mem->p[0][0]-o->mem->hx; cy=o->mem->p[0][1]-o->mem->hy;
  dr=cx*cx+cy*cy;
  for(i=0;i<5;i++)
  {
   cx=ulo->mem->p[i][0]-o->mem->hx; cy=ulo->mem->p[i][1]-o->mem->hy;
   if(dr<cx*cx+cy*cy) // надо же, попали в список, освободим место для новой инфы и заполним
   {
    for(j=4;j>=i;j--) 
    { 
     ulo->mem->p[j+1][0]=ulo->mem->p[j][0]; ulo->mem->p[j+1][1]=ulo->mem->p[j][1];
     ulo->mem->pid[j+1]=ulo->mem->pid[j];
    }
    ulo->mem->p[i][0]=o->mem->p[0][0]; ulo->mem->p[i][1]=o->mem->p[0][1];
    ulo->mem->pid[i]=o->mem->pid[0];
    break;
   }
  }
 }
}

void spark_set_SOSsend(obj *o)
{
 printf("spark_set_SOSsend\n");
 o->s.me=NULL;
// пока просто умираю
}

void spark_set_Wpitanie(obj *o)
{
// посмотрим что есть в пузе
 obj *co=o->bo,*ro,*mo=NULL;
 obj *ftn_o=NULL;
 obj *crgo_o=NULL;
 float *rm,m=0.0;
 int i;
 evt *me;

// printf("spark_set_Wpitanie\n");
 while(co!=NULL)
 {
  if(co->tag==T_FTN) ftn_o=co;
  if(co->tag==T_CRGO) crgo_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || crgo_o==NULL) { printf("У спарка отсутствует FTN или CRGO\n"); exit(0); }
 ro=crgo_o->bo;
 while(ro!=NULL)
 {
  if(ro->tag==T_RES) 
  {
   if(ro->s.img!=6 && ro->dev.cm>0.0) // выбрасываю рес
   {
// выбрасываю рес
    if(mo==NULL) mo=obj_create_MUSOR(o->pl,o);
    obj_create_RES(mo,ro->s.img,ro->dev.cm);
// уменьшаю массу спарка
    ro->pl->dev.cm-=ro->dev.cm; ro->pl->pl->dev.cm-=ro->dev.cm; ro->dev.cm=0;
   }
   else // найдена еда, перегоняю рес в здоровье 1 к 1
   {
    m=ro->dev.cm; 
    if(ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1]<m) 
     m=ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1];
    ro->pl->dev.cm-=m; ro->pl->pl->dev.cm-=m; ro->dev.cm-=m;
   }
   ro=ro->next;
  }
  else // это не рес, тоже выбрасываю
  {
   if(mo==NULL) mo=obj_create_MUSOR(o->pl,o);
   co=ro->next; obj_ch_parent(ro,mo); ro=co;
  }
 }
 ftn_o->dev.pwr[1]+=m;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
}

void spark_set_ASpitanie(obj *o)
{
// посмотрим что есть в пузе
 obj *co=o->bo,*ro,*mo=NULL,*tmpo;
 obj *ftn_o=NULL,*crgo_o=NULL;
 float *rm,m=0.0;
 int i;
 evt *me;

// printf("spark_set_ASpitanie\n");
 while(co!=NULL)
 {
  if(co->tag==T_FTN) ftn_o=co;
  if(co->tag==T_CRGO) crgo_o=co;
  co=co->next;
 }
 if(ftn_o==NULL || crgo_o==NULL) { printf("У спарка отсутствует FTN или CRGO\n"); exit(0); }
 ro=crgo_o->bo;
 while(ro!=NULL)
 {
  if(ro->tag==T_RES) 
  {
   if(ro->s.img!=6 && ro->dev.cm>0.0) // выбрасываю рес
   {
// выбрасываю рес
    if(mo==NULL) mo=obj_create_MUSOR(o->pl->pl,o->pl);
    obj_create_RES(mo,ro->s.img,ro->dev.cm);
// уменьшаю массу спарка
    ro->pl->dev.cm-=ro->dev.cm; ro->pl->pl->dev.cm-=ro->dev.cm; ro->dev.cm=0;
   }
   else // найдена еда, перегоняю реc в здоровье 1 к 1
   {
    m=ro->dev.cm; 
    if(ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1]<m) 
     m=ftn_o->dev.pwr[0]-ftn_o->dev.pwr[1];
    ro->pl->dev.cm-=m; ro->pl->pl->dev.cm-=m; ro->dev.cm-=m;
   }
   ro=ro->next;
  }
  else // это не рес, тоже выбрасываю
  {
   if(mo==NULL) mo=obj_create_MUSOR(o->pl->pl,o->pl);
   tmpo=ro->next; obj_ch_parent(ro,mo); ro=tmpo;
  }
 }
 ftn_o->dev.pwr[1]+=m;
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
}

void spark_spy_act(obj *o)
{
 int res,mesto,golod,massa;
 int i;

 spark_act_ASkopanie(o);
 spark_act_ULpitanie(o);
 spark_act_Wgo(o);

 spark_act_ASenter(o);
 spark_act_ULenter(o);

 mesto=get_spark_mesto(o);
 golod=get_spark_golod(o);
 massa=get_spark_massa(o);

 res=spy_act[mesto][golod][massa];
 if(res==0)
 {
  spark_act_ULrazgruz(o);
  spark_set_ULexit(o);	  // выходим и даем приказ искать астероид
// почистим данные о самом старом астероиде из найденных
// и осводим место для нового в позиции 0
  for(i=4;i>=0;i--)
   { 
    o->mem->p[i+1][0]=o->mem->p[i][0]; o->mem->p[i+1][1]=o->mem->p[i][1]; 
    o->mem->pid[i+1]=o->mem->pid[i];
   }
  o->mem->p[0][0]=o->mem->p[0][1]=0.0; o->mem->pid[0]=0;
  return;
 }
 else if(res==1)
 {
  spark_act_ULrazgruz(o);
  spark_set_ULpitanie(o); // дадим приказ питаться дальше
  return;
 }
 else if(res==2)
 {
  spark_set_ASsearch(o);  // дадим приказ искать астероид 
  return;
 }
 else if(res==3)
 {
  spark_set_ULgo(o);  // дадим приказ лететь в улей
  return;
 }
 else if(res==4)
 {
  spark_set_SOSsend(o);  // дадим приказ послать сигнал SOS
  return;
 }
 else if(res==5)
 {
  spark_set_Wpitanie(o); // питаемся в космосе из резервов, сбрасываем несъедобные ресы
  return;
 }
 else if(res==6)
 {
  spark_set_ASkopanie(o); // даем приказ копать некоторое время
  return;
 }
 else if(res==7)
 {
  spark_set_ASexit(o); // выходим и даем приказ на возвращение в UL
  return;
 }
 else if(res==8)
 {
  o->mem->d[0]=6; // очень хочется есть, буду копать еду
  o->mem->ct=1; // сделаю вид, что только-что прилетел и еще не копал, дабы не плодить мелкий шлак
  o->mem->tt=GTIME;
  spark_set_ASpitanie(o); // выбрасываю несъедобный рес и кушаю съедобный
  return;
 }

}

// задача матки отстроить улей и накопить побольше еды, при 
// постоянном увеличении числа спарков в улье
// после накопления определенного количества еды, при наличии избыточного
// количества спарков, матка рождает новую матку и часть спарков покидает
// родной улей с запасом еды и строительного реса чтобы создать новый улей
// параметры используемые при принятии решений
// наличие боевых столкновений за прошедший период: 0 не было, 1 были
// количество еды потребленное за период
// количество еды произведенное за период
// количество еды
// количество разведанных астероидов

// Будем считать что войны нет
// новый спарк рождается если запасов еды хватает 
// на 10 часов при текущем потреблении и количество известных астероидов больше 1
// в противном случае раз в пять часов матка рождает разведчика
// если приход еды более чем в два раза превышает расход, то матка рождает воина
// в противном случае копатела

void drop_UL_stat(obj *o)
{
 o->pl->mem->d[1]=o->pl->mem->d[2]=o->pl->mem->d[3]=0;
 o->mem->tt=GTIME;
}

void spark_matka_act(obj *o)
{
 evt *me;
 obj *co;
 int i;

 if(o->mem->tt==0.0) o->mem->tt=GTIME;
 if(GTIME-o->mem->tt<18.0) // слишком рано принимать решения
  { me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return; }
 if(o->pl->mem->d[1]!=0) // идет война
 {
  if(o->pl->mem->d[2]<o->pl->mem->d[3]) obj_create_SP(o,4); 
  else obj_create_SP(o,2); 
  drop_UL_stat(o);
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return;
 }

 co=get_UL_eda(o->pl);
 for(i=0;i<6 && o->pl->mem->pid[i]!=0;i++);
 if((o->pl->mem->d[2]==0 || co->dev.cm/o->pl->mem->d[2]>20.0) && i>1)
 {
  if(o->pl->mem->d[2]*2<o->pl->mem->d[3]) obj_create_SP(o,2); 
  else obj_create_SP(o,2); 
  drop_UL_stat(o);
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return;
 }
 else if(1.0*rand()/RAND_MAX>0.8)
 {
  obj_create_SP(o,3); drop_UL_stat(o);
  me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me); return;
 }
 me=get_evt(); evt_spark_set(me,o,NULL,NULL,36.0); ins_evt(o,me);
}


// просто заберем память улья, точнее то, что касается целей
void spark_act_ULgetmem(obj *o)
{
 int i;
 obj *ulo=o->pl;
 for(i=0;i<6;i++)
 {
  o->mem->p[i][0]=ulo->mem->p[i][0];  o->mem->p[i][1]=ulo->mem->p[i][1];
  o->mem->pid[i]=ulo->mem->pid[i];  
 }
  o->mem->d[0]=ulo->mem->d[0];
  o->mem->cp=ulo->mem->cp; o->mem->tp=GTIME; 
}

void spark_fighter_act(obj *o)
{
 
}

void spark_rab_act(obj *o)
{
 int res,mesto,golod,massa;

 spark_act_ASkopanie(o);
 spark_act_ULpitanie(o);
 spark_act_Wgo(o);

 spark_act_ASenter(o);
 spark_act_ULenter(o);

 mesto=get_spark_mesto(o);
 golod=get_spark_golod(o);
 massa=get_spark_massa(o);

 res=spy_act[mesto][golod][massa];
 if(res==0)
 {
  spark_act_ULrazgruz(o);
  spark_act_ULgetmem(o);
  spark_set_ULexit(o);	  // выходим и даем приказ искать астероид
  return;
 }
 else if(res==1)
 {
  spark_act_ULrazgruz(o);
  spark_set_ULpitanie(o); // дадим приказ питаться дальше
  return;
 }
 else if(res==2)
 {
  spark_set_ASgo(o);  // дадим приказ искать астероид 
  return;
 }
 else if(res==3)
 {
  spark_set_ULgo(o);  // дадим приказ лететь в улей
  return;
 }
 else if(res==4)
 {
  spark_set_SOSsend(o);  // дадим приказ послать сигнал SOS
  return;
 }
 else if(res==5)
 {
  spark_set_Wpitanie(o); // питаемся в космосе из резервов, сбрасываем несъедобные ресы
  return;
 }
 else if(res==6)
 {
  spark_set_ASkopanie(o); // даем приказ копать некоторое время
  return;
 }
 else if(res==7)
 {
  spark_set_ASexit(o); // выходим и даем приказ на возвращение в UL
  return;
 }
 else if(res==8)
 {
  o->mem->d[0]=6; // очень хочется есть, буду копать еду
  o->mem->ct=1; // сделаю вид, что только-что прилетел и еще не копал, дабы не плодить мелкий шлак
  o->mem->tt=GTIME;
  spark_set_ASpitanie(o); // выбрасываю несъедобный рес и кушаю съедобный
  return;
 }
}

void spark_act_btl(obj *o,evt *e,lnk *l)
{
 obj *bo=o->pl;
 obj *co,*eo=NULL;
 obj *wo1=NULL,*wo2=NULL;
 evt *me;
 double minr=0,dr,dx,dy,cx,cy,r;
 double wt1,wt2; // текущее время подготовки оружия к выстрелу
 double dt,ct;
 int n; char *b;
 lnk *nl;

// del_evt(o,e); free_evt(o,e);

 dr=(GTIME-o->s.ct)*o->s.v; o->s.cx+=o->s.tx*dr; o->s.cy+=o->s.ty*dr;
 o->s.ct=GTIME; o->s.tx=0.0; o->s.ty=0.0; cx=o->s.cx; cy=o->s.cy;
  
 co=bo->bo;
 while(co!=NULL)
 {
  if(co->bside!=0 && co->bside!=o->bside) // противник найден
  {
   dr=(GTIME-co->s.ct)*co->s.v; dx=co->s.cx+co->s.tx*dr-cx; dy=co->s.cy+co->s.ty*dr-cy;
   dr=1.0/(dx*dx+dy*dy); if(dr>minr) { minr=dr; eo=co; }
  }
  co=co->next;
 }

 if(eo==NULL) // врагов нету, выхожу из боя
  { 
   obj_ch_parent(o,o->pl->pl); 
   me=get_evt(); evt_spark_set(me,o,NULL,NULL,1.0); ins_evt(o,me);
   return; 
  }

 dr=(GTIME-eo->s.ct)*eo->s.v; minr=sqrt(minr); 
 dx=eo->s.cx+eo->s.tx*dr-cx; dy=eo->s.cy+eo->s.ty*dr-cy; 
 r=1.0/minr; dx*=minr; dy*=minr;

// найдем параметры оружия
 co=o->bo;
 while(co!=NULL)
 {
  if(co->tag==T_WPN) if(co->s.img==1) wo1=co; else wo2=co;
  co=co->next;
 }

 wt1=GTIME-wo1->s.ct; wt2=GTIME-wo2->s.ct;

 if(wt1>=wo1->dev.pwr[2] && wo1->s.me==NULL) 
 {
// формирую команду выстрела по цели
  nl=get_lnk(); nl->st[0]=0; nl->o=eo; nl->tx=0.0; nl->ty=0.0;
  me=get_evt(); evt_buse_set(me,wo1,NULL,nl,0.0); ins_evt(wo1,me);
  wt1=0.0;
 }

 wt1=wo1->dev.pwr[2]-wt1; // время до следующего выстрела

 if(r<=wo2->dev.pwr[0] && wt2>=wo2->dev.pwr[2] && wo2->s.me==NULL)
 {
// формирую команду выстрела по цели
  nl=get_lnk(); nl->st[0]=0; nl->o=eo; nl->tx=0.0; nl->ty=0.0;
  me=get_evt(); evt_buse_set(me,wo2,NULL,nl,0.0); ins_evt(wo2,me);
  dt=wo2->dev.pwr[2];
 }
 else dt=wo2->dev.pwr[2]-wt2;

 if(r>wo2->dev.pwr[0]) // подсчитаем время подлета
 {
  dt=(r-wo2->dev.pwr[0])/o->s.v;
// формирую команду подлета к цели
  o->s.ct=GTIME; o->s.tx=dx; o->s.ty=dy;
 }

 if(dt>wt1) dt=wt1;

 me=get_evt(); evt_spark_set(me,o,NULL,NULL,dt+1.0); ins_evt(o,me);
}
