// выделяет объект и проводит его начальную инициализацию
obj *get_obj(obj *po, int tag, char *gn);
void ins_obj(obj *po, obj *o);
void del_obj(obj *o);
void free_obj(obj *o);
int find_obj_ombase(obj *o);
void obj_ch_parent(obj *o,obj *po);
int is_obj_onmap(obj *o);
void obj_new_name(obj *o);
obj *obj_create_MUSOR(obj *po,obj *so);
obj *obj_create_PERS(char *name,char *pass);
obj *obj_create_RES(obj *po,int img,float mass);
obj *obj_create_SP(obj *po, int img);
float get_obj_dist(obj *o,obj *eo);
void set_ghab(obj *o);
