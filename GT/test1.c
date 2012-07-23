#include <windows.h>
main()
{
 long long time=0;
 int i,sz=8;
for(i=0;i<1000000;i++)
{
 if((i%1000)==0) printf("%d %Ld\n",i,time); sz=8;
 RegQueryValueEx(HKEY_PERFORMANCE_DATA,"",NULL,NULL,(char *)(&time),&sz);
}
RegCloseKey(HKEY_PERFORMANCE_DATA); 
}