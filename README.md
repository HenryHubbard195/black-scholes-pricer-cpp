# Black-Scholes Option Pricer — C++

A C++17 implementation of the Black-Scholes-Merton option pricing model, including analytical Greeks, Newton-Raphson implied volatility and automated numerical validation.

## Overview

This project implements a Black-Scholes option pricing engine in C++ and validates the analytical outputs against numerical finite-difference approximations.

The project was developed to explore the implementation of quantitative finance models in a compiled language, with particular focus on numerical correctness and computational performance.

## Features

- European call and put option pricing
- Analytical calculation of:
  - Delta
  - Gamma
  - Vega
  - Theta
  - Rho
- Newton-Raphson implied volatility solver
- Put-call parity validation
- Numerical finite-difference validation of Greeks
- Automated C++ test suite
- Python vs C++ performance benchmark
- Identical 100,000-option dataset used for both implementations

## Mathematical Model

For a European call option:

$$
C = S N(d_1) - Ke^{-rT}N(d_2)
$$

For a European put option:

$$
P = Ke^{-rT}N(-d_2) - SN(-d_1)
$$

where

$$
d_1 =
\frac{
\ln(S/K) + (r + \frac{1}{2}\sigma^2)T
}{
\sigma\sqrt{T}
}
$$

and

$$
d_2 = d_1 - \sigma\sqrt{T}
$$

where:

- $S$ = underlying asset price
- $K$ = strike price
- $T$ = time to maturity
- $r$ = continuously compounded risk-free rate
- $\sigma$ = volatility
- $N(\cdot)$ = standard normal cumulative distribution function

## Greeks

The implementation calculates the standard Black-Scholes sensitivities:

- **Delta** — sensitivity to the underlying price
- **Gamma** — sensitivity of Delta to the underlying price
- **Vega** — sensitivity to volatility
- **Theta** — sensitivity to time decay
- **Rho** — sensitivity to the risk-free rate

The implementation uses conventional market units for Vega, Theta and Rho.

## Implied Volatility

Implied volatility is solved using the Newton-Raphson method:

$$
\sigma_{n+1}
=
\sigma_n -
\frac{V(\sigma_n)-V_{market}}
{\text{Vega}(\sigma_n)}
$$

The solver iterates until the pricing error falls below the specified tolerance or a maximum number of iterations is reached.

## Validation

The implementation includes tests for:

- Call price
- Put price
- Put-call parity
- Delta
- Gamma
- Vega
- Theta
- Rho

Analytical Greeks are independently checked using finite-difference approximations.

All tests pass.

## Performance Benchmark

The C++ and Python implementations were benchmarked using the **same 100,000 option inputs** over **10 runs**.

| Metric | Python | C++ |
|---|---:|---:|
| Options per run | 100,000 | 100,000 |
| Runs | 10 | 10 |
| Median runtime | 3.476408 s | 0.002614 s |
| Median per option | 34.764 μs | 0.026 μs |

The benchmark therefore measured approximately **1,330× lower runtime** for the C++ implementation on the test machine.

The first and last option prices were also identical between implementations, providing an additional check that both were processing the same inputs consistently.

## Project Structure

```text
black-scholes-pricer-cpp/
│
├── bs_pricer.hpp
├── bs_pricer.cpp
│
├── tests/
│   └── test_pricer.cpp
│
├── benchmark_cpp.cpp
├── benchmark_python.py
├── benchmark_inputs.csv
│
└── .gitignore
