void evt_col_loc_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_cold_loc_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_col_loc_act(obj *o,evt *e,lnk *l);
void evt_cold_loc_act(obj *o,evt *e,lnk *l);
void loc_col(obj *o,evt *e);
void loc_obj_reg(obj *o, int sendflag);
void loc_obj_unreg(obj *o);
void locs_send_obj(obj *o);
void locs_send_mv(obj *o);
double get_loc_col_time(obj *o);
void loc_col_bmv(obj *o);
void locs_send_myobj(obj *po);


