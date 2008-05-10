--TEST--
cTemplate: google cTemplate template engine
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

$tpl = new cTemplate ("test", $a, STRIP_BLANK_LINES, TC_HTML);
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
	$one->Set ("JUMP_TO_URL", "<script>http://fatpipi.com/1.php?tes</script>");
}

$out = $tpl->Expand ($dict);
echo $out;
?>
===DONE===
--EXPECTF--
Hello John Smith,
   You have just won $ %d!
   Well, $ %f, after taxes.
<blockquota>
0
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
1
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
2
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
3
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
4
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
5
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
6
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
7
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
8
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
<blockquota>
9
&lt;script&gt;http://fatpipi.com/1.php?tes&lt;/script&gt;
===DONE===
