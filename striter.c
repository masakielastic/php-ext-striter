#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_striter.h"

// Global class entry
zend_class_entry *striter_string_iterator_ce;

// str_iter function implementation
PHP_FUNCTION(str_iter)
{
    zend_string *str;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();
    
    // Create new StringIterator object
    object_init_ex(return_value, striter_string_iterator_ce);
    
    // Get the object structure
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(return_value));
    
    // Initialize the iterator
    obj->str = zend_string_copy(str);
    obj->position = 0;
    obj->char_index = 0;
    obj->total_chars = striter_count_utf8_chars(ZSTR_VAL(str), ZSTR_LEN(str));
}

// Utility function to count UTF-8 characters
size_t striter_count_utf8_chars(const char *str, size_t len) {
    size_t char_count = 0;
    size_t pos = 0;
    
    while (pos < len) {
        size_t advance = 0;
        zend_result status;
        php_next_utf8_char((unsigned char *)str + pos, len - pos, &advance, &status);
        
        // Status is ignored - all sequences are treated as characters
        if (advance == 0) {
            advance = 1; // Ensure we advance at least 1 byte
        }
        pos += advance;
        char_count++;
    }
    
    return char_count;
}

// Utility function to get character at specific position
zend_string *striter_get_char_at_position(const char *str, size_t str_len, size_t char_index, size_t *byte_pos) {
    size_t current_char = 0;
    size_t pos = 0;
    
    while (pos < str_len && current_char <= char_index) {
        size_t advance = 0;
        zend_result status;
        unsigned int char_code = php_next_utf8_char((unsigned char *)str + pos, str_len - pos, &advance, &status);
        
        if (current_char == char_index) {
            // Found the character, extract it
            if (advance == 0) {
                advance = 1; // Ensure we advance at least 1 byte
            }
            if (byte_pos) {
                *byte_pos = pos;
            }
            return zend_string_init(str + pos, advance, 0);
        }
        
        if (advance == 0) {
            advance = 1; // Ensure we advance at least 1 byte
        }
        pos += advance;
        current_char++;
    }
    
    return NULL;
}

// Function entries
const zend_function_entry striter_functions[] = {
    PHP_FE(str_iter, arginfo_str_iter)
    PHP_FE_END
};

// Module entry
zend_module_entry striter_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_STRITER_EXTNAME,
    striter_functions,
    PHP_MINIT(striter),
    PHP_MSHUTDOWN(striter),
    NULL,
    NULL,
    PHP_MINFO(striter),
    PHP_STRITER_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_STRITER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(striter)
#endif

// Module initialization
PHP_MINIT_FUNCTION(striter)
{
    // Initialize StringIterator class
    striter_string_iterator_init();
    
    return SUCCESS;
}

// Module shutdown
PHP_MSHUTDOWN_FUNCTION(striter)
{
    return SUCCESS;
}

// Module info
PHP_MINFO_FUNCTION(striter)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "striter support", "enabled");
    php_info_print_table_row(2, "Version", PHP_STRITER_VERSION);
    php_info_print_table_end();
}