--TEST--
cTemplate: template_dict::Dump test
--SKIPIF--
<?php if (!extension_loaded("cTemplate")) print "skip"; ?>
--FILE--
<?php
$dict = new cTemplate_Dict ();
$dict->set ("NAME", "John Smith");

$dict->set ("VALUE", '1234');
$dict->dump ();
?>
===DONE===
--EXPECTF--
global dictionary {
   BI_NEWLINE: >
<
   BI_SPACE: > <
};
dictionary 'default' {
   NAME: >John Smith<
   VALUE: >1234<
}
===DONE===
