/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006,2007,2008 San Tai (Vanilla) Hsu                        |
  +----------------------------------------------------------------------+
  | This source file is subject to the BSD license, that is bundled with |
  | this package in the file LICENSE                                     |
  +----------------------------------------------------------------------+
  | Author:  San Tai (Vanilla) Hsu (vanilla@FreeBSD.org>                 |
  +----------------------------------------------------------------------+
*/
/* $Header: /home/ncvs/php_extension/cTemplate/cTemplate.cpp,v 1.12 2007/07/17 09:45:59 vanilla Exp $ */

extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_cTemplate.h"
#include "zend_exceptions.h"
#include "zend_object_handlers.h"
#include "zend_hash.h"
}

#include <string>
#include "google/template.h"
#include "google/template_from_string.h"
#include "google/template_dictionary.h"

using namespace std;
using namespace google;

class cTemplateTpl
{
public:
    Template *t;
    TemplateFromString *s;
    unsigned int is_from_string:1;

    cTemplateTpl();
    ~cTemplateTpl();
};

class cTemplateDict
{
public:
    TemplateDictionary d;
    TemplateDictionary *p;
    unsigned int is_root:1;

    cTemplateDict();
    ~cTemplateDict();
};

cTemplateTpl::cTemplateTpl () : t (), s (), is_from_string (0)
{
}

cTemplateTpl::~cTemplateTpl ()
{
    if (s)
        s->ClearCache ();

    if (t)
        t->ClearCache ();
}

cTemplateDict::cTemplateDict () : d ("default"), is_root (1)
{
}

cTemplateDict::~cTemplateDict ()
{
}

typedef struct
{
    zend_object std;
    cTemplateTpl *obj;
} php_cTemplateTpl;

typedef struct
{
    zend_object std;
    cTemplateDict *obj;
} php_cTemplateDict;

typedef struct
{
    const template_modifiers::TemplateModifier *m;
} minfo;

static minfo m[] = {
    { &template_modifiers::html_escape },
    { &template_modifiers::xml_escape },
    { &template_modifiers::javascript_escape },
    { &template_modifiers::json_escape },
    { &template_modifiers::url_query_escape },
    { &template_modifiers::pre_escape },
    { NULL }
};

static void cTemplateTpl_init (TSRMLS_D);
static void cTemplateDict_init (TSRMLS_D);
zend_class_entry *cTemplateTpl_ce, *cTemplateDict_ce;
static zend_object_handlers cTemplateTpl_object_handlers;
static zend_object_handlers cTemplateDict_object_handlers;
static void cTemplateTpl_free_storage (void *object TSRMLS_DC);
static void cTemplateDict_free_storage (void *object TSRMLS_DC);
static zend_object_value cTemplateTpl_object_new (zend_class_entry *ce TSRMLS_DC);
static zend_object_value cTemplateTpl_object_new_ex (zend_class_entry *ce, php_cTemplateTpl **ptr TSRMLS_DC);
static zend_object_value cTemplateTpl_object_clone (zval *this_ptr TSRMLS_DC);
static zend_object_value cTemplateDict_object_new (zend_class_entry *ce TSRMLS_DC);
static zend_object_value cTemplateDict_object_new_ex (zend_class_entry *ce, php_cTemplateDict **ptr TSRMLS_DC);
static zend_object_value cTemplateDict_object_clone (zval *this_ptr TSRMLS_DC);
static zval *cTemplateDict_instance (zend_class_entry *dict_ce, zval *object TSRMLS_DC);
static void _fill_dict (TemplateDictionary *d, HashTable *val, char *secName TSRMLS_DC);

zend_function_entry cTemplate_functions[] = {
    PHP_FE(cTemplate_reload, NULL)
    PHP_FE(cTemplate_clearcache, NULL)
    {NULL, NULL, NULL}
};

zend_function_entry cTemplateTpl_functions[] = {
    PHP_ME(cTemplateTpl, __construct, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, Expand, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, Dump, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, state, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, template_file, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, ReloadIfChanged, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, WriteHeaderEntries, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateTpl, __wakeup, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(cTemplateTpl, __sleep, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    { NULL, NULL, NULL }
};

zend_function_entry cTemplateDict_functions[] = {
    PHP_ME(cTemplateDict, __construct, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, SetArray, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, Set, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, SetEscaped, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, SetGlobal, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, SetTemplateGlobal, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, AddSection, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, Show, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, AddInclude, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, Filename, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, Dump, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, DumpToString, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, SetAnnotateOutput, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(cTemplateDict, __wakeup, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(cTemplateDict, __sleep, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    { NULL, NULL, NULL }
};

zend_module_entry cTemplate_module_entry = {
    STANDARD_MODULE_HEADER,
    "cTemplate",
    cTemplate_functions,
    PHP_MINIT(cTemplate),
    PHP_MSHUTDOWN(cTemplate),
    NULL,
    NULL,
    PHP_MINFO(cTemplate),
    "$Revision: 1.12 $",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CTEMPLATE
extern "C" {
ZEND_GET_MODULE(cTemplate)
}
#endif

PHP_MINIT_FUNCTION(cTemplate)
{
    REGISTER_LONG_CONSTANT ("DO_NOT_STRIP", DO_NOT_STRIP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("STRIP_BLANK_LINES", STRIP_BLANK_LINES, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("STRIP_WHITESPACE", STRIP_WHITESPACE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("NUM_STRIPS", NUM_STRIPS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TS_EMPTY", TS_EMPTY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TS_ERROR", TS_ERROR, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TS_READY", TS_READY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TS_SHOULD_RELOAD", TS_SHOULD_RELOAD, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TC_HTML", TC_HTML, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TC_JS", TC_JS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TC_CSS", TC_CSS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TC_JSON", TC_JSON, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("TC_XML", TC_XML, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("HTML_ESCAPE", 0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("XML_ESCAPE", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("JAVASCRIPT_ESCAPE", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("JSON_ESCAPE", 3, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("URL_ESCAPE", 4, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("PRE_ESCAPE", 5, CONST_CS | CONST_PERSISTENT);

    cTemplateTpl_init (TSRMLS_C);
    cTemplateDict_init (TSRMLS_C);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(cTemplate)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(cTemplate)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "cTemplate support", "enabled");
    php_info_print_table_end();
}

PHP_FUNCTION(cTemplate_reload)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Template::ReloadAllIfChanged();
    RETURN_TRUE;
}

PHP_FUNCTION(cTemplate_clearcache)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Template::ClearCache ();
    RETURN_TRUE;
}

PHP_METHOD (cTemplateTpl, __construct)
{
    zval *object = getThis(), *arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *arg4 = NULL;
    php_cTemplateTpl *tpl = NULL;


    if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "zz|zz", &arg1, &arg2, &arg3, &arg4) == FAILURE)
        RETURN_FALSE;

    if (Z_TYPE_P (arg2) == IS_LONG)
    {
        tpl = (php_cTemplateTpl*) zend_object_store_get_object(object TSRMLS_CC);

        if (ZEND_NUM_ARGS() >= 3 && Z_TYPE_P (arg3) == IS_STRING)
            Template::SetTemplateRootDirectory (Z_STRVAL_P (arg3));
        else
            Template::SetTemplateRootDirectory ("./");

        tpl->obj = new cTemplateTpl;

        if (ZEND_NUM_ARGS() == 4 && Z_TYPE_P (arg4) == IS_LONG)
            tpl->obj->t = Template::GetTemplateWithAutoEscaping (Z_STRVAL_P (arg1), (Strip) Z_LVAL_P (arg2), (TemplateContext) Z_LVAL_P(arg4));
        else
            tpl->obj->t = Template::GetTemplate (Z_STRVAL_P (arg1), (Strip) Z_LVAL_P (arg2));

        if (tpl->obj->t == NULL)
        {
            zend_throw_exception (zend_exception_get_default(TSRMLS_C), "get template fail", 0 TSRMLS_CC);
            return;
        }
        tpl->obj->s = NULL;
        tpl->obj->is_from_string = 0;
        return;
    }
    else if (Z_TYPE_P (arg2) == IS_STRING)
    {
        tpl = (php_cTemplateTpl*) zend_object_store_get_object(object TSRMLS_CC);
        Template::SetTemplateRootDirectory ("./");

        tpl->obj = new cTemplateTpl;

        if (ZEND_NUM_ARGS() == 4 && Z_TYPE_P (arg4) == IS_LONG)
            tpl->obj->s = TemplateFromString::GetTemplateWithAutoEscaping (Z_STRVAL_P (arg1), Z_STRVAL_P (arg2), (Strip) Z_LVAL_P (arg3), (TemplateContext) Z_LVAL_P (arg4));
        else
            tpl->obj->s = TemplateFromString::GetTemplate (Z_STRVAL_P (arg1), Z_STRVAL_P (arg2), (Strip) Z_LVAL_P (arg3));

        if (tpl->obj->s == NULL)
        {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "get template from string fail", 0 TSRMLS_CC);
            return;
        }
        tpl->obj->t = NULL;
        tpl->obj->is_from_string = 1;
        return;
    }

    ZVAL_NULL (object);
    RETURN_FALSE;
}

PHP_METHOD (cTemplateTpl, Expand)
{
    zval *val = NULL, **value_ptr, **value_ptr2;
    php_cTemplateTpl *tpl = NULL;
    php_cTemplateDict *dict = NULL;
    HashTable *ar = NULL;
    char *key = NULL;
    unsigned long idx, type;
    string ret;

    if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "z", &val) == FAILURE)
        RETURN_FALSE;

    tpl = (php_cTemplateTpl *) zend_object_store_get_object(getThis() TSRMLS_CC);
    if (tpl->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template object not exist", 0 TSRMLS_CC);
        return;
    }

    if (Z_TYPE_P (val) == IS_OBJECT)
    {
        dict = (php_cTemplateDict *) zend_object_store_get_object(val TSRMLS_CC);
        if (dict->obj == NULL)
        {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template dictionary object not exist", 0 TSRMLS_CC);
            return;
        }

        if (dict->obj->is_root)
        {
            if (tpl->obj->is_from_string)
                tpl->obj->s->Expand (&ret, &(dict->obj->d));
            else
                tpl->obj->t->Expand (&ret, &(dict->obj->d));
        }
        else
        {
            if (tpl->obj->is_from_string)
                tpl->obj->s->Expand (&ret, dict->obj->p);
            else
                tpl->obj->t->Expand (&ret, dict->obj->p);
        }
        RETURN_STRINGL ((char *)ret.c_str(), ret.length(), 1);
    }
    else if (Z_TYPE_P (val) == IS_ARRAY)
    {
        TemplateDictionary d ("default");

        ar = HASH_OF (val);

        for (zend_hash_internal_pointer_reset(ar);
            zend_hash_get_current_data(ar, (void **)&value_ptr ) == SUCCESS;
            zend_hash_move_forward(ar))
        {
            type = zend_hash_get_current_key (ar, &key, &idx, 0);
            if (type != HASH_KEY_IS_STRING)
            {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "all element keys must be string, not index", 0 TSRMLS_CC);
                return;
            }

            switch (Z_TYPE_PP (value_ptr))
            {
                case IS_DOUBLE:
                    d.SetIntValue (key, Z_DVAL_PP (value_ptr));
                    break;
                case IS_STRING:
                    d.SetValue (key, Z_STRVAL_PP (value_ptr));
                    break;
                case IS_LONG:
                    d.SetIntValue (key, Z_LVAL_PP (value_ptr));
                    break;
                case IS_ARRAY:
                    for (zend_hash_internal_pointer_reset(Z_ARRVAL_PP (value_ptr));
                        zend_hash_get_current_data (Z_ARRVAL_PP (value_ptr), (void **)&value_ptr2 ) == SUCCESS;
                        zend_hash_move_forward( Z_ARRVAL_PP (value_ptr)))
                    {
                        _fill_dict (&d, Z_ARRVAL_PP (value_ptr2), key TSRMLS_CC);
                    }
                    break;
                case IS_BOOL:
                    convert_to_boolean_ex (value_ptr);
                    if (zval_is_true (*value_ptr))
                        d.ShowSection (key);
                    break;
                default:
                    php_error (E_WARNING, "default");
            }
        }

        if (tpl->obj->is_from_string)
            tpl->obj->s->Expand (&ret, &d);
        else
            tpl->obj->t->Expand (&ret, &d);
        
        RETURN_STRINGL ((char *)ret.c_str(), ret.length(), 1);
    }

    RETURN_FALSE;
}

PHP_METHOD (cTemplateTpl, Dump)
{
    php_cTemplateTpl *tpl = NULL;

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    tpl = (php_cTemplateTpl *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (tpl->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template object not exist", 0 TSRMLS_CC);
        return;
    }

    if (tpl->obj->is_from_string)
        tpl->obj->s->Dump ("/dev/stdout");
    else
        tpl->obj->t->Dump ("/dev/stdout");

    RETURN_TRUE;
}

PHP_METHOD (cTemplateTpl, state)
{
    php_cTemplateTpl *tpl = NULL;
    long state;

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    tpl = (php_cTemplateTpl *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (tpl->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template object not exist", 0 TSRMLS_CC);
        return;
    }

    if (tpl->obj->is_from_string)
        state = tpl->obj->s->state ();
    else
        state = tpl->obj->t->state ();

    RETURN_LONG (state);
}

PHP_METHOD (cTemplateTpl, template_file)
{
    php_cTemplateTpl *tpl = NULL;

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    tpl = (php_cTemplateTpl *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (tpl->obj == NULL)
    {
       zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template object not exist", 0 TSRMLS_CC);
        return;
    }

    if (tpl->obj->is_from_string)
    {
        RETURN_STRING ("", 1);
    }
    else
    {
        RETURN_STRING ((char *) tpl->obj->t->template_file(), 1);
    }
}

PHP_METHOD (cTemplateTpl, ReloadIfChanged)
{
    php_cTemplateTpl *tpl = NULL;
    zend_bool b;

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    tpl = (php_cTemplateTpl *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (tpl->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template object not exist", 0 TSRMLS_CC);
        return;
    }

    if (tpl->obj->is_from_string)
        b = true;
    else
        b = tpl->obj->t->ReloadIfChanged();

    RETURN_BOOL (b);
}

PHP_METHOD (cTemplateTpl, WriteHeaderEntries)
{
    php_cTemplateTpl *tpl = NULL;
    string ret;

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    tpl = (php_cTemplateTpl *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (tpl->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Template object not exist", 0 TSRMLS_CC);
        return;
    }

    if (tpl->obj->is_from_string)
        tpl->obj->s->WriteHeaderEntries (&ret);
    else
        tpl->obj->t->WriteHeaderEntries (&ret);

    RETURN_STRINGL ((char *)ret.c_str(), ret.length(), 1);
}

PHP_METHOD (cTemplateTpl, __wakeup)
{
    zend_throw_exception (zend_exception_get_default(TSRMLS_C), "You cannot serialize or unserialize cTemplateTpl instances", 0 TSRMLS_CC);
}

PHP_METHOD (cTemplateTpl, __sleep)
{
    zend_throw_exception (zend_exception_get_default(TSRMLS_C), "You cannot serialize or unserialize cTemplateTpl instances", 0 TSRMLS_CC);
}

PHP_METHOD (cTemplateDict, __construct)
{
    php_cTemplateDict *dict = NULL;

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    dict = (php_cTemplateDict *) zend_object_store_get_object(getThis() TSRMLS_CC);
    dict->obj = new cTemplateDict;
    return;
}

PHP_METHOD(cTemplateDict, SetArray)
{
    php_cTemplateDict *dict = NULL;
    zval *array = NULL, **value_ptr;
    HashTable *ar = NULL;
    char *key = NULL;
    unsigned long idx, type;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC, "a", &array) == FAILURE)
        RETURN_FALSE;

    ar = HASH_OF (array);

    for (zend_hash_internal_pointer_reset(ar);
        zend_hash_get_current_data(ar, (void **)&value_ptr ) == SUCCESS;
        zend_hash_move_forward(ar))
    {
        type = zend_hash_get_current_key (ar, &key, &idx, 0);
        if (type != HASH_KEY_IS_STRING)
        {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "all element keys must be string, not index", 0 TSRMLS_CC);
            return;
        }

        convert_to_string_ex (value_ptr);
        if (dict->obj->is_root)
            dict->obj->d.SetValue (key, Z_STRVAL_PP (value_ptr));
        else
            dict->obj->p->SetValue (key, Z_STRVAL_PP (value_ptr));
    }

    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, Set)
{
    php_cTemplateDict *dict = NULL;
    const char *key = NULL, *sec = NULL;
    int key_len, sec_len;
    zval *val = NULL;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC, "sz|s", &key, &key_len, &val, &sec, &sec_len) == FAILURE)
        RETURN_FALSE;

    if (Z_TYPE_P(val) == IS_STRING)
    {
        if (sec != NULL)
        {
            if (dict->obj->is_root)
                dict->obj->d.SetValueAndShowSection (key, Z_STRVAL_P(val), sec);
            else
                dict->obj->p->SetValueAndShowSection (key, Z_STRVAL_P(val), sec);
        }
        else
        {
            if (dict->obj->is_root)
                dict->obj->d.SetValue (key, Z_STRVAL_P(val));
            else
                dict->obj->p->SetValue (key, Z_STRVAL_P(val));
        }
        RETURN_TRUE;
    }
    else if (Z_TYPE_P(val) == IS_LONG)
    {
        if (sec != NULL)
        {
            if (dict->obj->is_root)
            {
                dict->obj->d.SetIntValue (key, Z_LVAL_P(val));
                dict->obj->d.ShowSection (sec);
            }
            else
            {
                dict->obj->p->SetIntValue (key, Z_LVAL_P(val));
                dict->obj->p->ShowSection (sec);
            }
        }
        else
        {
            if (dict->obj->is_root)
                dict->obj->d.SetIntValue (key, Z_LVAL_P(val));
            else
                dict->obj->p->SetIntValue (key, Z_LVAL_P(val));
        }
        RETURN_TRUE;
    }
    else
        RETURN_FALSE;
}

PHP_METHOD(cTemplateDict, SetEscaped)
{
    php_cTemplateDict *dict = NULL;
    const char *key = NULL, *val = NULL, *sec = NULL;
    int key_len, val_len, sec_len;
    long e;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl|s", &key, &key_len, &val, &val_len, &e, &sec, &sec_len) == FAILURE)
        RETURN_FALSE;

    if (sec != NULL)
    {
        if (dict->obj->is_root)
            dict->obj->d.SetEscapedValueAndShowSection (key, val, *(m[e].m), sec);
        else
            dict->obj->p->SetEscapedValueAndShowSection (key, val, *(m[e].m), sec);
    }
    else
    {
        if (dict->obj->is_root)
            dict->obj->d.SetEscapedValue (key, val, *(m[e].m));
        else
            dict->obj->p->SetEscapedValue (key, val, *(m[e].m));
    }
    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, SetGlobal)
{
    php_cTemplateDict *dict = NULL;
    const char *key = NULL, *val = NULL;
    int key_len, val_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &key_len, &val, &val_len) == FAILURE)
        RETURN_FALSE;

    if (dict->obj->is_root)
        dict->obj->d.SetGlobalValue (key, val);
    else
        dict->obj->p->SetGlobalValue (key, val);

    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, SetTemplateGlobal)
{
    php_cTemplateDict *dict = NULL;
    const char *key = NULL, *val = NULL;
    int key_len, val_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &key_len, &val, &val_len) == FAILURE)
        RETURN_FALSE;

    if (dict->obj->is_root)
        dict->obj->d.SetTemplateGlobalValue (key, val);
    else
        dict->obj->p->SetTemplateGlobalValue (key, val);

    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, AddSection)
{
    php_cTemplateDict *dict = NULL, *new_dict = NULL;
    const char *sec = NULL;
    int sec_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &sec, &sec_len) == FAILURE)
        RETURN_NULL ();

    cTemplateDict_instance (cTemplateDict_ce, return_value TSRMLS_CC);
    new_dict = (php_cTemplateDict *) zend_object_store_get_object (return_value TSRMLS_CC);
    new_dict->obj = new cTemplateDict;

    if (new_dict->obj)
    {
        zend_objects_store_add_ref (getThis() TSRMLS_CC);

        if (dict->obj->is_root)
            new_dict->obj->p = dict->obj->d.AddSectionDictionary (sec);
        else
            new_dict->obj->p = dict->obj->p->AddSectionDictionary (sec);
        new_dict->obj->is_root = 0;

        return;
    }

    zval_dtor (return_value);

    RETURN_FALSE;
}

PHP_METHOD(cTemplateDict, Show)
{
    php_cTemplateDict *dict = NULL;
    const char *sec = NULL;
    int sec_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &sec, &sec_len) == FAILURE)
        RETURN_FALSE;

    if (dict->obj->is_root)
        dict->obj->d.ShowSection (sec);
    else
        dict->obj->p->ShowSection (sec);

    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, AddInclude)
{
    php_cTemplateDict *dict = NULL, *new_dict = NULL;
    const char *sec = NULL;
    int sec_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &sec, &sec_len) == FAILURE)
        RETURN_NULL ();

    cTemplateDict_instance (cTemplateDict_ce, return_value TSRMLS_CC);
    new_dict = (php_cTemplateDict *) zend_object_store_get_object (return_value TSRMLS_CC);
    new_dict->obj = new cTemplateDict;

    if (new_dict->obj)
    {
        if (dict->obj->is_root)
            new_dict->obj->p = dict->obj->d.AddIncludeDictionary (sec);
        else
            new_dict->obj->p = dict->obj->p->AddIncludeDictionary (sec);
        new_dict->obj->is_root = 0;

        return;
    }

    zval_dtor (return_value);

    RETURN_FALSE;
}

PHP_METHOD(cTemplateDict, Filename)
{
    php_cTemplateDict *dict = NULL;
    const char *fn = NULL;
    char *out = NULL;
    int fn_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &fn, &fn_len) == FAILURE)
        RETURN_FALSE;

    out = php_trim ((char *)fn, fn_len, NULL, 0, NULL, 3 TSRMLS_CC);

    if (dict->obj->is_root)
        dict->obj->d.SetFilename (out);
    else
        dict->obj->p->SetFilename (out);

    efree (out);
    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, Dump)
{
    php_cTemplateDict *dict = NULL;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    if (dict->obj->is_root)
        dict->obj->d.Dump ();
    else
        dict->obj->p->Dump ();

    RETURN_TRUE;
}

PHP_METHOD(cTemplateDict, DumpToString)
{
    php_cTemplateDict *dict = NULL;
    string ret;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    if (dict->obj->is_root)
        dict->obj->d.DumpToString (&ret);
    else
        dict->obj->p->DumpToString (&ret);

    RETURN_STRINGL ((char *)ret.c_str(), ret.length(), 1);
}

PHP_METHOD(cTemplateDict, SetAnnotateOutput)
{
    php_cTemplateDict *dict = NULL;
    const char *path = NULL;
    int path_len;

    dict = (php_cTemplateDict *) zend_object_store_get_object (getThis() TSRMLS_CC);
    if (dict->obj == NULL)
    {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Dictionary object not exist", 0 TSRMLS_CC);
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    if (dict->obj->is_root)
        dict->obj->d.SetAnnotateOutput (path);
    else
        dict->obj->p->SetAnnotateOutput (path);

    RETURN_TRUE;
}

PHP_METHOD (cTemplateDict, __wakeup)
{
    zend_throw_exception (zend_exception_get_default(TSRMLS_C), "You cannot serialize or unserialize cTemplateDict instances", 0 TSRMLS_CC);
}

PHP_METHOD (cTemplateDict, __sleep)
{
    zend_throw_exception (zend_exception_get_default(TSRMLS_C), "You cannot serialize or unserialize cTemplateDict instances", 0 TSRMLS_CC);
}

static void cTemplateTpl_free_storage (void *object TSRMLS_DC)
{
    php_cTemplateTpl *tpl = (php_cTemplateTpl *)object;

    if (tpl->obj)
        delete tpl->obj;

    zend_object_std_dtor (&tpl->std TSRMLS_CC);
    efree(object);
}

static void cTemplateDict_free_storage (void *object TSRMLS_DC)
{
    php_cTemplateDict *dict = (php_cTemplateDict *)object;

    if (dict->obj)
        delete dict->obj;

    zend_object_std_dtor (&dict->std TSRMLS_CC);
    efree(object);
}

static zend_object_value cTemplateTpl_object_new_ex (zend_class_entry *ce, php_cTemplateTpl **ptr TSRMLS_DC)
{
    zend_object_value retval;
    php_cTemplateTpl *tpl = NULL;
    zval *tmp;

    tpl = (php_cTemplateTpl *) emalloc (sizeof (*tpl));
    memset (tpl, 0, sizeof (php_cTemplateTpl));
    if (ptr)
        *ptr = tpl;

    zend_object_std_init (&tpl->std, ce TSRMLS_CC);
    zend_hash_copy (tpl->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof (zval *));

    retval.handle = zend_objects_store_put (tpl, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t)cTemplateTpl_free_storage, NULL TSRMLS_CC);
    retval.handlers = &cTemplateTpl_object_handlers;
    return retval;
}

static zend_object_value cTemplateDict_object_new_ex (zend_class_entry *ce, php_cTemplateDict **ptr TSRMLS_DC)
{
    zend_object_value retval;
    php_cTemplateDict *dict = NULL;
    zval *tmp;

    dict = (php_cTemplateDict *) emalloc (sizeof (php_cTemplateDict));
    memset (dict, 0, sizeof (php_cTemplateDict));
    if (ptr)
        *ptr = dict;

    zend_object_std_init (&dict->std, ce TSRMLS_CC);
    zend_hash_copy (dict->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof (zval *));

    retval.handle = zend_objects_store_put (dict, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t)cTemplateDict_free_storage, NULL TSRMLS_CC);
    retval.handlers = &cTemplateDict_object_handlers;
    return retval;
}

static zend_object_value cTemplateTpl_object_new (zend_class_entry *ce TSRMLS_DC)
{
    return cTemplateTpl_object_new_ex (ce, NULL TSRMLS_CC);
}

static zend_object_value cTemplateDict_object_new (zend_class_entry *ce TSRMLS_DC)
{
    return cTemplateDict_object_new_ex (ce, NULL TSRMLS_CC);
}

static zend_object_value cTemplateTpl_object_clone (zval *this_ptr TSRMLS_DC)
{
    php_cTemplateTpl *new_obj = NULL;
    php_cTemplateTpl *old_obj = (php_cTemplateTpl *) zend_object_store_get_object (this_ptr TSRMLS_CC);
    zend_object_value new_ov = cTemplateTpl_object_new_ex (old_obj->std.ce, &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    return new_ov;
}

static zend_object_value cTemplateDict_object_clone (zval *this_ptr TSRMLS_DC)
{
    php_cTemplateDict *new_obj = NULL;
    php_cTemplateDict *old_obj = (php_cTemplateDict *) zend_object_store_get_object (this_ptr TSRMLS_CC);
    zend_object_value new_ov = cTemplateDict_object_new_ex (old_obj->std.ce, &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    return new_ov;
}

void cTemplateTpl_init (TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY (ce, "cTemplate", cTemplateTpl_functions);
    ce.create_object = cTemplateTpl_object_new;
    cTemplateTpl_ce = zend_register_internal_class_ex (&ce, NULL, NULL TSRMLS_CC);
    memcpy (&cTemplateTpl_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    cTemplateTpl_object_handlers.clone_obj = cTemplateTpl_object_clone;
}

void cTemplateDict_init (TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY (ce, "cTemplate_Dict", cTemplateDict_functions);
    ce.create_object = cTemplateDict_object_new;
    cTemplateDict_ce = zend_register_internal_class_ex (&ce, NULL, NULL TSRMLS_CC);
    memcpy (&cTemplateDict_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    cTemplateDict_object_handlers.clone_obj = cTemplateDict_object_clone;
}

static zval *cTemplateDict_instance (zend_class_entry *dict_ce, zval *object TSRMLS_DC)
{
    if (!object)
    {
        ALLOC_ZVAL (object);
    }

    Z_TYPE_P(object) = IS_OBJECT;
    object_init_ex(object, dict_ce);
    object->refcount = 1;
    object->is_ref = 1;

    return object;
}

static void _fill_dict (TemplateDictionary *d, HashTable *val, char *secName TSRMLS_DC)
{
    zval **value_ptr;
    char *key = NULL;
    unsigned long type, idx;
    TemplateDictionary *e = d->AddSectionDictionary (secName);

    for (zend_hash_internal_pointer_reset(val);
        zend_hash_get_current_data(val, (void **)&value_ptr ) == SUCCESS;
        zend_hash_move_forward(val))
    {
        type = zend_hash_get_current_key (val, &key, &idx, 0);

        switch (Z_TYPE_PP (value_ptr))
        {
            case IS_DOUBLE:
                e->SetIntValue (key, Z_DVAL_PP (value_ptr));
                break;
            case IS_STRING:
                e->SetValue (key, Z_STRVAL_PP (value_ptr));
                break;
            case IS_LONG:
                e->SetIntValue (key, Z_LVAL_PP (value_ptr));
                break;
            case IS_BOOL:
                convert_to_boolean_ex (value_ptr);
                if (zval_is_true (*value_ptr))
                    e->ShowSection (key);
                break;
            default:
                php_error (E_WARNING, "default");
        }
    }
}
/*
 * vim:ts=4:expandtab:sw=4
 */
