// ARC caching policy implementation
#include <list>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include "arc.h"
#include <string.h>
using namespace std;

arccache::arccache(int n) {
	capacity = n;
	targetRecent = 0;
	hits = 0;
	total_hits = 0;
	calls = 0;
	total_calls = 0;
	migration = 0;
	total_migration = 0;
	readHits = 0;
	writeHits = 0;
	evictedDirtyPage = 0;

	std::cout << "ARC Algorithm is used" << std::endl;
	std::cout << "Cache size is: " << capacity << std::endl;
}

arccache::~arccache() {
	capacity = 0;
	targetRecent = 0;
	hits = 0;
	total_hits = 0;
	calls = 0;
	total_calls = 0;
	migration = 0;
	total_migration = 0;
	readHits = 0;
	writeHits = 0;
	evictedDirtyPage = 0;
	accessType.clear();

	recentCache.clear();
	frequentCache.clear();
	recentGhost.clear();
	frequentGhost.clear();
	recentPos.clear();
	frequentPos.clear();
	recentGhostPos.clear();
	frequentGhostPos.clear();
}

void arccache::moveToFrequent(long long int key) {
	// Move a page into the frequent region (T2) regardless of its previous position.
	auto itRecent = recentPos.find(key);
	if (itRecent != recentPos.end()) {
		recentCache.erase(itRecent->second);
		recentPos.erase(itRecent);
	}
	auto itFreq = frequentPos.find(key);
	if (itFreq != frequentPos.end()) {
		frequentCache.erase(itFreq->second);
		frequentPos.erase(itFreq);
	}
	frequentCache.push_front(key);
	frequentPos[key] = frequentCache.begin();
}

void arccache::moveToRecent(long long int key) {
	// Insert page at the front of the recent region (T1).
	auto itRecent = recentPos.find(key);
	if (itRecent != recentPos.end()) {
		recentCache.erase(itRecent->second);
		recentPos.erase(itRecent);
	}
	recentCache.push_front(key);
	recentPos[key] = recentCache.begin();
}

void arccache::recordGhostHitAdjustment(bool hitRecentGhost) {
	// Adjust targetRecent following ARC: hit in B1 grows T1, hit in B2 shrinks T1.
	if (hitRecentGhost) {
		int delta = std::max(1, (int)frequentGhost.size() / std::max(1, (int)recentGhost.size()));
		targetRecent = std::min(capacity, targetRecent + delta);
	} else {
		int delta = std::max(1, (int)recentGhost.size() / std::max(1, (int)frequentGhost.size()));
		targetRecent = std::max(0, targetRecent - delta);
	}
}

void arccache::evictOne(long long int incoming) {
	// Decide whether to evict from recentCache or frequentCache based on adaptive split.
	bool evictRecent = (!recentCache.empty() && ((frequentGhostPos.find(incoming) != frequentGhostPos.end() && (int)recentCache.size() == targetRecent) || (int)recentCache.size() > targetRecent));

	if (evictRecent) {
		long long int victim = recentCache.back();
		recentCache.pop_back();
		recentPos.erase(victim);
		if (accessType[victim] == "Write") {
			evictedDirtyPage++;
		}
		recentGhost.push_front(victim);
		recentGhostPos[victim] = recentGhost.begin();
	} else if (!frequentCache.empty()) {
		long long int victim = frequentCache.back();
		frequentCache.pop_back();
		frequentPos.erase(victim);
		if (accessType[victim] == "Write") {
			evictedDirtyPage++;
		}
		frequentGhost.push_front(victim);
		frequentGhostPos[victim] = frequentGhost.begin();
	}
}

void arccache::refer(long long int key, string rwtype) {
	calls++;

	auto itRecent = recentPos.find(key);
	auto itFrequent = frequentPos.find(key);
	auto itRecentGhost = recentGhostPos.find(key);
	auto itFrequentGhost = frequentGhostPos.find(key);

	// Case 1: Hit in frequent cache (T2).
	if (itFrequent != frequentPos.end()) {
		frequentCache.erase(itFrequent->second);
		frequentCache.push_front(key);
		frequentPos[key] = frequentCache.begin();

		hits++;
		if (rwtype == "Read") {
			readHits++;
		} else {
			writeHits++;
			accessType[key] = "Write";
		}
		return;
	}

	// Case 2: Hit in recent cache (T1) -> promote to frequent.
	if (itRecent != recentPos.end()) {
		moveToFrequent(key);

		hits++;
		if (rwtype == "Read") {
			readHits++;
		} else {
			writeHits++;
			accessType[key] = "Write";
		}
		return;
	}

	// Case 3: Miss but key present in recent ghost list (B1).
	if (itRecentGhost != recentGhostPos.end()) {
		recordGhostHitAdjustment(true);
		evictOne(key);

		recentGhost.erase(itRecentGhost->second);
		recentGhostPos.erase(itRecentGhost);
		moveToFrequent(key);

		if (rwtype == "Write") {
			accessType[key] = "Write";
		} else if (accessType.find(key) == accessType.end()) {
			accessType[key] = "Read";
		}
		return;
	}

	// Case 4: Miss but key present in frequent ghost list (B2).
	if (itFrequentGhost != frequentGhostPos.end()) {
		recordGhostHitAdjustment(false);
		evictOne(key);

		frequentGhost.erase(itFrequentGhost->second);
		frequentGhostPos.erase(itFrequentGhost);
		moveToFrequent(key);

		if (rwtype == "Write") {
			accessType[key] = "Write";
		} else if (accessType.find(key) == accessType.end()) {
			accessType[key] = "Read";
		}
		return;
	}

	// Case 5: Completely new page; manage sizes then insert into recent cache.
	if ((int)(recentCache.size() + recentGhost.size()) == capacity) {
		if ((int)recentCache.size() < capacity) {
			if (!recentGhost.empty()) {
				long long int oldGhost = recentGhost.back();
				recentGhost.pop_back();
				recentGhostPos.erase(oldGhost);
			}
			evictOne(key);
		} else {
			long long int victim = recentCache.back();
			recentCache.pop_back();
			recentPos.erase(victim);
			if (accessType[victim] == "Write") {
				evictedDirtyPage++;
			}
			recentGhost.push_front(victim);
			recentGhostPos[victim] = recentGhost.begin();
		}
	} else if ((int)(recentCache.size() + recentGhost.size() + frequentCache.size() + frequentGhost.size()) >= capacity) {
		if ((int)(recentCache.size() + recentGhost.size() + frequentCache.size() + frequentGhost.size()) >= 2 * capacity) {
			if (!frequentGhost.empty()) {
				long long int trim = frequentGhost.back();
				frequentGhost.pop_back();
				frequentGhostPos.erase(trim);
			}
		}
		evictOne(key);
	}

	moveToRecent(key);
	if (rwtype == "Write") {
		accessType[key] = "Write";
	} else if (accessType.find(key) == accessType.end()) {
		accessType[key] = "Read";
	}
}

void arccache::display() {
	for (std::list<long long int>::iterator xi = recentCache.begin(); xi != recentCache.end(); xi++) {
		std::cout << *xi << " ";
	}
	for (std::list<long long int>::iterator xi = frequentCache.begin(); xi != frequentCache.end(); xi++) {
		std::cout << *xi << " ";
	}
	std::cout << std::endl;
}

void arccache::cachehits() {
	std::cout << "calls: " << calls << ", hits: " << hits << ", readHits: " << readHits << ", writeHits: " << writeHits << ", evictedDirtyPage: " << evictedDirtyPage << std::endl;

	std::ofstream result("ExperimentalResult.txt", std::ios_base::app);
	if (result.is_open()) {
		result << "ARC " << "CacheSize " << capacity << " calls " << calls << " hits " << hits << " hitRatio " << float(hits) / calls << " readHits " << readHits << " readHitRatio " << float(readHits) / calls << " writeHits " << writeHits << " writeHitRatio " << float(writeHits) / calls << " evictedDirtyPage " << evictedDirtyPage << "\n";
	}
	result.close();
}

void arccache::refresh() {
	calls = 0;
	hits = 0;
	migration = 0;
}

void arccache::summary() {
	std::cout << "the total number of cache hits is: " << total_hits << std::endl;
	std::cout << "the total number of total refered calls is " << total_calls << std::endl;
	std::cout << "the total data migration size into the optane is: " << ((double)total_migration) * 16 / 1024 / 1024 << "GB" << std::endl;
}