#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

const int PACKET_LEN = 188;
const int MAX_SEEK = PACKET_LEN*32;
void print_packet(unsigned char* buf, unsigned long length){
    unsigned char first;
    unsigned char second;

    for (int i = 0; i< length; ++i){
        first = buf[i]/16;
        second = buf[i]%16;
        printf("%x%x ", (unsigned char)first, second);
    }
    printf("\n");
}

void match_head(FILE* fp1, FILE* fp2){
    //find 3 0x47 in a row in fp2
    bool got_head = 0;
    unsigned long cnt = 0;
    unsigned char* buf = (unsigned char*)malloc(sizeof(char)*MAX_SEEK+1);

    while (1){
        //find first 0x47
        fread(buf,sizeof(char), 1, fp1);
        cnt++;
        if (buf[0] == 0x47){
            fread(buf, sizeof(char),3*PACKET_LEN-1,fp1);
            if (buf[PACKET_LEN-1] == 0x47 && buf[2*PACKET_LEN -1] ==0x47)//got three 0x47
                break;
            else
                continue;
        }
    }

    //find three 0x47 in fp2
    char* needle = (char*) buf;
    needle[3*PACKET_LEN-1] = '\0';
    fread(NULL, sizeof(char),cnt,fp2);
    char* haystack = (char*) malloc(sizeof(char)*MAX_SEEK+1);
    haystack = fread(lock, sizeof(char), MAX_SEEK, fp2);

    unsigned long delay = strstr(haystack, needle) - haystack;

    rewind(fp1);
    rewind(fp2);



    while (1){
        fread(buf,sizeof(char), 1, fp);
        cnt++;
        if (buf[0] == 0x47){
            fread(buf, sizeof(char),PACKET_LEN-1,fp);
            if (((buf[0] & 0x1f) == 0) && (buf[1] == 0)){
                printf("%d\n", cnt);
                cnt = 0;
                getchar();
                while (!feof(fp)){
                    fread(buf, sizeof(char),PACKET_LEN,fp);
                    if (buf[0] == 0x47 &&((buf[0] & 0x1f) == 0) && (buf[1] == 0)){
                        cnt++;
                        printf("PAT%d\n", cnt);
                        getchar();
                    }
                    // print_packet(buf, PACKET_LEN);
                    // if (buf[0] != 0x47)
                    //     printf("Not Match!");
                    // else
                    //     printf("%d\r", cnt);
                }
                printf("\n");
                break;
            }
        }
    }
}

void find_message(){

}

/*
    unsigned char pointer: ptr1, ptr2, result (input and output)
    unsigned long: len (length of the char array)
    unsigned long: thres (if zero count is more than thres, message is over)
*/
unsigned long get_message(unsigned char* ptr1, unsigned char ptr2, unsigned char* result,
            unsigned long len, unsigned long thres = 188){

    unsigned long cnt = 0;
    unsigned long zero_cnt = 0;
    while (cnt < len){
        unsigned char tmp = ptr1[cnt] ^ ptr2[cnt];
        if (tmp == 0)
            zero_cnt++;
        else
            zero_cnt = 0;
        if (zero_cnt > thres){
            return cnt;
        }
        cnt++;
    }

    return 0;
}

int main(){
    FILE* fp = fopen("./04-11rx_2k_rate1_2.ts","rb");
    unsigned char* buf = (unsigned char*)malloc(sizeof(char)*PACKET_LEN+1);

    int cnt = 0;
    int tmp = 0;



    fclose(fp);
}
