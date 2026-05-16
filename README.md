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
| PKE keygen | **80,206** | 83,889 | RRLWR-128 |
| PKE encrypt | **94,074** | 111,697 | RRLWR-128 |
| PKE decrypt | **14,064** | 37,953 | RRLWR-128 |
| KEM keygen | 101,160 | **98,811** | Kyber512 |
| KEM encaps | **107,502** | 123,238 | RRLWR-128 |
| KEM decaps | **119,339** | 161,981 | RRLWR-128 |

## RRLWR-256 vs Kyber1024

Commands:

```sh
./test/test_KEM256
./test/test_speed_KEM256
./test/test_speed1024
```

| Stage | RRLWR-256 (median cycles) | Kyber1024 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | 219,221 | **218,613** | Kyber1024 |
| PKE encrypt | 264,464 | **258,267** | Kyber1024 |
| PKE decrypt | **46,258** | 63,391 | RRLWR-256 |
| KEM keygen | **245,766** | 255,706 | RRLWR-256 |
| KEM encaps | 286,856 | **276,026** | Kyber1024 |
| KEM decaps | **333,117** | 352,327 | RRLWR-256 |

## RRLWR-512 Standalone

Command:

```sh
./test/test_speed_KEM512
```

| Stage | Median cycles |
| --- | ---: |
| poly_mul_toom4 | 2,447 |
| ring_mul full | 681,177 |
| ring_mul_Awin full | 679,813 |
| ring_mul 1 coefficient | 42,219 |
| ring_mul_Awin 1 coefficient | 39,867 |
| PKE keygen | 733,713 |
| PKE encrypt | 905,348 |
| PKE decrypt | 167,370 |
| KEM keygen | 785,944 |
| KEM encaps | 933,390 |
| KEM decaps | 1,128,110 |

