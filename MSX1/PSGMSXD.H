void INIT(void);
void STOP(void);
void FEEDO(void);

extern short PARA;
extern char PARA2;
extern char PARA3;

//void play_bgm(unsigned char no, unsigned char noise)
void play_bgm(unsigned char mode) __sdcccall(1)
{
}

void playbgm(void) __sdcccall(1)
{
//	unsigned char *mem=(unsigned char *)INIT;
//	mem[0] = 0
//	mem[3] = noise;


	PARA = 0;

	INIT();
}

void stopbgm(void) __sdcccall(1)
{
	STOP();
}

char checkbgm(void) __sdcccall(1)
{
	return 1;
}

