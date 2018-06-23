#include <stdio.h>
#include <string.h>

#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"

#include "projects/memalloc/memalloctest.h"

void run_memalloc_test(char **argv UNUSED)
{
	if(pallocator == 0)
	{
		size_t i;
		char* dynamicmem[11];

		for (i=0; i<10; i++) {
			dynamicmem[i] = (char *) malloc (145000);
			memset (dynamicmem[i], 0x00, 145000);
		}
		
		dynamicmem[10] = (char *) malloc (16000);
		memset (dynamicmem[10], 0x00, 16000);
		printf ("dump page status : \n");
		palloc_get_status (0);

		thread_sleep (100);

		for (i=0; i<11; i++) {
			free(dynamicmem[i]);
			printf ("dump page status : \n");
			palloc_get_status (0);
		}
	}
	
	if(pallocator == 1)
	{
		size_t i;
		char* dynamicmem[11];
		
		for (i=0; i<5; i++){
			dynamicmem[i] = (char *) malloc (145000);
			memset (dynamicmem[i], 0x00, 145000);
			printf ("dump page status : \n");
			palloc_get_status(0);
		}
		
		thread_sleep (100);
		
		for (i=0; i<5; i++){
			free(dynamicmem[i]);
			printf ("dump page status : \n");
			palloc_get_status(0);
		}
		
		for (i=7; i<10; i++){
			dynamicmem[i] = (char *) malloc (145000);
			memset (dynamicmem[i], 0x00, 145000);
			printf ("dump page status : \n");
			palloc_get_status(0);
		}
		
		for (i=7; i<10; i++){
			free(dynamicmem[i]);
			printf ("dump page status : \n");
			palloc_get_status(0);
		}

	}
	
	if(pallocator == 2)
	{
		size_t i;
		char* dynamicmem[11];

		
		
		dynamicmem[0] = (char *) malloc (145000);
		memset (dynamicmem[0], 0x00, 145000);
		
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[1] = (char *) malloc (145000);
		memset (dynamicmem[1], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[2] = (char *) malloc (145000);
		memset (dynamicmem[2], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[3] = (char *) malloc (90000);
		memset (dynamicmem[3], 0x00, 90000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		
		dynamicmem[4] = (char *) malloc (145000);
		memset (dynamicmem[4], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[5] = (char *) malloc (145000);
		memset (dynamicmem[5], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[6] = (char *) malloc (70000);
		memset (dynamicmem[6], 0x00, 70000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[7] = (char *) malloc (145000);
		memset (dynamicmem[7], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[8] = (char *) malloc (65000);
		memset (dynamicmem[8], 0x00, 65000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[9] = (char *) malloc (145000);
		memset (dynamicmem[9], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		free(dynamicmem[3]);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		
		free(dynamicmem[6]);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		free(dynamicmem[8]);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		
		dynamicmem[10] = (char *) malloc (60000);
		memset (dynamicmem[10], 0x00, 60000);
		
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		thread_sleep (100);

		for (i=0; i<11; i++) {
			if(i != 3 && i != 6 && i != 8)
				free(dynamicmem[i]);
			printf ("dump page status : \n");
			palloc_get_status (0);
		}
		
	}

	if(pallocator == 3)
	{
		size_t i;
		char* dynamicmem[4];

		
		
		dynamicmem[0] = (char *) malloc (145000);
		memset (dynamicmem[0], 0x00, 145000);
		
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[1] = (char *) malloc (45000);
		memset (dynamicmem[1], 0x00, 45000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[2] = (char *) malloc (145000);
		memset (dynamicmem[2], 0x00, 145000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
		dynamicmem[3] = (char *) malloc (90000);
		memset (dynamicmem[3], 0x00, 90000);
		printf ("dump page status : \n");
		palloc_get_status (0);
		
	

		for (i=0; i<4; i++) {
				free(dynamicmem[i]);
			//printf ("dump page status : \n");
			//palloc_get_status (0);
		}
		
	}
	
	
	
}