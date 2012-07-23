// обработчик транзакций
#include <stdio.h>
#include <direct.h>
#include <dos.h>
#include <io.h>

#include "prep.h"
extern struct conf cfg;

#define TR_UPDATE	1
#define TR_CREATE	2
#define TR_DELETE	3
#define TR_RENAME	4
#define TR_CPOINT	5


char ftn[32];
char fn1[4096];
char fn2[4096];

int tr_type()
{
 FILE *tr;
 int tt;

 while((tr=fopen(ftn,"rb"))==NULL) sleep(1);
 while(fread(&tt,sizeof(tt),1,tr)<=0) 
 { 
  fclose(tr); sleep(1); tr=fopen(ftn,"rb");
 }
 fclose(tr);
 return tt;
}

void myrmdir(char *dn)
{
 DIR *rd;
 struct dirent *tf;

 chdir(dn);
 rd=opendir(".");
 while((tf=readdir(rd))!=NULL) 
 {
  if(tf->d_name[0]!='.') 
   {
    if((tf->d_attr & _A_SUBDIR)==0) remove(tf->d_name);
    else
    {
     myrmdir(tf->d_name);
    }
   }
 }
 closedir(rd);
 chdir("..");
 rmdir(dn);
}

void tr_process(long long n)
{
 FILE *tr,*fp;
 int tt;
 char s,*p;
 int cp,sz;

 sprintf(ftn,"%018Ld",n);
 tr=fopen(ftn,"rb");
 fread(&tt,sizeof(tt),1,tr);
 p=fn1; do{ fread(&s,1,1,tr); *p=s; p++;} while(s!=0);
 if(tt==TR_RENAME) 
  { 
   p=fn2; do{ fread(&s,1,1,tr); *p=s; p++;} while(s!=0); fclose(tr);
   rename(fn1,fn2); remove(ftn); return;
  }
 if(tt==TR_DELETE) 
  {
   fclose(tr); p=getcwd(NULL,0); myrmdir(fn1); chdir(p); remove(ftn); return;
  }
 if(tt==TR_UPDATE || tt==TR_CPOINT) 
  {
   cp=ftell(tr); fseek(tr,0,SEEK_END); sz=ftell(tr)-cp-2; fseek(tr,cp,SEEK_SET);
   fp=fopen(fn1,"wb");
    while(sz>4096) { fread(fn2,4096,1,tr); fwrite(fn2,4096,1,fp); sz-=4096; }
    if(sz>0) { fread(fn2,sz,1,tr); fwrite(fn2,sz,1,fp); }
   fclose(fp); fclose(tr); remove(ftn); return;
  }
 if(tt==TR_CREATE)
  { 
   mkdir(fn1); p=fn1; do{ fread(&s,1,1,tr); *p=s; p++;} while(s!=0);
   cp=ftell(tr); fseek(tr,0,SEEK_END); sz=ftell(tr)-cp-2; fseek(tr,cp,SEEK_SET);
   fp=fopen(fn1,"wb");
    while(sz>4096) { fread(fn2,4096,1,tr); fwrite(fn2,4096,1,fp); sz-=4096; }
    if(sz>0) { fread(fn2,sz,1,tr); fwrite(fn2,sz,1,fp); }
   fclose(fp); fclose(tr); remove(ftn); return;
  }
}

int main()
{
 DIR *ld; // папка с файлами транзакций
 struct dirent *tf;
 FILE *tr;
 long long i,lcp=-1,mtn=0x7fffffffffffffff; // минимальный номер транзакций из последовательности
 int tt;

 if(getcfg()!=0) { printf("2 Ошибка при чтении файла настроек\n"); return 1; }
 if(chdir(cfg.log_path)!=0) { printf("2 Папка журнала не найдена\n"); return 1; };

// найдем транзакцию с минимальным номером
 ld=opendir(".");
 while((tf=readdir(ld))!=NULL) 
 {
  if(tf->d_name[0]!='.') 
   { sscanf(tf->d_name,"%Ld",&i); if(i<mtn) mtn=i; }
 }
 closedir(ld);

 if(mtn!=0x7fffffffffffffff)
 {
// найдем последнюю контрольную точку
  i=mtn; sprintf(ftn,"%018Ld",i);
  while((tr=fopen(ftn,"rb"))!=NULL)
  {
   fread(&tt,sizeof(tt),1,tr); if(tt==TR_CPOINT) lcp=i;
   fclose(tr); i++; sprintf(ftn,"%018Ld",i);
  }

// удалим все транзакции после контрольной точки
  i=lcp+1; sprintf(ftn,"%018Ld",i);
  while(access(ftn,0)==0) { remove(ftn); i++; sprintf(ftn,"%018Ld",i); }

// обработаем все транзакции и очистим журнал
  for(i=mtn;i<=lcp;i++) tr_process(i);
 }

 mtn=0; lcp=-1;
 for(i=0;i<0x7fffffffffffffff;i++)
 {
  sprintf(ftn,"%018Ld",i); while(access(ftn,0)!=0) sleep(1);
  tt=tr_type(); if(tt==TR_CPOINT) lcp=i;
  for(;mtn<=lcp;mtn++) tr_process(mtn);
 }
 return 0;
}

