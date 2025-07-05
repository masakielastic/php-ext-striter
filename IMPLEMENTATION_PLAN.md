# str_iter Implementation Plan

## Overview
Implement a `str_iter` function that takes a string parameter and returns a StringIterator object implementing IteratorAggregate interface for UTF-8 character iteration.

## Function Signature
```c
PHP_FUNCTION(str_iter)
{
    zend_string *str;
    zend_string *mode = NULL;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(str)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(mode)
    ZEND_PARSE_PARAMETERS_END();
    
    // Default mode is "grapheme"
    // Supported modes: "grapheme", "codepoint"
    
    // Create and return StringIterator object
}
```

## Second Parameter Specification
- **Default**: `"grapheme"` - Uses PCRE2 to handle Unicode grapheme clusters
- **Alternative 1**: `"codepoint"` - Uses `php_next_utf8_char` for individual codepoints
- **Alternative 2**: `"byte"` - Iterates through individual bytes (raw byte access)
- **Grapheme mode**: Handles complex Unicode sequences like emoji with modifiers, combining characters
- **Codepoint mode**: Maintains original behavior for individual Unicode codepoints
- **Byte mode**: Provides raw byte-level access to string data

## StringIterator Class Structure

### Class Definition
```c
typedef enum {
    STRITER_MODE_GRAPHEME = 0,
    STRITER_MODE_CODEPOINT = 1,
    STRITER_MODE_BYTE = 2
} striter_mode_t;

typedef struct _striter_string_iterator_obj {
    zend_string *str;           // Source string
    size_t position;            // Current byte position in string
    size_t char_index;          // Current character index (0-based)
    size_t total_chars;         // Total characters in string
    striter_mode_t mode;        // Iteration mode (grapheme or codepoint)
    zend_object std;            // Standard object
} striter_string_iterator_obj;
```

### Required Methods

#### 1. `__construct(string $str, string $mode = "grapheme")`
- Initialize the iterator with the source string and mode
- Calculate total character/byte count based on mode:
  - Grapheme mode: Use PCRE2 with `\X` pattern for grapheme clusters
  - Codepoint mode: Use `php_next_utf8_char` for individual codepoints
  - Byte mode: Use string length for total byte count

#### 2. `getIterator(): Iterator`
- Return `$this` (self-iterator pattern)

#### 3. `current(): string`
- Return current character/grapheme cluster/byte as string
- Mode-dependent extraction:
  - Grapheme mode: Use PCRE2 to extract grapheme cluster at current position
  - Codepoint mode: Use `php_next_utf8_char` to extract codepoint at current position
  - Byte mode: Return single byte at current position

#### 4. `key(): int`
- Return current character index (0-based)

#### 5. `next(): void`
- Advance to next character/grapheme cluster/byte
- Mode-dependent advancement logic:
  - Grapheme/Codepoint mode: Advance by character index
  - Byte mode: Advance by single byte

#### 6. `rewind(): void`
- Reset position to beginning (position = 0, char_index = 0)

#### 7. `valid(): bool`
- Check if current position is valid:
  - Grapheme/Codepoint mode: char_index < total_chars
  - Byte mode: char_index < string_byte_length

## Character Iteration Strategy

### Invalid Byte Sequence Handling
**Important specification**: Invalid byte sequences are treated as regular characters. The implementation ignores error conditions and processes all bytes as characters regardless of UTF-8 validity.

### Triple Mode Implementation

#### Grapheme Mode (Default) - Using PCRE2
```c
// Use PCRE2 with \X pattern for grapheme cluster extraction
pcre2_code *re;
PCRE2_SPTR pattern = (PCRE2_SPTR)"\\X";  // Unicode extended grapheme cluster
re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, PCRE2_UTF | PCRE2_UCP, &errorcode, &erroroffset, NULL);
```

#### Codepoint Mode - Using php_next_utf8_char
Based on json_encoder.c usage pattern:
```c
size_t pos = 0;
zend_result status;
unsigned int char_code = php_next_utf8_char((unsigned char *)str_val, remaining_len, &pos, &status);

// Note: status is ignored - invalid sequences are treated as characters
// pos now contains the number of bytes consumed
// char_code contains the Unicode codepoint (or invalid byte value)
```

#### Byte Mode - Direct Byte Access
```c
// Simple byte-by-byte iteration
size_t byte_count = str_len;  // Total bytes = string length
char current_byte = str[position];  // Direct byte access
```

### Character Extraction Process

#### Grapheme Mode Process
1. Use PCRE2 with `\X` pattern to match grapheme clusters
2. Extract matched substring as current grapheme cluster
3. Handle PCRE2 known bug with consecutive emoji sequences
4. Track grapheme cluster index

#### Codepoint Mode Process
1. Start at byte position 0
2. Use `php_next_utf8_char` to get character and advance position
3. **Ignore status return value** - treat all byte sequences as characters
4. Convert Unicode codepoint back to UTF-8 string for `current()` method
5. Track both byte position and character index

#### Byte Mode Process
1. Start at byte position 0
2. Return single byte at current position as string
3. Advance position by 1 byte
4. Track byte index (same as position)
5. Total count equals string length in bytes

### PCRE2 Known Issues
- **Consecutive emoji bug**: PCRE2 may incorrectly handle sequences of multiple emoji
- **Workaround**: Implement fallback logic or additional validation for emoji sequences

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
- **PCRE2 library** for grapheme cluster handling
- Standard Zend iterator interfaces
- Zend string manipulation functions

## Build Configuration
```m4
PHP_ARG_ENABLE(striter, whether to enable striter support,
[  --enable-striter        Enable striter support])

if test "$PHP_STRITER" != "no"; then
  PHP_CHECK_LIBRARY(pcre2-8, pcre2_compile_8, [
    PHP_ADD_LIBRARY(pcre2-8, 1, STRITER_SHARED_LIBADD)
    AC_DEFINE(HAVE_PCRE2, 1, [Have PCRE2 library])
  ], [
    AC_MSG_ERROR([PCRE2 library not found. Please install libpcre2-dev])
  ])
  
  PHP_NEW_EXTENSION(striter, striter.c string_iterator.c, $ext_shared)
  PHP_SUBST(STRITER_SHARED_LIBADD)
fi
```