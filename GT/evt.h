#include "evt_type.h"

typedef struct Evt
{
 int	tag;
 struct Evt  *prev;
 struct Evt  *next;
 struct Evt  *pe; 	// используется только если является первым событием, указывает на положение события-ссылки в родительской очереди
 struct Lnk  *ql;	// это указатель на кеширующую ссылку очереди
 double t;		// время
 struct Obj  *o;	// объект, значение зависит от типа события
 struct Evt  *e;	// альтернативное событие, в основном используется в событиях уничтожения как указатель на уничтожаемое событие.
 struct Lnk  *l;        // указатель на ссылку из списка зависимости, только для реакций
} evt;

extern double GTIME;
extern long long OGEN;
