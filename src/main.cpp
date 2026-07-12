#include "LRUCache.h"
#include "NaiveLRU.h"
#include "FIFOCache.h"
#include "BeladyCache.h"
#include "SLRU.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <random>

using namespace std;
namespace fs = std::filesystem;
using Clock = chrono::high_resolution_clock;
using Ms = chrono::duration<double, milli>;

// ── Helpers ───────────────────────────────────────────────────────────────────

vector<int> loadSequence(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error: Cannot open " << path << "\n";
        return {};
    }
    vector<int> seq;
    int x;
    while (file >> x) seq.push_back(x);
    cout << "Loaded " << seq.size() << " requests from " << path << "\n";
    return seq;
}

void appendCSV(ofstream& out, const string& algo, const string& seqName,
               int cacheSize, int hits, int misses, double timeMs)
{
    double hitRatio = (hits + misses > 0) ? static_cast<double>(hits) / (hits + misses) : 0.0;
    out << algo << "," << seqName << "," << cacheSize << ","
        << hits << "," << misses << ","
        << fixed << setprecision(4) << hitRatio << ","
        << fixed << setprecision(4) << timeMs << "\n";
}

struct SimResult { int hits, misses; double timeMs; };

SimResult runLRU(const vector<int>& seq, int k) {
    LRUCache cache(k);
    auto t0 = Clock::now();
    for (int req : seq) {
        if (cache.get(req) == -1) cache.put(req, req);
    }
    return {cache.hits(), cache.misses(), Ms(Clock::now() - t0).count()};
}

SimResult runNaiveLRU(const vector<int>& seq, int k) {
    NaiveLRU cache(k);
    auto t0 = Clock::now();
    for (int req : seq) {
        if (cache.get(req) == -1) cache.put(req, req);
    }
    return {cache.hits(), cache.misses(), Ms(Clock::now() - t0).count()};
}

SimResult runFIFO(const vector<int>& seq, int k) {
    FIFOCache cache(k);
    auto t0 = Clock::now();
    for (int req : seq) {
        if (cache.get(req) == -1) cache.put(req, req);
    }
    return {cache.hits(), cache.misses(), Ms(Clock::now() - t0).count()};
}

SimResult runBelady(const vector<int>& seq, int k) {
    BeladyCache cache(k, seq);
    auto t0 = Clock::now();
    auto [h, m] = cache.simulate();
    return {h, m, Ms(Clock::now() - t0).count()};
}

SimResult runSLRU(const vector<int>& seq, int k) {
    SLRU cache(k);
    auto t0 = Clock::now();
    for (int req : seq) {
        if (cache.get(req) == -1) cache.put(req, req);
    }
    return {cache.hits(), cache.misses(), Ms(Clock::now() - t0).count()};
}

// ── Hit/Miss Simulation ───────────────────────────────────────────────────────

void runHitMissSimulation(const vector<pair<string,string>>& datasets,
                          const vector<int>& cacheSizes) {
    fs::create_directories("results");
    ofstream hitCsv("results/hit_miss_ratios.csv");
    ofstream timeCsv("results/execution_times.csv");

    const string header = "Algorithm,Sequence,CacheSize,Hits,Misses,HitRatio,TimeMs\n";
    hitCsv << header;
    timeCsv << header;

    for (const auto& [dataPath, seqName] : datasets) {
        cout << "\nDataset: " << seqName << "\n";
        vector<int> seq = loadSequence(dataPath);
        if (seq.empty()) continue;

        for (int k : cacheSizes) {
            cout << "k=" << setw(3) << k << " ";

            auto lru    = runLRU(seq, k);
            auto naive  = runNaiveLRU(seq, k);
            auto fifo   = runFIFO(seq, k);
            auto belady = runBelady(seq, k);
            auto slru   = runSLRU(seq, k);

            for (auto& csv : {ref(hitCsv), ref(timeCsv)}) {
                appendCSV(csv, "OptimizedLRU", seqName, k, lru.hits,    lru.misses,    lru.timeMs);
                appendCSV(csv, "NaiveLRU",     seqName, k, naive.hits,  naive.misses,  naive.timeMs);
                appendCSV(csv, "FIFO",         seqName, k, fifo.hits,   fifo.misses,   fifo.timeMs);
                appendCSV(csv, "Belady",       seqName, k, belady.hits, belady.misses, belady.timeMs);
                appendCSV(csv, "SLRU",         seqName, k, slru.hits,   slru.misses,   slru.timeMs);
            }

            cout << " LRU:" << setprecision(1) << fixed << lru.hits*100.0/(lru.hits+lru.misses) << "%"
                 << " SLRU:" << slru.hits*100.0/(slru.hits+slru.misses) << "%"
                 << " FIFO:" << fifo.hits*100.0/(fifo.hits+fifo.misses) << "%"
                 << " Belady:" << belady.hits*100.0/(belady.hits+belady.misses) << "%\n";
        }
    }

    cout << "\nResults written to results/hit_miss_ratios.csv and results/execution_times.csv\n";
}

// ── Scalability Benchmark ─────────────────────────────────────────────────────
// Generates sequences in-memory at increasing sizes to prove O(1) LRU
// vs O(n*k) Belady as the request count scales up.

vector<int> generateInMemory(int n, int keyRange, int seed = 42) {
    mt19937 rng(seed);
    uniform_int_distribution<int> dist(0, keyRange - 1);
    vector<int> seq(n);
    for (auto& x : seq) x = dist(rng);
    return seq;
}

void runScalabilityBenchmark() {
    cout << "\n=== Scalability Benchmark (fixed k=16, varying request count) ===\n";

    const int CACHE_SIZE = 16;
    const int KEY_RANGE  = 100;
    // Request counts: 10K to 1M
    const vector<int> sizes = {10000, 50000, 100000, 250000, 500000, 1000000};

    fs::create_directories("results");
    ofstream csv("results/scalability.csv");
    csv << "Algorithm,RequestCount,TimeMs\n";

    cout << setw(12) << "Requests"
         << setw(12) << "LRU(ms)"
         << setw(12) << "SLRU(ms)"
         << setw(12) << "FIFO(ms)"
         << setw(14) << "Belady(ms)"
         << setw(14) << "NaiveLRU(ms)"
         << "\n";

    for (int n : sizes) {
        vector<int> seq = generateInMemory(n, KEY_RANGE);

        auto lru    = runLRU(seq, CACHE_SIZE);
        auto slru   = runSLRU(seq, CACHE_SIZE);
        auto fifo   = runFIFO(seq, CACHE_SIZE);
        auto belady = runBelady(seq, CACHE_SIZE);
        auto naive  = runNaiveLRU(seq, CACHE_SIZE);

        csv << "OptimizedLRU," << n << "," << fixed << setprecision(3) << lru.timeMs    << "\n";
        csv << "SLRU,"         << n << "," << fixed << setprecision(3) << slru.timeMs   << "\n";
        csv << "FIFO,"         << n << "," << fixed << setprecision(3) << fifo.timeMs   << "\n";
        csv << "Belady,"       << n << "," << fixed << setprecision(3) << belady.timeMs << "\n";
        csv << "NaiveLRU,"     << n << "," << fixed << setprecision(3) << naive.timeMs  << "\n";

        cout << setw(12) << n
             << setw(11) << fixed << setprecision(2) << lru.timeMs    << "ms"
             << setw(11) << slru.timeMs   << "ms"
             << setw(11) << fifo.timeMs   << "ms"
             << setw(13) << belady.timeMs << "ms"
             << setw(13) << naive.timeMs  << "ms"
             << "\n";
    }

    cout << "\nScalability results written to results/scalability.csv\n";
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    const vector<int> cacheSizes = {2, 4, 8, 12, 16, 24, 32, 48, 64};
    const vector<pair<string,string>> datasets = {
        {"data/uniform_random_seq.txt", "uniform_random"},
        {"data/high_locality_seq.txt",  "high_locality"}
    };

    cout << "=== Hit/Miss Simulation ===\n";
    runHitMissSimulation(datasets, cacheSizes);

    cout << "\n";
    runScalabilityBenchmark();

    cout << "\nNext step: python3 scripts/generate_plots.py\n";
    return 0;
}
