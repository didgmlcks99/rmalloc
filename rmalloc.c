#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 

rm_header rm_free_list = { 0x0, 0 } ;
rm_header rm_used_list = { 0x0, 0 } ;

// initiats policy to default First Fit policy
rm_option policy = FirstFit;

void * rmalloc (size_t s) 
{
	size_t tot_size = 0;

	// first fit policy case
	if(policy == FirstFit){
		rm_header_ptr free = &rm_free_list;
		// search through free list
		while(free->next != 0x0){
			tot_size += free->next->size;
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
			// when found the first chunk the fits user's request
			else if(s <= tot_size){
				rm_header_ptr free2 = &rm_free_list;
				size_t coal_size = free2->size;
				// minimum coalescing happens
				while(coal_size != tot_size){
					// increment size for coalescing
					coal_size += free2->next->size;
					// bring first node to temp
					rm_header_ptr temp2 = free2->next;
					// connect current node to the node after target node
					free2->next = free2->next->next;
					// deallocate temp
					munmap(temp2, (temp2->size + sizeof(rm_header)));
				}

				// when the coalesced space has a remaining space
				if(s != tot_size){
					// allocate for second part of memory after split and connect it back to the free list
					rm_header_ptr split = mmap(NULL, ((coal_size-s) + sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
					split->next = free2->next;
					split->size = coal_size - s;
					free2->next = split;
				}

				// send code to allocation and connection to used list at user request amount
				break;
			}
			free = free->next;
		}
		
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
	// best fit policy case
	else if(policy == BestFit){
		size_t min_fit = 1000000;
		rm_header_ptr points_min_chunk;

		rm_header_ptr free = &rm_free_list;
		// walk through free list to find best fit
		while(free->next != 0x0){
			// total size is increment as walking
			tot_size += free->next->size;
			// when it meets a node that is big enough for user requeset and remaining space smaller than min
			if((int)(free->next->size - s) >= 0 && (free->next->size - s) < min_fit){
				min_fit = (free->next->size - s);
				points_min_chunk = free;
				// when there are not remaining space at all
				if(min_fit == 0){
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
			}
			free = free->next;
		}
		
		//when there has been found a best fit memory
		if(min_fit != 1000000){
			// bring target node to temp
			rm_header_ptr temp = points_min_chunk->next;
			// current node pointer changes to the node after the target node
			points_min_chunk->next = points_min_chunk->next->next;
			// deallocate target node
			munmap(temp, (temp->size) + sizeof(rm_header));

			// allocate for second part of memory after split and connect it back to the free list
			rm_header_ptr split = mmap(NULL, ((min_fit) + sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
			split->next = points_min_chunk->next;
			split->size = min_fit;
			points_min_chunk->next = split;
		}
		// when best fit was not found coalescing happens (in possible cases)
		else if(s <= tot_size){
			rm_header_ptr free2 = &rm_free_list;
			size_t coal_size = free2->size;
			// minimum coalescing happens
			while(coal_size < s){
				// increment size for coalescing
				coal_size += free2->next->size;
				// bring first node to temp
				rm_header_ptr temp2 = free2->next;
				// connect current node to the node after target node
				free2->next = free2->next->next;
				// deallocate temp
				munmap(temp2, (temp2->size + sizeof(rm_header)));
			}
			
			// when the coalesced space has a remaining space
			if(s != tot_size){
				// allocate for second part of memory after split and connect back to the free list
				rm_header_ptr split = mmap(NULL, ((coal_size-s) + sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
				split->next = free2->next;
				split->size = coal_size - s;
				free2->next = split;
			}
		}

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
	// worst fit policy case
	else if(policy == WorstFit){
		size_t max_fit = 0;
		rm_header_ptr points_max_chunk;
		
		// walk through free list to find memory space with the largest remainging space with user request
		rm_header_ptr free = &rm_free_list;
		while(free->next != 0x0){
			tot_size += free->next->size;
			// when the current memory space is feasible for user request and the remainging space is larger than initial max fit 
			if((int)(free->next->size - s) > 0 && (free->next->size - s) > max_fit){
				max_fit = (free->next->size - s);
				points_max_chunk = free;
			}
			free = free->next;
		}

		// when there has been found a feasible memory space with the largest remaining space
		if(max_fit != 0){
			// bring target node to temp
			rm_header_ptr temp = points_max_chunk->next;
			// current node pointer changes to the node after the target node
			points_max_chunk->next = points_max_chunk->next->next;
			// deallocate target node
			munmap(temp, (temp->size) + sizeof(rm_header));

			// allocate for second part of memory after split connect it back to the free list
			rm_header_ptr split = mmap(NULL, ((max_fit) + sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
			split->next = points_max_chunk->next;
			split->size = max_fit;
			points_max_chunk->next = split;
		}
		// when there was no memory space feasible for user request -> coalescing happens (in possible cases)
		else if(s <= tot_size){
			rm_header_ptr free2 = &rm_free_list;
			size_t coal_size = free2->size;
			// maximum coalescing happens
			while(free2->next != 0x0){
				// increment size for coalescing
				coal_size += free2->next->size;
				// bring first node to temp
				rm_header_ptr temp2 = free2->next;
				// connect current node to the node after target node
				free2->next = free2->next->next;
				// deallocate temp
				munmap(temp2, (temp2->size + sizeof(rm_header)));
			}
			
			// when the coalesced space has a remaining space
			if(s != tot_size){
				// allocate for second part of memory after split and connect it back to the free list
				rm_header_ptr split = mmap(NULL, ((coal_size-s) + sizeof(rm_header)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
				split->next = free2->next;
				split->size = coal_size - s;
				free2->next = split;
			}
		}

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

			rm_header_ptr free = &rm_free_list;
			// walks through free list to meet the end place
			while(free->next != 0x0){
				free = free->next;
			}

			// connect target memory space to the end of free list
			temp->next = 0x0;
			free->next = temp;
			return ;
		}
		used = used->next;
	}
	printf("[ERROR] Segmentation Fault > accessing inaccessible memory.\n");
}

void * rrealloc (void * p, size_t s) 
{
	// TODO
	return 0x0 ; // erase this 
}

void rmshrink () 
{
	rm_header_ptr free = &rm_free_list;
	while(free->next != 0x0){
		rm_header_ptr temp = free->next;
		free->next = free->next->next;
		munmap(temp, (temp->size + sizeof(rm_header)));
	}
}

void rmconfig (rm_option opt) 
{
	// make global variable policy to user request option
	policy = opt;
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
