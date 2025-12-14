#include <string.h>
#include <list>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <ctime>
#ifndef _lirs_H
#define _lirs_H

struct PageEntry {
	bool isLIR;
	bool isResident;
	bool inS;
};

class LIRSCache
{
	//Store the recency order
	std::list<long long int> S;
	std::list<long long int> Q;

	std::unordered_map<long long int, std::list<long long int>::iterator> S_map;
	std::unordered_map<long long int, std::list<long long int>::iterator> Q_map;

	int capPulse;

	// Page table entries
	std::unordered_map<long long int, PageEntry> table;

	// record read-write type of cached page
	std::unordered_map<long long int, std::string> accessType; 
	
	// size of LIR and HIR
	int LIR_cap, HIR_cap;
	int LIR_size;

	long long int calls, total_calls;
	long long int hits, total_hits;

	long long int readHits; 
	long long int writeHits; 
	long long int evictedDirtyPage; 


	long long int migration, total_migration;

public:
	LIRSCache(int);
	~LIRSCache();
	void refer(long long int, std::string);
	void display();

	// summary results
	void cachehits();

	void refresh();
	void summary();

	void prune();
	void evictFromQ();
	void demoteBottomLIR();

};
#endif
