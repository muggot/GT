#include "gt.h"
#include "evt_oload.h"
#include "evt_oinit.h"


void dev_CRGO_mnt(obj *o,int mnt)
{
}

void dev_ENG_mnt(obj *o,int mnt)
{
 if(mnt==1)
 {
  o->pl->dev.pwr[0]+=o->dev.pwr[0]; // �㬬���� ��魮��� �����⥫�� ��ࠡ��
  o->pl->dev.pwr[1]+=o->dev.pwr[1]; // �㬬���� ��魮��� �����⥫�� ��ࠡ��
  o->pl->dev.pwr[2]+=1.0/o->dev.pwr[2]; // �㬬���� �����७����� �����⥫�� ��ࠡ��
  return;
 }
 o->pl->dev.pwr[0]-=o->dev.pwr[0];
 o->pl->dev.pwr[1]-=o->dev.pwr[1];
 o->pl->dev.pwr[2]-=1.0/o->dev.pwr[2];
}

void dev_FLD_mnt(obj *o,int mnt)
{
}

void dev_FTN_mnt(obj *o,int mnt)
{
}

void dev_HULL_mnt(obj *o,int mnt)
{
}

void dev_WPN_mnt(obj *o,int mnt)
{
}

void dev_ARS_mnt(obj *o,int mnt)
{
}


// ��䥪� �� ����஢���� ���ன�⢠ ��।������ ⨯�� ���ன�⢠, � �� ᫮�.
void (*dev_func_mnt[OBJ_TAG_NUM])(obj *o,int mnt)=
{
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 dev_CRGO_mnt,
 dev_ENG_mnt,
 dev_FLD_mnt,
 dev_FTN_mnt,
 dev_HULL_mnt,
 dev_WPN_mnt,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 dev_ARS_mnt,
 NULL
};

void dev_mnt(obj *o)
{
 dev_func_mnt[o->tag](o,1);
}

void dev_umnt(obj *o)
{
 dev_func_mnt[o->tag](o,0);
}


