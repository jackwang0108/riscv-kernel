/**
 * @file page.c
 * @author Jack Wang
 * @brief A simple physical memory management. 
 * 		  There will be no virtual memory management, we simple allocate physical pages. 
 * 		  Currently, there's no lock, so the operating of allocatable page flag array is dangerous
 * @version 0.1
 * @date 2023-04-02
 * 
 * @copyright Copyright (c) 2023
 * 
*/
#include "os.h"

/**
 * @brief Physical Addressing Layout
 * 
 *		Address
 * 		 0x0000-0000		,-----------------------------------,
 * 			   .			|									|
 * 			   .			|		    ROM Mapping Area  		|
 * 			   .			|									|
 * 		 0x8000-0000		|-----------------------------------|	<--- MEMORY_START,	TEXT_START
 * 							|			    .text				|
 * 			  M 			|-----------------------------------|	<--- TEXT_END, 		RODATA_START
 * 			  e 			|				.rodata				|
 * 			  m				|-----------------------------------|	<--- RODATA_END, 	DATA_START
 * 			  o				|				.data				|
 * 			  r				|-----------------------------------|	<--- DATA_END, 		BSS_START
 * 			  y				|				.bss				|
 * 			  				|-----------------------------------|	<--- BSS_END,		HEAP_START
 * 			  M 			|									|
 * 			  a 			|									|
 * 			  p				|									|
 * 			  p				|									|
 * 			  i				|									|
 * 			  n				|									|
 * 			  g				|				Heap				|
 * 			   				|									|
 * 			  A				|									|
 * 			  r				|									|
 * 			  e				|									|
 * 			  a				|									|
 * 							|									|
 * 		0x8800-0000			|-----------------------------------|	<---- HEAP_END,		MEMORY_END
 * 							|									|
 * 							|									|
 * 							|	  Other Device Mapping Area		|
 * 							|									|
 * 							|									|
 * 							`-----------------------------------'
*/

/*
 * Following global vars are defined in mem.S
 */
extern uint32_t TEXT_START;
extern uint32_t TEXT_END;
extern uint32_t DATA_START;
extern uint32_t DATA_END;
extern uint32_t RODATA_START;
extern uint32_t RODATA_END;
extern uint32_t BSS_START;
extern uint32_t BSS_END;
extern uint32_t HEAP_START;
extern uint32_t HEAP_SIZE;


/**
 * @brief Heap Layout 
 * 
 * 				Page ID	
 * 	MemStart	0			,===================================,
 * 							|		   Page Discriptor 0		|	----------------------------------------,
 * 							|-----------------------------------|											|
 * 							|		   Page Discriptor 1		|	------------------------------------,	|
 * 							|-----------------------------------|										|	|
 * 							|		   Page Discriptor 2		|	--------------------------------,	|	|
 * 							|-----------------------------------|									|	|	|
 * 							|				  . 				|									|	|	|
 * 							|				  . 				|				 					|	|	|
 * 							|				  . 				|				 					|	|	|
 * 							|-----------------------------------|				 					|	|	|
 * 							|		   Page Discriptor N		|				 					|	|	|
 * 				1			|===================================|				.					|	|	|
 * 							|		   Page Discriptor N + 1	|				.					|	|	|
 * 							|-----------------------------------|				.					|	|	|
 * 							|		   Page Discriptor N + 2	|				.					|	|	|
 * 							|-----------------------------------|				.					|	|	|
 * 							|				  . 				|				.					|	|	|
 * 							|				  . 				|				.					|	|	|
 * 							|				  . 				|				 					|	|	|
 * 							|-----------------------------------|				 					|	|	|
 * 							|		   Page Discriptor 2N		|				 					|	|	|
 * 				2			|===================================|				 					|	|	|
 * 							|				  . 				|				 					|	|	|
 * 							|				  . 				|				 					|	|	|
 * 				. 			|				  . 				|				 					|	|	|
 * 				.   		|				  . 				|				 					|	|	|
 * 				.   		|				  . 				|				 					|	|	|
 * 				    		|				  . 				|		_alloc_start		 		|	|	|
 * 				7   		|===================================| 	<---------------------------------------'
 * 							|	 First Page will be allocated	|									|	|
 * 							|									|									|	|
 * 				8   		|===================================|	<-----------------------------------'
 * 							|	 Second Page will be allocated	|									|
 * 							|									|									|
 * 				9   		|===================================|	<-------------------------------'
 * 							|	 Third Page will be allocated	|
 * 							|									|
 * 							|									|
 * 				10  		|===================================|
 * 							|				 .  				|
 * 				.   		|				 .  				|
 * 				.   		|				 .  				|
 * 				.   		|				 .  				|
 * 							|									|
 * 	MemEnd 		N   		'==================================='	<---- _alloc_end
 *	
 *
*/

/*
 * _alloc_start points to the actual start address of heap pool
 * _alloc_end points to the actual end address of heap pool
 * _num_pages holds the actual max number of pages we can allocate.
 */
static uint32_t _alloc_start = 0;
static uint32_t _alloc_end = 0;
static uint32_t _num_pages = 0;

#define PAGE_SIZE 4096
#define PAGE_ORDER 12

#define PAGE_TAKEN (uint8_t)(1 << 0)
#define PAGE_LAST  (uint8_t)(1 << 1)

/*
 * Page Descriptor 
 *		A descriptor is a byte, which describe a page. We will take first 8 pages to store descriptors.
 *		So, there will be 8 * 4096 = 32768 pages, which manage 32768 * 4K = 128 MB memory.
 * flags:
 * 		- bit 0: flag if this page is taken(allocated)
 * 		- bit 1: flag if this page is the last page of the memory block allocated
 * 		- bit 2-7: reserved
 */
struct Page {
	uint8_t flags;
};

static inline void _clear(struct Page *page)
{
	page->flags = 0;
}

static inline int _is_free(struct Page *page)
{
	return page->flags & PAGE_TAKEN ? 0 : 1;
}

static inline void _set_flag(struct Page *page, uint8_t flags)
{
	page->flags |= flags;
}

static inline int _is_last(struct Page *page)
{
	return page->flags & PAGE_LAST ? 1 : 0;
}

/*
 * align the address to the border of page(4K)
 */
static inline uint32_t _align_page(uint32_t address)
{
	// order = 4095, 0x0FFF, 0b0000_1111_1111_1111
	uint32_t order = (1 << PAGE_ORDER) - 1;
	return (address + order) & (~order);
}

void page_init()
{
	/* 
	 * We reserved 8 Page (8 x 4096) to hold the Page structures.
	 * It should be enough to manage at most 128 MB (8 x 4096 x 4096) 
	 */
	_num_pages = (HEAP_SIZE / PAGE_SIZE) - 8;
	printf("HEAP_START = %x, HEAP_SIZE = %x, num of pages = %d\n", HEAP_START, HEAP_SIZE, _num_pages);
	
	struct Page *page = (struct Page *)HEAP_START;
	for (int i = 0; i < _num_pages; i++) {
		_clear(page);
		page++;	
	}

	_alloc_start = _align_page(HEAP_START + 8 * PAGE_SIZE);
	_alloc_end = _alloc_start + (PAGE_SIZE * _num_pages);

	printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
	printf("HEAP:   0x%x -> 0x%x\n", _alloc_start, _alloc_end);
}

/*
 * Allocate a memory block which is composed of contiguous physical pages
 * - npages: the number of PAGE_SIZE pages to allocate
 */
void *page_alloc(int npages)
{
	/* Note we are searching the page descriptor bitmaps. */
	int found = 0;
	struct Page *page_i = (struct Page *)HEAP_START;
	for (int i = 0; i <= (_num_pages - npages); i++) {
		if (_is_free(page_i)) {
			found = 1;
			/* 
			 * meet a free page, continue to check if following
			 * (npages - 1) pages are also unallocated.
			 */
			struct Page *page_j = page_i + 1;
			for (int j = i + 1; j < (i + npages); j++) {
				if (!_is_free(page_j)) {
					found = 0;
					break;
				}
				page_j++;
			}
			/*
			 * get a memory block which is good enough for us,
			 * take housekeeping, then return the actual start
			 * address of the first page of this memory block
			 */
			if (found) {
				struct Page *page_k = page_i;
				for (int k = i; k < (i + npages); k++) {
					_set_flag(page_k, PAGE_TAKEN);
					page_k++;
				}
				page_k--;
				_set_flag(page_k, PAGE_LAST);
				return (void *)(_alloc_start + i * PAGE_SIZE);
			}
		}
		page_i++;
	}
	return NULL;
}

/*
 * Free the memory block
 * - p: start address of the memory block
 */
void page_free(void *p)
{
	/*
	 * Assert (TBD) if p is invalid
	 */
	if (!p || (uint32_t)p >= _alloc_end) {
		return;
	}
	/* get the first page descriptor of this memory block */
	struct Page *page = (struct Page *)HEAP_START;
	page += ((uint32_t)p - _alloc_start)/ PAGE_SIZE;
	/* loop and clear all the page descriptors of the memory block */
	while (!_is_free(page)) {
		if (_is_last(page)) {
			_clear(page);
			break;
		} else {
			_clear(page);
			page++;;
		}
	}
}

void page_test()
{
	void *p = page_alloc(2);
	printf("p = 0x%x\n", p);
	//page_free(p);

	void *p2 = page_alloc(7);
	printf("p2 = 0x%x\n", p2);
	page_free(p2);

	void *p3 = page_alloc(4);
	printf("p3 = 0x%x\n", p3);
}
