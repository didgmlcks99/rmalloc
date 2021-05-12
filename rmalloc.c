#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 

rm_header rm_free_list = { 0x0, 0 } ;
rm_header rm_used_list = { 0x0, 0 } ;

rm_option policy = FirstFit;

void * rmalloc (size_t s) 
{
	// first fit policy case
	if(policy == FirstFit){
		rm_header_ptr free = &rm_free_list;
		// search through free list
		while(free->next != 0x0){
			// found the first chunk the fits user's request
			if(s <= free->size){
				
				return 0x0;
			}
			free = free->next;
		}
		rm_header_ptr used = &rm_used_list;
		// search through used list, when there is no chunk in the free list that fits user's request
		while(used->next != 0x0){
			// brings used to the last place of user list
			used = used->next;
		}
		// add new allocated header to the end of used list
		rm_header * new;
		new = mmap(NULL, (s+sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		new->next = 0x0;
		new->size = s;
		used->next = new;
		return new;
	}
	return 0x0;
}

void rfree (void * p) 
{
	// TODO 
}

void * rrealloc (void * p, size_t s) 
{
	// TODO
	return 0x0 ; // erase this 
}

void rmshrink () 
{
	// TODO
}

void rmconfig (rm_option opt) 
{
	// TODO
}


void 
rmprint () 
{
	rm_header_ptr itr ;
	int i ;

	printf("==================== rm_free_list ====================\n") ;
	for (itr = rm_free_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

	printf("==================== rm_used_list ====================\n") ;
	for (itr = rm_used_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}
