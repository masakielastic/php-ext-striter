<?php
// Test for grapheme cluster mode (default)

echo "Test: Grapheme cluster mode (default)\n";
echo "====================================\n\n";

// Test 1: ASCII string (both modes should be identical)
echo "Test 1: ASCII string\n";
$str1 = "Hello";
echo "Grapheme mode:\n";
$iter1 = str_iter($str1, "grapheme");
foreach ($iter1 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "Codepoint mode:\n";
$iter1_cp = str_iter($str1, "codepoint");
foreach ($iter1_cp as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 2: UTF-8 string with Japanese characters
echo "Test 2: Japanese characters\n";
$str2 = "こんにちは";
echo "Grapheme mode:\n";
$iter2 = str_iter($str2, "grapheme");
foreach ($iter2 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "Codepoint mode:\n";
$iter2_cp = str_iter($str2, "codepoint");
foreach ($iter2_cp as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 3: String with emoji (should show difference)
echo "Test 3: Emoji characters\n";
$str3 = "🌸🌺🌻";
echo "Grapheme mode:\n";
$iter3 = str_iter($str3, "grapheme");
foreach ($iter3 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "Codepoint mode:\n";
$iter3_cp = str_iter($str3, "codepoint");
foreach ($iter3_cp as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 4: String with combining characters
echo "Test 4: Combining characters (café)\n";
$str4 = "café"; // e with combining acute accent
echo "Grapheme mode:\n";
$iter4 = str_iter($str4, "grapheme");
foreach ($iter4 as $key => $char) {
    echo "[$key] => '$char' (" . bin2hex($char) . ")\n";
}
echo "Codepoint mode:\n";
$iter4_cp = str_iter($str4, "codepoint");
foreach ($iter4_cp as $key => $char) {
    echo "[$key] => '$char' (" . bin2hex($char) . ")\n";
}
echo "\n";

// Test 5: Mixed content
echo "Test 5: Mixed content\n";
$str5 = "Hello🌍世界";
echo "Grapheme mode:\n";
$iter5 = str_iter($str5, "grapheme");
foreach ($iter5 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "Codepoint mode:\n";
$iter5_cp = str_iter($str5, "codepoint");
foreach ($iter5_cp as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

// Test 6: Default mode (should be grapheme)
echo "Test 6: Default mode (should be grapheme)\n";
$iter6 = str_iter("🌸🌺");
foreach ($iter6 as $key => $char) {
    echo "[$key] => '$char'\n";
}
echo "\n";

echo "Grapheme cluster tests completed!\n";
?>