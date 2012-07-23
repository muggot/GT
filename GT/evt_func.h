
// ����砥� ᢮������ ᮡ�⨥ ��� ���쭥�襣� �ᯮ�짮�����
evt *get_evt(void);

// ����砥� ᢮������ ��뫪� ��� ���쭥�襣� �ᯮ�짮�����
lnk *get_lnk(void);

// ��⠢��� ᮡ�⨥ � ��।�.
void ins_evt(obj *o,evt *e);

// �᢮������� ��뫪�
void free_lnk(obj*o,lnk *l);

// �᢮������� ᮡ�⨥
void free_evt(obj *o,evt *e);

void ins_par_evt(obj *o,evt *e);
void del_par_evt(obj *o);

// 㤠��� ᮡ�⨥ �� ��।�
void del_evt(obj *o,evt *e);

// ������� ��������� ᮡ��� � ��।� � ᮮ⢥��⢨� � ��⠭������� �६����
// ᮡ�⨥ ᭠砫� 㤠�����, � ��⥬ ��⠢����� ᭮�� ��� ��������� �᫠ ᮡ�⨩
void move_evt(obj *o,evt *e);

// ��⮢�� ᮡ�⨥ ��஦���� ����� ᮡ�⨥ � ��砩�� ᬥ饭���
// �� �६��� �⭮�⥫쭮 ��������� � ���ࢠ�� (0.0,1.0)
void rand_evt(evt *e);

// �㫥��� 横� ������஢����
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
