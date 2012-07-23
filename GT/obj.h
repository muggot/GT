#include "shape.h"
#include "orbit.h"
#include "devpar.h"

#define	GOF_EXIST	1	//признак существования
#define	GOF_ONMAP	2	//расположен на карте
#define	GOF_ISNIF	4	//имеет удаленное управление

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
  char		name[ONAM_LEN];		//отображаемое имя
  char		password[ONAM_LEN]; 	// пароль перса
 };

 char		dn[ONAM_LEN];	//имя папки содержащее структуру, для объектов 
				// T_JUMP длина имени не должна превышать 3 символа
 union
 {
  DIR		*dirp;		//папка в которой расположен объект
  SOCKET	cs;		//сокет объекта T_NIF
 };
 

 struct Obj	*pl;	//ссылка на родительский объект
 int		cnt;	//число внешних ссылок на объект
 int		bitf;	//битовые флаги различного назначения, в том числе флаг существования

 struct Obj	*bo;	//список используемых структур объектов
 int		 lnkn;          //длина списка ссылок
 struct Lnk	*blnk;		//список ссылок на каждый sqrt(evtn) элемент очереди
 int		 evtn;		//длина локальной очереди событий
 struct Evt	*bevt;		//локальная очередь событий

 struct Lnk	*elst[ELST_NUM];	//списки ссылок на зависимые события

 union
 {
  shape		s;
  struct Nif	nb;		// буфера приема и отправки объекта T_NIF
  struct Lnk	*bo_turns;	// планы ходов участников боя в локации T_BL
 };

 union
 {
  struct Lnk	*locl;	//список локаций в которые попадает объект.
  struct Loc	*locs;	//двумерный массив локаций, для объекта T_WORLD
 };

 int		gview;  //признак глобальной видимости объекта
 int		colc;	//набор битов класса столкновений
 double		rt;	//время последней проверки реального режима

 union
 {
  struct Obj	*nif;	// указатель на объект - сетевой интерфейс для кораблика
  int		nact;	//счетчик подобъектов T_BL еще не сделавших ход + 1
  char 	*fpath;		// путь для T_SHAB
 };

 orbit		orb;	// параметры орбиты для орбитальных объектов
 devpar		dev;	// параметры устройства корабля
 mempar		*mem;	// память простейших ботов

 int		bside;  // сторона за которую воюет объект в пошаговом режиме, для объекта T_BL - число сторон в битве
 int		bmode;  // режим боевой локации: 0 -реалтайм, 1 -пошаговый
 int		turn;	// номер хода для объекта T_BL и объектов с признаком GOF_ONMAP

 struct Obj	*port;	// указатель на портал для T_PERS

} obj;
