#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned int uint;

typedef struct cacheLine {
	uint 	tag;
	uint	index;
	uint	offset;
} CacheLine;

typedef struct cache {
	bool	valid;
	uint	padding: 3;
	uint	tag;
} Cache;

uint getN(uint startPoint, uint cacheBlockSize) {
	if(cacheBlockSize & 1 == 1)
		return startPoint;
	return getN(startPoint + 1, cacheBlockSize >> 1);
} uint getN(uint cacheBlockSize) {
	return getN(0, cacheBlockSize);
} 

uint createMask(uint retval, uint current, uint length, uint offset) {
	if(current == length)
		return ( retval - 1 ) << offset;
	else return createMask(retval << 1, current + 1, length, offset);
} uint createMask(uint length, uint offset) {
	return createMask(1, 0, length, offset);
}

void memAccess(const CacheLine param, uint accessAt, CacheLine *result) {
	printf("[ACC:");
	uint offset = 32 - param.tag;
	result->tag = ( accessAt & createMask(param.tag, offset) ) >> offset;
	result->index = ( accessAt & createMask(param.index, param.offset) ) >> param.offset;
	result->offset = ( accessAt & createMask(param.offset, 0) );
	printf("%u]", accessAt);
}

bool cacheSetIsValid(Cache **cache, uint nAssociativity, CacheLine request) {
	for(uint i = 0; i < nAssociativity; ++i) {
		if(cache[request.index][i].valid) {
			printf("[VLD:1]");
			return true;
		}
	}
	printf("[VLD:0]");
	return false;
}

bool cacheSetTagMatch(Cache **cache, uint nAssociativity, CacheLine request) {
	for(uint i = 0; i < nAssociativity; ++i) {
		if(cache[request.index][i].tag == request.tag) {
			printf("[TAG:1]");
			return true;
		}
	}
	printf("[TAG:0]");
	return false;
}

void insertIntoCache(Cache **cache, uint nAssociativity, CacheLine request) {
	for(uint i = 0; i < nAssociativity; ++i) {
		if(cache[request.index][i].valid == false) {
			cache[request.index][i].valid = true;
			cache[request.index][i].tag = request.tag;
			printf("[INSRT]");
			return;
		}
	}
	// using random replacement method for simplicity
	srand(time(NULL));
	if(nAssociativity > 1)
		--nAssociativity;
	uint replacementIndex = ( rand() % nAssociativity );
	cache[request.index][replacementIndex].tag = request.tag;
	printf("[RPLCE]");
}

int main(void) {
	uint cacheSize, cacheBlockSize, nCacheBlocks, nCacheSets, nAccesses, nCurrentAccess, nWayAssociativity;
	// take in all user parameters
	printf("Enter cache size: ");
	scanf("%u", &cacheSize);
	printf("Enter cache block size: ");
	scanf("%u", &cacheBlockSize);
	printf("Enter N, where N = N-way Associativity: ");
	scanf("%u", &nWayAssociativity);
	printf("How many accesses: ");
	scanf("%u", &nAccesses);
	
	printf("\n\n");
	for(int i = 0; i < 30; ++i) {
		printf("- ");
	}
	printf("\n\n");
	
	// create cache
	
	CacheLine param, currentResult;
	// find OFFSET
	param.offset = getN(cacheBlockSize);
	// find NUMBER OF CACHE BOCKS
	nCacheBlocks = cacheSize / cacheBlockSize;
	// find NUMBER OF CACHE SETS
	nCacheSets = nCacheBlocks / nWayAssociativity;
	// find CACHE INDEX
	param.index = getN(nCacheSets);
	// find TAG
	param.tag = 32 - ( param.index + param.offset );
	
	Cache **cache = (Cache**)malloc((nCacheSets+1)*sizeof(Cache*));
	for(uint i = 0; i < nCacheSets; ++i) {
		cache[i] = (Cache*)malloc((nWayAssociativity+1)*sizeof(Cache));
		for(uint j = 0; j < nWayAssociativity; ++j) {
			cache[i][j].valid = false;
		}
	}
	
	uint hit = 0;
	uint miss = 0;
	bool isHit;
	for(uint i = 0; i < nAccesses; ++i) {
		isHit = false;
		scanf("%u", &nCurrentAccess);
		memAccess(param, nCurrentAccess, &currentResult);
		if( ( cacheSetIsValid(cache, nWayAssociativity, currentResult) == true ) &&
			( cacheSetTagMatch(cache, nWayAssociativity, currentResult) == true ) ) {
			isHit = true;
			++hit;
		} else {
			++miss;
			insertIntoCache(cache, nWayAssociativity, currentResult);
		}
		printf("\tACC-TAG-INDEX-OFFSET: %u-%u-%u-%u %s\n",
			nCurrentAccess,
			currentResult.tag,
			currentResult.index,
			currentResult.offset,
			(isHit ? "[HIT]":"[MISS]"));
	}
	double result = ( static_cast<double>(hit) / static_cast<double>(nAccesses) ) * 100;
	printf("hit rate: %.3lf%s\n", result, "%");
	printf("total hits: %u\n", hit);
	printf("total misses: %u\n", miss);
	free(cache);
	return 0;
}