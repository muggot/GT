void evt_bmv_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_buse_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_bobj_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_bkill_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_bm_act(obj *o,evt *e,lnk *l);
void evt_bmv_act(obj *o,evt *e,lnk *l);
void evt_bkill_act(obj *o,evt *e,lnk *l);
void evt_buse_act(obj *o,evt *e,lnk *l);
void evt_bobj_act(obj *o,evt *e,lnk *l);
void BL_init_rt(obj *o);

