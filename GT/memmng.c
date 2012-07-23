// менеджер памяти для разноразмерных ресурсов
// память выделяется блоками размером равным степени 2
// всего 32 типа блоков
#include <stdio.h>
#include <stdlib.h>

typedef struct fMBlock
{
 struct fMBlock *next;
 struct fMBlock *prev;
} fmblock;

typedef struct fMPool
{
 fmblock *(fl[32]);
} fmpool;

static fmpool FMP={0};

void *mymalloc(int size)
{
 fmblock *nb; char *c;
 int index=0; 
 int sz=size;

 if(size<=0) return NULL;

 if((size=(sz>>16))!=0) { index+=16; sz=size; }
 if((size=(sz>>8))!=0)  { index+=8;  sz=size; }
 if((size=(sz>>4))!=0)  { index+=4;  sz=size; }
 if((size=(sz>>2))!=0)  { index+=2;  sz=size; }
 if((size=(sz>>1))!=0)  { index+=1;  sz=size; }
 index++;

 if(FMP.fl[index]==NULL) 
  {
   nb=(fmblock *)malloc((1<<index)+8); printf("Выделение блока %d\n",index);
   FMP.fl[index]=nb; nb->next=NULL; nb->prev=NULL;
  }
 nb=FMP.fl[index]; FMP.fl[index]=nb->next;
 if(FMP.fl[index]!=NULL) FMP.fl[index]->prev=NULL;
 c=(char *)nb; c[0]=index; c[1]=c[2]=c[3]=0xaa;
 index=(1<<index)+4; *(int *)(&c[index])=*(int *)c;
 return (void *)&c[4];
}

void myfree(void *b)
{
 fmblock *nb,*fb;
 unsigned char *i=b;
 int *i4=b,*j4,k;
 FILE *fp;

 if(b==NULL) return;
 i-=4; i4-=1; j4=(int *)(&i[(1<<i[0])+4]);
 if(i[0]>32 || i[1]!=0xaa || i[2]!=0xaa || i[3]!=0xaa || (i4[0]^j4[0])!=0)
  {
   fp=fopen("free_err.log","at");
   fprintf(fp,"Ошибка освобождения блока памяти по адресу %x\n",b);
   fprintf(fp,"Сохраняю содержимое первых 10 байт блока:\n");
   for(k=0;k<10;k++) fprintf(fp,"%02x",i[k]);
   fprintf(fp,"\n");
   fclose(fp);
  }
 fb=(fmblock *)i;
 nb=FMP.fl[*i]; FMP.fl[*i]=fb; fb->next=nb; fb->prev=NULL;
 if(nb!=NULL) nb->prev=fb;
}

