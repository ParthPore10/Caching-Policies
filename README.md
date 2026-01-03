# Modern Caching Policies

## Overview

This project presents a **comprehensive experimental evaluation of modern cache replacement policies** under diverse real-world I/O workloads. The objective is to understand how **workload characteristics and cache size** influence cache performance, and why certain algorithms outperform others in specific scenarios.

The study goes beyond traditional policies like LRU by analyzing **reuse-aware, adaptive, and learning-based caching strategies**, highlighting their strengths and limitations in dynamic systems.

**Key focus areas:**
- Systems performance
- Memory efficiency
- Workload-aware algorithm design

---

## Caching Policies Implemented

All policies were implemented and evaluated using **C++**, with modular designs to ensure fair comparison.

| Policy | Description |
|------|------------|
| **LRU (Least Recently Used)** | Evicts the block that has not been accessed for the longest time. Simple but prone to scan pollution. |
| **LFU (Least Frequently Used)** | Retains blocks with high access frequency; struggles when access patterns shift. |
| **LIRS (Low Inter-reference Recency Set)** | Tracks reuse distance instead of recency, making it resistant to sequential scans. |
| **ARC (Adaptive Replacement Cache)** | Dynamically balances recency and frequency using multiple LRU lists. |
| **CACHEUS** | Learning-based caching policy that predicts future reuse from historical access patterns. |

---

## Experimental Setup

### Workloads Evaluated

The caching policies were tested on **realistic storage and metadata workloads**, each exhibiting distinct access behaviors:

- **hm_0, hm_1** – Write-intensive workloads with strong temporal locality  
- **mds_0, mds_1** – Metadata-heavy workloads with irregular and scan-like access patterns  
- **proj_3** – Heterogeneous workload with mixed short- and long-term reuse  

---

### Cache Configurations

- Cache sizes evaluated at **small, medium, and large capacities**
- Sizes ranged from approximately **1% to 10% of working set size**
- Cache size plotted on a **logarithmic scale** to capture meaningful performance transitions

---

### Metrics

**Cache Hit Ratio**

Hit Ratio = Cache Hits / Total Accesses


A higher hit ratio implies:
- Lower average latency
- Reduced I/O traffic
- Improved overall system performance

---

## Results & Key Observations

### Effect of Cache Size

- Increasing cache size improves hit ratio across all policies, but **at different rates**
- Performance gains are most significant at **small and medium cache sizes**
- Once the cache can hold most of the working set, performance differences between policies diminish

---

### Policy Behavior by Workload

#### LRU
- Performs well for workloads with **strong temporal locality**
- Suffers from **cache pollution** in scan-heavy workloads

#### LFU
- Excels when **hot data remains stable**
- Adapts slowly to changing access patterns

#### LIRS
- Consistently strong across **mixed and scan-heavy workloads**
- Effectively distinguishes reusable blocks from one-time accesses

#### ARC
- Delivers **robust performance across diverse workloads**
- Automatically adapts between recency and frequency without manual tuning

#### CACHEUS
- Achieves top performance in **complex and non-stationary workloads**
- Learning overhead can reduce efficiency at very small cache sizes

---

## Comparative Summary

| Policy | Strengths | Weaknesses | Best Use Case |
|------|---------|-----------|--------------|
| LRU | Simple, efficient | Scan pollution | Strong temporal locality |
| LFU | Retains hot data | Poor adaptability | Stable access patterns |
| LIRS | Scan-resistant | Metadata overhead | Mixed / irregular workloads |
| ARC | Self-tuning | Moderate complexity | Dynamic workloads |
| CACHEUS | Predictive, adaptive | Learning overhead | Highly variable workloads |

**Key takeaway:**  
> No single caching policy is optimal for all workloads.

---

## Key Takeaways

- Cache performance depends on **both cache size and workload behavior**
- Adaptive and reuse-aware policies (**LIRS, ARC**) provide the most consistent performance
- Learning-based caching (**CACHEUS**) highlights the potential of ML-driven systems design
- Effective caching is fundamentally a **systems problem**, not just an algorithmic one

---

## Why This Project Matters

This project demonstrates:
- Strong **C++ systems programming**
- Understanding of **memory hierarchy and performance tradeoffs**
- Experience evaluating algorithms under **realistic workloads**
- Relevance to **ML infrastructure, retrieval systems, and high-performance computing**

These concepts directly apply to:
- ML inference systems
- Retrieval pipelines (RAG)
- Database engines
- OS-level caching
- Distributed systems

---

## Tech Stack

- **Language:** C++  
- **Focus Areas:** Cache design, performance analysis, workload modeling  
- **Metrics:** Hit ratio, eviction behavior, workload sensitivity  

