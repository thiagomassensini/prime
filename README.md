# Prime Number Conjectures

A mathematical research repository exploring non-standard L-functions encoding twin prime distribution through 2-adic valuations.

## Overview

This repository contains research on a novel Dirichlet series that attempts to encode the distribution of twin primes via 2-adic structure. Unlike classical L-functions, this "mutant" L-function lacks multiplicativity but exhibits interesting analytic properties.

## Paper

The main research paper `paper.tex` introduces and analyzes:

- **Z_twin(s)**: A Dirichlet series over twin primes weighted by 2-adic valuations
- **Convergence analysis**: Holomorphic properties for Re(s) > 1
- **Structural analysis**: Why this fails to be a classical L-function
- **Numerical observations**: Evidence for approximate functional equation behavior
- **2-adic interpretation**: Connection to the 2-adic tree structure

### Building the Paper

Requires LaTeX with standard packages (amsmath, amsthm, hyperref, etc.):

```bash
make            # Build paper.pdf
make clean      # Remove auxiliary files
make distclean  # Remove all generated files
```

Or manually:
```bash
pdflatex paper.tex
bibtex paper
pdflatex paper.tex
pdflatex paper.tex
```

## Mathematical Content

The central object of study is:

```
Z_twin(s) = Σ_{p twin} 2^{-v_2(p+1)} · p^{-s}
```

where:
- The sum runs over twin primes p (pairs (p, p+2))
- v_2(n) is the 2-adic valuation of n
- The weighting encodes depth in the 2-adic tree near -1

## Key Results

1. **Convergence**: Absolute convergence for Re(s) > 1
2. **Non-multiplicativity**: No standard Euler product exists
3. **Phase triviality**: Originally proposed phase factor e^{2πi v_2(p+1)} equals 1
4. **Approximate symmetry**: Numerical evidence for functional equation-like behavior

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Citation

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.17685922.svg)](https://doi.org/10.5281/zenodo.17685922)
