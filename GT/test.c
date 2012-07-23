#include <stdio.h>
main()
{
 int i;
 FILE *fp;
 fp=fopen("ct","wb");
 for(i=0;i<256;i++) fwrite(&i,1,1,fp);
 fclose(fp);
}