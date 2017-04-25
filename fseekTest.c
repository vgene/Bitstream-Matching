#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const int PACKET_LEN = 188;
const int MAX_SEEK = 2048;

int mycmp(char* str1, char* str2, unsigned long length)
{
	for (int i=0; i<length; ++i){
		if(str1[i] != str2[i])
			return 0;
	}
	return 1;
}

void print_packet(char* cbuf, unsigned long length)
{
	unsigned char* buf = (unsigned char*) cbuf;
    unsigned char first;
    unsigned char second;

    for (int i = 0; i< length; ++i){
        first = buf[i]/16;
        second = buf[i]%16;
        printf("%x%x ", (unsigned char)first, second);
    }
    printf("\n");
}


// get three 0x47 in a row, which means we have an unchanged section
int get_head(FILE* fp, unsigned int start_pos)
{
	fseek(fp, start_pos, SEEK_SET);

	char ch;
	int check_len = 3*PACKET_LEN-1;
	char* buf = (char*) malloc(sizeof(char)*check_len);

	unsigned long cnt = start_pos;
	while (1){
		fread(&ch,sizeof(char), 1, fp);
#ifdef DEBUG
		print_packet((unsigned char* )&ch,1);
#endif
		if (ch == 0x47){
			fread(buf, sizeof(char), check_len, fp);
			if (buf[PACKET_LEN-1] == 0x47 && buf[PACKET_LEN*2 -1] ==0x47)
				break;
			fseek(fp, -check_len, SEEK_CUR);
		}
		cnt++;
	}

	free(buf);
	return cnt;
}

// get delay bytes from MAX_SEEK area
// @TODO should check from tail to head
unsigned long get_delay(FILE* fp1, FILE* fp2,
				unsigned long pos1, unsigned long pos2)
{
    //find the needle
    char* needle = (char*)malloc(sizeof(char)*PACKET_LEN);
    fread(needle, sizeof(char), PACKET_LEN, fp2);
#ifdef DEBUG
    printf("NEEDLE:\n");
    print_packet(needle, PACKET_LEN);
#endif
    unsigned long pin;
    char* candidate = (char*) malloc(sizeof(char)*PACKET_LEN);
    //find the needle in haystack
    for (pin = pos1; pin <= pos2; pin += PACKET_LEN){
    	// printf("%ld\n", pin);
    	fseek(fp1, pin, SEEK_SET);
    	fread(candidate, sizeof(char), PACKET_LEN, fp1);
		// print_packet(candidate, PACKET_LEN);
		// print_packet(needle, PACKET_LEN);
    	int ismatch = mycmp(needle, candidate, PACKET_LEN);
    	if (ismatch){
    		//find needle!
#ifdef DEBUG
    		printf("CANDIDATE:\n");
			print_packet(candidate, PACKET_LEN);
#endif
			// print_packet(needle, PACKET_LEN);
    		break;
    	}
    }
    //delay = pos2-pin
    unsigned long delay = pos2 - pin;
    return delay;
}


// find the first different bit
unsigned long get_message_head(FILE* fp1, FILE* fp2,
			unsigned long start_pos_data, unsigned long delay)
{
	unsigned long pos_data_2 = start_pos_data;
	unsigned long pos_data_1 = start_pos_data - delay;

	fseek(fp1, pos_data_1, SEEK_SET);
	fseek(fp2, pos_data_2, SEEK_SET);

	char ch1, ch2;
	while(1){
		// if finish one file, no message found
		if (feof(fp1) || feof(fp2)){
			printf("No Message Found!\n");
			return 0;
		}

		// read one byte
		fread(&ch1, sizeof(char), 1, fp1);
		fread(&ch2, sizeof(char), 1, fp2);
		if (ch1 != ch2)
			return pos_data_2;

		pos_data_1++;
		pos_data_2++;
	}

	return 0;
}

void get_message(FILE* fp1, FILE* fp2,
		unsigned long message_head_pos, unsigned long delay)
{

}

int main()
{
    FILE* fp1 = fopen("./04-11rx_2k_rate1_2.ts","rb");
    FILE* fp2 = fopen("./04-11rx_2k_rate1_2_2.ts","rb");

    unsigned long pos2 = get_head(fp2, MAX_SEEK+1);
    unsigned long pos1 = get_head(fp1, pos2-MAX_SEEK);
    fseek(fp1, pos1, SEEK_SET);
    fseek(fp2, pos2, SEEK_SET);

    unsigned long delay = get_delay(fp1, fp2, pos1, pos2);

    printf("Delay: %ld\n", delay);

    unsigned long message_head_pos = get_message_head(fp1, fp2, pos2, delay);

    printf("Message Pos:%ld\n", message_head_pos);

    fclose(fp1);
    fclose(fp2);
}