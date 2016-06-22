//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include "limits.h"

//
// TODO:Student Information
//
const char *studentName = "";
const char *studentID   = "";
const char *email       = "";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

uint32_t offsetBits;     // number of bits for offset
uint32_t i_indexBits;    // number of index bits of i-cache 
uint32_t i_tagBits;      // number of tag bits of i-cache
uint32_t d_indexBits;    // number of index bits of d-cache
uint32_t d_tagBits;      // number of tag bits of d-cache
uint32_t l2_indexBits;    // number of index bits of L2-cache
uint32_t l2_tagBits;      // number of tag bits of L2-cache

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//
#define power2(x) (1<<x) // calculate 2^x

static uint8_t mylog2 (uint32_t val) {
	if (val == 1) return 0;
	uint8_t ret = 0;
	while (val > 1) {
		val >>= 1;
		ret++;
	}
	return ret;
}

int mask(int x)
{
	// generate x bit '1'
	x = (1<<x);
	x--;
	return x;
}

// 
typedef struct cacheLine
{
	int valid;
	int lruCounter;
	uint32_t address;
	unsigned int tag;
} cacheLine;

typedef struct set{
	cacheLine* cl;
} set;

typedef struct cache{
	set* s;
} cache;

cache i_cache;
cache d_cache;
cache l2_cache;

//------------------------------------//
//          Cache Functions           //
//------------------------------------//
void icache_add(uint32_t addr);
void dcache_add(uint32_t addr);
void l2cache_add(uint32_t addr);
uint8_t icache_isin(uint32_t addr);
uint8_t dcache_isin(uint32_t addr);
uint8_t l2cache_isin(uint32_t addr);

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;

  //
  //TODO: Initialize Cache Simulator Data Structures
  //
  offsetBits = mylog2(blocksize);
  if (icacheSets > 0) {
	  i_indexBits = mylog2(icacheSets);
	  i_tagBits = 32 - i_indexBits - offsetBits;

	  i_cache.s = malloc(sizeof(set) * icacheSets);
	  for(int i = 0; i < icacheSets; i++)
	  {
		  i_cache.s[i].cl = malloc(sizeof(cacheLine) * icacheAssoc);
	  }

	  for (int i = 0; i < icacheSets; i++)
	  {
		  for (int j = 0; j < icacheAssoc; j++)
		  {
			  i_cache.s[i].cl[j].valid = 0;
			  i_cache.s[i].cl[j].lruCounter = 0;
			  i_cache.s[i].cl[j].address = 0;
			  i_cache.s[i].cl[j].tag = 0;
		  }
	  }

  }

  if (dcacheSets > 0) {

	  d_indexBits = mylog2(dcacheSets);
	  d_tagBits = 32 - d_indexBits - offsetBits;

	  d_cache.s = malloc(sizeof(set) * dcacheSets);
	  for(int i = 0; i < dcacheSets; i++)
	  {
		  d_cache.s[i].cl = malloc(sizeof(cacheLine) * dcacheAssoc);
	  }

	  for (int i = 0; i < dcacheSets; i++)
	  {
		  for (int j = 0; j < dcacheAssoc; j++)
		  {
			  d_cache.s[i].cl[j].valid = 0;
			  d_cache.s[i].cl[j].lruCounter = 0;
			  d_cache.s[i].cl[j].address = 0;
			  d_cache.s[i].cl[j].tag = 0;
		  }
	  }

  }

  if (l2cacheSets > 0) {
	  l2_indexBits = mylog2(l2cacheSets);
	  l2_tagBits = 32 - l2_indexBits - offsetBits;

	  l2_cache.s = malloc(sizeof(set) * l2cacheSets);
	  for(int i = 0; i < l2cacheSets; i++)
	  {
		  l2_cache.s[i].cl = malloc(sizeof(cacheLine) * l2cacheAssoc);
	  }

	  for (int i = 0; i < l2cacheSets; i++)
	  {
		  for (int j = 0; j < l2cacheAssoc; j++)
		  {
			  l2_cache.s[i].cl[j].valid = 0;
			  l2_cache.s[i].cl[j].lruCounter = 0;
			  l2_cache.s[i].cl[j].address = 0;
			  l2_cache.s[i].cl[j].tag = 0;
		  }
	  }
  }

}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
	//
	//TODO: Implement I$
	//
	uint32_t cur_icache_penalty;
	uint32_t icache_access_time;

	if (icacheSets)
	{
		// there exists icache
		icacheRefs++;

		if ((!icache_isin(addr)) && (l2cacheSets > 0))
		{
			// not in icache, check l2 cache
			icacheMisses++;
			l2cacheRefs++;
			cur_icache_penalty = l2cacheHitTime + l2cache_access(addr);
			if (!l2cache_isin(addr))
			{
				// not in l2, copy from mem to l2 and l1
				l2cache_add(addr);
				icache_add(addr);
			}
			else
			{
				// in l2, copy from l2 to l1
				icache_add(addr);
			}

		}
		else if ((!icache_isin(addr)))
		{
			// not in l1, there is no l2
			icacheMisses++;
			cur_icache_penalty = memspeed;
			icache_add(addr);
		}
		else
		{
			// in icache, no penalty
			cur_icache_penalty = 0;
		}

		icachePenalties += cur_icache_penalty;
		icache_access_time = icacheHitTime + cur_icache_penalty;
	}
	else
	{
		// there is no icache
		if (l2cacheSets)
		{
			// there is l2 cache
			l2cacheRefs++;
			cur_icache_penalty = l2cache_access(addr);
			if (!l2cache_isin(addr))
			{
				// not in l2, copy from mem to l2
				l2cache_add(addr);
			}
			icache_access_time = l2cacheHitTime + cur_icache_penalty;
		}
		else
		{
			// no l2 cache, all data brought from mem
			cur_icache_penalty = memspeed;
			icache_access_time = cur_icache_penalty;
		}
	}
	return icache_access_time;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
	//
	//TODO: Implement D$
	//
	uint32_t cur_dcache_penalty;
	uint32_t dcache_access_time;

	if (dcacheSets)
	{
		// there is dcache
		dcacheRefs++;

		if ((!dcache_isin(addr)) && (l2cacheSets > 0))
		{
			// not in dcache, check l2 cache
			dcacheMisses++;
			l2cacheRefs++;
			cur_dcache_penalty = l2cacheHitTime + l2cache_access(addr);
			if (!l2cache_isin(addr))
			{
				// not in l2, copy from mem to l2 and l1
				l2cache_add(addr);
				dcache_add(addr);
			}
			else
			{
				// in l2, copy from l2 to l1
				dcache_add(addr);
			}
		}
		else if (!dcache_isin(addr))
		{
			// not in l1, there is no l2
			dcacheMisses++;
			cur_dcache_penalty = memspeed;
			dcache_add(addr);
		}
		else
		{
			// in dcache, no penalty
			cur_dcache_penalty = 0;
		}
		dcachePenalties += cur_dcache_penalty;
		dcache_access_time = dcacheHitTime + cur_dcache_penalty;
	}
	else
	{
		// there is no dcache
		if (l2cacheSets)
		{
			// no dcache, l2 cache exists
			l2cacheRefs++;
			cur_dcache_penalty = l2cache_access(addr);
			if (!l2cache_isin(addr))
			{
				// not in l2, copy from mem to l2
				l2cache_add(addr);
			}
			dcache_access_time = l2cacheHitTime + cur_dcache_penalty;

		}
		else
		{
			// no dcache, no l2 cache
			cur_dcache_penalty = memspeed;
			dcache_access_time = cur_dcache_penalty;
		}
	}
	return dcache_access_time;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //

	uint32_t cur_l2cache_penalty;
	if (!l2cache_isin(addr))
	{
		// not in l2 cache
		l2cacheMisses++;
		cur_l2cache_penalty = memspeed;
	}
	else
	{
		// in l2cache, no penalty
		cur_l2cache_penalty = 0;
	}
	l2cachePenalties += cur_l2cache_penalty;
	return cur_l2cache_penalty;
}

uint8_t icache_isin(uint32_t addr)
{

	uint32_t tmp = addr >> (offsetBits);
	uint32_t index = tmp & mask(i_indexBits);
	tmp = tmp >> (i_indexBits);
	uint32_t pcTag = tmp & mask(i_tagBits);

	uint8_t hitFlag = 0;

	for(int j = 0; j < icacheAssoc; j++)
	{
		if (i_cache.s[index].cl[j].valid == 1 && i_cache.s[index].cl[j].tag == pcTag)
		{
			hitFlag = 1;
			i_cache.s[index].cl[j].lruCounter = icacheRefs;
			break;
		}
	}

	return hitFlag;
}

uint8_t dcache_isin(uint32_t addr)
{

	uint32_t tmp = addr >> (offsetBits);
	uint32_t index = tmp & mask(d_indexBits);
	tmp = tmp >> (d_indexBits);
	uint32_t pcTag = tmp & mask(d_tagBits);

	uint8_t hitFlag = 0;

	for(int j = 0; j < dcacheAssoc; j++)
	{
		if (d_cache.s[index].cl[j].valid == 1 && d_cache.s[index].cl[j].tag == pcTag)
		{
			hitFlag = 1;
			d_cache.s[index].cl[j].lruCounter = dcacheRefs;
			break;
		}
	}

	return hitFlag;
}

uint8_t l2cache_isin(uint32_t addr)
{

    uint32_t tmp = addr >> (offsetBits);
	uint32_t index = tmp & mask(l2_indexBits);
	tmp = tmp >> (l2_indexBits);
	uint32_t pcTag = tmp & mask(l2_tagBits);

	uint8_t hitFlag = 0;

	for(int j = 0; j < l2cacheAssoc; j++)
	{
		if (l2_cache.s[index].cl[j].valid == 1 && l2_cache.s[index].cl[j].tag == pcTag)
		{
			hitFlag = 1;
			l2_cache.s[index].cl[j].lruCounter = l2cacheRefs;
			break;
		}
	}

	return hitFlag;
}

void icache_add(uint32_t addr)
{
	// update icache
	uint32_t tmp = addr >> (offsetBits);
	uint32_t index = tmp & mask(i_indexBits);
	tmp = tmp >> (i_indexBits); 
	uint32_t pcTag = tmp & mask(i_tagBits);

	int replaceFlag = 0;
	for(int j = 0; j < icacheAssoc; j++)
	{
		if (i_cache.s[index].cl[j].valid == 0)
		{
			i_cache.s[index].cl[j].tag = pcTag;
			i_cache.s[index].cl[j].valid = 1;
			i_cache.s[index].cl[j].lruCounter = icacheRefs;
			i_cache.s[index].cl[j].address = addr;
			break;
		}
		else if ( j == icacheAssoc -1)
		{
			replaceFlag = 1; // all lines in current set is full
		}		
	}

	if (replaceFlag)
	{
		int minLRU = INT_MAX;
		int curLRU;
		int repLineIdx = 0; 
		for (int j = 0; j < icacheAssoc; j++)
		{
			curLRU = i_cache.s[index].cl[j].lruCounter;
			if (minLRU > curLRU)
			{
				minLRU = curLRU;
				repLineIdx = j;
			}
		}
		i_cache.s[index].cl[repLineIdx].tag = pcTag;
		i_cache.s[index].cl[repLineIdx].valid = 1;
		i_cache.s[index].cl[repLineIdx].lruCounter = icacheRefs;
		i_cache.s[index].cl[repLineIdx].address = addr;
	}

}

void dcache_add(uint32_t addr)
{
	// update icache
	uint32_t tmp = addr >> (offsetBits);
	uint32_t index = tmp & mask(d_indexBits);
	tmp = tmp >> (d_indexBits);
	uint32_t pcTag = tmp & mask(d_tagBits);

	int replaceFlag = 0;
	for(int j = 0; j < dcacheAssoc; j++)
	{
		if (d_cache.s[index].cl[j].valid == 0)
		{
			d_cache.s[index].cl[j].tag = pcTag;
			d_cache.s[index].cl[j].valid = 1;
			d_cache.s[index].cl[j].lruCounter = dcacheRefs;
			d_cache.s[index].cl[j].address = addr;
			break;
		}
		else if ( j == dcacheAssoc -1)
		{
			replaceFlag = 1; // all lines in current set is full
		}		
	}

	if (replaceFlag)
	{
		int minLRU = INT_MAX;
		int curLRU;
		int repLineIdx = 0; 
		for (int j = 0; j < dcacheAssoc; j++)
		{
			curLRU = d_cache.s[index].cl[j].lruCounter;
			if (minLRU > curLRU)
			{
				minLRU = curLRU;
				repLineIdx = j;
			}
		}
		d_cache.s[index].cl[repLineIdx].tag = pcTag;
		d_cache.s[index].cl[repLineIdx].valid = 1;
		d_cache.s[index].cl[repLineIdx].lruCounter = dcacheRefs;
		d_cache.s[index].cl[repLineIdx].address = addr;
	}
}


void l2cache_add(uint32_t addr)
{
	// update icache
    uint32_t tmp = addr >> (offsetBits);
	uint32_t index = tmp & mask(l2_indexBits);
	tmp = tmp >> (l2_indexBits);
	uint32_t pcTag = tmp & mask(l2_tagBits);

	int replaceFlag = 0;
	for(int j = 0; j < l2cacheAssoc; j++)
	{
		if (l2_cache.s[index].cl[j].valid == 0)
		{
			l2_cache.s[index].cl[j].tag = pcTag;
			l2_cache.s[index].cl[j].valid = 1;
			l2_cache.s[index].cl[j].lruCounter = l2cacheRefs;
			l2_cache.s[index].cl[j].address = addr;
			break;
		}
		else if ( j == l2cacheAssoc -1)
		{
			replaceFlag = 1; // all lines in current set is full
		}		
	}

	if (replaceFlag)
	{
		int minLRU = INT_MAX;
		int curLRU;
		int repLineIdx = 0; 
		for (int j = 0; j < l2cacheAssoc; j++)
		{
			curLRU = l2_cache.s[index].cl[j].lruCounter;
			if (minLRU > curLRU)
			{
				minLRU = curLRU;
				repLineIdx = j;
			}
		}
		uint32_t old_addr = l2_cache.s[index].cl[repLineIdx].address;
		l2_cache.s[index].cl[repLineIdx].tag = pcTag;
		l2_cache.s[index].cl[repLineIdx].valid = 1;
		l2_cache.s[index].cl[repLineIdx].lruCounter = l2cacheRefs;
		l2_cache.s[index].cl[repLineIdx].address = addr;

		if (inclusive)
		{
			// evict the previous entry in L1
			if (icacheSets)
			{
				if (icache_isin(old_addr))
				{

					uint32_t tmp_addr = old_addr >> (offsetBits);
					uint32_t tmp_index = tmp_addr & mask(i_indexBits);
					tmp_addr = tmp_addr >> (i_indexBits);
					uint32_t tmp_pcTag = tmp_addr & mask(i_tagBits);

					for(int k = 0; k < icacheAssoc; k++)
					{
						if (i_cache.s[tmp_index].cl[k].valid == 1 && i_cache.s[tmp_index].cl[k].tag == tmp_pcTag)
						{
							i_cache.s[tmp_index].cl[k].valid = 0;
							i_cache.s[tmp_index].cl[k].tag = 0;
						}
					}
				}
			}

			if (dcacheSets)
			{
				if (dcache_isin(old_addr))
				{

					uint32_t tmp_addr = old_addr >> (offsetBits);
					uint32_t tmp_index = tmp_addr & mask(d_indexBits);
					tmp_addr = tmp_addr >> (d_indexBits);
					uint32_t tmp_pcTag = tmp_addr & mask(d_tagBits);

					for(int k = 0; k < dcacheAssoc; k++)
					{
						if (d_cache.s[tmp_index].cl[k].valid == 1 && d_cache.s[tmp_index].cl[k].tag == tmp_pcTag)
						{
							d_cache.s[tmp_index].cl[k].valid = 0;
							d_cache.s[tmp_index].cl[k].tag = 0;
						}
					}
				}
			}
		}
	}
}

