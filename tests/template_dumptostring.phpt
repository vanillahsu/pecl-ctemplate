--TEST--
cTemplate: template::from_string & template_dict::DumpToString test
--SKIPIF--
<?php if (!extension_loaded("cTemplate")) print "skip"; ?>
--FILE--
<?php
$a =<<<EOT
Hello {{NAME}},
   You have just won $ {{VALUE}}!
   {{#IN_CA}}Well, $ {{TAXED_VALUE}}, after taxes.{{/IN_CA}}

{{#ONE_RESULT}}
{{#SUBITEM_SECTION}}<blockquota>{{/SUBITEM_SECTION}}
{{I}}
{{JUMP_TO_URL}}
{{/ONE_RESULT}}
EOT;

$tpl = new cTemplate ("test", $a, STRIP_BLANK_LINES);
$dict = new cTemplate_Dict ();
$dict->Set ("NAME", "John Smith");
$winnings = rand() % 100000;
$dict->Set ("VALUE", $winnings, "IN_CA");
$value = sprintf ("%.2f", $winnings * 0.83);
$dict->Set ("TAXED_VALUE", $value);

for ($i=0;$i<10;$i++)
{
	$one = $dict->AddSection ("ONE_RESULT");
	$one->Set ("I", $i);
	$one->Show ("SUBITEM_SECTION");
	$one->SetEscaped ("JUMP_TO_URL", "<script>http://fatpipi.com/1.php?tes</script>", JAVASCRIPT_ESCAPE);
}

$out = $tpl->DumpToString ();
echo $out;
?>
===DONE===
--EXPECTF--
------------Start Template Dump [/dev/null]--------------
  Section Start: __{{MAIN}}__
    Text Node: -->|Hello |<--
    Variable Node: NAME
    Text Node: -->|,
   You have just won $ |<--
    Variable Node: VALUE
    Text Node: -->|!
   |<--
    Section Start: IN_CA
      Text Node: -->|Well, $ |<--
      Variable Node: TAXED_VALUE
      Text Node: -->|, after taxes.|<--
    Section End: IN_CA
    Text Node: -->|
|<--
    Section Start: ONE_RESULT
      Section Start: SUBITEM_SECTION
        Text Node: -->|<blockquota>|<--
      Section End: SUBITEM_SECTION
      Text Node: -->|
|<--
      Variable Node: I
      Text Node: -->|
|<--
      Variable Node: JUMP_TO_URL
      Text Node: -->|
|<--
    Section End: ONE_RESULT
  Section End: __{{MAIN}}__
------------End Template Dump----------------
===DONE===
