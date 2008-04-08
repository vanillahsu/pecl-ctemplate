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

$tpl = new cTemplate ("test", $a, STRIP_BLANK_LINES);
$winnings = rand() % 100000;
$value = sprintf ("%.2f", $winnings * 0.83);

$array['NAME'] = 'John Smith';
$array['VALUE'] = $winnings;
$array['TAXED_VALUE'] = $value;
$array['IN_CA'] = true;
$array['ONE_RESULT'] = array();

for ($i=0;$i<10;$i++)
{
	$a2['I'] = $i;
	$a2['SUBITEM_SECTION'] = true;
	$a2['JUMP_TO_URL'] = '<script>http://fatpipi.com/1.php?tes</script>';
	array_push ($array['ONE_RESULT'], $a2);
}
$out = $tpl->Expand ($array);
echo $out;
?>
===DONE===
--EXPECTF--
Hello John Smith,
   You have just won $ %d!
   Well, $ %f, after taxes.
<blockquota>
0
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
1
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
2
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
3
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
4
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
5
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
6
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
7
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
8
<script>http://fatpipi.com/1.php?tes</script>
<blockquota>
9
<script>http://fatpipi.com/1.php?tes</script>
===DONE===
