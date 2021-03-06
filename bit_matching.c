/*
* Ziyang Xu (ziyang.pku@gmail.com)
* Bit Stream Matching Module for Backscatter-DVBT
* April. 25, 2017
* Modify: May. 4, 2017
*/
#define MESSAGE_CONTINUE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const int PACKET_LEN = 188;
const int MAX_SEEK = 0x500000; // where should i start to search, aka.forward offset
const int MAX_MSG_LEN = 100+1024;
const int ROBUST_CMP_CNT = 1;
const int MATCH_LEN = PACKET_LEN*3;

int mycmp(char* str1, char* str2, unsigned long length)
{
	int miss_cnt = 0;

	for (int i=0; i<length; ++i){
		if(str1[i] != str2[i]){
			miss_cnt++;
			if (miss_cnt>=ROBUST_CMP_CNT)
				return 0;
		}
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
    printf("\n\n");
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
    char* needle = (char*)malloc(sizeof(char)*MATCH_LEN);
    fread(needle, sizeof(char), MATCH_LEN, fp2);
#ifdef DEBUG
    printf("NEEDLE:\n");
    print_packet(needle, PACKET_LEN);
#endif
    unsigned long pin;
    char* candidate = (char*) malloc(sizeof(char)*MATCH_LEN);
    //find the needle in haystack
    int found_needle = 0;
    for (pin = pos1; pin <= pos2; pin += PACKET_LEN){
    	// printf("%ld\n", pin);
    	fseek(fp1, pin, SEEK_SET);
    	fread(candidate, sizeof(char), MATCH_LEN, fp1);
		// print_packet(candidate, MATCH_LEN);
		// print_packet(needle, MATCH_LEN);
    	int ismatch = mycmp(needle, candidate, MATCH_LEN);
    	if (ismatch){
    		//find needle!
    		printf("FOUND NEEDLE\n");
    		found_needle = 1;
#ifdef DEBUG
    		printf("CANDIDATE:\n");
			print_packet(candidate, MATCH_LEN);
#endif
			// print_packet(needle, PACKET_LEN);
    		break;
    	}
    }
    if (!found_needle)
    	return (unsigned long)-1;
    printf("%ld,%ld\n", pos2, pin);
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

unsigned long get_message(FILE* fp1, FILE* fp2, char* content,
		unsigned long message_head_pos, unsigned long delay)
{
	fseek(fp2, message_head_pos, SEEK_SET);
	fseek(fp1, message_head_pos-delay, SEEK_SET);

	unsigned long cnt = 0;
	int zero_cnt = 0;
	char ch1, ch2, ch;
	while(1){
#ifdef DEBUG
		unsigned long len = ftell(fp2);
		printf("POS FP2: %ld\n", len);
		len = ftell(fp1);
		printf("POS FP1: %ld\n", len);
#endif
		if (feof(fp1) || feof(fp2) || cnt >= MAX_MSG_LEN){
			printf("Message End!\n");
			return cnt;
		}
		fread(&ch1, sizeof(char), 1, fp1);
		fread(&ch2, sizeof(char), 1, fp2);

		ch = ch1^ch2;
#ifdef DEBUG
		print_packet(&ch, 1);
#endif

		content[cnt] = ch;
		cnt++;

		if (ch == 0)
			zero_cnt++;
		else
			zero_cnt=0;
	}

	content[cnt+1] = '\0';
	return cnt;
}

void get_message_continue(FILE* fp1, FILE* fp2, unsigned long message_head_pos,
			unsigned long delay)
{
	fseek(fp2, message_head_pos, SEEK_SET);
	fseek(fp1, message_head_pos-delay, SEEK_SET);

	unsigned long cnt = 0;
	int zero_cnt = 0;
	char ch1, ch2, ch;
	while(1){
		if (feof(fp1) || feof(fp2)){
			printf("Message End!\n");
			return;
		}
		fread(&ch1, sizeof(char), 1, fp1);
		fread(&ch2, sizeof(char), 1, fp2);

		ch = ch1^ch2;
		fwrite(&ch, 1, 1, stdout);
		cnt++;
		// if (ch == 0)
		// 	zero_cnt++;
		// else
		// 	zero_cnt=0;
	}
}


int main(int argc, char const *argv[])
{
	if (argc != 3){
		printf("Usage: %s TSfile1 TSfile2\n", argv[0]);
		return -1;
	}
	// open two files
    FILE* fp1 = fopen(argv[1],"rb");
    FILE* fp2 = fopen(argv[2],"rb");

    // get head (three 0x47 in a row) for both file1 and file2
    unsigned long pos2 = get_head(fp2, MAX_SEEK+1);
    unsigned long pos1 = get_head(fp1, pos2-MAX_SEEK);
    fseek(fp1, pos1, SEEK_SET);
    fseek(fp2, pos2, SEEK_SET);

    printf("pos1, pos2: %ld, %ld\n", pos1, pos2);
    // get delay byte count
    unsigned long delay = get_delay(fp1, fp2, pos1, pos2);
    if (delay == (unsigned long) -1){
    	printf("Doesn't Match\n");
    	return -1;
    }

    printf("Delay: %ld\n", delay);

    // get message head position
    unsigned long message_head_pos = get_message_head(fp1, fp2, pos2, delay);
    printf("Message Pos:%ld\n", message_head_pos);

    if (message_head_pos != 0){
#ifdef MESSAGE_CONTINUE
    	get_message_continue(fp1,fp2,message_head_pos,delay);
#else
	    char* content = (char*) malloc(sizeof(char)* (MAX_MSG_LEN+1));
	    unsigned long message_length = get_message(fp1, fp2, content, message_head_pos, delay);
	    printf("Message: ");
        print_packet(content, message_length);
#endif

	}

    fclose(fp1);
    fclose(fp2);

    return 0;
}