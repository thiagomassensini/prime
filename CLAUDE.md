# CLAUDE.md - AI Assistant Guide for Prime Number Conjectures Repository

## Repository Overview

**Project Name:** prime
**Description:** Prime Number Conjectures - A repository dedicated to prime number conjectures and related mathematical research
**Author:** Thiago Motta
**License:** MIT
**DOI:** 10.5281/zenodo.17685922

This is an academic mathematics repository focused on prime number theory and conjectures. The project is in its early stages and will be used for mathematical research, conjecture documentation, and potentially computational verification.

## Current Repository Structure

```
prime/
├── .git/                 # Git repository data
├── .zenodo.json         # Zenodo metadata for academic archiving and DOI
├── LICENSE              # MIT License (Copyright 2025 Thiago Motta)
└── README.md            # Basic project description with Zenodo badge
```

## Project Purpose and Scope

This repository is designed to:
- Document prime number conjectures and mathematical theories
- Provide computational tools for testing prime number hypotheses
- Maintain academic rigor through proper versioning and DOI registration
- Serve as an open-source research platform for number theory

## Development Guidelines for AI Assistants

### 1. Academic and Mathematical Standards

**Mathematical Rigor:**
- All mathematical statements must be precise and well-defined
- Use standard mathematical notation and terminology
- Include proper citations and references when building on existing work
- Clearly distinguish between proven theorems, conjectures, and hypotheses

**Documentation Standards:**
- Document all algorithms with their time/space complexity
- Provide mathematical proofs or justifications where applicable
- Include references to relevant number theory literature
- Use LaTeX notation for complex mathematical expressions

### 2. Code Conventions (When Implemented)

When code is added to this repository, follow these guidelines:

**Language Selection:**
- Python is recommended for mathematical computations (NumPy, SymPy, SciPy)
- Consider performance-critical sections in C/C++, Rust, or Julia
- Use Jupyter notebooks for exploratory analysis and visualization

**Code Quality:**
- Write clear, self-documenting code with meaningful variable names
- Include docstrings with mathematical context
- Add unit tests for computational functions
- Verify results against known prime sequences (OEIS)

**Performance Considerations:**
- Prime number operations can be computationally intensive
- Document algorithmic complexity
- Consider memoization and caching strategies
- Profile code before optimization

### 3. File Organization Guidelines

As the project grows, organize content as follows:

```
prime/
├── conjectures/          # Documentation of specific conjectures
├── proofs/              # Mathematical proofs (formal or computational)
├── algorithms/          # Implementation of prime-related algorithms
├── tests/               # Unit tests and verification scripts
├── notebooks/           # Jupyter notebooks for exploration
├── data/                # Computed sequences, test data
├── docs/                # Extended documentation, papers
├── references/          # Bibliography and related papers
└── utils/               # Helper functions and utilities
```

### 4. Git Workflow and Branch Conventions

**Branch Naming:**
- Feature branches: `claude/[feature-name]-[session-id]`
- Main development occurs on feature branches
- All branches must start with `claude/` prefix for AI-assisted work

**Commit Messages:**
- Use clear, descriptive commit messages
- Format: `[Category] Brief description`
- Categories: `Add`, `Update`, `Fix`, `Refactor`, `Document`, `Test`
- Example: `Add Goldbach conjecture verification algorithm`

**Git Operations:**
- Always use `git push -u origin <branch-name>` for pushing
- Fetch specific branches: `git fetch origin <branch-name>`
- Retry network operations up to 4 times with exponential backoff (2s, 4s, 8s, 16s)
- Never force push to main/master without explicit permission

### 5. Academic Integration

**Zenodo Integration:**
- The repository is registered with Zenodo for DOI generation
- Update `.zenodo.json` when adding significant contributions
- Consider version releases for major milestones
- Maintain proper metadata for academic citation

**Citation Guidelines:**
- When using this repository, cite using the DOI: 10.5281/zenodo.17685922
- Keep the Zenodo badge in README.md updated
- Document all external mathematical sources

### 6. Testing and Verification

**Mathematical Verification:**
- Cross-reference results with Online Encyclopedia of Integer Sequences (OEIS)
- Test against known prime sequences: 2, 3, 5, 7, 11, 13, ...
- Verify edge cases: 1 (not prime), 2 (only even prime), negative numbers
- Use probabilistic tests (Miller-Rabin) for large primes

**Computational Testing:**
- Include unit tests for all numerical functions
- Test boundary conditions and edge cases
- Verify against mathematical properties (e.g., Fundamental Theorem of Arithmetic)
- Document any computational limitations

### 7. Documentation Best Practices

**README Updates:**
- Keep README.md concise and focused on overview
- Link to detailed documentation in separate files
- Include usage examples for any implemented algorithms
- Maintain the Zenodo DOI badge

**Mathematical Documentation:**
- Use Markdown with LaTeX for mathematical expressions
- Provide intuitive explanations alongside formal definitions
- Include examples and counterexamples
- Reference theorem numbers and existing literature

**Code Documentation:**
```python
def is_prime(n):
    """
    Determines if n is a prime number using trial division.

    Mathematical Context:
        A prime number is a natural number greater than 1 that has
        no positive divisors other than 1 and itself.

    Algorithm:
        Trial division with optimization: only test divisors up to sqrt(n)

    Complexity:
        Time: O(sqrt(n))
        Space: O(1)

    Args:
        n (int): The number to test for primality

    Returns:
        bool: True if n is prime, False otherwise

    Examples:
        >>> is_prime(7)
        True
        >>> is_prime(1)
        False
    """
    # Implementation here
```

### 8. Common Prime Number Algorithms to Consider

When implementing prime-related functionality, consider these standard algorithms:

**Primality Testing:**
- Trial division (simple, O(sqrt(n)))
- Sieve of Eratosthenes (multiple primes, O(n log log n))
- Miller-Rabin (probabilistic, fast for large numbers)
- AKS (deterministic polynomial time, theoretical importance)

**Prime Generation:**
- Sieve methods for ranges
- Incremental search with primality testing
- Probabilistic methods for cryptographic primes

**Prime-Related Functions:**
- Prime counting function π(n)
- nth prime number
- Prime factorization
- Greatest common divisor (Euclid's algorithm)
- Euler's totient function φ(n)

### 9. Security and Ethics

**Open Science:**
- All research should be openly accessible
- Respect academic integrity and proper attribution
- Share negative results and failed approaches (valuable for research)

**Code Security:**
- No malicious code or backdoors
- Validate all user inputs if interactive tools are created
- Consider integer overflow in calculations with large numbers

### 10. AI Assistant Interaction Guidelines

**When Working on This Repository:**
1. **Always read existing files** before proposing changes
2. **Understand the mathematical context** before implementing algorithms
3. **Verify correctness** through testing and mathematical reasoning
4. **Maintain academic standards** in all documentation
5. **Keep changes focused** - don't over-engineer or add unnecessary features
6. **Document assumptions** and limitations clearly
7. **Use TodoWrite tool** for tracking multi-step mathematical/computational tasks

**Communication Style:**
- Be precise and technical when discussing mathematics
- Provide mathematical justification for implementations
- Cite sources and acknowledge uncertainty
- Focus on correctness over performance initially

**Task Management:**
- Use TodoWrite for complex mathematical derivations
- Track verification steps separately
- Mark tasks complete only after testing
- Break down proofs into logical steps

### 11. Resources and References

**Mathematical Resources:**
- OEIS (Online Encyclopedia of Integer Sequences): https://oeis.org/
- Prime Pages: https://t5k.org/
- MathWorld (Prime Numbers): https://mathworld.wolfram.com/PrimeNumber.html

**Computational Tools:**
- SymPy (symbolic mathematics in Python)
- SageMath (comprehensive mathematics software)
- PARI/GP (number theory computations)
- GMP (GNU Multiple Precision Arithmetic Library)

## Current Development Status

**Phase:** Initial Setup
**Last Updated:** 2025-11-23
**Current State:** Repository structure established, academic metadata configured

**Immediate Next Steps:**
- Define specific conjectures to investigate
- Set up computational environment (if needed)
- Create initial documentation structure
- Implement basic prime number utilities

## Questions or Issues?

This is a research repository. When contributing:
- Verify mathematical correctness first
- Test computational implementations thoroughly
- Document all assumptions and limitations
- Maintain academic integrity and proper attribution

## License

All code and documentation in this repository is licensed under the MIT License.
Copyright (c) 2025 Thiago Motta

---

**Note for AI Assistants:** This repository is in early development. Focus on mathematical correctness, clear documentation, and academic rigor. When in doubt about mathematical claims, indicate uncertainty and suggest verification approaches.
