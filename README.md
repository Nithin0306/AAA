# Optimal Caching Analysis: Evaluating LRU

## 📌 Project Overview
This repository contains a comprehensive implementation and performance analysis of the **Least Recently Used (LRU)** cache replacement algorithm. The project explores the theoretical foundations of cache maintenance, temporal locality, and offline vs. online algorithm paradigms.

To validate the efficiency of our optimized LRU implementation, we conduct rigorous comparative testing against three other caching strategies:
1. **Belady’s Farthest-in-Future:** The theoretical offline optimal algorithm.
2. **First-In-First-Out (FIFO):** A basic online baseline susceptible to Belady's Anomaly.
3. **Naive LRU:** An unoptimized $O(k)$ implementation used to demonstrate the necessity of proper data structure alignment.

## 📂 Repository Structure
```text
optimal-caching-analysis/
│
├── .gitignore                  # Ignores build/, data/, results/, and .DS_Store
├── CMakeLists.txt              # Top-level build configuration (Primary)
├── Makefile                    # Fallback build script
├── README.md                   # Main landing page (The markdown I provided earlier)
│
├── include/                    # C++ Headers (The "Interface")
│   ├── LRUCache.h
│   ├── NaiveLRU.h
│   ├── BeladyCache.h
│   └── FIFOCache.h
│
├── src/                        # C++ Source Code (The "Implementation")
│   ├── LRUCache.cpp
│   ├── NaiveLRU.cpp
│   ├── BeladyCache.cpp
│   ├── FIFOCache.cpp
│   └── main.cpp                # The simulation driver that runs everything
│
├── tests/                      # Unit Tests (Defends your edge cases)
│   ├── test_beladys_anomaly.cpp
│   └── test_cache_pollution.cpp
│
├── scripts/                    # Python Tooling (Moves them out of the root)
│   ├── generate_testcases.py
│   ├── generate_plots.py
│   └── generate_combined_plot.py
│
├── data/                       # Input Data (Keeps root clean)
│   ├── uniform_random_seq.txt
│   └── high_locality_seq.txt
│
├── results/                    # Output Data (Where your C++ driver saves its logs)
│   ├── execution_times.csv
│   ├── hit_miss_ratios.csv
│   └── plots/                  # Where Python saves the generated PNG graphs
│
└── docs/                       # Documentation
    └── report.pdf
```

## 🧠 Algorithms Implemented

### 1. Optimized LRU (Our Primary Implementation)
* **Architecture:** Combines a Doubly Linked List with a Hash Map (`std::unordered_map`).
* **Time Complexity:** $O(1)$ for both `get` and `put` operations.
* **Mechanism:** Evicts the item that has not been accessed for the longest time, operating on the principle of temporal locality.

### 2. Naive LRU (Implementation Baseline)
* **Architecture:** Standard Array / `std::vector`.
* **Time Complexity:** $O(k)$ due to linear searching and shifting elements upon eviction.

### 3. Belady's Algorithm (Theoretical Upper Bound)
* **Architecture:** Max-Priority Queue and Hash Map.
* **Mechanism:** An offline algorithm that looks ahead into the request sequence and evicts the item needed furthest in the future. Yields the absolute minimum possible cache misses.

### 4. FIFO (Online Baseline)
* **Architecture:** Standard Queue.
* **Mechanism:** Evicts the oldest item in the cache regardless of access frequency.