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
			// found the first chunk the exactly fits user's request
			if(s == free->next->size){
				// bring target data to temporary
				rm_header_ptr temp = free->next;
				// connect free list to the next node after target node
				free->next = free->next->next;
				rm_header_ptr used = &rm_used_list;
				// walks through used list to meet the end place
				while(used->next != 0x0){
					used = used->next;
				}
				// add target address at the end of used list
				temp->next = 0x0;
				used->next = temp;
				return temp;
			}
			// found the first chunk the fits user's request with remaining space
			else if(s <= free->next->size){
				return 0x0;
			}
			free = free->next;
		}
		rm_header_ptr used = &rm_used_list;
		// walks through used list to meet the end place
		while(used->next != 0x0){
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
	rm_header_ptr used = &rm_used_list;
	// searches through used list
	while(used->next != 0x0){
		// when used list meets the target address
		if(used->next == p){
			// bring target data to temporary
			rm_header_ptr temp = used->next;
			// connect used list to the next node after target node
			used->next = used->next->next;
			rm_header_ptr free = &rm_free_list;
			// walks through free list to meet the end place
			while(free->next != 0x0){
				// brings used to the last place of user list
				free = free->next;
			}
			// add target address at the end of free list
			temp->next = 0x0;
			free->next = temp;
			break;
		}
		used = used->next;
	}
	// case where there has been no equall address in the used list
	if(used->next == 0x0){
		printf("Your process don't have such address allocated.\n");
		printf("No free has happend.");
	}
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
