# Contributing to DIS

We welcome contributions! This guide will help you get started.

## Ways to Contribute

- 🐛 **Report bugs**: Open an issue describing the problem
- 💡 **Suggest features**: Share your ideas for improvements
- 📝 **Improve documentation**: Fix typos, add examples, clarify explanations
- 🔧 **Submit code**: Implement features or fix bugs
- ✅ **Write tests**: Increase test coverage
- 📊 **Benchmarking**: Compare performance with other libraries

## Development Setup

### Clone Repository

```bash
git clone https://github.com/yourusername/disjoint_interval_set.git
cd disjoint_interval_set
```

### Build Tests

```bash
mkdir build && cd build
cmake ..
make
ctest
```

## Code Style

Follow these guidelines:

- **C++ Standard**: C++17 minimum, prefer C++20 features when available
- **Naming**: `snake_case` for functions/variables, `PascalCase` for types
- **Const correctness**: Mark methods `const` when they don't modify state
- **Documentation**: Use Doxygen-style comments for public APIs
- **Testing**: Write comprehensive tests for new features

## Pull Request Process

1. **Fork** the repository
2. **Create branch**: `git checkout -b feature/amazing-feature`
3. **Commit changes**: `git commit -m 'Add amazing feature'`
4. **Push**: `git push origin feature/amazing-feature`
5. **Open PR**: Submit pull request with clear description

## License

By contributing, you agree your contributions will be licensed under the MIT License.
