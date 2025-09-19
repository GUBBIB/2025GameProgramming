#include<stdio.h>
#include<windows.h>
void print_frequency(int octave);
int calc_frequency(int octave, int inx);

int main(void) {
    const char *scale[] = { "도", "레", "미",
                            "파", "솔", "라", "시", "도"};
    int i, octave;
    int freq[8];
    int index[] = {0, 2, 4, 5, 7, 9, 11, 12};
    
    printf("음계와 주파수\n\n");
    printf("음계\t");
    for (i = 0; i < 12; i++) {
        printf("%-6s", scale[i]);
    }
    printf("\n");

    for (i = 0; i <= 70; i++) printf("-");
    printf("\n");
    
    for(i=0; i<=7; i++){
    	freq[i] = calc_frequency(4, index[i]);
	}

	int song[] = {0, 0, 4, 4, 5, 5, 4, 3, 3, 2, 2, 1, 1, 0,
					4, 4, 3, 3, 2, 2, 1, 4, 4, 3, 3, 2, 2, 1,
					0, 0, 4, 4, 5, 5, 4, 3, 3, 2, 2, 1, 1, 0};

	
	

	for(i=0; )
	

    return 0;
}

void print_frequency(int octave)
{
	double do_scale=32.7032;
	double ratio = pow(2., 1/12.), temp;
	
	int i;
	temp = do_scale*pow(2, octave-1);
	printf("%d 옥타브 : ", octave);
	for(i=0; i<12; i++){
		printf("%ld ", (unsigned long) (temp+0.5));
		temp *= ratio;
	}
	printf("\n");
}

int calc_frequency(int octave, int inx){
	double do_scale=32.7032;
	double ratio=pow(2., 1/12.), temp;
	int i;
	temp = do_scale*pow(2, octave-1);
	for(i=0; i<inx; i++){
		temp=(int) (temp+0.5);
		temp*=ratio;
	}
	
	return (int) temp;
}