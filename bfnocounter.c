#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#define BLOOM_FILTER_SIZE (1UL << (10 + 10))
#define BLOOM_FILTER_MASK ((1UL << (10 + 10)) - 1)
#define BITS_PER_LONG 32

unsigned int bloom_filter_exist = 0;
unsigned int bloom_filter_noexist = 0;

void set_bit(int nr, unsigned long *addr)
{
	addr[nr / BITS_PER_LONG] |= 1UL << (nr % BITS_PER_LONG);
}
int test_bit(unsigned int nr, const unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(((unsigned long *)addr)[nr / BITS_PER_LONG])) != 0;
}

int bloom_filter_real(unsigned char hash[], unsigned char* bloom_filter)
{
	int i;
	unsigned int* pos = (unsigned int*)hash;
	for(i = 0; i < 4; i++){
		if(!test_bit((*pos)&BLOOM_FILTER_MASK, (unsigned long *)bloom_filter))
			return 1;
		pos++;
	}
	return 0;
}

void bloom_filter_add(unsigned char hash[], unsigned char* bloom_filter)
{
	int i;
	unsigned int* pos = (unsigned int*)hash;
	if(bloom_filter_real(hash, bloom_filter)){
		bloom_filter_noexist++;
		for(i = 0; i < 4; i++){
			set_bit((*pos)&BLOOM_FILTER_MASK, (unsigned long *)bloom_filter);
			pos++;
		}
		return;
	}
	bloom_filter_exist++;
}

int main()
{
	FILE *file = NULL;
	int i;
	unsigned char* bloom_filter = malloc(BLOOM_FILTER_SIZE);
	memset(bloom_filter, 0, BLOOM_FILTER_SIZE);
	unsigned char hash[16];
	unsigned char src[4096];

	file = fopen("/root/blk/512M", "r");
	if(!file){
		printf("fopen error\n");
		return 0;
	}
	while(1){
		memset(&src, 0, 4096);
		if(0 == fread(src, sizeof(unsigned char), 4096, file))
			break;
		MD5(src, 4096, hash);
		bloom_filter_add(hash, bloom_filter);
		//for(i = 0; i < 2; i++)
		//	printf("%lx", *(unsigned long*)&hash[i * 8]);
		//putchar('\n');
	}

	printf("%x\n", BLOOM_FILTER_SIZE);
	printf("bloom_filter_exist=%d\n", bloom_filter_exist);
	printf("bloom_filter_noexist=%d\n", bloom_filter_noexist);

	free(bloom_filter);
	return 0;
}