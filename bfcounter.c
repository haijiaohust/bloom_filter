#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#define BITS_PER_LONG 32
#define BLK_SIZE 4096

unsigned int bloom_filter_exist = 0;
unsigned int bloom_filter_noexist = 0;

struct bloom_filter_info{
	unsigned int bloom_filter_size;
	unsigned int bloom_filter_mask;
	unsigned short* bloom_filter;
};

int bloom_filter_real(unsigned char hash[], struct bloom_filter_info* bf)
{
	int i;
	unsigned int* pos = (unsigned int*)hash;
	for(i = 0; i < 4; i++){
		if(!bf->bloom_filter[(*pos)&bf->bloom_filter_mask])
			return 1;
		pos++;
	}
	return 0;
}

void bloom_filter_add(unsigned char hash[], struct bloom_filter_info* bf)
{
	int i;
	unsigned int* pos = (unsigned int*)hash;
	if(bloom_filter_real(hash, bf)){
		bloom_filter_noexist++;
		for(i = 0; i < 4; i++){
			bf->bloom_filter[(*pos)&bf->bloom_filter_mask]++;
			pos++;
		}
		return;
	}
	bloom_filter_exist++;
}

int main()
{
	FILE *file = NULL;
	FILE *out = NULL;
	int i, j, k;
	char path[20] = "/root/blk/";
	char file_name[7][9] = {"512M", "512M_1", "1G", "2G", "4G", "8G", "16G"};
	struct bloom_filter_info bf;
	unsigned char hash[16];
	unsigned char src[BLK_SIZE];

	out = fopen("result", "w+");
	if(!out){
		printf("fopen result error\n");
		return 0;
	}
	bf.bloom_filter_size = (1UL << (9 + 10));
	bf.bloom_filter_mask = bf.bloom_filter_size - 1;
	for(k = 0; k < 6; k++){
		bloom_filter_exist = 0;
		bloom_filter_noexist = 0;
		bf.bloom_filter_size <<= 1;
		bf.bloom_filter_mask = (bf.bloom_filter_size/sizeof(unsigned short)) - 1;
		bf.bloom_filter = malloc(bf.bloom_filter_size);
		memset(bf.bloom_filter, 0, bf.bloom_filter_size);
		for(j = 0; j < 7; j++){
			strcat(path, file_name[j]);
			file = fopen(path, "r");
			if(!file){
				printf("fopen path error\n");
				return 0;
			}
			path[10] = '\0';
			while(1){
				memset(&src, 0, BLK_SIZE);
				if(0 == fread(src, sizeof(unsigned char), BLK_SIZE, file))
					break;
				MD5(src, BLK_SIZE, hash);
				bloom_filter_add(hash, &bf);
				//for(i = 0; i < 2; i++)
				//	printf("%lx", *(unsigned long*)&hash[i * 8]);
				//putchar('\n');
			}
			fprintf(out, "bf_size file_size exist noexist\t%x\t%s\t%d\t%d\n", 
				bf.bloom_filter_size, file_name[j], bloom_filter_exist, bloom_filter_noexist);
		}
		free(bf.bloom_filter);
	}

	return 0;
}