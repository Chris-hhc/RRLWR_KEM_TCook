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
| PKE keygen | 87,140 | **84,023** | Kyber512 |
| PKE encrypt | **105,555** | 111,851 | RRLWR-128 |
| PKE decrypt | **20,645** | 37,984 | RRLWR-128 |
| KEM keygen | 108,115 | **99,434** | Kyber512 |
| KEM encaps | **119,382** | 123,198 | RRLWR-128 |
| KEM decaps | **136,771** | 161,816 | RRLWR-128 |

## RRLWR-256 vs Kyber1024

Commands:

```sh
./test/test_KEM256
./test/test_speed_KEM256
./test/test_speed1024
```

| Stage | RRLWR-256 (median cycles) | Kyber1024 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | 237,057 | **218,874** | Kyber1024 |
| PKE encrypt | 287,678 | **268,328** | Kyber1024 |
| PKE decrypt | **59,175** | 63,355 | RRLWR-256 |
| KEM keygen | 267,196 | **265,411** | Kyber1024 |
| KEM encaps | 308,703 | **287,059** | Kyber1024 |
| KEM decaps | 365,250 | **353,471** | Kyber1024 |

## RRLWR-512 Standalone

Command:

```sh
./test/test_speed_KEM512
```

| Stage | Median cycles |
| --- | ---: |
| poly_mul_toom4 | 2,469 |
| ring_mul full | 678,939 |
| ring_mul_Awin full | 682,018 |
| ring_mul 1 coefficient | 42,167 |
| ring_mul_Awin 1 coefficient | 39,585 |
| PKE keygen | 768,004 |
| PKE encrypt | 959,852 |
| PKE decrypt | 191,204 |
| KEM keygen | 809,794 |
| KEM encaps | 978,641 |
| KEM decaps | 1,191,063 |

