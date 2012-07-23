
// получает свободное событие для дальнейшего использования
evt *get_evt(void);

// получает свободную ссылку для дальнейшего использования
lnk *get_lnk(void);

// вставляет событие в очередь.
void ins_evt(obj *o,evt *e);

// освобождает ссылку
void free_lnk(obj*o,lnk *l);

// освобождает событие
void free_evt(obj *o,evt *e);

void ins_par_evt(obj *o,evt *e);
void del_par_evt(obj *o);

// удаляет событие из очереди
void del_evt(obj *o,evt *e);

// изменяет положение события в очереди в соответствии с установленным временем
// событие сначала удалется, а затем вставляется снова без изменения числа событий
void move_evt(obj *o,evt *e);

// тестовое событие порождает новое событие со случайным смещением
// по времени относительно заданного в интервале (0.0,1.0)
void rand_evt(evt *e);

// нулевой цикл моделирования
void proc_evts(obj *o);

//#ifdef M_PROC
extern void (*evt_func_act[E_NUMBER])(obj *o,evt *e,lnk *l);
extern void (*evt_func_set[E_NUMBER])(evt *ne,obj *o,evt *e,lnk *l,double time);
extern obj *fload_obj(obj *po,char *name);
//#endif

void evt_lnks_act(int tag,obj *o);
lnk *evt_lnk_ins(int tag,obj *o,evt *e,lnk *l2);
void evt_lnk_del(lnk *l);

extern int lnks_tags[E_NUMBER];
