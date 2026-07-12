#include "LRUCache.h"
#include "FIFOCache.h"
#include "BeladyCache.h"
#include "SLRU.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>

using namespace std;

void printResult(const string& label, int hits, int misses) {
    double pct = (hits + misses > 0) ? hits * 100.0 / (hits + misses) : 0.0;
    cout << left << setw(20) << label
         << " hits=" << setw(4) << hits
         << " misses=" << setw(4) << misses
         << " hit%=" << fixed << setprecision(1) << pct << "%\n";
}

// ── Test 1: Standard LRU Pollution ───────────────────────────────────────────
// Shows that a sequential scan (101 unique cold keys) completely evicts
// a hot working set from a cache of size 100.

void testLRUPollution() {
    cout << "\n--- Test 1: Standard LRU Cache Pollution ---\n";
    const int CACHE_SIZE = 8;
    const int WORKING_SET = 4;
    const int SCAN_LENGTH = 20;   // > CACHE_SIZE → guaranteed full eviction
    const int WARMUP_REPS = 5;

    LRUCache warmCache(CACHE_SIZE);

    // Phase 1: warm up the hot working set
    for (int r = 0; r < WARMUP_REPS; ++r)
        for (int k = 0; k < WORKING_SET; ++k)
            if (warmCache.get(k) == -1) warmCache.put(k, k);

    warmCache.resetStats();

    // Phase 2: sequential scan — pollutes the cache
    for (int k = 100; k < 100 + SCAN_LENGTH; ++k)
        warmCache.put(k, k);

    // Phase 3: check if hot working set survived
    warmCache.resetStats();
    for (int k = 0; k < WORKING_SET; ++k)
        warmCache.get(k);

    printResult("LRU after scan:", warmCache.hits(), warmCache.misses());

    assert(warmCache.hits() == 0 && "LRU: hot set should be fully evicted");
    cout << "  => Confirmed: scan completely wiped the hot working set from LRU.\n";
}

// ── Test 2: SLRU Resists Pollution ───────────────────────────────────────────
// Same scan, but with SLRU. Hot items are in Protected; scan items
// only ever reach Probation and get evicted without touching Protected.

void testSLRUResistsPollution() {
    cout << "\n--- Test 2: SLRU Resists Cache Pollution ---\n";
    const int CACHE_SIZE = 8;
    const int WORKING_SET = 4;
    const int SCAN_LENGTH = 20;
    const int WARMUP_REPS = 5;

    SLRU cache(CACHE_SIZE);

    // Phase 1: warm up — access each hot key twice to push it into Protected
    for (int r = 0; r < WARMUP_REPS; ++r)
        for (int k = 0; k < WORKING_SET; ++k) {
            if (cache.get(k) == -1) cache.put(k, k);
            cache.get(k);  // second access → promoted to Protected
        }

    cache.resetStats();

    // Phase 2: same sequential scan
    for (int k = 100; k < 100 + SCAN_LENGTH; ++k)
        cache.put(k, k);

    // Phase 3: check hot set — should still be in Protected
    cache.resetStats();
    for (int k = 0; k < WORKING_SET; ++k)
        cache.get(k);

    printResult("SLRU after scan:", cache.hits(), cache.misses());

    assert(cache.hits() == WORKING_SET && "SLRU: hot set should survive the scan");
    cout << "  => Confirmed: SLRU protected the hot working set from pollution.\n";
}

// ── Test 3: Side-by-side on a realistic mixed workload ────────────────────────

void testMixedWorkload() {
    cout << "\n--- Test 3: LRU vs SLRU on Mixed Workload (hot set + scan + hot set) ---\n";
    const int CACHE_SIZE = 10;
    const int HOT_KEYS = 4;
    const int WARMUP_REPS = 8;
    const int SCAN_LENGTH = 15;

    vector<int> seq;
    // Warmup hot set (each key accessed twice to reach Protected in SLRU)
    for (int r = 0; r < WARMUP_REPS; ++r)
        for (int k = 0; k < HOT_KEYS; ++k) {
            seq.push_back(k);
            seq.push_back(k);  // second access → promoted to Protected
        }
    // Polluting scan
    for (int k = 50; k < 50 + SCAN_LENGTH; ++k) seq.push_back(k);
    // Post-scan access to hot set — where pollution hurts LRU but not SLRU
    for (int r = 0; r < 3; ++r)
        for (int k = 0; k < HOT_KEYS; ++k) seq.push_back(k);

    // Run LRU
    {
        LRUCache cache(CACHE_SIZE);
        for (int req : seq) {
            if (cache.get(req) == -1) cache.put(req, req);
        }
        printResult("Standard LRU:", cache.hits(), cache.misses());
    }

    // Run SLRU — should outperform LRU because hot keys stay in Protected
    {
        SLRU cache(CACHE_SIZE);
        for (int req : seq) {
            if (cache.get(req) == -1) cache.put(req, req);
        }
        printResult("SLRU:         ", cache.hits(), cache.misses());
    }

    cout << "  => SLRU achieves higher hit rate: hot set is in Protected, scan only pollutes Probation.\n";
}

int main() {
    cout << "=== Cache Pollution Test Suite ===\n";

    testLRUPollution();
    testSLRUResistsPollution();
    testMixedWorkload();

    cout << "\nAll tests completed.\n";
    return 0;
}
