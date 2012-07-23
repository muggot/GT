
#define	OBJ_IN_MEMLST	100
#define LNK_IN_MEMLST	OBJ_IN_MEMLST*sizeof(obj)/sizeof(lnk)
#define EVT_IN_MEMLST	OBJ_IN_MEMLST*sizeof(obj)/sizeof(evt)
#define LOC_IN_MEMLST	OBJ_IN_MEMLST*sizeof(obj)/sizeof(loc)

typedef union Memblock
{
 obj	omem[OBJ_IN_MEMLST];
 lnk	lmem[LNK_IN_MEMLST];
 evt	emem[EVT_IN_MEMLST];
 loc	gmem[LOC_IN_MEMLST];
} mblk;

typedef struct Memlst
{
 struct Memlst *next;
 mblk	m;
} memlst;

typedef struct BlockPool
{
 struct Memlst	*ml;	//список областей памяти для размещения структур объектов
 struct Obj	*fo;	//список свободных структур объектов
 struct Lnk	*fl;	//список свободных ссылок
 struct Evt	*fe;	//список свободных событий
 struct Loc	*fg;	//список свободных локаций
} blockpool;

extern	blockpool	GP;

void *mymalloc(int size);
void myfree(void *mem);
