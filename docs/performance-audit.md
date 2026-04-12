# Performance Audit: Series & Rower Hot Path

**Date:** 2026-04-10
**Target:** ESP32-S3 (WEMOS LOLIN S3 Mini)
**Environment:** `genericAir-lolinS3-mini`
**Hot path:** `StrokeService::processData()` — called per flywheel impulse
**Benchmark:** `micros()` timer (1µs resolution), 5291 iterations per `IMPULSE_DATA_ARRAY_LENGTH` value (3–21), using `rapid-benchmark` stash

## Baseline Results

| N | Avg (µs) | Max (µs) |
| --- | --- | --- |
| 3 | 377 | 829 |
| 4 | 452 | 904 |
| 5 | 532 | 986 |
| 6 | 629 | 1080 |
| 7 | 744 | 1197 |
| 8 | 890 | 1322 |
| 9 | 1053 | 1520 |
| 10 | 1203 | 1661 |
| 11 | 1420 | 1901 |
| 12 | 1666 | 2108 |
| 13 | 1883 | 2334 |
| 14 | 2081 | 2535 |
| 15 | 2376 | 2936 |
| 16 | 2798 | 3416 |
| 17 | 3078 | 3592 |
| 18 | 3443 | 4190 |
| 19 | 3669 | 4397 |
| 20 | 4154 | 5163 |
| 21 | 4891 | 5862 |

Production profiles typically use N=6–10 (629–1203µs avg).

---

## Attempt 1: Flatten `vector<vector>` to stride-based 1D buffer

**Hypothesis:** Replacing `vector<vector<precision>>` (slopes in TSLinearSeries, seriesA in TSQuadraticSeries) with a flat 1D buffer using stride-based indexing would improve CPU cache locality and reduce pointer-chasing overhead.

**Implementation:**

- Replaced `vector<vector<precision>> slopes` with `vector<precision> flatSlopes` + `vector<unsigned char> rowSizes` + stride = maxSeriesLength - 1
- Same approach for TSQuadraticSeries seriesA with stride = maxSeriesAInnerLength
- Added `medianScratch` pre-allocated buffer to avoid per-call heap allocation in `median()`
- Cached `TSLinearSeries linearResidue` as a member in TSQuadraticSeries (reused via `softReset()` instead of constructing a new instance every `push()`)

**Results (flat buffer + medianScratch + cached linearResidue):**

| N | Baseline (µs) | Optimized (µs) | Δ (%) |
| --- | --- | --- | --- |
| 3 | 377 | 297 | -21% |
| 4 | 452 | 392 | -13% |
| 5 | 532 | 476 | -11% |
| 6 | 629 | 578 | -8% |
| 7 | 744 | 700 | -6% |
| 8 | 890 | 879 | -1% |
| 9 | 1053 | 1077 | +2% |
| 10 | 1203 | 1273 | +6% |
| 11 | 1420 | 1547 | +9% |
| 12 | 1666 | 1881 | +13% |
| 13 | 1883 | 2195 | +17% |
| 14 | 2081 | 2506 | +20% |
| 15 | 2376 | 2929 | +23% |
| 16 | 2798 | 3521 | +26% |
| 17 | 3078 | 3981 | +29% |
| 18 | 3443 | 4550 | +32% |
| 19 | 3669 | 5008 | +36% |
| 20 | 4154 | 5856 | +41% |
| 21 | 4891 | 7464 | +53% |

**Verdict:** REJECTED — severe regression at N≥9 (up to +53%)

**Root cause:** The erase-from-front operation on the flat buffer moves far more data than `vector<vector>`:

- `vector<vector>` erase: moves `(N-1)` vector objects = `(N-1) × 12` bytes on 32-bit (just pointer/size/capacity structs)
- Flat buffer erase: moves `(N-1) × stride × sizeof(precision)` bytes (all data including padding for unused row slots)

For N=15 with double precision:

- Original: 14 × 12 = 168 bytes moved
- Flat buffer: 14 × 14 × 8 = 1,568 bytes moved (9.3× more)

For TSQuadraticSeries with stride = maxSeriesAInnerLength = 91 at N=15: 14 × 91 × 8 = 10,192 bytes — 60× more data movement.

The cache locality benefit of contiguous storage exists but is completely overwhelmed by the memmove cost during erase-from-front.

---

## Attempt 2: medianScratch + cached linearResidue only (no flattening)

**Hypothesis:** After finding the flat buffer caused regression, we isolated the remaining two optimizations to measure their individual impact:

- Pre-allocated `mutable medianScratch` buffers in Series, TSLinearSeries, and TSQuadraticSeries to avoid heap alloc/dealloc on every `median()` call
- Cached `TSLinearSeries linearResidue` as member in TSQuadraticSeries, reused via `softReset()` instead of constructing+destructing per `push()`

**Results (medianScratch + cached linearResidue only):**

| N | Baseline avg | B+C avg | Δ avg | Baseline max | B+C max | Δ max |
| --- | --- | --- | --- | --- | --- | --- |
| 3 | 377 | 370 | -1.9% | 829 | 831 | +0.2% |
| 4 | 452 | 453 | +0.2% | 904 | 911 | +0.8% |
| 5 | 532 | 535 | +0.5% | 986 | 999 | +1.3% |
| 6 | 629 | 631 | +0.3% | 1080 | 1097 | +1.6% |
| 7 | 744 | 745 | +0.1% | 1197 | 1209 | +1.0% |
| 8 | 890 | 890 | 0.0% | 1322 | 1340 | +1.4% |
| 9 | 1053 | 1053 | 0.0% | 1520 | 1522 | +0.1% |
| 10 | 1203 | 1202 | -0.1% | 1661 | 1651 | -0.6% |
| 11 | 1420 | 1419 | 0.0% | 1901 | 1868 | -1.7% |
| 12 | 1666 | 1664 | -0.1% | 2108 | 2095 | -0.6% |
| 13 | 1883 | 1884 | 0.0% | 2334 | 2336 | +0.1% |
| 14 | 2081 | 2080 | -0.1% | 2535 | 2531 | -0.2% |
| 15 | 2376 | 2375 | -0.1% | 2936 | 2948 | +0.4% |
| 16 | 2798 | 2799 | 0.0% | 3416 | 3408 | -0.2% |
| 17 | 3078 | 3078 | 0.0% | 3592 | 3599 | +0.2% |
| 18 | 3443 | 3445 | +0.1% | 4190 | 4180 | -0.2% |
| 19 | 3669 | 3671 | 0.0% | 4397 | 4406 | +0.2% |
| 20 | 4154 | 4155 | 0.0% | 5163 | 5165 | 0.0% |
| 21 | 4891 | 4892 | 0.0% | 5862 | 5865 | +0.1% |

**Verdict:** REJECTED — no measurable improvement (all values within ±2% noise)

**Root cause:** The ESP32's heap allocator is fast enough that saving a few alloc/dealloc calls per `processData()` invocation produces sub-microsecond differences — below the `micros()` timer's 1µs resolution. The allocations being eliminated are small (tens to hundreds of elements) and the allocator likely serves them from a fast-path slab or similar mechanism.

---

## Other approaches evaluated (not benchmarked)

### Circular buffer for Series

Previously benchmarked and the current approach of `vector::erase(begin())` + `push_back()` (shift-based FIFO) outperforms a circular buffer implementation. The compiler/runtime optimizes the memmove for small contiguous buffers effectively on ESP32-S3.

### `clear()` vs swap-to-empty in `reset()`

Previously benchmarked and swap-to-empty (`vector<T> tmp; tmp.reserve(n); v.swap(tmp)`) is faster than `v.clear()` for forcing deallocation. The current codebase already uses this pattern.

---

## Key Takeaways

1. **`vector<vector>` erase-from-front is cheap on ESP32 (32-bit):** erasing the first inner vector only moves `(N-1) × 12` bytes (vector object = pointer + size + capacity = 12 bytes on 32-bit). Flat buffer alternatives move orders of magnitude more data.

2. **ESP32 heap allocator has low overhead for small allocations:** eliminating per-call `vector` construction/destruction doesn't produce measurable throughput gains at 1µs timer resolution.

3. **The hot path is compute-bound, not allocation-bound:** the O(N²) and O(N³) slope/coefficient calculations dominate execution time. Algorithmic complexity improvements (if possible without changing the Theil-Sen statistical method) would be the only path to significant gains.

4. **Benchmark before committing:** the "obvious" optimization (flatten for cache locality) was actually a significant regression due to the specific access pattern (erase-from-front with large stride).
