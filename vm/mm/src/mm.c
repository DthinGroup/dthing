/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/18 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: mm.c#1
 */

/**
 * The file provided the basic memory management functions.
 */
#include <mm.h>
#include <opl_mm.h>

/* ------------------- size_t and alignment properties -------------------- */
/* The byte and bit size of a uint32_t */
#define UINT32_SIZE         (sizeof(uint32_t))
#define UINT32_BITSIZE      (sizeof(uint32_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define UINT32_ZERO         ((uint32_t)0)
#define UINT32_ONE          ((uint32_t)1)
#define UINT32_TWO          ((uint32_t)2)
#define UINT32_FOUR         ((uint32_t)4)

//Memory alignment, default should be 8 bytes.
#define MEM_ALIGNMENT ((int32_t)(2 * sizeof(void *)))
//Align up the data.
#define ALIGN_UP(x, n) ((int32_t)((x)+(n)-1)&(~((n)-1)))

#define CHUNK_OVERHEAD  (2 * sizeof(uint32_t))
#define pad_request2size(n) \
			ALIGN_UP((n + CHUNK_OVERHEAD), MEM_ALIGNMENT)

//memory global struture aligned size
#define MG_SIZE_ALIGNED   ALIGN_UP(sizeof(mpglobal), MEM_ALIGNMENT)

#define SMALLBIN_SHIFT    (3U)
#define TREEBIN_SHIFT     (8U)
#define MAX_SMALL_PAD_SIZE (1 << TREEBIN_SHIFT)

#define sbins_size2index(s) \
			(uint32_t)((s)  >> SMALLBIN_SHIFT)
#define sbins_index2size(i) \
			(uint32_t)((i)  << SMALLBIN_SHIFT)

#define MIN_CHUNK_SIZE ((uint32_t)sizeof(mchunk))

#define chunk2mem(p)        ((void*)((char*)(p)       + CHUNK_OVERHEAD))
#define mem2chunk(mem)      ((mchunkptr)((char*)(mem) - CHUNK_OVERHEAD))



/* ------------------ Operations on head fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  GCMARK_BIT is used for GC to mark object is non-reference.
*/
#define PINUSE_BIT          ((binmap_t)(1) << 0)
#define CINUSE_BIT          ((binmap_t)(1) << 1)
#define GCMARK_BIT          ((binmap_t)(1) << 2)
#define INUSE_BITS          (PINUSE_BIT|CINUSE_BIT)
#define FLAG_BITS           (PINUSE_BIT|CINUSE_BIT|GCMARK_BIT)

/* extraction of fields from head words */
#define cinuse(p)           ((p)->head & CINUSE_BIT)
#define pinuse(p)           ((p)->head & PINUSE_BIT)
#define is_gcmarked(p)      ((p)->head & GCMARK_BIT)
#define is_inuse(p)         (((p)->head & INUSE_BITS) != PINUSE_BIT)

#define chunksize(p)        ((p)->head & ~(FLAG_BITS))

#define clear_pinuse(p)     ((p)->head &= ~PINUSE_BIT)
#define set_gcmarkbit(p)    ((p)->head |= GCMARK_BIT)
#define clear_gcmarkbit(p)  ((p)->head &= ~GCMARK_BIT)

/* Treat space at ptr +/- offset as a chunk */
#define chunk_plus_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)(((char*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->head & ~FLAG_BITS)))
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#define next_pinuse(p)  ((next_chunk(p)->head) & PINUSE_BIT)

/* Get/set size at footer */
#define get_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot)
#define set_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#define set_size_and_pinuse_of_free_chunk(p, s)\
  ((p)->head = (s|PINUSE_BIT), set_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#define set_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

/* Set cinuse bit and pinuse bit of next chunk */
#define set_inuse(p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#define set_inuse_and_pinuse(p,s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#define set_size_and_pinuse_of_inuse_chunk(p, s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT))


/* ---------------------------- Indexing Bins ---------------------------- */

#define is_small(s)         (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)      (bindex_t)((s)  >> SMALLBIN_SHIFT)
#define small_index2size(i) ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX     (small_index(MIN_CHUNK_SIZE))

/* addressing by index. See above about smallbin repositioning */
#define smallbin_at(M, i)   ((sbinptr)((char*)&((M)->sbins[(i)<<1])))
#define treebin_at(M,i)     (&((M)->tbins[i]))

/* assign tree index for size S to variable I. Use asm if possible  */
#define compute_tree_index(S, I)\
do {\
	uint32_t X = S >> TREEBIN_SHIFT;\
	if (X == 0) {\
		I = 0;\
	} \
	else if (X > 0xFFFF) {\
		I = NTREEBINS-1;\
	}\
	else {\
		uint32_t Y = (uint32_t)X;\
		uint32_t N = ((Y - 0x100) >> 16) & 8;\
		uint32_t K = (((Y <<= N) - 0x1000) >> 16) & 4;\
		N += K;\
		N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
		K = 14 - N + ((Y <<= K) >> 15);\
		I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));\
	}\
} while(0)


/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_tree_index(i) \
   (i == NTREEBINS-1)? (UINT32_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_tree_index(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((UINT32_BITSIZE-UINT32_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i) \
   ((UINT32_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
   (((size_t)((i) & UINT32_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))

/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#define index2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->sbinsmap |=  index2bit(i))
#define clear_smallmap(M,i)     ((M)->sbinsmap &= ~index2bit(i))
#define smallmap_is_marked(M,i) ((M)->sbinsmap &   index2bit(i))

#define mark_treemap(M,i)       ((M)->tbinsmap |=  index2bit(i))
#define clear_treemap(M,i)      ((M)->tbinsmap &= ~index2bit(i))
#define treemap_is_marked(M,i)  ((M)->tbinsmap &   index2bit(i))

/* isolate the least set bit of a bitmap */
#define least_bit(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#define same_or_left_bits(x) ((x) | -(x))

/* A little helper macro for trees */
#define leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#define insert_small_chunk(M, P, S) \
do {\
	uint32_t  I = sbins_size2index(S);\
	mchunkptr B = smallbin_at(M, I);\
	mchunkptr F = B;\
	if (!smallmap_is_marked(M, I)) {\
		mark_smallmap(M, I);\
	} \
	else {\
		F = B->fd;\
	}\
	B->fd = P;\
	F->bk = P;\
	P->fd = F;\
	P->bk = B;\
} while (0)

/* Unlink a chunk from a smallbin  */
#define unlink_small_chunk(M, P, S) \
do {\
	mchunkptr F = P->fd;\
	mchunkptr B = P->bk;\
	uint32_t  I = sbins_size2index(S);\
	if ((F == smallbin_at(M, I)) || (F->bk == P)) { \
		if (B == F) {\
			clear_smallmap(M, I);\
		}\
		else if ((B == smallbin_at(M,I)) || (B->fd == P)) {\
			F->bk = B;\
			B->fd = F;\
		}\
	}\
} while (0)

/* Unlink the first chunk from a smallbin */
#define unlink_first_small_chunk(M, B, P, I) \
do {\
	mchunkptr F = P->fd;\
	if (B == F) {\
		clear_smallmap(M, I);\
	}\
	else if (F->bk == P) {\
		F->bk = B;\
		B->fd = F;\
	}\
} while (0)

/* index corresponding to given bit. Use asm if possible */
#define compute_bit2idx(X, I)\
do {\
	uint32_t Y = X - 1;\
	uint32_t K = Y >> (16-4) & 16;\
	uint32_t N = K;            Y >>= K;\
	N += K = Y >> (8-3) &  8;  Y >>= K;\
	N += K = Y >> (4-2) &  4;  Y >>= K;\
	N += K = Y >> (2-1) &  2;  Y >>= K;\
	N += K = Y >> (1-0) &  1;  Y >>= K;\
	I = (uint32_t)(N + Y);\
} while(0)


/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#define insert_large_chunk(M, X, S) \
do {\
	tbinptr * H;\
	uint32_t  I;\
	compute_tree_index(S, I);\
	H = treebin_at(M, I);\
	X->index = I;\
	X->child[0] = X->child[1] = 0;\
	if (!treemap_is_marked(M, I)) {\
		mark_treemap(M, I);\
		*H = X;\
		X->parent = (tchunkptr)H;\
		X->fd = X->bk = X;\
	}\
	else {\
		tchunkptr T = *H;\
		uint32_t K = S << leftshift_for_tree_index(I);\
		for (;;) {\
			if (chunksize(T) != S) {\
				tchunkptr* C = &(T->child[(K >> (UINT32_BITSIZE-UINT32_ONE)) & 1]);\
				K <<= 1;\
				if (*C != 0) {\
					T = *C;\
				}\
				else {\
					*C = X;\
					X->parent = T;\
					X->fd = X->bk = X;\
					break;\
				}\
			}\
			else {\
				tchunkptr F = T->fd;\
				T->fd = F->bk = X;\
				X->fd = F;\
				X->bk = T;\
				X->parent = 0;\
				break;\
			}\
		}\
	}\
} while (0)

/*
  Unlink steps:

  1. If x is a chained node, unlink it from its same-sized fd/bk links
     and choose its bk node as its replacement.
  2. If x was the last node of its size, but not a leaf node, it must
     be replaced with a leaf node (not merely one with an open left or
     right), to make sure that lefts and rights of descendents
     correspond properly to bit masks.  We use the rightmost descendent
     of x.  We could use any other leaf, but this is easy to locate and
     tends to counteract removal of leftmosts elsewhere, and so keeps
     paths shorter than minimally guaranteed.  This doesn't loop much
     because on average a node in a tree is near the bottom.
  3. If x is the base of a chain (i.e., has parent links) relink
     x's parent and children to x's replacement (or null if none).
*/

#define unlink_large_chunk(M, X)\
do {\
	tchunkptr XP = X->parent;\
	tchunkptr R;\
	if (X->bk != X) {\
		tchunkptr F = X->fd;\
		R = X->bk;\
		if (F->bk == X && R->fd == X) {\
			F->bk = R;\
			R->fd = F;\
		}\
	}\
	else {\
		tchunkptr* RP;\
		if (((R = *(RP = &(X->child[1]))) != 0) ||\
			((R = *(RP = &(X->child[0]))) != 0)) {\
			tchunkptr* CP;\
			while ((*(CP = &(R->child[1])) != 0) ||\
				(*(CP = &(R->child[0])) != 0)) {\
				R = *(RP = CP);\
			}\
			*RP = 0;\
		}\
	}\
	if (XP != 0) {\
		tbinptr* H = treebin_at(M, X->index);\
		if (X == *H) {\
			if ((*H = R) == 0) \
				clear_treemap(M, X->index);\
		}\
		else {\
			if (XP->child[0] == X) \
				XP->child[0] = R;\
			else \
				XP->child[1] = R;\
		}\
		if (R != 0) {\
			tchunkptr C0, C1;\
			R->parent = XP;\
			if ((C0 = X->child[0]) != 0) {\
				R->child[0] = C0;\
				C0->parent = R;\
			}\
			if ((C1 = X->child[1]) != 0) {\
				R->child[1] = C1;\
				C1->parent = R;\
			}\
		}\
	}\
} while(0)

/* Relays to large vs small bin operations */

#define insert_chunk(M, P, S)\
do {\
	if (is_small(S)) {\
		insert_small_chunk(M, P, S);\
	}\
	else {\
		tchunkptr TP = (tchunkptr)(P);\
		insert_large_chunk(M, TP, S);\
	}\
} while(0)

#define unlink_chunk(M, P, S)\
do {\
	if (is_small(S)) {\
		unlink_small_chunk(M, P, S);\
	}\
	else {\
		tchunkptr TP = (tchunkptr)(P);\
		unlink_large_chunk(M, TP);\
	}\
} while(0)




//memory pool global 
PUBLIC mpglobal * mg;


/**
 * Initialized memory pool global structures.
 * @ _mg the allocated memory pool from platform.
 */
LOCAL mpglobal * initialMemoryPoolGlobal(void ** mpool, int32_t size)
{
	mpglobal * _mg;
	uint32_t   align_size;
	
	//ensure it's 8 bytes alignment.
	align_size = size & (~0x7);
	_mg = (mpglobal *)*mpool;

	CRTL_memset(_mg, 0x0, sizeof(mpglobal));
	_mg->mem_start  = ((char *)_mg) + MG_SIZE_ALIGNED;
	_mg->mem_end    = _mg->mem_start;
	_mg->perm_start = ((char *)_mg) + align_size;
	_mg->perm_end   = _mg->perm_start;

	_mg->free_size  = ((char *)_mg->mem_end) - ((char *)_mg->perm_start);

	return _mg;
}

/* allocate a small request from the best fitting chunk in a treebin */
LOCAL void * tmalloc_small(mpglobal * m, uint32_t nbytes)
{
	tchunkptr t, v, r;
	uint32_t  rsize;
	uint32_t  i;
	binmap_t  leastbit = least_bit(m->tbinsmap);

	compute_bit2idx(leastbit, i);
	v = t = *treebin_at(m, i);
	rsize = chunksize(t) - nbytes;

	while ((t = leftmost_child(t)) != 0)
	{
	    uint32_t trem = chunksize(t) - nbytes;
		if (trem < rsize)
		{
			rsize = trem;
			v = t;
	    }
	}

	r = chunk_plus_offset(v, nbytes);

	unlink_large_chunk(m, v);

	if (rsize < MIN_CHUNK_SIZE)
	{
		set_inuse_and_pinuse(v, (rsize + nbytes));
	}
	else
	{
		set_size_and_pinuse_of_inuse_chunk(v, nbytes);
		set_size_and_pinuse_of_free_chunk(r, rsize);
		insert_chunk(m, r, rsize);
	}
	return chunk2mem(v);

}

/**
 * Malloc small memory from top memory area.
 */
LOCAL void * top_malloc(mpglobal * m, uint32_t nbytes)
{
	mchunkptr p;
	
	if (m->free_size < nbytes) 
	{
		//not enough memory in current pool. 
		return NULL;
	}

	p = (mchunkptr)m->mem_end;
	set_size_and_pinuse_of_inuse_chunk(p, nbytes);
	m->mem_end = (void *)chunk_plus_offset((mchunkptr)p, nbytes);
	
	return chunk2mem(p);
}

/* allocate a large request from the best fitting chunk in a treebin */
LOCAL void * tmalloc_large(mpglobal * m, uint32_t nbytes) 
{
	tchunkptr v = 0;
	tchunkptr t;
	uint32_t  rsize = -nbytes; /* Unsigned negation */
	uint32_t  idx;
	compute_tree_index(nbytes, idx);

	if ((t = *treebin_at(m, idx)) != 0) 
	{
		/* Traverse tree for this bin looking for node with size == nb */
		uint32_t sizebits = nbytes << leftshift_for_tree_index(idx);
		tchunkptr rst = 0;  /* The deepest untaken right subtree */
		for (;;)
		{
			tchunkptr rt;
			uint32_t  trem = chunksize(t) - nbytes;
			if (trem < rsize) 
			{
				v = t;
				if ((rsize = trem) == 0)
				{
					break;
				}
			}

			rt = t->child[1];
			t = t->child[(sizebits >> (UINT32_BITSIZE - UINT32_ONE)) & 1];
			if (rt != 0 && rt != t)
			{
				rst = rt;
			}

			if (t == 0) 
			{
				t = rst; /* set t to least subtree holding sizes > nb */
				break;
			}
			sizebits <<= 1;
		}
	}
	if (t == 0 && v == 0) /* set t to root of next non-empty treebin */
	{
		uint32_t leftbits = left_bits(index2bit(idx)) & m->tbinsmap;
		if (leftbits != 0) 
		{
			uint32_t i;
			uint32_t leastbit = least_bit(leftbits);
			compute_bit2idx(leastbit, i);
			t = *treebin_at(m, i);
		}
	}

	while (t != 0)  /* find smallest of tree or subtree */
	{
		uint32_t trem = chunksize(t) - nbytes;
		if (trem < rsize)
		{
			rsize = trem;
			v = t;
		}
		t = leftmost_child(t);
	}

	if (v != 0)
	{
		mchunkptr r = chunk_plus_offset(v, nbytes);
		unlink_large_chunk(m, v);
		if (rsize < MIN_CHUNK_SIZE)
		{
			set_inuse_and_pinuse(v, (rsize + nbytes));
		}
		else 
		{
			set_size_and_pinuse_of_inuse_chunk(v, nbytes);
			set_size_and_pinuse_of_free_chunk(r, rsize);
			insert_chunk(m, r, rsize);
		}
		return chunk2mem(v);
	}
	return NULL;
}

LOCAL mpglobal * getPoolGlobalByMemory(void * mem)
{
	mpglobal * _mg = mg;
	do 
	{
		if (mem > _mg->mem_start && mem < _mg->perm_end)
		{
			//find one, break;
			break;
		}
		_mg = _mg->next;
	} while(_mg);

	return _mg;
}

/**
 * memory initialization, global usage. will be called
 * by vm lifecycle.
 */
PUBLIC bool_t DVM_mm_initialize()
{
	int32_t size;
	void  * mem_pool;

	Sys_mm_getMemoryPool(&mem_pool, &size);

	if (mem_pool == NULL || size ==0 || (size < sizeof(mpglobal)))
	{
		DVMTraceErr("DVM_mm_initialize - ERROR: alloc 1st memory pool failure\n");
		return FALSE;
	}

	mg = initialMemoryPoolGlobal(&mem_pool, size);

	return TRUE;
}

PUBLIC void DVM_mm_finalize()
{
	mpglobal * _mg = mg;

	do {
		mg = mg->next;
		Sys_mm_freeMemoryPool((void *)_mg);
		_mg = mg;
	} while (mg != NULL);
	
	//mg should be null here, force to NULL again?
	mg = NULL;
}


PUBLIC void * dmalloc(uint32_t size)
{
	void     * mem = NULL;
	mpglobal * _mg = mg;
	uint32_t   align_size = pad_request2size(size);
	
	do {

		if (align_size > MAX_SMALL_PAD_SIZE)
		{
			//tree chunk process.

			//malloc from tree bins.
			if (_mg->tbinsmap != 0)
			{
				mem = tmalloc_large(_mg, align_size);
			}
			//malloc from top free memory.
			else
			{
				mem = top_malloc(_mg, align_size);
			}
		} 
		else
		{
			//small chunk process.
			uint32_t index = sbins_size2index(align_size);
			binmap_t smbits = _mg->sbinsmap >> index;
			
			//best fit small bins
			if (smbits & 3U != 0)
			{
				mchunkptr b, p;

				index += ~smbits & 1; /* Uses next bin if idx empty */
				b = smallbin_at(_mg, index);
				p = b->fd;

				unlink_first_small_chunk(_mg, b, p, index);
				set_inuse_and_pinuse(p, sbins_index2size(index));
				mem = chunk2mem(p);
			} 
			// Find other fit small chunk 
			else if (smbits != 0)
			{
				mchunkptr b, p, r;
				uint32_t  rsize;
				uint32_t  i;
				binmap_t leftbits = (smbits << index) & left_bits(index2bit(index));
				binmap_t leastbit = least_bit(leftbits);
				compute_bit2idx(leastbit, i);
				b = smallbin_at(_mg, i);
				p = b->fd;

				unlink_first_small_chunk(_mg, b, p, i);
				rsize = sbins_index2size(i) - align_size;

				/* Fit here, MIN CHUNK will saved into current allocated */
				if (rsize < MIN_CHUNK_SIZE)
				{
					set_inuse_and_pinuse(p, small_index2size(i));
				}
				else
				{
					set_size_and_pinuse_of_inuse_chunk(p, align_size);
					r = chunk_plus_offset(p, align_size);
					set_size_and_pinuse_of_free_chunk(r, rsize);
					insert_small_chunk(_mg, r, rsize);
				}
				mem = chunk2mem(p);
			}
			//find in tree chunk
			else if (_mg->tbinsmap != 0)
			{
				mem = tmalloc_small(_mg, align_size);
			}
			//find in rest memory pool.
			else 
			{
				mem = top_malloc(_mg, align_size);
			}
		}
		
		if (mem != NULL) 
		{
			//already allocated suitable memory chunk, end while.
			break;
		}

		/* add more memory pools into list if system has*/
		if (_mg->next == NULL)
		{
			int32_t request_size;
			void  * mem_pool;

			//what's small size of current request. 
			request_size = ALIGN_UP(size, MEM_ALIGNMENT) + MG_SIZE_ALIGNED;
			Sys_mm_getMemoryPool(&mem_pool, &request_size);
			if (mem_pool == NULL || request_size == 0 || 
				(request_size < ALIGN_UP(size, MEM_ALIGNMENT) + MG_SIZE_ALIGNED))
			{
				DVMTraceErr("DVM_mm_initialize - ERROR: alloc memory pool failure\n");
				break;
			}
			_mg->next = initialMemoryPoolGlobal(&mem_pool, request_size);
		}

		//check next memory pool.
		_mg = _mg->next;
	} while (_mg != NULL);

	return mem;
}

void * dcalloc(uint32_t num, uint32_t size)
{
	void * mem;
	uint32_t req_size;

	req_size = num * size;

	mem = dmalloc(req_size);
	if (mem != NULL)
	{
		CRTL_memset(mem, 0x0, req_size);
	}

	return mem;
}


void dfree(void * mem)
{
	mchunkptr p;
	mpglobal * _mg;
	
	_mg = getPoolGlobalByMemory(mem);
	if (_mg == NULL)
	{
		//invalid memory pointer
		return;
	}

	p = mem2chunk(mem);

	if (((void *)next_chunk(p)) == _mg->mem_end)
	{
		//adjacent to top chunk, just recycle it by top chunk.
		_mg->mem_end = (void *)(((char *)_mg->mem_end) - chunksize(p));
	}
	else 
	{
		mchunkptr f, b;
		uint32_t csize;
		
		csize = chunksize(p);

		if (!pinuse(p))
		{	
			uint32_t fsize = p->prev_foot;

			f = prev_chunk(p);
			unlink_chunk(_mg, f, fsize);
			
			csize += fsize;
			p = f;
		}
		b = next_chunk(p);

		if (!cinuse(b)) 
		{
			uint32_t bsize = chunksize(b);

			unlink_chunk(_mg, b, bsize);
			csize += bsize;

			set_size_and_pinuse_of_free_chunk(p, csize);
		}
		else
		{
            set_free_with_pinuse(p, csize, b);
		}

		if (is_small(csize)) 
		{
			insert_small_chunk(_mg, p, csize);
		}
		else 
		{
			tchunkptr tp = (tchunkptr)p;
			insert_large_chunk(_mg, tp, csize);
		}
	}
}


void * endmalloc(uint32_t size)
{
	void     * mem;
	mpglobal * _mg;
	mchunkptr  p;   
	uint32_t   req_size;

	/**
	 * only malloc permanent data from first memory pool.
	 * FIX ME: how to handle while there are not enough memory 
	 * space in first memory pool?
	 */
	_mg = mg;
	mem = NULL;
	req_size = pad_request2size(size);

	if (_mg->free_size > req_size)
	{
		p = (mchunkptr)(((char *)_mg->perm_start) - req_size);
		_mg->free_size -= req_size;
		
		if (_mg->perm_start == _mg->perm_end)
		{
			//the first permanent data, set current inuse.
			((p)->head = (req_size|CINUSE_BIT));
		}
		else if (_mg->perm_start < _mg->perm_end)
		{
			((p)->head = (req_size|CINUSE_BIT));
			(((mchunkptr)(((char*)(p)) + (req_size)))->head |= PINUSE_BIT);
		}
		else
		{
			//should never go this place.
		}

		mem = chunk2mem(p);
	}

	return mem;
}

