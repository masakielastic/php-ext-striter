#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_striter.h"

// Object handlers
static zend_object_handlers striter_string_iterator_handlers;

// Object creation function
zend_object *striter_string_iterator_create_object(zend_class_entry *ce)
{
    striter_string_iterator_obj *obj = zend_object_alloc(sizeof(striter_string_iterator_obj), ce);
    
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    
    obj->std.handlers = &striter_string_iterator_handlers;
    
    // Initialize fields
    obj->str = NULL;
    obj->position = 0;
    obj->char_index = 0;
    obj->total_chars = 0;
    obj->mode = STRITER_MODE_GRAPHEME;
    
    return &obj->std;
}

// Object destructor
static void striter_string_iterator_free_object(zend_object *object)
{
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(object);
    
    if (obj->str) {
        zend_string_release(obj->str);
    }
    
    zend_object_std_dtor(&obj->std);
}

// _StrIterIterator::__construct method
PHP_METHOD(_StrIterIterator, __construct)
{
    zend_string *str;
    zend_string *mode = NULL;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(str)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(mode)
    ZEND_PARSE_PARAMETERS_END();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    // Parse mode parameter
    striter_mode_t iter_mode = STRITER_MODE_GRAPHEME; // Default
    if (mode != NULL) {
        iter_mode = striter_parse_mode(ZSTR_VAL(mode));
    }
    
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


// _StrIterIterator::current method
PHP_METHOD(_StrIterIterator, current)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    if (!obj->str || obj->char_index >= obj->total_chars) {
        RETURN_NULL();
    }
    
    size_t byte_pos;
    zend_string *char_str = NULL;
    
    // Mode-dependent character extraction
    if (obj->mode == STRITER_MODE_GRAPHEME) {
#ifdef HAVE_PCRE2
        char_str = striter_get_grapheme_at_position(
            ZSTR_VAL(obj->str), 
            ZSTR_LEN(obj->str), 
            obj->char_index, 
            &byte_pos
        );
#endif
        // Fallback to codepoint mode if PCRE2 not available or fails
        if (char_str == NULL) {
            char_str = striter_get_char_at_position(
                ZSTR_VAL(obj->str), 
                ZSTR_LEN(obj->str), 
                obj->char_index, 
                &byte_pos
            );
        }
    } else if (obj->mode == STRITER_MODE_CODEPOINT) {
        char_str = striter_get_char_at_position(
            ZSTR_VAL(obj->str), 
            ZSTR_LEN(obj->str), 
            obj->char_index, 
            &byte_pos
        );
    } else if (obj->mode == STRITER_MODE_BYTE) {
        char_str = striter_get_byte_at_position(
            ZSTR_VAL(obj->str), 
            ZSTR_LEN(obj->str), 
            obj->char_index
        );
    }
    
    if (char_str) {
        RETURN_STR(char_str);
    } else {
        RETURN_NULL();
    }
}

// _StrIterIterator::key method
PHP_METHOD(_StrIterIterator, key)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_LONG(obj->char_index);
}

// _StrIterIterator::next method
PHP_METHOD(_StrIterIterator, next)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    if (!obj->str || obj->char_index >= obj->total_chars) {
        return;
    }
    
    // Advance to next character
    obj->char_index++;
}

// _StrIterIterator::rewind method
PHP_METHOD(_StrIterIterator, rewind)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    obj->position = 0;
    obj->char_index = 0;
}

// _StrIterIterator::valid method
PHP_METHOD(_StrIterIterator, valid)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_BOOL(obj->str && obj->char_index < obj->total_chars);
}

// Method entries for _StrIterIterator class
static const zend_function_entry striter_string_iterator_methods[] = {
    PHP_ME(_StrIterIterator, __construct, arginfo_striteriterator_construct, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, current, arginfo_striteriterator_current, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, key, arginfo_striteriterator_key, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, next, arginfo_striteriterator_next, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, rewind, arginfo_striteriterator_rewind, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, valid, arginfo_striteriterator_valid, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

// Initialize _StrIterIterator class
void striter_string_iterator_init(void)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "_StrIterIterator", striter_string_iterator_methods);
    striter_string_iterator_ce = zend_register_internal_class(&ce);
    striter_string_iterator_ce->create_object = striter_string_iterator_create_object;
    
    // Set up object handlers
    memcpy(&striter_string_iterator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    striter_string_iterator_handlers.free_obj = striter_string_iterator_free_object;
    striter_string_iterator_handlers.offset = XtOffsetOf(striter_string_iterator_obj, std);
    
    // Implement Iterator interface only
    zend_class_implements(striter_string_iterator_ce, 1, zend_ce_iterator);
}