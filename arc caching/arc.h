/* ARC (Adaptive Replacement Cache) implementation. The interface mirrors lru.h. */
#include <string.h>
using namespace std;
#ifndef _arc_H
#define _arc_H

class arccache
{
	// recentCache/frequentCache correspond to T1/T2; ghost lists remember history.
	std::list<long long int> recentCache;
	std::list<long long int> frequentCache;
	std::list<long long int> recentGhost;
	std::list<long long int> frequentGhost;

	// Track iterator positions for O(1) membership/erase.
	std::unordered_map<long long int, std::list<long long int>::iterator> recentPos;
	std::unordered_map<long long int, std::list<long long int>::iterator> frequentPos;
	std::unordered_map<long long int, std::list<long long int>::iterator> recentGhostPos;
	std::unordered_map<long long int, std::list<long long int>::iterator> frequentGhostPos;

	int capacity;      // maximum capacity of cache
	int targetRecent;  // adaptive target size for recentCache

	std::unordered_map<long long int, string> accessType;

	long long int calls, total_calls;
	long long int hits, total_hits;
	long long int readHits;
	long long int writeHits;
	long long int evictedDirtyPage;
	long long int migration, total_migration;

	void evictOne(long long int incoming);
	void moveToFrequent(long long int key);
	void moveToRecent(long long int key);
	void recordGhostHitAdjustment(bool hitRecentGhost);

public:
	arccache(int);
	~arccache();
	void refer(long long int, string);
	void display();

	void cachehits();

	void refresh();
	void summary();
};
#endif
