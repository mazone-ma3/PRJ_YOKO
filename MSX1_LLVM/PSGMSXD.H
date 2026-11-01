void INIT(void);
void STOP(void);
void FEEDO(void);

extern short PARA;
extern char _PARA2;
extern char _PARA3;

//void play_bgm(unsigned char no, unsigned char noise)
void play_bgm(unsigned char mode)
{
}

void playbgm(void)
{
//	unsigned char *mem=(unsigned char *)INIT;
//	mem[0] = 0
//	mem[3] = noise;


	PARA = 0;

	INIT();
}

void stopbgm(void)
{
	STOP();
}

char checkbgm(void)
{
	return 1;
}

