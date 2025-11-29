# 2-Adic Thermodynamics of Prime Numbers

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.17759738.svg)](https://doi.org/10.5281/zenodo.17759738)

A novel framework connecting binary structure, statistical mechanics, and prime number theory.

## Overview

This project presents a complete theory linking:

- **Binary operations (XOR)** → Classification of primes into families
- **2-adic valuation** → Stratification by trailing zeros
- **Boltzmann statistics** → Thermodynamic interpretation
- **Hardy-Littlewood** → Emergent connection (99.9% accuracy)

## Key Results

### Theorem: 2-Adic Stratification of Odd Primes

Every odd prime P belongs to exactly one of three disjoint families:

| Family | Condition | Signature (T, K) |
|--------|-----------|------------------|
| **C_twin** | (P, P+2) are twins | K = 2^{T+1} - 2 |
| **C_≥2** | Non-twin, v₂(P-1) ≥ 2 | K = 2 |
| **C_T=1** | Non-twin, v₂(P-1) = 1 | K = 2^m - 2, m ≥ 3 |

Where:
- T = v₂(P+1) for twins, v₂(P-1) otherwise (trailing zeros = 2-adic valuation)
- K = P ⊕ (P+2) for twins, (P-1) ⊕ (P+1) otherwise (XOR signature)

**This is a proven theorem, not a conjecture.**

### Empirical Discovery: Boltzmann Distribution

The distribution of T-values follows:

```
P(T = k) = 2^{-k}
```

This is exactly a **Boltzmann distribution** with:
- Energy levels: E_k = k · ε
- Temperature: kT = 1/ln(2) ≈ 1.44

Verified with 0.05% error on 43M twin primes.

### Independent Discovery: Connection to Hardy-Littlewood

The thermodynamic framework was developed **independently**, without prior knowledge of the Hardy-Littlewood conjecture. Only after the empirical analysis revealed the gap distribution pattern did we discover that our results align with Hardy-Littlewood:

```
kT(p) = ln²(p) / (2C₂) - 20
```

| Parameter | H-L Theoretical | Our Empirical | Agreement |
|-----------|-----------------|---------------|-----------|
| Slope | 0.7575 | 0.7566 | 99.9% |
| R² | - | 0.9997 | - |

**This independent convergence suggests a deep structural connection between binary/2-adic properties and classical analytic number theory.**

## The Hierarchy

```
                     BITS (0 and 1)
                          │
            ┌─────────────┴─────────────┐
            │                           │
            ▼                           ▼
      v₂(p+1) = k                  p mod 30
            │                           │
            ▼                           ▼
      P(k) = 2^{-k}             class ∈ {11,17,29}
      kT = 1/ln(2)                      │
            │                           ▼
            │                    gap_min(c₁→c₂)
            │                           │
            └───────────┬───────────────┘
                        │
                        ▼
          P(g) ∝ exp(-(g-gap_min)/kT(p))
                        │
                        ▼
      ┌─────────────────┴─────────────────┐
      │                                   │
      ▼                                   ▼
 PRIMES (|H|=1)                    TWINS (|H|=2)
 kT ~ ln(p)                        kT ~ ln²(p)/(2C₂)
 R² = 0.9999                       R² = 0.9997
      │                                   │
      └─────────────────┬─────────────────┘
                        │
                        ▼
           GENERAL LAW: kT_H = (ln p)^|H| / C_H
                        │
                        ▼
               HARDY-LITTLEWOOD
```

## Proven Results vs Empirical Observations

### ✓ Proven (Theorem)
- **XOR Classification**: Every odd prime belongs to exactly one of three families (C_twin, C_≥2, C_T=1)
- **XOR-T Relationship**: For twin primes, K = 2^{T+1} - 2

### ◐ Empirical (High confidence, verified on 43M+ primes)
- **Boltzmann Distribution**: P(T=k) = 2^{-k} for v₂(p+1)
- **Hardy-Littlewood Connection**: kT ≈ 0.757 × ln²(p) with R² = 0.9997

### ○ Interpretive Framework
- **Thermodynamic Analogy**: Treating gaps as "energies" - useful model, not physical claim

## Papers

### Published

1. **2-Adic Stratification of Odd Primes via XOR Signatures**
   - DOI: [10.5281/zenodo.17759739](https://doi.org/10.5281/zenodo.17759739)
   - Proves the three-family classification theorem

### In Preparation

2. **Thermodynamics of Prime Numbers: From Boltzmann to Hardy-Littlewood**
3. **2-Adic L-Functions and Correlations with Riemann Zeros**
4. **Fibonacci as Attractor for Sophie Germain Primes**

## Key Equations

### XOR Signature for Twin Primes
```
K = P ⊕ (P+2) = 2^{T+1} - 2
```

### Boltzmann Distribution
```
P(T = k) = 2^{-k} = e^{-k·ln(2)} = e^{-βE_k}
```

### Gap Distribution
```
P(g | H, p) = (1/kT_H(p)) × exp(-(g - gap_min) / kT_H(p))
```

### Temperature Scaling Law
```
kT_H(p) = (ln p)^|H| / C_H
```

### Twin Prime Temperature
```
kT_twin(p) = ln²(p) / (2C₂) ≈ 0.757 × ln²(p)
```

## The Fundamental Constant

```
ln(2) connects: Bits ↔ Shannon ↔ Boltzmann ↔ Number Theory
```

## Repository Structure

```
prime/
├── README.md                 # Project overview (this file)
├── MANUAL.md                 # Technical documentation
├── LICENSE                   # MIT License
│
├── papers/
│   └── 2adic_stratification_primes.pdf
│
├── src/
│   └── thermodynamics.cpp    # Main C++ implementation
│
└── data/                     # (optional) Sample outputs
    └── sample_output.csv
```

## Author

**Thiago Fernandes Motta Massensini Silva**  
📧 thiago@massensini.com.br

## Citation

```bibtex
@software{massensini_2adic_2025,
  author       = {Massensini Silva, Thiago Fernandes Motta},
  title        = {2-Adic Thermodynamics of Prime Numbers},
  year         = 2025,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.17759738},
  url          = {https://doi.org/10.5281/zenodo.17759738}
}
```

## License

MIT License

---

*"The thermodynamics of primes is Hardy-Littlewood written in the language of Boltzmann."*