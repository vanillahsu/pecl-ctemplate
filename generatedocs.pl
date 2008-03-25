#!/usr/local/bin/perl -wp

BEGIN
{
   print qq(
/** \\file $ARGV[0]
 */
);
   $/=" }}} */" if $ARGV[0] =~ /cpp$/;
}

while(/#include/) {
   s/^#include.*$//gm;
};

if(/PHP_FUNCTION/) {
   s/[\[\]]//g;
   s/{{{ *(?:proto)? *(\w+).*?\((.*?)\)(.*?)PHP_FUNCTION\((.*?)\)/$3$1 $4($2)/sg;
}

s/REGISTER_LONG_CONSTANT\("(.*?)", (.*?),.*/define("$1", $2);/;

END
{
}
