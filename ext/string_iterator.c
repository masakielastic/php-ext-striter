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

// Count elements handler for Countable interface
static zend_result striter_string_iterator_count_elements(zend_object *object, zend_long *count)
{
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(object);
    *count = obj->total_chars;
    return SUCCESS;
}

// Internal iterator functions for IteratorAggregate
static void striter_iterator_dtor(zend_object_iterator *iter)
{
    striter_iterator *iterator = (striter_iterator*)iter;
    if (Z_TYPE(iterator->current_value) != IS_UNDEF) {
        zval_ptr_dtor(&iterator->current_value);
    }
    zval_ptr_dtor(&iter->data);
}

static void striter_iterator_rewind(zend_object_iterator *iter)
{
    ((striter_iterator*)iter)->current_pos = 0;
}

static zend_result striter_iterator_valid(zend_object_iterator *iter)
{
    striter_iterator *iterator = (striter_iterator*)iter;
    striter_string_iterator_obj *object = striter_string_iterator_from_obj(Z_OBJ(iter->data));

    if (iterator->current_pos < object->total_chars) {
        return SUCCESS;
    }
    return FAILURE;
}

static zval *striter_iterator_get_current(zend_object_iterator *iter)
{
    striter_iterator *iterator = (striter_iterator*)iter;
    striter_string_iterator_obj *object = striter_string_iterator_from_obj(Z_OBJ(iter->data));

    if (iterator->current_pos >= object->total_chars) {
        return &EG(uninitialized_zval);
    }

    size_t byte_pos;
    zend_string *char_str = NULL;
    
    // Mode-dependent character extraction
    if (object->mode == STRITER_MODE_GRAPHEME) {
#ifdef HAVE_PCRE2
        char_str = striter_get_grapheme_at_position(
            ZSTR_VAL(object->str), 
            ZSTR_LEN(object->str), 
            iterator->current_pos, 
            &byte_pos
        );
#endif
        // Fallback to codepoint mode if PCRE2 not available or fails
        if (char_str == NULL) {
            char_str = striter_get_char_at_position(
                ZSTR_VAL(object->str), 
                ZSTR_LEN(object->str), 
                iterator->current_pos, 
                &byte_pos
            );
        }
    } else if (object->mode == STRITER_MODE_CODEPOINT) {
        char_str = striter_get_char_at_position(
            ZSTR_VAL(object->str), 
            ZSTR_LEN(object->str), 
            iterator->current_pos, 
            &byte_pos
        );
    } else if (object->mode == STRITER_MODE_BYTE) {
        char_str = striter_get_byte_at_position(
            ZSTR_VAL(object->str), 
            ZSTR_LEN(object->str), 
            iterator->current_pos
        );
    }

    if (char_str) {
        // Store the current value in the iterator structure
        striter_iterator *iterator = (striter_iterator*)iter;
        if (Z_TYPE(iterator->current_value) != IS_UNDEF) {
            zval_ptr_dtor(&iterator->current_value);
        }
        ZVAL_STR(&iterator->current_value, char_str);
        return &iterator->current_value;
    } else {
        return &EG(uninitialized_zval);
    }
}

static void striter_iterator_get_key(zend_object_iterator *iter, zval *key)
{
    striter_iterator *iterator = (striter_iterator*)iter;
    ZVAL_LONG(key, iterator->current_pos);
}

static void striter_iterator_move_forward(zend_object_iterator *iter)
{
    ((striter_iterator*)iter)->current_pos++;
}

// Iterator function table
static const zend_object_iterator_funcs striter_iterator_funcs = {
    striter_iterator_dtor,
    striter_iterator_valid,
    striter_iterator_get_current,
    striter_iterator_get_key,
    striter_iterator_move_forward,
    striter_iterator_rewind,
    NULL,
    NULL,
};

// Get iterator handler for IteratorAggregate
static zend_object_iterator *striter_string_iterator_get_iterator(zend_class_entry *ce, zval *object, int by_ref)
{
    if (by_ref) {
        zend_throw_error(NULL, "An iterator cannot be used with foreach by reference");
        return NULL;
    }

    // Create internal iterator that wraps our object and implements Iterator interface
    striter_iterator *iterator = emalloc(sizeof(striter_iterator));
    zend_iterator_init((zend_object_iterator*)iterator);

    ZVAL_OBJ_COPY(&iterator->intern.data, Z_OBJ_P(object));
    iterator->intern.funcs = &striter_iterator_funcs;
    iterator->current_pos = 0;
    ZVAL_UNDEF(&iterator->current_value);

    return &iterator->intern;
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

// _StrIterIterator::getIterator method  
PHP_METHOD(_StrIterIterator, getIterator)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    // Return self since this class implements Iterator
    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

// _StrIterIterator::count method
PHP_METHOD(_StrIterIterator, count)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_LONG(obj->total_chars);
}

// Method entries for _StrIterIterator class
static const zend_function_entry striter_string_iterator_methods[] = {
    PHP_ME(_StrIterIterator, __construct, arginfo_striteriterator_construct, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, current, arginfo_striteriterator_current, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, key, arginfo_striteriterator_key, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, next, arginfo_striteriterator_next, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, rewind, arginfo_striteriterator_rewind, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, valid, arginfo_striteriterator_valid, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, getIterator, arginfo_striteriterator_getiterator, ZEND_ACC_PUBLIC)
    PHP_ME(_StrIterIterator, count, arginfo_striteriterator_count, ZEND_ACC_PUBLIC)
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
    striter_string_iterator_handlers.count_elements = striter_string_iterator_count_elements;
    
    // Set get_iterator handler for IteratorAggregate
    striter_string_iterator_ce->get_iterator = striter_string_iterator_get_iterator;
    
    // Implement IteratorAggregate and Countable interfaces
    zend_class_implements(striter_string_iterator_ce, 2, zend_ce_aggregate, zend_ce_countable);
}