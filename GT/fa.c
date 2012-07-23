// формирует и заполняет таблицу конечного автомата на основе
// списка строковых переменных
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

long long *fa_b(int tagn, const char **tags)
{
 int i,j,maxl=0;
 long long *fa_tab,k;

 if(tagn>64) return NULL;

 for(i=0;i<tagn;i++) if(strlen(tags[i])>maxl) maxl=strlen(tags[i]);
 maxl++;
 fa_tab=calloc(128*maxl,sizeof(long long));

 k=1;
 for(i=0;i<tagn;i++)
  {
   maxl=strlen(tags[i]);
   for(j=0;j<maxl;j++) fa_tab[(j<<7)+tags[i][j]]|=k;
   fa_tab[(j<<7)+' ']|=k; // для контроля длины тэга
   k<<=1;
  }
 return(fa_tab);
}

// определяет номер переменной в списке
int fa_s(char *tag, long long *fa_tab)
{
 long long res=0xffffffffffffffff;
 int len=strlen(tag);
 int *tr=(int *)(&res),i,size;
 for(i=0;i<len;i++) res&=fa_tab[(i<<7)+tag[i]];
 res&=fa_tab[(i<<7)+' '];
 if(res==0) return -1; //нет такой строки в списке
 
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
 return i;
}
