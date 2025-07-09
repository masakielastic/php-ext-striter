#ifndef PHP_STRITER_H
#define PHP_STRITER_H

#include "php.h"
#include "ext/standard/html.h"
#include "zend_interfaces.h"

#ifdef HAVE_PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#endif

#define PHP_STRITER_VERSION "1.0.0"
#define PHP_STRITER_EXTNAME "striter"

extern zend_module_entry striter_module_entry;
#define phpext_striter_ptr &striter_module_entry

// _StrIterIterator class entry
extern zend_class_entry *striter_string_iterator_ce;

// Iterator mode enumeration
typedef enum {
    STRITER_MODE_GRAPHEME = 0,
    STRITER_MODE_CODEPOINT = 1,
    STRITER_MODE_BYTE = 2
} striter_mode_t;

// _StrIterIterator object structure
typedef struct _striter_string_iterator_obj {
    zend_string *str;           // Source string
    size_t position;            // Current byte position in string
    size_t char_index;          // Current character index (0-based)
    size_t total_chars;         // Total characters in string
    striter_mode_t mode;        // Iteration mode (grapheme or codepoint)
    zend_object std;            // Standard object
} striter_string_iterator_obj;

// Object accessor macro
static inline striter_string_iterator_obj *striter_string_iterator_from_obj(zend_object *obj) {
    return (striter_string_iterator_obj*)((char*)(obj) - XtOffsetOf(striter_string_iterator_obj, std));
}

// Internal iterator structure for IteratorAggregate
typedef struct _striter_iterator {
    zend_object_iterator intern;
    size_t current_pos;
    zval current_value;
} striter_iterator;

// Function declarations
PHP_FUNCTION(str_iter);

// ArgInfo declarations
ZEND_BEGIN_ARG_INFO_EX(arginfo_str_iter, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 1, "\"grapheme\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_construct, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 1, "\"grapheme\"")
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_current, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_key, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_next, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_rewind, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_valid, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_getiterator, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_striteriterator_count, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(striter);
PHP_MSHUTDOWN_FUNCTION(striter);
PHP_MINFO_FUNCTION(striter);

// _StrIterIterator class method declarations
PHP_METHOD(_StrIterIterator, __construct);
PHP_METHOD(_StrIterIterator, current);
PHP_METHOD(_StrIterIterator, key);
PHP_METHOD(_StrIterIterator, next);
PHP_METHOD(_StrIterIterator, rewind);
PHP_METHOD(_StrIterIterator, valid);
PHP_METHOD(_StrIterIterator, getIterator);
PHP_METHOD(_StrIterIterator, count);

// Internal utility functions
size_t striter_count_utf8_chars(const char *str, size_t len);
zend_string *striter_get_char_at_position(const char *str, size_t str_len, size_t char_index, size_t *byte_pos);

#ifdef HAVE_PCRE2
extern pcre2_code *striter_grapheme_pattern;
#ifdef ZTS
extern zend_mutex_t striter_pattern_mutex;
#endif
size_t striter_count_graphemes_pcre2(const char *str, size_t len);
zend_string *striter_get_grapheme_at_position(const char *str, size_t str_len, size_t char_index, size_t *byte_pos);
pcre2_code *striter_get_grapheme_pattern(void);
#endif

striter_mode_t striter_parse_mode(const char *mode_str);
size_t striter_count_bytes(const char *str, size_t len);
zend_string *striter_get_byte_at_position(const char *str, size_t str_len, size_t byte_index);

// _StrIterIterator class initialization
void striter_string_iterator_init(void);
zend_object *striter_string_iterator_create_object(zend_class_entry *ce);

#endif /* PHP_STRITER_H */