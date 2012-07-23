/* Число типов космических объектов */
#define		OBJ_TAG_NUM	27

#ifdef M_PROC
#define MEL ={
#else
#define MEL ;
#endif

/* Тэги космических объектов */
/* Имена типов космических объектов */

extern const char *oname[OBJ_TAG_NUM] MEL
#define		T_EMP		-1	//структура не используется
#ifdef M_PROC
 "GAME",
#endif
#define		T_GAME		0	//игра
#ifdef M_PROC
 "W",
#endif
#define		T_WORLD		1	//мир
#ifdef M_PROC
 "FF",
#endif
#define		T_FF		2	//защитное поле
#ifdef M_PROC
 "S",
#endif
#define		T_STATION	3	//станция
#ifdef M_PROC
 "JG",
#endif
#define		T_JUMP		4	//портал
#ifdef M_PROC
 "SY",
#endif
#define		T_SY		5	//верфь
#ifdef M_PROC
 "FP",   
#endif
#define		T_FP		6	//заправка
#ifdef M_PROC
 "P",
#endif
#define		T_PERS		7	//корабль
#ifdef M_PROC
 "NIF",
#endif
#define		T_NIF		8	//сетевое соединение
#ifdef M_PROC
 "AS",
#endif
#define		T_AS		9	//астероид
#ifdef M_PROC
 "BL",
#endif
#define		T_BL		10	//боевая локация
#ifdef M_PROC
 "SHP",
#endif
#define		T_SHP		11	//корпус корабля
#ifdef M_PROC
 "CRGO",
#endif
#define		T_CRGO		12	//трюм
#ifdef M_PROC
 "ENG",
#endif
#define		T_ENG		13	//двигатель
#ifdef M_PROC
 "FLD",
#endif
#define		T_FLD		14	//генератор защитного поля
#ifdef M_PROC
 "FTN",
#endif
#define		T_FTN		15	//топливный бак
#ifdef M_PROC
 "HULL",
#endif
#define		T_HULL		16	//броня
#ifdef M_PROC
 "WPN",
#endif
#define		T_WPN		17	//оружие
#ifdef M_PROC
 "BS",
#endif
#define		T_BS		18	//пространственный разлом
#ifdef M_PROC
 "UL",
#endif
#define		T_UL		19	//улей спарков
#ifdef M_PROC
 "SP",
#endif
#define		T_SP		20	//спарк
#ifdef M_PROC
 "MUSOR",
#endif
#define		T_MUSOR		21	//космический мусор (контейнер)
#ifdef M_PROC
 "RES",
#endif
#define		T_RES		22	//рес
#ifdef M_PROC
 "ARS",
#endif
#define		T_ARS		23	//автоматическая ремонтная система
#ifdef M_PROC
 "SHAB",
#endif
#define		T_SHAB		24	//контейнер шаблонных объектов
#ifdef M_PROC
 "DIR",
#endif
#define		T_DIR		25	//папка однотипных объектов внутри T_WORLD
#ifdef M_PROC
 "SHOP"
#endif
#define		T_SHOP		26	//магазин это объект, а бартер свойство двух объектов

#ifdef M_PROC
};
#endif

