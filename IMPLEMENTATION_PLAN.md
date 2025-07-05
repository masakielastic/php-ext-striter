# str_iter Implementation Plan

## Overview
Implement a `str_iter` function that takes a string parameter and returns a StringIterator object implementing IteratorAggregate interface for UTF-8 character iteration.

## Function Signature
```c
PHP_FUNCTION(str_iter)
{
    zend_string *str;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();
    
    // Create and return StringIterator object
}
```

## StringIterator Class Structure

### Class Definition
```c
typedef struct _striter_string_iterator_obj {
    zend_string *str;           // Source string
    size_t position;            // Current byte position in string
    size_t char_index;          // Current character index (0-based)
    size_t total_chars;         // Total UTF-8 characters in string
    zend_object std;            // Standard object
} striter_string_iterator_obj;
```

### Required Methods

#### 1. `__construct(string $str)`
- Initialize the iterator with the source string
- Calculate total character count using `php_next_utf8_char`

#### 2. `getIterator(): Iterator`
- Return `$this` (self-iterator pattern)

#### 3. `current(): string`
- Return current UTF-8 character as string
- Use `php_next_utf8_char` to extract character at current position

#### 4. `key(): int`
- Return current character index (0-based)

#### 5. `next(): void`
- Advance to next UTF-8 character
- Use `php_next_utf8_char` to find next character boundary

#### 6. `rewind(): void`
- Reset position to beginning (position = 0, char_index = 0)

#### 7. `valid(): bool`
- Check if current position is valid (char_index < total_chars)

## UTF-8 Character Iteration Strategy

### Invalid Byte Sequence Handling
**Important specification**: Invalid byte sequences are treated as regular characters. The implementation ignores the `status` parameter from `php_next_utf8_char` and processes all bytes as characters regardless of UTF-8 validity.

### Using php_next_utf8_char
Based on json_encoder.c usage pattern:
```c
size_t pos = 0;
zend_result status;
unsigned int char_code = php_next_utf8_char((unsigned char *)str_val, remaining_len, &pos, &status);

// Note: status is ignored - invalid sequences are treated as characters
// pos now contains the number of bytes consumed
// char_code contains the Unicode codepoint (or invalid byte value)
```

### Character Extraction Process
1. Start at byte position 0
2. Use `php_next_utf8_char` to get character and advance position
3. **Ignore status return value** - treat all byte sequences as characters
4. Convert Unicode codepoint back to UTF-8 string for `current()` method
5. Track both byte position and character index

### Total Character Count Calculation
```c
static size_t count_utf8_chars(const char *str, size_t len) {
    size_t char_count = 0;
    size_t pos = 0;
    
    while (pos < len) {
        size_t old_pos = pos;
        zend_result status;
        php_next_utf8_char((unsigned char *)str + old_pos, len - old_pos, &pos, &status);
        
        // Status is ignored - all sequences are treated as characters
        pos += old_pos; // Adjust position to be absolute
        char_count++;
    }
    
    return char_count;
}
```

## Implementation Files Structure

### 1. `php_striter.h`
- Class entry declarations
- Function declarations
- Object structure definitions

### 2. `striter.c`
- Extension initialization
- `str_iter` function implementation
- Module entry definition

### 3. `string_iterator.c`
- StringIterator class implementation
- All iterator methods
- UTF-8 character handling utilities

### 4. `config.m4`
- Extension configuration
- Dependency on ext/standard (for html.h)

## Key Implementation Details

### Memory Management
- Use `zend_string_addref()` to reference input string
- Proper cleanup in object destructor
- Handle empty strings and NULL inputs

### Error Handling
- Memory allocation failures
- Parameter validation
- Note: Invalid UTF-8 sequences are NOT treated as errors per specification

### Performance Considerations
- Cache total character count on construction
- Minimize UTF-8 parsing operations
- Use efficient string operations

## Testing Strategy

### Unit Tests
1. ASCII strings
2. UTF-8 strings with various characters
3. Empty strings
4. Invalid UTF-8 sequences (treated as regular characters)
5. Very long strings
6. Mixed character sets
7. Binary data with invalid UTF-8 sequences

### Integration Tests
- Use in foreach loops
- Multiple iterator instances
- Iterator reuse scenarios

## Dependencies
- `ext/standard/html.h` for `php_next_utf8_char`
- Standard Zend iterator interfaces
- Zend string manipulation functions

## Build Configuration
```m4
PHP_ARG_ENABLE(striter, whether to enable striter support,
[  --enable-striter        Enable striter support])

if test "$PHP_STRITER" != "no"; then
  PHP_NEW_EXTENSION(striter, striter.c string_iterator.c, $ext_shared)
fi
```