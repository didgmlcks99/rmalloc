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
			// when found the first chunk that exactly fits user's request
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
				return (void *)temp + sizeof(rm_header);
			}
			// when found the first chunk the fits user's request with remaining space
			else if(s <= free->next->size){
				//allocate remaining space after split
				rm_header_ptr split;
				split = mmap(NULL, ((free->next->size - s)+sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
				// split points to the intial address
				split->next = free->next->next;
				// split size is decreased by user's reqest
				split->size = (free->next->size - s);
				// remove initial address
				munmap(free->next, (free->next->size + sizeof(rm_header)));
				// node pointed to initial address now points to remaining space
				free->next = split;

				// send code to allocation and connection to used list at user request amount
				break;
			}
			free = free->next;
		}
		
		// rm_header_ptr test = &rm_used_list;
		// while(test->next != 0x0){
		// 	printf("%p\n", (test->next) - 1);
		// 	test = test->next;
		// 	// rmprint();
		// }
		// when there are no feasible memory region for user's request
		rm_header_ptr used = &rm_used_list;
		// walks through used list to meet the end place
		while(used->next != 0x0){
			used = used->next;
		}
		
		// add new allocated header to the end of used list
		rm_header_ptr new;
		new = mmap(NULL, (s+sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		new->next = 0x0;
		new->size = s;
		used->next = new;
		return (void *)new + sizeof(rm_header);
	}
	return 0x0;
}

void rfree (void * p) 
{
	rm_header_ptr used = &rm_used_list;
	// searches through used list
	while(used->next != 0x0){
		// when used list meets the target address
		if(used->next == (void *)(p - sizeof(rm_header))){
			// bring target data to temporary
			rm_header_ptr temp = used->next;
			// connect used list to the next node after target node
			used->next = used->next->next;
			
			size_t coal_size = temp->size;
			// deallocate temp
			munmap(temp, (temp->size + sizeof(rm_header)));
			rm_header_ptr free = &rm_free_list;
			// walks through free list to meet the end place
			while(free->next != 0x0){
				// increment size for coalescing
				coal_size += free->next->size;
				// bring first node to temp
				rm_header_ptr temp2 = free->next;
				// connect current node to the node after target node
				free->next = free->next->next;
				// deallocate temp
				munmap(temp2, (temp2->size + sizeof(rm_header)));
			}

			rm_header_ptr coal = mmap(NULL, (coal_size + sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
			// add coalesced address at the end of free list
			coal->next = 0x0;
			coal->size = coal_size;
			free->next = coal;

			break;
		}
		used = used->next;
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
