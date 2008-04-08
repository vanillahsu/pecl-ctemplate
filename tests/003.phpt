--TEST--
cTemplate: google cTemplate template engine
--SKIPIF--
<?php if (!extension_loaded("cTemplate")) print "skip"; ?>
--FILE--
<?php
$tpl = new cTemplate ("example.tpl", STRIP_BLANK_LINES);

$winnings = rand() % 100000;
$value = sprintf ("%.2f", $winnings * 0.83);

$array['NAME'] = 'John Smith';
$array['VALUE'] = $winnings;
$array['IN_CA'] = true;
$array['TAXED_VALUE'] = $value;

$out = $tpl->expand ($array);
echo $out;
?>
===DONE===
--EXPECTF--
Hello John Smith,
   You have just won $%d!
   Well, $%f, after taxes.
===DONE===
