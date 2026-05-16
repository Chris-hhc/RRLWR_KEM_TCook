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
| PKE keygen | **80,267** | 84,023 | RRLWR-128 |
| PKE encrypt | **97,328** | 111,851 | RRLWR-128 |
| PKE decrypt | **17,489** | 37,984 | RRLWR-128 |
| KEM keygen | 102,161 | **99,434** | Kyber512 |
| KEM encaps | **110,618** | 123,198 | RRLWR-128 |
| KEM decaps | **126,699** | 161,816 | RRLWR-128 |

## RRLWR-256 vs Kyber1024

Commands:

```sh
./test/test_KEM256
./test/test_speed_KEM256
./test/test_speed1024
```

| Stage | RRLWR-256 (median cycles) | Kyber1024 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | 219,476 | **218,874** | Kyber1024 |
| PKE encrypt | 272,207 | **268,328** | Kyber1024 |
| PKE decrypt | **52,870** | 63,355 | RRLWR-256 |
| KEM keygen | **249,103** | 265,411 | RRLWR-256 |
| KEM encaps | 293,646 | **287,059** | Kyber1024 |
| KEM decaps | **343,752** | 353,471 | RRLWR-256 |

## RRLWR-512 Standalone

Command:

```sh
./test/test_speed_KEM512
```

| Stage | Median cycles |
| --- | ---: |
| poly_mul_toom4 | 2,456 |
| ring_mul full | 685,578 |
| ring_mul_Awin full | 675,845 |
| ring_mul 1 coefficient | 42,321 |
| ring_mul_Awin 1 coefficient | 39,481 |
| PKE keygen | 737,629 |
| PKE encrypt | 924,429 |
| PKE decrypt | 177,968 |
| KEM keygen | 775,953 |
| KEM encaps | 947,984 |
| KEM decaps | 1,138,326 |

