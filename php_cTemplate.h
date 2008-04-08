/*
 *   +----------------------------------------------------------------------+
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
/* $Header: /home/ncvs/php_extension/cTemplate/php_cTemplate.h,v 1.5 2007/06/17 03:31:27 vanilla Exp $ */

#ifndef PHP_CTEMPLATE_H
#define PHP_CTEMPLATE_H

extern zend_module_entry cTemplate_module_entry;
#define phpext_cTemplate_ptr &cTemplate_module_entry

#ifdef PHP_WIN32
#define PHP_CTEMPLATE_API __declspec(dllexport)
#else
#define PHP_CTEMPLATE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(cTemplate);
PHP_MSHUTDOWN_FUNCTION(cTemplate);
PHP_MINFO_FUNCTION(cTemplate);

PHP_FUNCTION(cTemplate_reload);
PHP_FUNCTION(cTemplate_clearcache);

PHP_METHOD(cTemplateTpl, __construct);
PHP_METHOD(cTemplateTpl, Expand);
PHP_METHOD(cTemplateTpl, Dump);
PHP_METHOD(cTemplateTpl, state);
PHP_METHOD(cTemplateTpl, template_file);
PHP_METHOD(cTemplateTpl, ReloadIfChanged);
PHP_METHOD(cTemplateTpl, WriteHeaderEntries);
PHP_METHOD(cTemplateTpl, __wakeup);
PHP_METHOD(cTemplateTpl, __sleep);

PHP_METHOD(cTemplateDict, __construct);
PHP_METHOD(cTemplateDict, SetArray);
PHP_METHOD(cTemplateDict, Set);
PHP_METHOD(cTemplateDict, SetEscaped);
PHP_METHOD(cTemplateDict, SetGlobal);
PHP_METHOD(cTemplateDict, SetTemplateGlobal);
PHP_METHOD(cTemplateDict, AddSection);
PHP_METHOD(cTemplateDict, Show);
PHP_METHOD(cTemplateDict, AddInclude);
PHP_METHOD(cTemplateDict, Filename);
PHP_METHOD(cTemplateDict, Dump);
PHP_METHOD(cTemplateDict, DumpToString);
PHP_METHOD(cTemplateDict, SetAnnotateOutput);
PHP_METHOD(cTemplateDict, __wakeup);
PHP_METHOD(cTemplateDict, __sleep);

#ifdef ZTS
#define CTEMPLATE_G(v) TSRMG(cTemplate_globals_id, zend_cTemplate_globals *, v)
#else
#define CTEMPLATE_G(v) (cTemplate_globals.v)
#endif

#endif	/* PHP_CTEMPLATE_H */
