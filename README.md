# Cache Replacement Algorithm Analysis

**Course:** Advanced Algorithm Analysis | **Language:** C++ (C++17) | **Tooling:** CMake, Python 3

A rigorous comparative study of five cache replacement algorithms — Optimized LRU, Segmented LRU, Naive LRU, FIFO, and Belady's Optimal — benchmarked across two workload types with formal proof tests and matplotlib visualization.

---

## Background

A cache stores a small, fast subset of a large, slow dataset. When a requested item is not in the cache (a *miss*), it must be fetched from the slower source, then stored in the cache. When the cache is full and a new item must be inserted, the *eviction policy* decides which existing item to remove.

The central question of this study: **which eviction policy is best, and under what conditions?**

This project implements five policies at different levels of optimality and practicality, then uses empirical benchmarks and formal proof tests to answer that question.

---

## Algorithms

### 1. Optimized LRU — `O(1)`
The primary implementation. Evicts the item that has not been accessed for the longest time (*Least Recently Used*). Implemented using a **Doubly Linked List** with dummy sentinel nodes paired with an `unordered_map`. Every `get` and `put` is a constant number of pointer rewires — strictly O(1). LRU exploits **temporal locality**: items used recently are likely to be used again.

### 2. Segmented LRU (SLRU) — `O(1)`
An upgraded LRU that solves its primary weakness. Splits the cache into two zones:
- **Probation (20%):** all new items land here
- **Protected (80%):** items promoted here after a second access

A sequential scan of cold items never escapes Probation, so the hot working set in the Protected zone survives. This makes SLRU immune to **cache pollution** — the scenario where a one-time scan evicts all frequently-accessed data.

### 3. Naive LRU — `O(k)`
Implements the same LRU eviction policy using a `std::vector`. Every `get` and `put` requires a linear scan and element shift — O(k) time. Hit rates are **identical** to Optimized LRU. Included purely to demonstrate that the data structure choice is as important as the algorithmic choice.

### 4. FIFO — `O(1)`
Evicts the item that has been in the cache the longest, regardless of how recently it was accessed. Uses a `std::queue` and `unordered_map`. Simple and fast, but **blind to access frequency**. Critically, FIFO is susceptible to **Belady's Anomaly**: increasing cache size can increase cache misses.

### 5. Belady's Optimal — Offline (Theoretical Benchmark)
Requires the complete future request sequence before processing a single request. At each eviction, it looks ahead and removes the item whose next access is farthest in the future. This guarantees the **mathematically minimum number of cache misses** for any given sequence.

> **Belady's is not a real competitor.** It is the theoretical ceiling — included to show what perfection looks like and how close the online algorithms come. No online algorithm can ever match it, because the future is unknown in all real systems.

### Complexity Summary

| Algorithm | `get` | `put` | Space | Type | Belady's Anomaly |
|---|---|---|---|---|---|
| Optimized LRU | O(1) | O(1) | O(k) | Online | Immune |
| Segmented LRU | O(1) | O(1) | O(k) | Online | Immune |
| Naive LRU | O(k) | O(k) | O(k) | Online | Immune |
| FIFO | O(1) | O(1) | O(k) | Online | **Susceptible** |
| Belady's Optimal | O(k) | O(k) | O(n+k) | **Offline** | Immune |

---

## Key Findings

### LRU beats FIFO — Belady's Anomaly proof

Using the reference sequence `[3, 2, 1, 0, 3, 2, 4, 3, 2, 1, 0, 4]`:

| Algorithm | k=3 misses | k=4 misses | Outcome |
|---|---|---|---|
| FIFO | 9 | **10** | More cache = more misses (anomaly) |
| LRU | 10 | **8** | Monotonically improves (immune) |

LRU is a **stack algorithm** — the set of items cached at size k is always a subset of those cached at size k+1. This property mathematically guarantees immunity to Belady's Anomaly. FIFO does not have this property.

### SLRU beats LRU — Cache pollution proof

| Algorithm | Hit rate after a polluting scan | Working set survives? |
|---|---|---|
| Standard LRU | **0%** | No — hot set fully evicted |
| SLRU | **100%** | Yes — hot set intact in Protected zone |

A scan of 20 unique cold keys through a cache of size 8 completely destroys LRU's hot working set. SLRU traps those items in Probation and the Protected zone is untouched.

### LRU beats Belady's — Execution time

At 1,000,000 requests with k=16:

| Algorithm | Execution Time |
|---|---|
| FIFO | ~33ms |
| **Optimized LRU** | **~39ms** |
| SLRU | ~43ms |
| Belady's | **~62ms** (+60% slower) |
| Naive LRU | ~19ms* |

*NaiveLRU appears fast here only because k=16 is small. O(k) cost dominates at larger k.

Belady's pre-processes the entire future sequence in memory and performs an O(k) scan per eviction. LRU's O(1) constant-time pointer operations make it significantly faster at scale.

---

## Repository Structure

```
LRU_Cache/
├── CMakeLists.txt
├── Makefile
├── README.md
│
├── include/
│   ├── LRUCache.h
│   ├── NaiveLRU.h
│   ├── FIFOCache.h
│   ├── BeladyCache.h
│   └── SLRU.h
│
├── src/
│   ├── LRUCache.cpp
│   ├── NaiveLRU.cpp
│   ├── FIFOCache.cpp
│   ├── BeladyCache.cpp
│   ├── SLRU.cpp
│   └── main.cpp                  # Simulation driver + scalability benchmark
│
├── tests/
│   ├── test_beladys_anomaly.cpp  # Formal proof: FIFO anomaly & LRU immunity
│   └── test_cache_pollution.cpp  # Formal proof: LRU pollution & SLRU defence
│
├── scripts/
│   ├── generate_testcases.py     # Generates input sequences in data/
│   ├── generate_plots.py         # Outputs individual PNGs to results/plots/
│   └── generate_combined_plot.py # Outputs combined dashboard PNG
│
├── data/
│   ├── uniform_random_seq.txt
│   └── high_locality_seq.txt
│
└── results/
    ├── hit_miss_ratios.csv
    ├── execution_times.csv
    ├── scalability.csv
    └── plots/

```

---

## Requirements

**C++:** CMake ≥ 3.16, GCC or Clang with C++17

**Python:** Python 3.9+
```bash
pip install matplotlib
```

---

## Build

```bash
# Build all binaries
make

# Clean and rebuild from scratch
make clean && make
```

Binaries are placed in `build/bin/`: `cache_sim`, `test_anomaly`, `test_pollution`.

---

## Running the Simulation

### Step 1 — Generate input data
```bash
python3 scripts/generate_testcases.py
```
Creates two files in `data/`:
- `uniform_random_seq.txt` — 10,000 requests with no locality (worst case)
- `high_locality_seq.txt` — 10,000 requests with 80% hot-set access (real-world model)

### Step 2 — Run the simulation
```bash
make run
```
Sweeps cache sizes `k = 2, 4, 8, 12, 16, 24, 32, 48, 64` for all five algorithms, and runs a scalability benchmark from 10K to 1M requests. Writes results to `results/`.

### Step 3 — Generate plots
```bash
python3 scripts/generate_plots.py         # individual PNGs
python3 scripts/generate_combined_plot.py # combined dashboard
```

### Full pipeline (one command)
```bash
make clean && make && \
python3 scripts/generate_testcases.py && \
make run && \
python3 scripts/generate_plots.py && \
python3 scripts/generate_combined_plot.py
```

---

## Running the Tests

```bash
# Belady's Anomaly and LRU immunity proof
./build/bin/test_anomaly

# Cache pollution proof: LRU vs SLRU
./build/bin/test_pollution
```

Both test binaries use hard `assert()` statements. If any assertion fails the process exits non-zero. A clean exit with "All tests passed" means all proofs hold.

---

## Understanding the Output

### Hit Rate vs. Cache Size graphs
Belady's sits at the top as the unreachable theoretical ceiling. SLRU is the top online algorithm on high-locality workloads — it outperforms standard LRU at small-to-medium cache sizes because the Protected zone retains the hot working set more aggressively. Optimized LRU consistently beats FIFO. NaiveLRU overlaps exactly with Optimized LRU — they have the same hit rate because the eviction policy is identical; only the speed differs.

### Scalability graph
The x-axis is on a log scale from 10K to 1M requests, with cache size fixed at k=16. This graph is the empirical proof for LRU's O(1) advantage. LRU, SLRU, and FIFO rise nearly linearly. Belady's rises more steeply — it must pre-process the full sequence and scan all k cached keys per eviction. NaiveLRU rises steepest at large n because of its O(k) per-operation cost.

---

## Documentation

| File | Contents |
|---|---|
| `docs/algorithm_pseudocode.txt` | Standard academic pseudocode for all 5 algorithms with complexity annotations |
| `docs/implementation_analysis.txt` | Step-by-step proof traces, design decisions, workload analysis, and real-world relevance |