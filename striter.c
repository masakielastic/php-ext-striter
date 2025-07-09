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
    zend_string *mode = NULL;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(str)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(mode)
    ZEND_PARSE_PARAMETERS_END();
    
    // Parse mode parameter
    striter_mode_t iter_mode = STRITER_MODE_GRAPHEME; // Default
    if (mode != NULL) {
        iter_mode = striter_parse_mode(ZSTR_VAL(mode));
    }
    
    // Create new StringIterator object
    object_init_ex(return_value, striter_string_iterator_ce);
    
    // Get the object structure
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(return_value));
    
    // Initialize the iterator
    obj->str = zend_string_copy(str);
    obj->position = 0;
    obj->char_index = 0;
    obj->mode = iter_mode;
    
    // Calculate total character count based on mode
    if (iter_mode == STRITER_MODE_GRAPHEME) {
#ifdef HAVE_PCRE2
        obj->total_chars = striter_count_graphemes_pcre2(ZSTR_VAL(str), ZSTR_LEN(str));
#else
        // Fallback to codepoint mode if PCRE2 not available
        obj->total_chars = striter_count_utf8_chars(ZSTR_VAL(str), ZSTR_LEN(str));
        obj->mode = STRITER_MODE_CODEPOINT;
#endif
    } else if (iter_mode == STRITER_MODE_CODEPOINT) {
        obj->total_chars = striter_count_utf8_chars(ZSTR_VAL(str), ZSTR_LEN(str));
    } else if (iter_mode == STRITER_MODE_BYTE) {
        obj->total_chars = striter_count_bytes(ZSTR_VAL(str), ZSTR_LEN(str));
    }
}

// Utility function to count UTF-8 characters
size_t striter_count_utf8_chars(const char *str, size_t len) {
    if (str == NULL || len == 0) {
        return 0;
    }
    
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
    if (str == NULL || str_len == 0) {
        return NULL;
    }
    
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

// Parse mode string to enum
striter_mode_t striter_parse_mode(const char *mode_str) {
    if (mode_str == NULL) {
        return STRITER_MODE_GRAPHEME;
    }
    
    if (strcmp(mode_str, "codepoint") == 0) {
        return STRITER_MODE_CODEPOINT;
    } else if (strcmp(mode_str, "byte") == 0) {
        return STRITER_MODE_BYTE;
    }
    
    // Default to grapheme mode for any other value
    return STRITER_MODE_GRAPHEME;
}

#ifdef HAVE_PCRE2
// Global cached compiled pattern for grapheme clusters
pcre2_code *striter_grapheme_pattern = NULL;
#ifdef ZTS
zend_mutex_t striter_pattern_mutex;
#endif

// Thread-safe getter for grapheme pattern
pcre2_code *striter_get_grapheme_pattern(void)
{
#ifdef ZTS
    zend_mutex_lock(&striter_pattern_mutex);
#endif
    
    if (striter_grapheme_pattern == NULL) {
        PCRE2_SPTR pattern = (PCRE2_SPTR)"\\X";
        int errorcode;
        PCRE2_SIZE erroroffset;
        
        striter_grapheme_pattern = pcre2_compile(
            pattern,
            PCRE2_ZERO_TERMINATED,
            PCRE2_UTF | PCRE2_UCP,
            &errorcode,
            &erroroffset,
            NULL
        );
    }
    
    pcre2_code *result = striter_grapheme_pattern;
    
#ifdef ZTS
    zend_mutex_unlock(&striter_pattern_mutex);
#endif
    
    return result;
}

// Count grapheme clusters using PCRE2
size_t striter_count_graphemes_pcre2(const char *str, size_t len) {
    if (len == 0) {
        return 0;
    }
    
    // Use thread-safe pattern getter
    pcre2_code *pattern = striter_get_grapheme_pattern();
    if (pattern == NULL) {
        // Fallback to byte-by-byte counting if pattern not available
        return len;
    }
    
    pcre2_match_data *match_data;
    PCRE2_SPTR subject = (PCRE2_SPTR)str;
    size_t count = 0;
    PCRE2_SIZE start_offset = 0;
    int rc;
    
    // Create match data
    match_data = pcre2_match_data_create_from_pattern(pattern, NULL);
    if (match_data == NULL) {
        return len;
    }
    
    // Scan string for grapheme clusters
    while (start_offset < len) {
        rc = pcre2_match(
            pattern,
            subject,
            len,
            start_offset,
            0,
            match_data,
            NULL
        );
        
        if (rc < 0) {
            if (rc == PCRE2_ERROR_NOMATCH) {
                break;
            } else {
                // Error: skip one byte and continue
                // This ensures we don't get stuck in infinite loops
                if (start_offset < len) {
                    start_offset++;
                    count++;
                }
                continue;
            }
        }
        
        count++;
        
        // Get match boundaries
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        PCRE2_SIZE match_start = ovector[0];
        PCRE2_SIZE match_end = ovector[1];
        
        start_offset = match_end;
        
        // Prevent infinite loop
        if (match_end == match_start) {
            if (start_offset >= len) {
                break;
            }
            start_offset++;
        }
    }
    
    pcre2_match_data_free(match_data);
    
    return count;
}

// Get grapheme cluster at specific position using PCRE2
zend_string *striter_get_grapheme_at_position(const char *str, size_t str_len, size_t char_index, size_t *byte_pos) {
    if (str_len == 0 || char_index >= str_len) {
        return NULL;
    }
    
    // Use thread-safe pattern getter
    pcre2_code *pattern = striter_get_grapheme_pattern();
    if (pattern == NULL) {
        // Fallback to single byte
        if (byte_pos) {
            *byte_pos = char_index < str_len ? char_index : str_len - 1;
        }
        return zend_string_init(str + (char_index < str_len ? char_index : str_len - 1), 1, 0);
    }
    
    pcre2_match_data *match_data;
    PCRE2_SPTR subject = (PCRE2_SPTR)str;
    size_t current_char = 0;
    PCRE2_SIZE start_offset = 0;
    int rc;
    
    match_data = pcre2_match_data_create_from_pattern(pattern, NULL);
    if (match_data == NULL) {
        if (byte_pos) {
            *byte_pos = char_index < str_len ? char_index : str_len - 1;
        }
        return zend_string_init(str + (char_index < str_len ? char_index : str_len - 1), 1, 0);
    }
    
    // Find the char_index-th grapheme cluster
    while (start_offset < str_len && current_char <= char_index) {
        rc = pcre2_match(
            pattern,
            subject,
            str_len,
            start_offset,
            0,
            match_data,
            NULL
        );
        
        if (rc < 0) {
            if (rc == PCRE2_ERROR_NOMATCH) {
                break;
            } else {
                // Error: treat as single byte
                if (current_char == char_index) {
                    if (byte_pos) {
                        *byte_pos = start_offset;
                    }
                    pcre2_match_data_free(match_data);
                    return zend_string_init(str + start_offset, 
                        start_offset < str_len ? 1 : 0, 0);
                }
                if (start_offset < str_len) {
                    start_offset++;
                    current_char++;
                }
                continue;
            }
        }
        
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        PCRE2_SIZE match_start = ovector[0];
        PCRE2_SIZE match_end = ovector[1];
        
        if (current_char == char_index) {
            if (byte_pos) {
                *byte_pos = match_start;
            }
            size_t cluster_len = match_end - match_start;
            zend_string *result = zend_string_init(str + match_start, cluster_len, 0);
            
            pcre2_match_data_free(match_data);
            return result;
        }
        
        start_offset = match_end;
        current_char++;
        
        // Prevent infinite loop
        if (match_end == match_start) {
            if (start_offset >= str_len) {
                break;
            }
            start_offset++;
        }
    }
    
    pcre2_match_data_free(match_data);
    return NULL;
}
#endif

// Count bytes (simply return string length)
size_t striter_count_bytes(const char *str, size_t len) {
    return len;
}

// Get byte at specific position
zend_string *striter_get_byte_at_position(const char *str, size_t str_len, size_t byte_index) {
    if (str == NULL || str_len == 0 || byte_index >= str_len) {
        return NULL;
    }
    
    // Return single byte as string
    return zend_string_init(str + byte_index, 1, 0);
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
    
#ifdef HAVE_PCRE2
#ifdef ZTS
    // Initialize mutex for thread safety
    zend_mutex_alloc(&striter_pattern_mutex);
#endif
    
    // Pattern will be lazily compiled in thread-safe manner
    striter_grapheme_pattern = NULL;
#endif
    
    return SUCCESS;
}

// Module shutdown
PHP_MSHUTDOWN_FUNCTION(striter)
{
#ifdef HAVE_PCRE2
    // Free the cached compiled pattern
    if (striter_grapheme_pattern != NULL) {
        pcre2_code_free(striter_grapheme_pattern);
        striter_grapheme_pattern = NULL;
    }
    
#ifdef ZTS
    // Free mutex
    zend_mutex_free(&striter_pattern_mutex);
#endif
#endif
    
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