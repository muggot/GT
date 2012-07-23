#include "gt.h"

typedef struct Usertab
{
 int n;
 long long tab[256][ONAM_LEN];
 obj *o[64];
} usertab;

// static usertab UT[256]={0}; // потом нужно увеличить до 1024

usertab *UT;

void init_user_tab(void)
{
 UT=(struct Usertab *) malloc(256*sizeof(struct Usertab));
 memset(UT,0,256*sizeof(struct Usertab));
}

void add_user(char *str,obj *o)
{
 int i,k,tab=129,len=strlen(str);
 
 for(i=0;i<len;i++) tab=tab*str[i]&255; // получаю номер таблицы
 while(UT[tab].n==64) tab=(tab+1)&255;  // ищу первую неполную таблицу
 UT[tab].o[UT[tab].n]=o; k=(1<<UT[tab].n); UT[tab].n++; // добавляю новый объект в список
 
 for(i=0;i<len;i++) UT[tab].tab[i][str[i]]|=k;
 UT[tab].tab[i][' ']|=k; // для контроля длины тэга
}

obj *find_user(char *str)
{
 long long res=0xffffffffffffffff;
 int i,k,tab=129;
 int len=strlen(str);
 int *tr=(int *)(&res),size;
 
 for(i=0;i<len;i++) tab=tab*str[i]&255; // получаю номер таблицы
 tab--;
 do
 {
  tab=(tab+1)&255;
  for(i=0;i<len;i++) res&=UT[tab].tab[i][str[i]];
  res&=UT[tab].tab[i][' '];
 }
 while(UT[tab].n==64 && res==0);
 if(res==0) return NULL;

 if(tr[0]!=0)
  {
   i=0;
   if((size=(tr[0]>>16))!=0) { i+=16; tr[0]=size; }
   if((size=(tr[0]>>8))!=0)  { i+=8;  tr[0]=size; }
   if((size=(tr[0]>>4))!=0)  { i+=4;  tr[0]=size; }
   if((size=(tr[0]>>2))!=0)  { i+=2;  tr[0]=size; }
   if((size=(tr[0]>>1))!=0)  { i+=1;  tr[0]=size; }
  }
 else
  {
   i=32;
   if((size=(tr[1]>>16))!=0) { i+=16; tr[1]=size; }
   if((size=(tr[1]>>8))!=0)  { i+=8;  tr[1]=size; }
   if((size=(tr[1]>>4))!=0)  { i+=4;  tr[1]=size; }
   if((size=(tr[1]>>2))!=0)  { i+=2;  tr[1]=size; }
   if((size=(tr[1]>>1))!=0)  { i+=1;  tr[1]=size; }
  }
 return UT[tab].o[i];
}
