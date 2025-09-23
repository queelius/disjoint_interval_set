# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is a C++ header-only library implementing Disjoint Interval Sets (DIS), a Boolean algebra over sets of disjoint intervals with standard set-theoretic operations (union, intersection, complement).

## Key Architecture

- **Header-only library**: All code is in `include/disjoint_interval_set/`
- **Template-based design**: The main class `disjoint_interval_set<I>` is parameterized by interval type
- **Core components**:
  - `interval.hpp`: Base interval type modeling mathematical intervals with membership testing
  - `disjoint_interval_set.hpp`: Main DIS implementation with set operations
  - `disjoint_interval_set_algorithms.hpp`: Supporting algorithms for DIS operations
  - `disjoint_interval_set_parser.hpp`: Parsing utilities for DIS

## Development Workflow

Since this is a header-only library without build files or test infrastructure currently configured:

- **To compile test files**: Use standard C++ compiler with includes
  ```bash
  g++ -std=c++17 -I./include test.cpp -o test
  ```

- **Language standard**: C++17 or later required (uses `std::optional`)

## Design Principles

- The library models Boolean algebra concepts mathematically
- Intervals support open/closed boundaries: `(a,b)`, `[a,b]`, `(a,b]`, `[a,b)`
- Operations maintain disjoint interval invariants automatically
- Template design allows different numeric types (integers, reals)