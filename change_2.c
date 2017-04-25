#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const int PACKET_LEN = 188;
const int MAX_SEEK = 2048;

int main(){
	FILE* fp2 = fopen("./04-11rx_2k_rate1_2_2.ts","r+b");

	fseek(fp2, 0, SEEK_END);
	unsigned long len = ftell(fp2);
	printf("%ld\n", len);
	fseek(fp2, -400, SEEK_END);
	len = ftell(fp2);
	printf("%ld\n", len);

	for (char i = 0; (int)i<100;i++){
		fputc(i, fp2);
		// printf("%c\n", i);
	}
	len = ftell(fp2);
	printf("%ld\n", len);
	fclose(fp2);

	return 0;
}