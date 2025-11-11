# DIS Documentation

This directory contains the MkDocs documentation for the Disjoint Interval Set library.

## Building the Documentation

### Prerequisites

Install MkDocs and the Material theme:

```bash
# Using pip
pip install mkdocs mkdocs-material mkdocs-minify-plugin

# Or using conda
conda install -c conda-forge mkdocs mkdocs-material
```

### Local Development Server

Run a local development server with live reload:

```bash
# From the repository root
mkdocs serve

# Then open http://127.0.0.1:8000 in your browser
```

### Build Static Site

Generate the static HTML site:

```bash
# From the repository root
mkdocs build

# Output will be in ./site/
```

### Deploy to GitHub Pages

Deploy to GitHub Pages:

```bash
mkdocs gh-deploy
```

## Documentation Structure

```
docs/
├── index.md                          # Landing page
├── getting-started.md                # Quick start guide
├── installation.md                   # Installation instructions
├── examples.md                       # Complete examples
├── technical-report/                 # Academic technical report
│   ├── introduction.md
│   ├── mathematical-foundation.md
│   ├── design-implementation.md
│   ├── performance.md
│   ├── applications.md
│   └── related-work.md
├── api/                              # API reference
│   ├── interval.md
│   ├── disjoint-interval-set.md
│   ├── parser-formatter.md
│   ├── multi-dimensional.md
│   └── type-aliases.md
├── guide/                            # User guides
│   ├── basic-operations.md
│   ├── set-operations.md
│   ├── string-dsl.md
│   ├── functional-programming.md
│   └── compile-time-intervals.md
└── about/                            # About section
    ├── contributing.md
    ├── license.md
    └── citation.md
```

## Configuration

The site configuration is in `mkdocs.yml` at the repository root. Key settings:

- **Theme**: Material for MkDocs (modern, responsive)
- **Extensions**: Math rendering (MathJax), syntax highlighting, search
- **Navigation**: Organized into logical sections
- **Features**: Dark mode toggle, search suggestions, code copy button

## Adding New Pages

1. Create a new Markdown file in the appropriate directory
2. Add it to the `nav` section in `mkdocs.yml`
3. Commit and push your changes

## Writing Documentation

### Code Blocks

````markdown
```cpp
#include <dis/dis.hpp>
using namespace dis;

auto interval = real_interval::closed(0, 10);
```
````

### Mathematical Notation

Use LaTeX syntax for math:

```markdown
Inline: \(a \cup b\)

Display:
\[
A \cup B = \{x : x \in A \text{ or } x \in B\}
\]
```

### Admonitions

```markdown
!!! note
    This is a note.

!!! warning
    This is a warning.

!!! tip
    This is a tip.
```

### Links

```markdown
[Link text](page.md)
[External link](https://example.com)
[Section link](#section-name)
```

## Troubleshooting

### MathJax not rendering

Ensure `docs/javascripts/mathjax.js` exists and is referenced in `mkdocs.yml`.

### Build warnings about missing pages

Check that all pages referenced in `mkdocs.yml` navigation exist.

### Theme not loading

Install the Material theme: `pip install mkdocs-material`

## Contributing

See [Contributing Guide](about/contributing.md) for how to contribute documentation.
