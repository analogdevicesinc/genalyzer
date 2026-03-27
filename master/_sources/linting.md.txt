# Code Quality Tools

Genalyzer uses automated linting and formatting tools to maintain consistent code quality across both C++ and Python codebases. These checks run automatically in CI on every push and pull request.

## C++ Formatting: clang-format

All C and C++ source files are formatted with [clang-format](https://clang.llvm.org/docs/ClangFormat.html) using the project's `.clang-format` configuration. The configuration is based on the LLVM style with project-specific customizations including tab-based indentation and C++20 language standard.

### Running locally

```bash
# Check formatting (dry run)
find include/ src/ bindings/c/ -name '*.hpp' -o -name '*.h' -o -name '*.cpp' -o -name '*.c' | xargs clang-format --dry-run --Werror

# Apply formatting
find include/ src/ bindings/c/ -name '*.hpp' -o -name '*.h' -o -name '*.cpp' -o -name '*.c' | xargs clang-format -i
```

### Pre-commit hook

clang-format also runs as a [pre-commit](https://pre-commit.com/) hook. Install the hooks with:

```bash
pip install pre-commit
pre-commit install
```

After installation, clang-format runs automatically on staged C/C++ files before each commit.

## Python Linting: Ruff

[Ruff](https://docs.astral.sh/ruff/) is used for Python linting and formatting. It replaces several traditional tools (flake8, isort, pyupgrade) with a single fast linter. Configuration is in `bindings/python/pyproject.toml` under `[tool.ruff]`.

### Enabled rule sets

| Rule set | Description |
|----------|-------------|
| `E`, `W` | pycodestyle errors and warnings |
| `F`      | pyflakes (unused imports, undefined names) |
| `I`      | isort (import sorting) |
| `UP`     | pyupgrade (Python version upgrades) |
| `B`      | flake8-bugbear (common bug patterns) |
| `SIM`    | flake8-simplify (code simplification) |
| `RUF`    | Ruff-specific rules |

### Running locally

```bash
cd bindings/python

# Check for lint errors
ruff check genalyzer/

# Auto-fix safe issues
ruff check --fix genalyzer/

# Check formatting
ruff format --check genalyzer/

# Apply formatting
ruff format genalyzer/
```

## Python Type Checking: ty

[ty](https://docs.astral.sh/ty/) is used for Python type checking. It is an extremely fast type checker from the same team that builds Ruff. Configuration is in `bindings/python/pyproject.toml` under `[tool.ty]`.

The type checker is configured to ignore unresolved imports (third-party dependencies may not be installed in all environments) and relaxes rules for legacy modules (`simplified_beta`, `helpers`, `pytest`).

### Running locally

```bash
cd bindings/python

# Run type checks
ty check genalyzer/
```

## CI Integration

All three tools run automatically in CI through the GitHub Actions workflow defined in `.github/workflows/lint.yml`. The workflow runs on every push and pull request with three parallel jobs:

1. **clang-format** - Verifies C/C++ formatting
2. **ruff** - Checks Python linting and formatting
3. **ty** - Runs Python type checking

Additionally, the main test workflow (`.github/workflows/test.yml`) includes a combined lint job that runs pre-commit hooks, Ruff, and ty together.

## Adding new rules

To enable additional Ruff rules, add the rule code to the `select` list in `bindings/python/pyproject.toml`. See the [Ruff rule reference](https://docs.astral.sh/ruff/rules/) for available rules.

To adjust ty strictness, modify the `[tool.ty.rules]` section. See the [ty rule reference](https://docs.astral.sh/ty/rules/) for available rules.
