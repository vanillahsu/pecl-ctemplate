--TEST--
cTemplate: template::state() test
--SKIPIF--
<?php if (!extension_loaded("cTemplate")) print "skip"; ?>
--FILE--
<?php
$tpl = new cTemplate ("example.tpl", STRIP_BLANK_LINES);
echo $tpl->state ()."\n";
?>
===DONE===
--EXPECTF--
3
===DONE===
