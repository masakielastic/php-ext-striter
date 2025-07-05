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

// StringIterator::__construct method
PHP_METHOD(StringIterator, __construct)
{
    zend_string *str;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    // Initialize the iterator
    obj->str = zend_string_copy(str);
    obj->position = 0;
    obj->char_index = 0;
    obj->total_chars = striter_count_utf8_chars(ZSTR_VAL(str), ZSTR_LEN(str));
}


// StringIterator::current method
PHP_METHOD(StringIterator, current)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    if (!obj->str || obj->char_index >= obj->total_chars) {
        RETURN_NULL();
    }
    
    size_t byte_pos;
    zend_string *char_str = striter_get_char_at_position(
        ZSTR_VAL(obj->str), 
        ZSTR_LEN(obj->str), 
        obj->char_index, 
        &byte_pos
    );
    
    if (char_str) {
        RETURN_STR(char_str);
    } else {
        RETURN_NULL();
    }
}

// StringIterator::key method
PHP_METHOD(StringIterator, key)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_LONG(obj->char_index);
}

// StringIterator::next method
PHP_METHOD(StringIterator, next)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    if (!obj->str || obj->char_index >= obj->total_chars) {
        return;
    }
    
    // Advance to next character
    obj->char_index++;
}

// StringIterator::rewind method
PHP_METHOD(StringIterator, rewind)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    obj->position = 0;
    obj->char_index = 0;
}

// StringIterator::valid method
PHP_METHOD(StringIterator, valid)
{
    ZEND_PARSE_PARAMETERS_NONE();
    
    striter_string_iterator_obj *obj = striter_string_iterator_from_obj(Z_OBJ_P(ZEND_THIS));
    
    RETURN_BOOL(obj->str && obj->char_index < obj->total_chars);
}

// Method entries for StringIterator class
static const zend_function_entry striter_string_iterator_methods[] = {
    PHP_ME(StringIterator, __construct, arginfo_stringiterator_construct, ZEND_ACC_PUBLIC)
    PHP_ME(StringIterator, current, arginfo_stringiterator_current, ZEND_ACC_PUBLIC)
    PHP_ME(StringIterator, key, arginfo_stringiterator_key, ZEND_ACC_PUBLIC)
    PHP_ME(StringIterator, next, arginfo_stringiterator_next, ZEND_ACC_PUBLIC)
    PHP_ME(StringIterator, rewind, arginfo_stringiterator_rewind, ZEND_ACC_PUBLIC)
    PHP_ME(StringIterator, valid, arginfo_stringiterator_valid, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

// Initialize StringIterator class
void striter_string_iterator_init(void)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "StringIterator", striter_string_iterator_methods);
    striter_string_iterator_ce = zend_register_internal_class(&ce);
    striter_string_iterator_ce->create_object = striter_string_iterator_create_object;
    
    // Set up object handlers
    memcpy(&striter_string_iterator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    striter_string_iterator_handlers.free_obj = striter_string_iterator_free_object;
    striter_string_iterator_handlers.offset = XtOffsetOf(striter_string_iterator_obj, std);
    
    // Implement Iterator interface only
    zend_class_implements(striter_string_iterator_ce, 1, zend_ce_iterator);
}