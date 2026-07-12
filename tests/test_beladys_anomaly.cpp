#include "FIFOCache.h"
#include "LRUCache.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>

using namespace std;

struct Result { int hits; int misses; };

Result runFIFO(const vector<int>& seq, int k) {
    FIFOCache cache(k);
    for (int req : seq) {
        if (cache.get(req) == -1) cache.put(req, req);
    }
    return {cache.hits(), cache.misses()};
}

Result runLRU(const vector<int>& seq, int k) {
    LRUCache cache(k);
    for (int req : seq) {
        if (cache.get(req) == -1) cache.put(req, req);
    }
    return {cache.hits(), cache.misses()};
}

void printRow(const string& algo, int k, int hits, int misses) {
    double ratio = (hits + misses > 0) ? hits * 100.0 / (hits + misses) : 0.0;
    cout << left << setw(12) << algo << " k=" << k
         << " hits=" << setw(3) << hits
         << " misses=" << setw(3) << misses
         << " hit%=" << fixed << setprecision(1) << ratio << "%\n";
}

void testFIFOAnomaly() {
    cout << "\nTest: Belady's Anomaly in FIFO\n";
    const vector<int> seq = {3, 2, 1, 0, 3, 2, 4, 3, 2, 1, 0, 4};

    auto [hits3, misses3] = runFIFO(seq, 3);
    auto [hits4, misses4] = runFIFO(seq, 4);

    printRow("FIFO", 3, hits3, misses3);
    printRow("FIFO", 4, hits4, misses4);

    if (misses4 > misses3) {
        cout << "Anomaly Confirmed: FIFO(k=4) has more misses than FIFO(k=3)\n";
    }

    assert(misses3 == 9);
    assert(misses4 == 10);
}

void testLRUImmunity() {
    cout << "\nTest: LRU Immunity to Belady's Anomaly\n";
    const vector<int> seq = {3, 2, 1, 0, 3, 2, 4, 3, 2, 1, 0, 4};

    auto [hits3, misses3] = runLRU(seq, 3);
    auto [hits4, misses4] = runLRU(seq, 4);

    printRow("LRU", 3, hits3, misses3);
    printRow("LRU", 4, hits4, misses4);

    if (misses4 <= misses3) {
        cout << "Immunity Confirmed: LRU does not exhibit the anomaly\n";
    }

    assert(misses4 <= misses3);
}

void traceFIFO_k3() {
    cout << "\nTest: Step-by-step FIFO trace (k=3)\n";
    const vector<int> seq = {3, 2, 1, 0, 3, 2, 4, 3, 2, 1, 0, 4};
    FIFOCache cache(3);
    
    int step = 1;
    for (int req : seq) {
        int result = cache.get(req);
        if (result == -1) {
            cache.put(req, req);
            cout << "Step " << step << " req=" << req << " MISS\n";
        } else {
            cout << "Step " << step << " req=" << req << " HIT\n";
        }
        ++step;
    }
}

int main() {
    testFIFOAnomaly();
    testLRUImmunity();
    traceFIFO_k3();
    cout << "\nAll tests passed.\n";
    return 0;
}
