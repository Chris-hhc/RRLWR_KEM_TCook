# RRLWR KEM Toom-Cook-4

This implementation uses Toom-Cook-4 polynomial multiplication for the ring
arithmetic path. Polynomial coefficients are represented with 16-bit storage,
and the current multiplication path accumulates Toom-Cook-4 products directly
into the row accumulator for `ring_mul_Awin`.

## Test CPU

CPU information was collected with `lscpu` on each benchmark machine.

### Reference machine: Intel Xeon E5-2686 v4 (Broadwell-EP)

| Field | Value |
| --- | --- |
| Architecture | x86_64 |
| CPU model | Intel(R) Xeon(R) CPU E5-2686 v4 @ 2.30GHz |
| Microarchitecture | **Broadwell-EP** (Xeon E5-2600 v4, codename Broadwell) |
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

### Additional machine: Intel Xeon E5-2666 v3 (Haswell-EP)

| Field | Value |
| --- | --- |
| Architecture | x86_64 |
| CPU model | Intel(R) Xeon(R) CPU E5-2666 v3 @ 2.90GHz |
| Microarchitecture | **Haswell-EP** (Xeon E5-2600 v3, codename Haswell; family 6, model 63) |
| CPUs | 2 |
| Cores per socket | 1 |
| Threads per core | 2 |
| Socket(s) | 1 |
| L1d cache | 32 KiB (1 instance) |
| L1i cache | 32 KiB (1 instance) |
| L2 cache | 256 KiB (1 instance) |
| L3 cache | 25 MiB (1 instance) |
| Hypervisor | Xen |
| AVX2 | supported |

The E5-2666 v3 results were measured on **Haswell-EP**, one generation older than
the reference **Broadwell-EP** E5-2686 v4, with smaller caches and SMT enabled
under Xen. Absolute cycle counts are not directly comparable across machines;
relative rankings (which implementation is faster) are still useful.

## Benchmark Notes

All cycle counts in the tables below are **medians**, not averages.

Run each speed binary **one at a time** on an otherwise idle machine. Running
multiple benchmarks in parallel (or back-to-back under load) inflates cycle
counts and is not representative.

Kyber timings use `indcpa_keypair` / `indcpa_enc` / `indcpa_dec` for PKE and
`kyber_keypair` / `kyber_encaps` / `kyber_decaps` for KEM.

Correctness on the E5-2666 v3 machine was verified with:

```sh
./test/unit_tests_KEM256
./test/unit_tests_KEM512
cd ../kyber/ref && ./test/test_vectors1024
```

## RRLWR-128 vs Kyber512

Commands:

```sh
./test/test_speed_KEM128
cd ../kyber/ref && ./test/test_speed512
```

### E5-2686 v4 / Broadwell-EP (reference)

| Stage | RRLWR-128 (median cycles) | Kyber512 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | **80,255** | 83,969 | RRLWR-128 |
| PKE encrypt | **93,730** | 111,796 | RRLWR-128 |
| PKE decrypt | **13,877** | 38,003 | RRLWR-128 |
| KEM keygen | 100,860 | **98,689** | Kyber512 |
| KEM encaps | **107,272** | 123,047 | RRLWR-128 |
| KEM decaps | **118,865** | 161,068 | RRLWR-128 |

### E5-2666 v3 / Haswell-EP

| Stage | RRLWR-128 (median cycles) | Kyber512 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | **68,201** | 84,168 | RRLWR-128 |
| PKE encrypt | **79,814** | 112,223 | RRLWR-128 |
| PKE decrypt | **11,625** | 36,823 | RRLWR-128 |
| KEM keygen | **85,962** | 99,036 | RRLWR-128 |
| KEM encaps | **91,383** | 122,168 | RRLWR-128 |
| KEM decaps | **100,961** | 159,191 | RRLWR-128 |

## RRLWR-256 vs Kyber1024

Commands:

```sh
./test/test_KEM256
./test/test_speed_KEM256
cd ../kyber/ref && ./test/test_speed1024
```

### E5-2686 v4 / Broadwell-EP (reference)

| Stage | RRLWR-256 (median cycles) | Kyber1024 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | **219,289** | 219,185 | RRLWR-256 |
| PKE encrypt | 264,350 | **258,425** | Kyber1024 |
| PKE decrypt | **46,206** | 63,337 | RRLWR-256 |
| KEM keygen | **248,376** | 255,353 | RRLWR-256 |
| KEM encaps | 285,234 | **275,709** | Kyber1024 |
| KEM decaps | **328,823** | 352,585 | RRLWR-256 |

### E5-2666 v3 / Haswell-EP

| Stage | RRLWR-256 (median cycles) | Kyber1024 (median cycles) | Faster |
| --- | ---: | ---: | --- |
| PKE keygen | **186,504** | 222,948 | RRLWR-256 |
| PKE encrypt | **225,532** | 280,814 | RRLWR-256 |
| PKE decrypt | **38,897** | 60,425 | RRLWR-256 |
| KEM keygen | **210,411** | 260,612 | RRLWR-256 |
| KEM encaps | **243,088** | 293,800 | RRLWR-256 |
| KEM decaps | **280,344** | 361,090 | RRLWR-256 |

## RRLWR-512 Standalone

Command:

```sh
./test/test_speed_KEM512
```

### E5-2686 v4 / Broadwell-EP (reference)

| Stage | Median cycles |
| --- | ---: |
| poly_mul_toom4 | 2,356 |
| ring_mul full | 673,348 |
| ring_mul_Awin full | 680,574 |
| ring_mul 1 coefficient | 40,048 |
| ring_mul_Awin 1 coefficient | 39,637 |
| PKE keygen | 731,765 |
| PKE encrypt | 894,538 |
| PKE decrypt | 165,226 |
| KEM keygen | 777,336 |
| KEM encaps | 932,767 |
| KEM decaps | 1,108,791 |

### E5-2666 v3 / Haswell-EP

| Stage | Median cycles |
| --- | ---: |
| poly_mul_toom4 | 2,061 |
| ring_mul full | 595,605 |
| ring_mul_Awin full | 580,690 |
| ring_mul 1 coefficient | 34,299 |
| ring_mul_Awin 1 coefficient | 34,162 |
| PKE keygen | 617,169 |
| PKE encrypt | 773,033 |
| PKE decrypt | 202,146 |
| KEM keygen | 653,259 |
| KEM encaps | 801,526 |
| KEM decaps | 954,137 |
