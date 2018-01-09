#include <cstdio>

union tag{
	unsigned char c[8];
	unsigned long long n;
}u;

int main(){
 	u.c[0] = 0b00000000;
 	u.c[1] = 0b00000000;
 	u.c[2] = 0b00000000;
 	u.c[3] = 0b00000000;
 	u.c[4] = 0b00000000;
 	u.c[5] = 0b00000000;
 	u.c[6] = 0b00000000;
 	u.c[7] = 0b00000000;

 	u.c[0] = 0b00000000;
 	u.c[1] = 0b00111100;
 	u.c[2] = 0b01111110;
 	u.c[3] = 0b01111110;
 	u.c[4] = 0b01111110;
 	u.c[5] = 0b01111110;
 	u.c[6] = 0b00111100;
 	u.c[7] = 0b00000000;

	printf("0x%llx\n", u.n);
	return 0;
 }

