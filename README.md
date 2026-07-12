# Optimal Caching Analysis: LRU vs Belady vs FIFO vs SLRU

A comparative performance analysis of cache replacement algorithms implemented in C++, with Python-based visualization. Built for an Advanced Algorithms university assignment.

---

## TL;DR — Who Actually Wins and Why

> **Belady's algorithm has the best hit rate on every graph. This is expected and intentional — it is a theoretical benchmark, not a real competitor.**

Belady's is an **offline** algorithm. It requires the complete future request sequence before processing a single step. In any real system — a web browser, database, or OS — the future is unknown. Belady's wins by cheating with a crystal ball.

The actual competition is between the **online** algorithms: LRU, SLRU, and FIFO.

| Comparison | Winner | Reason |
|---|---|---|
| LRU vs Belady (hit rate) | Belady | Belady cheats — it sees the future |
| LRU vs Belady (speed) | **LRU** | O(1) vs O(n·k); Belady is ~65% slower at 1M requests |
| LRU vs FIFO (hit rate) | **LRU** | LRU tracks recency; FIFO evicts blindly |
| LRU vs FIFO (anomaly) | **LRU** | LRU is immune to Belady's Anomaly; FIFO is not |
| LRU vs SLRU (pollution) | **SLRU** | SLRU's Protected zone survives scan floods |
| NaiveLRU vs OptimizedLRU | **Optimized** | Same hit rate, O(1) vs O(k) speed |

**How to read the graphs:**
- Belady's curve = the hard theoretical ceiling no algorithm can exceed
- LRU and SLRU = the practical winners for real-world deployment
- FIFO = the cautionary example (anomaly-prone, locality-blind)
- NaiveLRU = proof that data structure choice determines real-world performance

---

## Algorithms Implemented

| Algorithm | Type | Time Complexity | Data Structure |
|---|---|---|---|
| **Optimized LRU** | Online | O(1) | DLL + HashMap |
| **Segmented LRU (SLRU)** | Online | O(1) | Two-zone DLL + HashMap |
| **Naive LRU** | Online | O(k) | `std::vector` |
| **FIFO** | Online | O(1) | `std::queue` + HashMap |
| **Belady's Optimal** | **Offline (theoretical only)** | O(n) pre-process | Future-access Queue |

---

## Why LRU Wins

### Against Belady's: Speed is the argument

Belady's Farthest-in-Future requires seeing the entire future request sequence before running. This makes it impossible to deploy in any real system. While it achieves the minimum possible misses, the scalability benchmark (`results/scalability.csv`) shows its execution time grows ~65% faster than Optimized LRU at 1M requests. LRU's O(1) architecture processes each request instantly regardless of sequence length.

### Against FIFO: Belady's Anomaly

FIFO evicts the oldest item regardless of access frequency. Worse, it suffers from **Belady's Anomaly** — giving it more memory can *increase* cache misses. Using the reference sequence `[3,2,1,0,3,2,4,3,2,1,0,4]`:
- FIFO k=3 → 9 misses
- FIFO k=4 → **10 misses** (more memory = more misses — the anomaly)
- LRU k=3 → 10 misses
- LRU k=4 → **8 misses** (monotonically improves — mathematically guaranteed)

LRU is a **stack algorithm** and is formally proven immune to Belady's Anomaly.

### LRU's Weakness Acknowledged: Cache Pollution

Standard LRU fails during sequential scans. A background process reading 1000 unique files once evicts the entire hot working set. The fix is **SLRU**: 20% Probation zone + 80% Protected zone. New items enter Probation. Only after a second access are they promoted to Protected. Scan items (accessed once) never escape Probation.

Proof (`test_cache_pollution.cpp`):
- Standard LRU after scan: **0% hit rate** (hot set wiped — assert passes)
- SLRU after same scan: **100% hit rate** (hot set intact — assert passes)

---

## Repository Structure

```
LRU_Cache/
├── CMakeLists.txt
├── Makefile
├── README.md
├── include/
│   ├── LRUCache.h          # O(1) LRU — DLL + HashMap
│   ├── NaiveLRU.h          # O(k) LRU — vector baseline
│   ├── FIFOCache.h         # FIFO — anomaly demonstration
│   ├── BeladyCache.h       # Offline optimal — theoretical ceiling
│   └── SLRU.h              # Segmented LRU — pollution-resistant
├── src/
│   ├── LRUCache.cpp
│   ├── NaiveLRU.cpp
│   ├── FIFOCache.cpp
│   ├── BeladyCache.cpp
│   ├── SLRU.cpp
│   └── main.cpp            # Simulation driver + scalability benchmark
├── tests/
│   ├── test_beladys_anomaly.cpp   # Proves FIFO anomaly & LRU immunity
│   └── test_cache_pollution.cpp   # Proves SLRU resists scan pollution
├── scripts/
│   ├── generate_testcases.py
│   ├── generate_plots.py
│   └── generate_combined_plot.py
├── data/
│   ├── uniform_random_seq.txt
│   └── high_locality_seq.txt
├── results/
│   ├── hit_miss_ratios.csv
│   ├── execution_times.csv
│   ├── scalability.csv
│   └── plots/
└── docs/
    └── implementation_analysis.txt
```

---

## Requirements

- CMake ≥ 3.16, GCC/Clang with C++17
- Python 3.9+, `matplotlib` (`pip install matplotlib`)

---

## How to Build and Run

### Full pipeline (one command)

```bash
make clean && make && python3 scripts/generate_testcases.py && make run && python3 scripts/generate_plots.py && python3 scripts/generate_combined_plot.py
```

### Step by step

```bash
# 1. Build all binaries
make

# 2. Generate input sequences
python3 scripts/generate_testcases.py

# 3. Run the simulation (hit/miss sweep + scalability benchmark)
make run

# 4. Generate all plots
python3 scripts/generate_plots.py
python3 scripts/generate_combined_plot.py
```

---

## Running the Tests

```bash
# Proves FIFO Belady's Anomaly and LRU immunity (hard asserts)
./build/bin/test_anomaly

# Proves LRU cache pollution and SLRU defence (hard asserts)
./build/bin/test_pollution
```

| Test | Expected Output |
|---|---|
| FIFO k=3 | 9 misses |
| FIFO k=4 | 10 misses (anomaly confirmed) |
| LRU k=4 | 8 misses (immunity confirmed) |
| LRU after scan | 0% hit rate (pollution confirmed) |
| SLRU after scan | 100% hit rate (defence confirmed) |
| SLRU vs LRU mixed | SLRU wins by ~4-5% |

---

## Understanding the Graphs

### Hit Rate vs. Cache Size
Belady sits at the top as the unreachable ceiling. SLRU beats standard LRU at small-to-medium cache sizes on locality workloads. NaiveLRU and OptimizedLRU overlap exactly — the hit rate is identical because it is the same eviction policy.

### Scalability Graph (the key graph for the LRU argument)
The x-axis is on a log scale. LRU, SLRU, and FIFO stay relatively flat (O(1) per request). Belady's line rises more steeply because it must pre-process future indices. NaiveLRU rises fastest (O(k) linear scan per operation). This graph is the empirical proof that O(1) matters at scale.

---

## Algorithm Details

**Optimized LRU:** Doubly Linked List with dummy sentinel nodes + `unordered_map`. MRU at `head->next`, LRU at `tail->prev`. Constant-time pointer rewire on every operation.

**Segmented LRU:** 20% Probation + 80% Protected. Second hit promotes to Protected. Protected overflow demotes its LRU back to Probation. Scan items accessed once never escape Probation.

**Naive LRU:** `vector<pair<int,int>>` with linear scan. O(k) time. Identical hit rate to OptimizedLRU — exists only to show why HashMap+DLL is necessary.

**Belady's Optimal (offline, theoretical):** Pre-processes the full sequence to build per-key future-access queues. At eviction, picks the key whose next access is farthest away. Provably optimal but strictly requires full future knowledge.

**FIFO:** `queue` for insertion order + `unordered_map` for lookup. Evicts the oldest item regardless of recency. Susceptible to Belady's Anomaly.