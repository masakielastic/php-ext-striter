<?php
// Basic test for str_iter function

// Test 1: ASCII string
echo "Test 1: ASCII string\n";
$str1 = "Hello";
$iter1 = str_iter($str1);
foreach ($iter1 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 2: UTF-8 string with various characters
echo "Test 2: UTF-8 string\n";
$str2 = "こんにちは";
$iter2 = str_iter($str2);
foreach ($iter2 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 3: Mixed ASCII and UTF-8
echo "Test 3: Mixed ASCII and UTF-8\n";
$str3 = "Hello世界";
$iter3 = str_iter($str3);
foreach ($iter3 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 4: Empty string
echo "Test 4: Empty string\n";
$str4 = "";
$iter4 = str_iter($str4);
foreach ($iter4 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "Empty string: no output expected\n";
echo "\n";

// Test 5: String with emoji
echo "Test 5: String with emoji\n";
$str5 = "Hello🌍";
$iter5 = str_iter($str5);
foreach ($iter5 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 6: Manual iterator methods
echo "Test 6: Manual iterator methods\n";
$iter6 = str_iter("ABC");
$iter6->rewind();
while ($iter6->valid()) {
    echo "Key: " . $iter6->key() . ", Value: '" . $iter6->current() . "'\n";
    $iter6->next();
}
echo "\n";

echo "All tests completed!\n";
?>