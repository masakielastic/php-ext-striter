<?php
// Test for invalid UTF-8 byte sequences

echo "Test: Invalid UTF-8 byte sequences\n";

// Test 1: Invalid UTF-8 bytes
echo "Test 1: Invalid UTF-8 bytes\n";
$str1 = "Hello\xFF\xFE World";
$iter1 = str_iter($str1);
foreach ($iter1 as $key => $char) {
    echo "[$key] => '" . bin2hex($char) . "' (hex)\n";
}
echo "\n";

// Test 2: Truncated UTF-8 sequence
echo "Test 2: Truncated UTF-8 sequence\n";
$str2 = "Hello\xE3\x81 World";  // Incomplete UTF-8 sequence
$iter2 = str_iter($str2);
foreach ($iter2 as $key => $char) {
    echo "[$key] => '" . bin2hex($char) . "' (hex)\n";
}
echo "\n";

// Test 3: Mixed valid and invalid UTF-8
echo "Test 3: Mixed valid and invalid UTF-8\n";
$str3 = "こ\xFF\xFEん";
$iter3 = str_iter($str3);
foreach ($iter3 as $key => $char) {
    echo "[$key] => '" . bin2hex($char) . "' (hex)\n";
}
echo "\n";

echo "Invalid UTF-8 tests completed!\n";
?>