#include <stdio.h>
#include <math.h>

void main(void)
{
	int i;
	short sin_num;
	printf("const short sin_table[256 * 4] = {\n\t");
	for(i = 0; i < (256*4); ++i){
//		sin_table[i]  =  sin(i * 0.12) * 55;
		sin_num =  sin(i * 0.12) * 55;
		printf("%d, ",sin_num);
	}
	printf("\n};\n");
}
