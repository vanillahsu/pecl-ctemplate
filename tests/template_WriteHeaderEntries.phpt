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
const char * const ke_NAME = "NAME";
const char * const ke_VALUE = "VALUE";
const char * const ke_IN_CA = "IN_CA";
const char * const ke_TAXED_VALUE = "TAXED_VALUE";
===DONE===
