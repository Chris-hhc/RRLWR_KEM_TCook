# RRLWR KEM Toom-Cook-4

This implementation uses Toom-Cook-4 polynomial multiplication for the ring
arithmetic path. Polynomial coefficients are represented with 16-bit storage,
and the current multiplication path accumulates Toom-Cook-4 products directly
into the row accumulator for `ring_mul_Awin`.

## Test CPU

The following CPU information was collected with `lscpu` on the benchmark
machine:

| Field | Value |
| --- | --- |
| Architecture | x86_64 |
| CPU model | Intel(R) Xeon(R) CPU E5-2686 v4 @ 2.30GHz |
| CPUs | 2 |
| Cores per socket | 2 |
| Threads per core | 1 |
| Socket(s) | 1 |
| L1d cache | 64 KiB (2 instances) |
| L1i cache | 64 KiB (2 instances) |
| L2 cache | 512 KiB (2 instances) |
| L3 cache | 45 MiB (1 instance) |
| Hypervisor | Xen |
| AVX2 | supported |

## Benchmark Notes

All cycle counts in the tables below are **medians**, not averages.

## RRLWR-128 vs Kyber512

Commands:

```sh
./test/test_speed_KEM128
./test/test_speed512
```

| Stage | RRLWR-128 (median cycles) | Kyber512 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | **80,172** | 83,969 | RRLWR-128 |
| PKE encrypt | **93,325** | 111,796 | RRLWR-128 |
| PKE decrypt | **13,656** | 38,003 | RRLWR-128 |
| KEM keygen | 100,219 | **98,689** | Kyber512 |
| KEM encaps | **107,358** | 123,047 | RRLWR-128 |
| KEM decaps | **117,859** | 161,068 | RRLWR-128 |

## RRLWR-256 vs Kyber1024

Commands:

```sh
./test/test_KEM256
./test/test_speed_KEM256
./test/test_speed1024
```

| Stage | RRLWR-256 (median cycles) | Kyber1024 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | **218,028** | 219,185 | RRLWR-256 |
| PKE encrypt | 266,399 | **258,425** | Kyber1024 |
| PKE decrypt | **45,844** | 63,337 | RRLWR-256 |
| KEM keygen | **247,174** | 255,353 | RRLWR-256 |
| KEM encaps | 291,551 | **275,709** | Kyber1024 |
| KEM decaps | **338,536** | 352,585 | RRLWR-256 |

## RRLWR-512 Standalone

Command:

```sh
./test/test_speed_KEM512
```

| Stage | Median cycles |
| --- | ---: |
| poly_mul_toom4 | 2,340 |
| ring_mul full | 672,744 |
| ring_mul_Awin full | 674,885 |
| ring_mul 1 coefficient | 40,109 |
| ring_mul_Awin 1 coefficient | 39,481 |
| PKE keygen | 731,093 |
| PKE encrypt | 897,261 |
| PKE decrypt | 164,760 |
| KEM keygen | 773,990 |
| KEM encaps | 935,741 |
| KEM decaps | 1,106,077 |

