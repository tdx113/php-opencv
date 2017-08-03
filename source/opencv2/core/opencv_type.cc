/*
 +----------------------------------------------------------------------+
 | PHP-OpenCV                                                           |
 +----------------------------------------------------------------------+
 | This source file is subject to version 2.0 of the Apache license,    |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.apache.org/licenses/LICENSE-2.0.html                      |
 | If you did not receive a copy of the Apache2.0 license and are unable|
 | to obtain it through the world-wide-web, please send a note to       |
 | hihozhou@gmail.com so we can mail you a copy immediately.            |
 +----------------------------------------------------------------------+
 | Author: HaiHao Zhou  <hihozhou@gmail.com>                            |
 +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../../../php_opencv.h"
#include "opencv_type.h"

zend_class_entry *opencv_point_ce;
zend_class_entry *opencv_scalar_ce;
zend_class_entry *opencv_size_ce;
zend_class_entry *opencv_rect_ce;

/**
 * -----------------------------------【CV\Point】--------------------------------------
 *
 * -------------------------------------------------------------------------------------
 */

zend_object_handlers opencv_point_object_handlers;

/**
 * Point __construct
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_point, __construct)
{
    long x=0, y=0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll", &x, &y) == FAILURE) {
        RETURN_NULL();
    }
    opencv_point_object *obj = Z_PHP_POINT_OBJ_P(getThis());
    Point point = Point((int)x, (int)y);
    obj->point = new Point(point);

    zend_update_property_long(opencv_point_ce, getThis(), "x", sizeof("x")-1, x);
    zend_update_property_long(opencv_point_ce, getThis(), "y", sizeof("y")-1, y);
}

/**
 * print Point data
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_point, print)
{
    opencv_point_object *obj = Z_PHP_POINT_OBJ_P(getThis());
    std::cout <<*(obj->point)<< std::endl;
    RETURN_NULL();
}



/**
 * opencv_point_methods[]
 */
const zend_function_entry opencv_point_methods[] = {
        PHP_ME(opencv_point, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
        PHP_ME(opencv_point, print, NULL, ZEND_ACC_PUBLIC) //todo why Point print method can effect Mat print method
        PHP_FE_END
};
/* }}} */

zend_object* opencv_point_create_handler(zend_class_entry *type)
{
    int size = sizeof(opencv_point_object);
    opencv_point_object *obj = (opencv_point_object *)ecalloc(1,size);
    memset(obj, 0, sizeof(opencv_point_object));
    zend_object_std_init(&obj->std, type);
    object_properties_init(&obj->std, type);
    obj->std.ce = type;
    obj->std.handlers = &opencv_point_object_handlers;
    return &obj->std;
}

void opencv_point_free_obj(zend_object *object)
{
    opencv_point_object *obj;
    obj = get_point_obj(object);
    delete obj->point;
    zend_object_std_dtor(object);
}

/**
 * Point Class write_property
 * @param object
 * @param member
 * @param value
 * @param cache_slot
 */
void opencv_point_write_property(zval *object, zval *member, zval *value, void **cache_slot){

    zend_string *str = zval_get_string(member);
    char *memberName=ZSTR_VAL(str);
    opencv_point_object *obj = Z_PHP_POINT_OBJ_P(object);

    if(strcmp(memberName, "x") == 0 && obj->point->x!=(int)zval_get_long(value)){
        obj->point->x=(int)zval_get_long(value);
    }else if(strcmp(memberName, "y") == 0 && obj->point->y!=(int)zval_get_long(value)){
        obj->point->y=(int)zval_get_long(value);
    }
    zend_string_release(str);//free zend_string not memberName(zend_string->val)
    std_object_handlers.write_property(object,member,value,cache_slot);

}

/**
 * Point Init
 */
void opencv_point_init(int module_number){
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce,OPENCV_NS, "Point", opencv_point_methods);
    opencv_point_ce = zend_register_internal_class(&ce);

    opencv_point_ce->create_object = opencv_point_create_handler;
    memcpy(&opencv_point_object_handlers,
           zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    opencv_point_object_handlers.clone_obj = NULL;
    opencv_point_object_handlers.write_property = opencv_point_write_property;
    opencv_point_object_handlers.free_obj = opencv_point_free_obj;
}





/**
 * -----------------------------------【CV\Scalar】--------------------------------------
 *
 * -------------------------------------------------------------------------------------
 */

zend_object_handlers opencv_scalar_object_handlers;

zend_object* opencv_scalar_create_handler(zend_class_entry *type)
{
    int size = sizeof(opencv_scalar_object);
    opencv_scalar_object *obj = (opencv_scalar_object *)ecalloc(1,size);
    memset(obj, 0, sizeof(opencv_scalar_object));
    zend_object_std_init(&obj->std, type);
    object_properties_init(&obj->std, type);
    obj->std.ce = type;
    obj->std.handlers = &opencv_scalar_object_handlers;
    return &obj->std;
}

void opencv_scalar_free_obj(zend_object *object)
{
    opencv_scalar_object *obj;
    obj = get_scalar_obj(object);
    delete obj->scalar;
    zend_object_std_dtor(object);
}


void opencv_scalar_update_property_by_c_scalar(zval *z,Scalar *scalar){
    zval val;
    array_init(&val);
    add_next_index_double(&val,scalar->val[0]);
    add_next_index_double(&val,scalar->val[1]);
    add_next_index_double(&val,scalar->val[2]);
    add_next_index_double(&val,scalar->val[3]);
    zend_update_property(opencv_scalar_ce, z, "val", sizeof("val")-1, &val);
    /**
     * 数组val在array_init()后refcount=1，
     * 插入成员属性zend_update_property()会自动加一次，变为2，
     * 对象销毁后只会减1，需要要在zend_update_property()后主动减一次引用
     */
    Z_DELREF(val);
}


/**
 * Scalar __construct
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_scalar, __construct)
{
    double value1 = 0, value2 = 0, value3 = 0, value4 = 0;//value: 8bit=0~255 16bit=0~65535...
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|dddd", &value1, &value2, &value3, &value4) == FAILURE) {
        RETURN_NULL();
    }
    opencv_scalar_object *obj = Z_PHP_SCALAR_OBJ_P(getThis());
    Scalar scalar = Scalar((int)value1, (int)value2, (int)value3, (int)value4);
    obj->scalar = new Scalar(scalar);
    opencv_scalar_update_property_by_c_scalar(getThis(), obj->scalar);
}


/**
 * print Scalar data
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_scalar, print)
{
    opencv_scalar_object *obj = Z_PHP_SCALAR_OBJ_P(getThis());
    std::cout <<*(obj->scalar)<< std::endl;
    RETURN_NULL();
}


/**
 * opencv_scalar_methods[]
 */
const zend_function_entry opencv_scalar_methods[] = {
        PHP_ME(opencv_scalar, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
        PHP_ME(opencv_scalar, print, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};
/* }}} */


/**
 * Scalar Init
 */
void opencv_scalar_init(int module_number){
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce,OPENCV_NS, "Scalar", opencv_scalar_methods);
    opencv_scalar_ce = zend_register_internal_class(&ce);

    opencv_scalar_ce->create_object = opencv_scalar_create_handler;
    memcpy(&opencv_scalar_object_handlers,
           zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    opencv_scalar_object_handlers.clone_obj = NULL;
    opencv_scalar_object_handlers.free_obj = opencv_scalar_free_obj;
}



//-----------------------------------【CV\Size】---------------------------------------
//
//-------------------------------------------------------------------------------------


zend_object_handlers opencv_size_object_handlers;

zend_object* opencv_size_create_handler(zend_class_entry *type)
{
    int size = sizeof(opencv_size_object);
    opencv_size_object *obj = (opencv_size_object *)ecalloc(1,size);
    memset(obj, 0, sizeof(opencv_size_object));
    zend_object_std_init(&obj->std, type);
    object_properties_init(&obj->std, type);
    obj->std.ce = type;
    obj->std.handlers = &opencv_size_object_handlers;
    return &obj->std;
}

void opencv_size_free_obj(zend_object *object)
{
    opencv_size_object *obj;
    obj = get_size_obj(object);
    delete obj->size;
    zend_object_std_dtor(object);
}

void opencv_size_update_property_by_c_size(zval *z, Size *size){
    zend_update_property_long(opencv_size_ce, z, "width", sizeof("width")-1, size->width);
    zend_update_property_long(opencv_size_ce, z, "height", sizeof("height")-1, size->height);
}


/**
 * Size __construct
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_size, __construct)
{
    long width=0, height = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll", &width, &height) == FAILURE) {
        RETURN_NULL();
    }
    opencv_size_object *obj = Z_PHP_SIZE_OBJ_P(getThis());
    Size size = Size((int)width, (int)height);
    obj->size = new Size(size);
    opencv_size_update_property_by_c_size(getThis(), obj->size);
}


/**
 * print Size data
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_size, print)
{
    opencv_size_object *obj = Z_PHP_SIZE_OBJ_P(getThis());
    std::cout <<*(obj->size)<< std::endl;
    RETURN_NULL();
}


/**
 * opencv_size_methods[]
 */
const zend_function_entry opencv_size_methods[] = {
        PHP_ME(opencv_size, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
        PHP_ME(opencv_size, print, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};
/* }}} */


/**
 * Size Class write_property
 * @param object
 * @param member
 * @param value
 * @param cache_slot
 */
void opencv_size_write_property(zval *object, zval *member, zval *value, void **cache_slot){

    zend_string *str = zval_get_string(member);
    char *memberName=ZSTR_VAL(str);
    opencv_size_object *obj = Z_PHP_SIZE_OBJ_P(object);

    if(strcmp(memberName, "width") == 0 && obj->size->width!=(int)zval_get_long(value)){
        obj->size->width=(int)zval_get_long(value);
    }else if(strcmp(memberName, "height") == 0 && obj->size->height!=(int)zval_get_long(value)){
        obj->size->height=(int)zval_get_long(value);
    }
    zend_string_release(str);//free zend_string not memberName(zend_string->val)
    std_object_handlers.write_property(object,member,value,cache_slot);

}


/**
 * Size Init
 */
void opencv_size_init(int module_number){
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce,OPENCV_NS, "Size", opencv_size_methods);
    opencv_size_ce = zend_register_internal_class(&ce);

    opencv_size_ce->create_object = opencv_size_create_handler;
    memcpy(&opencv_size_object_handlers,
           zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    opencv_size_object_handlers.clone_obj = NULL;
    opencv_size_object_handlers.write_property = opencv_size_write_property;
    opencv_size_object_handlers.free_obj = opencv_size_free_obj;
}



//-----------------------------------【CV\Rect】---------------------------------------
//
//-------------------------------------------------------------------------------------


zend_object_handlers opencv_rect_object_handlers;

zend_object* opencv_rect_create_handler(zend_class_entry *type)
{
    int size = sizeof(opencv_rect_object);
    opencv_rect_object *obj = (opencv_rect_object *)ecalloc(1,size);
    memset(obj, 0, sizeof(opencv_rect_object));
    zend_object_std_init(&obj->std, type);
    object_properties_init(&obj->std, type);
    obj->std.ce = type;
    obj->std.handlers = &opencv_rect_object_handlers;
    return &obj->std;
}

void opencv_rect_free_obj(zend_object *object)
{
    opencv_rect_object *obj;
    obj = get_rect_obj(object);
    delete obj->rect;
    zend_object_std_dtor(object);
}

void opencv_rect_update_property_by_c_rect(zval *z, Rect *rect){
    zend_update_property_long(opencv_rect_ce, z, "x", sizeof("x")-1, rect->x);
    zend_update_property_long(opencv_rect_ce, z, "y", sizeof("y")-1, rect->y);
    zend_update_property_long(opencv_rect_ce, z, "width", sizeof("width")-1, rect->width);
    zend_update_property_long(opencv_rect_ce, z, "height", sizeof("height")-1, rect->height);
}

/**
 * Rect __construct
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_rect, __construct)
{
    long x = 0, y = 0, width = 0, height = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|llll", &x, &y, &width, &height) == FAILURE) {
        RETURN_NULL();
    }
    opencv_rect_object *obj = Z_PHP_RECT_OBJ_P(getThis());
    Rect rect = Rect((int)x, (int)y, (int)width, (int)height);
    obj->rect = new Rect(rect);
    opencv_rect_update_property_by_c_rect(getThis(),obj->rect);
}


/**
 * print Rect data
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_rect, print)
{
    opencv_rect_object *obj = Z_PHP_RECT_OBJ_P(getThis());
    std::cout <<*(obj->rect)<< std::endl;
    RETURN_NULL();
}

/**
 * ! the top-left corner
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_rect, tl)
{
    opencv_rect_object *rect_obj = Z_PHP_RECT_OBJ_P(getThis());
    zval point_zval;

    object_init_ex(&point_zval,opencv_point_ce);
    opencv_point_object *point_obj = Z_PHP_POINT_OBJ_P(&point_zval);
    point_obj->point = new Point(rect_obj->rect->tl());

    zend_update_property_long(opencv_point_ce, &point_zval, "x", sizeof("x")-1, point_obj->point->x);
    zend_update_property_long(opencv_point_ce, &point_zval, "y", sizeof("y")-1, point_obj->point->y);

    RETURN_ZVAL(&point_zval,0,0); //return php Point object
}

/**
 * ! the bottom-right corner
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_rect, br)
{
    opencv_rect_object *rect_obj = Z_PHP_RECT_OBJ_P(getThis());
    zval point_zval;

    object_init_ex(&point_zval,opencv_point_ce);
    opencv_point_object *point_obj = Z_PHP_POINT_OBJ_P(&point_zval);
    point_obj->point = new Point(rect_obj->rect->br());

    zend_update_property_long(opencv_point_ce, &point_zval, "x", sizeof("x")-1, point_obj->point->x);
    zend_update_property_long(opencv_point_ce, &point_zval, "y", sizeof("y")-1, point_obj->point->y);

    RETURN_ZVAL(&point_zval,0,0); //return php Point object
}

/**
 * size (width, height) of the rectangle
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_rect, size)
{
    opencv_rect_object *rect_obj = Z_PHP_RECT_OBJ_P(getThis());
    zval size_zval;

    object_init_ex(&size_zval,opencv_size_ce);
    opencv_size_object *size_obj = Z_PHP_SIZE_OBJ_P(&size_zval);
    size_obj->size = new Size(rect_obj->rect->size());

    zend_update_property_long(opencv_size_ce, &size_zval, "width", sizeof("width")-1, size_obj->size->width);
    zend_update_property_long(opencv_size_ce, &size_zval, "height", sizeof("height")-1, size_obj->size->height);

    RETURN_ZVAL(&size_zval,0,0); //return php Point object
}

/**
 * ! area (width*height) of the rectangle
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(opencv_rect, area)
{
    opencv_rect_object *rect_obj = Z_PHP_RECT_OBJ_P(getThis());
    long area = rect_obj->rect->area();
    RETURN_LONG(area);
}


/**
 * opencv_rect_methods[]
 */
const zend_function_entry opencv_rect_methods[] = {
        PHP_ME(opencv_rect, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
        PHP_ME(opencv_rect, print, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(opencv_rect, tl, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(opencv_rect, br, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(opencv_rect, size, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(opencv_rect, area, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};
/* }}} */


/**
 * Rect Class write_property
 * @param object
 * @param member
 * @param value
 * @param cache_slot
 */
void opencv_rect_write_property(zval *object, zval *member, zval *value, void **cache_slot){

    zend_string *str = zval_get_string(member);
    char *memberName=ZSTR_VAL(str);
    opencv_rect_object *obj = Z_PHP_RECT_OBJ_P(object);

    if(strcmp(memberName, "x") == 0 && obj->rect->x!=(int)zval_get_long(value)){
        obj->rect->x=(int)zval_get_long(value);
    }else if(strcmp(memberName, "y") == 0 && obj->rect->y!=(int)zval_get_long(value)){
        obj->rect->y=(int)zval_get_long(value);
    }else if(strcmp(memberName, "width") == 0 && obj->rect->width!=(int)zval_get_long(value)){
        obj->rect->width=(int)zval_get_long(value);
    }else if(strcmp(memberName, "height") == 0 && obj->rect->height!=(int)zval_get_long(value)){
        obj->rect->height=(int)zval_get_long(value);
    }
    zend_string_release(str);//free zend_string not memberName(zend_string->val)
    std_object_handlers.write_property(object,member,value,cache_slot);

}


/**
 * Scalar Init
 */
void opencv_rect_init(int module_number){
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce,OPENCV_NS, "Rect", opencv_rect_methods);
    opencv_rect_ce = zend_register_internal_class(&ce);

    opencv_rect_ce->create_object = opencv_rect_create_handler;
    memcpy(&opencv_rect_object_handlers,
           zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    opencv_rect_object_handlers.clone_obj = NULL;
    opencv_rect_object_handlers.write_property = opencv_rect_write_property;
    opencv_rect_object_handlers.free_obj = opencv_rect_free_obj;
}


/**
 * Type Init
 */
void opencv_type_init(int module_number){
    opencv_point_init(module_number);
    opencv_scalar_init(module_number);
    opencv_size_init(module_number);
    opencv_rect_init(module_number);
}