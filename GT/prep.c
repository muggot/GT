#include <stdio.h>
#include <string.h>
#include "cfg.h"

#define MAXLEN 4096

struct conf cfg={0};

int gettag(FILE *fp)
{
 char tn[16];
 if(fscanf(fp,"%15s",tn)==EOF)  return 0;
 if(strstr("DATAPATH",tn)!=NULL) return 1;
 if(strstr("LOGPATH",tn)!=NULL) return 2;
 return -1;
}

int getcfg()
{
 FILE *fp;
 int tag;
 int res;

 fp=fopen("config","rt"); if(fp==NULL) return 1;
 while((tag=gettag(fp))!=0)
  {
   switch(tag)
    {
     case -1:
      if(fscanf(fp,"%*[^\n]")==EOF || fscanf(fp,"%*[\n]")==EOF) return 0;
      break;
     case 1:
      res=fscanf(fp,"%*[ \t]%255[^\n]",cfg.data_path);
      cfg.dpl=strlen(cfg.data_path);
      if(res==EOF || fscanf(fp,"%*[\n]")==EOF) return 0;
      break;
     case 2:
      res=fscanf(fp,"%*[ \t]%255[^\n]",cfg.log_path);
      cfg.lpl=strlen(cfg.log_path);
      if(res==EOF || fscanf(fp,"%*[\n]")==EOF) return 0;
      break;
     default:
      break;
    }                  
  }
 fclose(fp);
 return 0;
}
