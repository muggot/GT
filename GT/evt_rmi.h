void evt_mvs_user_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_mvf_user_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_mvfd_user_set(evt *ne,obj *o,evt *e,lnk *l,double time);
void evt_prich_user_set(evt *ne,obj *o,evt *e,lnk *l,double time);

void evt_mvs_user_act(obj *o,evt *e,lnk *l);
void evt_mvf_user_act(obj *o,evt *e,lnk *l);
void evt_mvfd_user_act(obj *o,evt *e,lnk *l);
void evt_prich_user_act(obj *o,evt *e,lnk *l);

void recv_cmds_proc(obj *o);

lnk *rmi_lnk_mv(obj *co,evt *de);
void bl_cmds_proc(obj *o);
