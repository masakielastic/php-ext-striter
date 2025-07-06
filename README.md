# PHP String Iterator Extension

A PHP extension that provides advanced string iteration capabilities for UTF-8 strings with support for grapheme clusters, Unicode codepoints, and byte-level iteration.

## Features

- **Grapheme Cluster Iteration**: Iterate over grapheme clusters (user-perceived characters) using PCRE2
- **Unicode Codepoint Iteration**: Iterate over individual Unicode codepoints
- **Byte-level Iteration**: Iterate over individual bytes for low-level string processing
- **UTF-8 Safe**: Proper handling of multibyte UTF-8 characters
- **Standard PHP Interfaces**: Implements Iterator, IteratorAggregate, and Countable interfaces for seamless integration

## Installation

### Requirements

- PHP 7.0 or higher
- PCRE2 library (libpcre2-dev)

### Build from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libpcre2-dev

# Build extension
phpize
./configure --enable-striter
make
make install
```

### Enable Extension

Add to your php.ini:
```ini
extension=striter.so
```

## Usage

### Basic Usage

```php
<?php
// Create a string iterator
$iterator = str_iter("Hello World");

// Iterate using foreach
foreach ($iterator as $index => $char) {
    echo "[$index] => '$char'\n";
}
```

### Iteration Modes

#### Grapheme Mode (Default)
Iterates over grapheme clusters (user-perceived characters):

```php
<?php
$text = "Hello🌍";
$iterator = str_iter($text, "grapheme");

foreach ($iterator as $index => $char) {
    echo "[$index] => '$char'\n";
}
// Output:
// [0] => 'H'
// [1] => 'e'
// [2] => 'l'
// [3] => 'l'
// [4] => 'o'
// [5] => '🌍'
```

#### Codepoint Mode
Iterates over individual Unicode codepoints:

```php
<?php
$text = "Hello🌍";
$iterator = str_iter($text, "codepoint");

foreach ($iterator as $index => $char) {
    echo "[$index] => '$char'\n";
}
```

#### Byte Mode
Iterates over individual bytes:

```php
<?php
$text = "Hello";
$iterator = str_iter($text, "byte");

foreach ($iterator as $index => $byte) {
    echo "[$index] => '" . ord($byte) . "'\n";
}
```

### Using Countable Interface

```php
<?php
$text = "Hello🌍";
$iterator = str_iter($text, "grapheme");

echo "Total characters: " . count($iterator) . "\n"; // Output: 6
```

### Using IteratorAggregate Interface

```php
<?php
$text = "ABC";
$iterator = str_iter($text);

// Get inner iterator for advanced operations
$innerIterator = $iterator->getIterator();
foreach ($innerIterator as $key => $value) {
    echo "[$key] => '$value'\n";
}
```

## API Reference

### Functions

#### `str_iter(string $str, string $mode = "grapheme")`

Creates a new string iterator.

**Parameters:**
- `$str` (string): The string to iterate over
- `$mode` (string, optional): Iteration mode - "grapheme", "codepoint", or "byte"

**Returns:** `_StrIterIterator` object

### Iterator Methods

The returned iterator implements PHP's IteratorAggregate and Countable interfaces:

**IteratorAggregate Methods:**
- `getIterator()`: Returns the iterator itself for nested iteration

**Countable Methods:**
- `count()`: Returns the total number of elements in the iterator

## Examples

### Working with Emoji and Complex Characters

```php
<?php
// Complex emoji with skin tone modifiers
$text = "👨‍👩‍👧‍👦👋🏽";
$iterator = str_iter($text, "grapheme");

foreach ($iterator as $index => $char) {
    echo "Grapheme $index: '$char'\n";
}
```

### Processing Japanese Text

```php
<?php
$text = "こんにちは世界";
$iterator = str_iter($text, "grapheme");

foreach ($iterator as $index => $char) {
    echo "Character $index: '$char'\n";
}
```

### Binary Data Processing

```php
<?php
$data = "\x48\x65\x6C\x6C\x6F"; // "Hello" in hex
$iterator = str_iter($data, "byte");

foreach ($iterator as $index => $byte) {
    echo "Byte $index: 0x" . dechex(ord($byte)) . "\n";
}
```

## Technical Details

### Grapheme Cluster Detection

The extension uses PCRE2's `\X` pattern to detect grapheme clusters, which properly handles:
- Base characters with combining marks
- Emoji sequences
- Regional indicator sequences
- Hangul syllable sequences

### UTF-8 Validation

The extension includes proper UTF-8 validation and handles invalid sequences gracefully by treating them as individual bytes.

### Memory Management

The extension properly manages memory for string copies and PCRE2 objects, preventing memory leaks.

## Testing

Run the included test files:

```bash
php test_basic.php
php test_grapheme.php
php test_byte_mode.php
php test_emoji_bug.php
php test_invalid_utf8.php
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is open source. Please refer to the project's license file for details.

## Changelog

### Version 1.0.0
- Initial release
- Support for grapheme, codepoint, and byte iteration modes
- PCRE2 integration for proper grapheme cluster detection
- Full Iterator interface implementation