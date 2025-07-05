# Git Commit Rules

This project follows the Conventional Commits specification for commit messages.

## Format

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

## Types

- **feat**: A new feature
- **fix**: A bug fix
- **docs**: Documentation only changes
- **style**: Changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc)
- **refactor**: A code change that neither fixes a bug nor adds a feature
- **perf**: A code change that improves performance
- **test**: Adding missing tests or correcting existing tests
- **build**: Changes that affect the build system or external dependencies
- **ci**: Changes to our CI configuration files and scripts
- **chore**: Other changes that don't modify src or test files
- **revert**: Reverts a previous commit

## Examples

```
feat: add user authentication system
fix: resolve memory leak in string processing
docs: update API documentation
style: format code according to PSR-12
refactor: extract common utility functions
perf: optimize string iteration algorithm
test: add unit tests for striter functions
build: update PHP extension build configuration
ci: add automated testing workflow
chore: update dependencies
```

## Breaking Changes

For breaking changes, add `!` after the type/scope:

```
feat!: change API signature for striter_process
```

Or include `BREAKING CHANGE:` in the footer:

```
feat: add new string processing mode

BREAKING CHANGE: The default behavior of striter_process has changed
```