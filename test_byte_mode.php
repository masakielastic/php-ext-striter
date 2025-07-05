<?php
// Test for byte mode

echo "Test: Byte mode functionality\n";
echo "==============================\n\n";

// Test 1: ASCII string - all three modes comparison
echo "Test 1: ASCII string 'Hello'\n";
$str1 = "Hello";
echo "String length: " . strlen($str1) . " bytes\n";

echo "Grapheme mode:\n";
$iter1_g = str_iter($str1, "grapheme");
foreach ($iter1_g as $key => $char) {
    echo "  [$key] => '$char' (hex: " . bin2hex($char) . ")\n";
}

echo "Codepoint mode:\n";
$iter1_c = str_iter($str1, "codepoint");
foreach ($iter1_c as $key => $char) {
    echo "  [$key] => '$char' (hex: " . bin2hex($char) . ")\n";
}

echo "Byte mode:\n";
$iter1_b = str_iter($str1, "byte");
foreach ($iter1_b as $key => $byte) {
    echo "  [$key] => '$byte' (hex: " . bin2hex($byte) . ", decimal: " . ord($byte) . ")\n";
}
echo "\n";

// Test 2: UTF-8 string with Japanese characters
echo "Test 2: UTF-8 string 'こんにちは'\n";
$str2 = "こんにちは";
echo "String length: " . strlen($str2) . " bytes\n";

echo "Grapheme mode:\n";
$iter2_g = str_iter($str2, "grapheme");
foreach ($iter2_g as $key => $char) {
    echo "  [$key] => '$char' (bytes: " . strlen($char) . ", hex: " . bin2hex($char) . ")\n";
}

echo "Codepoint mode:\n";
$iter2_c = str_iter($str2, "codepoint");
foreach ($iter2_c as $key => $char) {
    echo "  [$key] => '$char' (bytes: " . strlen($char) . ", hex: " . bin2hex($char) . ")\n";
}

echo "Byte mode:\n";
$iter2_b = str_iter($str2, "byte");
foreach ($iter2_b as $key => $byte) {
    echo "  [$key] => '$byte' (hex: " . bin2hex($byte) . ", decimal: " . ord($byte) . ")\n";
}
echo "\n";

// Test 3: Emoji string
echo "Test 3: Emoji string '🌸🌺'\n";
$str3 = "🌸🌺";
echo "String length: " . strlen($str3) . " bytes\n";

echo "Grapheme mode:\n";
$iter3_g = str_iter($str3, "grapheme");
foreach ($iter3_g as $key => $char) {
    echo "  [$key] => '$char' (bytes: " . strlen($char) . ", hex: " . bin2hex($char) . ")\n";
}

echo "Codepoint mode:\n";
$iter3_c = str_iter($str3, "codepoint");
foreach ($iter3_c as $key => $char) {
    echo "  [$key] => '$char' (bytes: " . strlen($char) . ", hex: " . bin2hex($char) . ")\n";
}

echo "Byte mode:\n";
$iter3_b = str_iter($str3, "byte");
foreach ($iter3_b as $key => $byte) {
    echo "  [$key] => '$byte' (hex: " . bin2hex($byte) . ", decimal: " . ord($byte) . ")\n";
}
echo "\n";

// Test 4: Binary data with invalid UTF-8
echo "Test 4: Binary data with invalid UTF-8\n";
$str4 = "Hello\xFF\xFE\x00World";
echo "String length: " . strlen($str4) . " bytes\n";

echo "Byte mode (all bytes visible):\n";
$iter4_b = str_iter($str4, "byte");
foreach ($iter4_b as $key => $byte) {
    $ord = ord($byte);
    $printable = ($ord >= 32 && $ord <= 126) ? $byte : '.';
    echo "  [$key] => '$printable' (hex: " . bin2hex($byte) . ", decimal: $ord)\n";
}
echo "\n";

// Test 5: Empty string
echo "Test 5: Empty string\n";
$str5 = "";
echo "String length: " . strlen($str5) . " bytes\n";

echo "Byte mode:\n";
$iter5_b = str_iter($str5, "byte");
$count = 0;
foreach ($iter5_b as $key => $byte) {
    echo "  [$key] => '$byte'\n";
    $count++;
}
echo "  Total iterations: $count\n";
echo "\n";

// Test 6: Single byte
echo "Test 6: Single byte 'A'\n";
$str6 = "A";
echo "String length: " . strlen($str6) . " bytes\n";

echo "Byte mode:\n";
$iter6_b = str_iter($str6, "byte");
foreach ($iter6_b as $key => $byte) {
    echo "  [$key] => '$byte' (hex: " . bin2hex($byte) . ", decimal: " . ord($byte) . ")\n";
}
echo "\n";

echo "Byte mode tests completed!\n";
?>