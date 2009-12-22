require 'pcre'

str =  "String11 abc string12
String21 string22 ABC
String31 ABC String32
"
p str
test = str.pcre_match('abc.*ABC',PCRE_DOTALL | PCRE_MULTILINE)
p test
