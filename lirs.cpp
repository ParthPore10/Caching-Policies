#include "lirs.h"

LIRSCache::LIRSCache(int n) {
	capPulse = n;
	hits = 0;	// "hits" records the number of cache hit
	total_hits = 0;
	calls = 0;	// "calls" records the the number of total calls 
	total_calls = 0;
	migration = 0;	// "migration" records the number of data that is not cached in the optane while calling and need to be migrated into Optane then
	total_migration = 0;

	// keep HIR as 1% floor; rest belongs to LIR
	HIR_cap = (1 > 0.01*capPulse)?(1):(0.01*capPulse);
	LIR_cap = capPulse - HIR_cap;
	LIR_size = 0;

	readHits = 0; 
	writeHits = 0; 
	evictedDirtyPage = 0; 
	
	std::cout << "LIRS Algorithm is used" << std::endl;
	std::cout << "Cache size is: " << capPulse <<  std::endl;
}

LIRSCache::~LIRSCache() {
	capPulse = 0;
	hits = 0;	// "hits" records the number of cache hit
	total_hits = 0;
	calls = 0;	// "calls" records the the number of total calls 
	total_calls = 0;
	migration = 0;	// "migration" records the number of data that is not cached in the optane while calling and need to be migrated into Optane then
	total_migration = 0;

	HIR_cap = 0;
	LIR_cap = 0;
	LIR_size = 0;

	readHits = 0; 
	writeHits = 0; 
	evictedDirtyPage = 0; 
	accessType.clear(); 

	S.clear();
	Q.clear();
	S_map.clear();
	Q_map.clear();
}

void LIRSCache::refer(long long int x, std::string rwtype) {
	calls++;

	const bool isWrite = (rwtype == "Write");
	if (isWrite) {
		// mark page dirty on writes so later eviction stats stay honest
		accessType[x] = "Write";
	}

	PageEntry &pe = table[x];
	const bool wasResident = pe.isResident;
    if (!pe.isResident && !pe.inS)
    {
        pe.isResident = true;
        pe.isLIR = false;

        // Insert into S
        S.push_front(x);
        S_map[x] = S.begin();
        pe.inS = true;

        // Insert into Q (HIR resident)
        Q.push_front(x);
        Q_map[x] = Q.begin();

        evictFromQ();
        prune();
        return;
    }
    if (pe.isLIR && pe.isResident)
    {
        if (pe.inS) {
            S.erase(S_map[x]);
        }
        S.push_front(x);
        S_map[x] = S.begin();
        pe.inS = true;

        prune();
        goto record_hit_and_exit;
    }
    if (!pe.isLIR && pe.isResident)
    {
        Q.erase(Q_map[x]);
        Q_map.erase(x);

        pe.isLIR = true;
        LIR_size++;

        if (pe.inS) S.erase(S_map[x]);
        S.push_front(x);
        S_map[x] = S.begin();
        pe.inS = true;

        // Must demote bottom-most LIR
        demoteBottomLIR();

        prune();
        evictFromQ();
        goto record_hit_and_exit;
    }
    if (!pe.isResident && pe.inS)
    {

        pe.isResident = true;
        pe.isLIR = true;
        LIR_size++;

        S.erase(S_map[x]);
        S.push_front(x);
        S_map[x] = S.begin();
        pe.inS = true;
        Q.push_front(x);
        Q_map[x] = Q.begin();
        Q.erase(Q_map[x]);
        Q_map.erase(x);
        demoteBottomLIR();

        prune();
        evictFromQ();
        return;
    }

record_hit_and_exit:
	if (wasResident) {
		hits++;
		if (isWrite) {
			writeHits++;
		} else {
			readHits++;
		}
	}
}


void LIRSCache::display() {
	for (std::list<long long int>::iterator xi = S.begin(); xi != S.end(); xi++) {
		PageEntry &pe = table[*xi];
		if(pe.isResident) {
			std::cout << *xi << " ";
		}
	}
	std::cout << std::endl;
}

void LIRSCache::cachehits() {
	std::cout<< "calls: " << calls << ", hits: " << hits << ", readHits: " << readHits << ", writeHits: " <<  writeHits << ", evictedDirtyPage: " << evictedDirtyPage << std::endl;


	std::ofstream result("ExperimentalResult.txt", std::ios_base::app);
	if (result.is_open()) { 
		result <<  "LIRS " << "CacheSize " << capPulse << " calls " << calls << " hits " << hits << " hitRatio " << float(hits)/calls << " readHits " << readHits << " readHitRatio " << float(readHits)/calls << " writeHits " << writeHits << " writeHitRatio " << float(writeHits)/calls << " evictedDirtyPage " << evictedDirtyPage << "\n" ;			
	}
	result.close();
}

void LIRSCache::refresh(){
	//when a new query is start, reset the "calls", "hits", and "migration" to zero
	calls = 0;
	hits = 0;
	migration = 0;
}

void LIRSCache::summary() {
	// print the number of total cache calls, hits, and data migration size
	std::cout << "the total number of cache hits is: " << total_hits << std::endl;
	std::cout << "the total number of total refered calls is " << total_calls << std::endl;
	std::cout << "the total data migration size into the optane is: " << ((double)total_migration) * 16 / 1024/ 1024 << "GB" << std::endl;

}

void LIRSCache::demoteBottomLIR()
{
    // scan from bottom
    for (auto rit = S.rbegin(); rit != S.rend(); ++rit) {
        long long k = *rit;
        PageEntry &pe = table[k];

        if (pe.isLIR) {
            // demote
            pe.isLIR = false;

            // Add to Q
            Q.push_front(k);
            Q_map[k] = Q.begin();

            LIR_size--;
            return;
        }
    }
}

void LIRSCache::evictFromQ()
{
    while ((int)Q.size() > HIR_cap) {
        long long victim = Q.back();
        Q.pop_back();
        Q_map.erase(victim);

		if(accessType[victim] == "Write")
			evictedDirtyPage++;

        PageEntry &pe = table[victim];
        pe.isResident = false;
    }
}

void LIRSCache::prune() {
    while (!S.empty()) {
        long long y = S.back();
        PageEntry &py = table[y];

        // If nonresident → prune it
        if (!py.isResident) {
            S.pop_back();
            py.inS = false;
        }
        else {
            // Stop when we reach any resident page (LIR or HIR)
            break;
        }
    }
}
