#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data);

int main(int argc, char *argv[]) {
    if(argc == 1){
        printf("Example 1:\n");
        int numbers[2] = {61810, 2025};
        memdump("ii", (char*)numbers);

        printf("Example 2:\n");
        memdump("S", "a string");

        printf("Example 3:\n");
        char *text = "another";
        memdump("s", (char*)&text);

        struct sample {
            char *ptr;
            int num1;
            short num2;
            char byte;
            char bytes[8];
        } demo;

        demo.ptr = "hello";
        demo.num1 = 1819438967;
        demo.num2 = 100;
        demo.byte = 'z';
        strcpy(demo.bytes, "xyzzy");

        printf("Example 4:\n");
        memdump("pihcS", (char*)&demo);

        printf("Example 5:\n");
        memdump("sccccc", (char*)&demo);

    } else if(argc == 2){
        char buffer[512];
        int total = 0;
        memset(buffer, 0, sizeof(buffer));

        int n;
        while(total < sizeof(buffer) && (n = read(0, buffer + total, sizeof(buffer) - total)) > 0){
            total += n;
        }

        memdump(argv[1], buffer);

    } else {
        printf("Usage: memdump [format]\n");
        exit(1);
    }

    exit(0);
}

static void print_hex32(uint32 val) {
    int started = 0;
    for(int i = 7; i >= 0; i--){
        int nib = (val >> (i*4)) & 0xF;
        if(nib || started){
            started = 1;
            char c = (nib < 10) ? ('0'+nib) : ('a'+(nib-10));
            write(1, &c, 1);
        }
    }
    if(!started){ char z='0'; write(1, &z, 1); }
    char nl='\n'; write(1, &nl, 1);
}

void memdump(char *fmt, char *data){
    for(char *f = fmt; *f; f++){
        switch(*f){
            case 'i': {
                uint32 val = 0;
                for(int i=0; i<4; i++) val |= (uint32)(unsigned char)data[i] << (8*i);
                printf("%d\n", (int)val);
                data += 4;
                break;
            }
            case 'p': {
                uint64 val = 0;
                for(int i=7; i>=0; i--) val = (val << 8) | (uint64)(unsigned char)data[i];
                print_hex32((uint32)val);
                data += 8;
                break;
            }
            case 'h': {
                uint16 val = 0;
                for(int i=0; i<2; i++) val |= (uint16)(unsigned char)data[i] << (8*i);
                printf("%d\n", (int)val);
                data += 2;
                break;
            }
            case 'c': {
                printf("%c\n", *data);
                data++;
                break;
            }
            case 's': {
                uint64 addr = 0;
                for(int i=7; i>=0; i--) addr = (addr << 8) | (uint64)(unsigned char)data[i];
                printf("%s\n", (char*)addr);
                data += 8;
                break;
            }
            case 'S': {
                printf("%s\n", data);
                return;
            }
        }
    }
}

