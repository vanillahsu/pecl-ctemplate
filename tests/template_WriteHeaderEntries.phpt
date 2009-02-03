--TEST--
cTemplate: template::WriteHeaderEntries() test
--SKIPIF--
<?php if (!extension_loaded("cTemplate")) print "skip"; ?>
--FILE--
<?php
$tpl = new cTemplate ("example.tpl", STRIP_BLANK_LINES);
echo $tpl->WriteHeaderEntries();
?>
===DONE===
--EXPECTF--
#include <google/template_string.h>
static const StaticTemplateString ke_NAME = STS_INIT_WITH_HASH(ke_NAME, "NAME", 11433977280339851189LLU);
static const StaticTemplateString ke_VALUE = STS_INIT_WITH_HASH(ke_VALUE, "VALUE", 3571384048577261229LLU);
static const StaticTemplateString ke_IN_CA = STS_INIT_WITH_HASH(ke_IN_CA, "IN_CA", 15931064057564334051LLU);
static const StaticTemplateString ke_TAXED_VALUE = STS_INIT_WITH_HASH(ke_TAXED_VALUE, "TAXED_VALUE", 10685086013714446631LLU);
===DONE===
