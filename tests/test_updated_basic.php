<?php
/**
 * Updated basic test for _StrIterIterator with IteratorAggregate and Countable
 */

echo "Test: Updated basic functionality with IteratorAggregate and Countable\n";
echo "====================================================================\n\n";

// Test 1: ASCII string
echo "Test 1: ASCII string\n";
$iter = str_iter("Hello");
echo "Count: " . count($iter) . "\n";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char'\n";
}
echo "\n";

// Test 2: UTF-8 string
echo "Test 2: UTF-8 string\n";
$iter = str_iter("こんにちは");
echo "Count: " . count($iter) . "\n";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char'\n";
}
echo "\n";

// Test 3: Mixed ASCII and UTF-8
echo "Test 3: Mixed ASCII and UTF-8\n";
$iter = str_iter("Hello世界");
echo "Count: " . count($iter) . "\n";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char'\n";
}
echo "\n";

// Test 4: Empty string
echo "Test 4: Empty string\n";
$iter = str_iter("");
echo "Count: " . count($iter) . "\n";
echo "Empty string: ";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char' ";
}
echo "no output expected\n\n";

// Test 5: String with emoji
echo "Test 5: String with emoji\n";
$iter = str_iter("Hello🌍");
echo "Count: " . count($iter) . "\n";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char'\n";
}
echo "\n";

// Test 6: Interface verification
echo "Test 6: Interface verification\n";
$iter = str_iter("Test");
echo "instanceof IteratorAggregate: " . ($iter instanceof IteratorAggregate ? "Yes" : "No") . "\n";
echo "instanceof Countable: " . ($iter instanceof Countable ? "Yes" : "No") . "\n";
echo "instanceof Traversable: " . ($iter instanceof Traversable ? "Yes" : "No") . "\n";
echo "\n";

// Test 7: getIterator method
echo "Test 7: getIterator method\n";
$iter = str_iter("ABC");
$internal_iter = $iter->getIterator();
echo "getIterator() returns: " . get_class($internal_iter) . "\n";
echo "Same object? " . ($iter === $internal_iter ? "Yes" : "No") . "\n";
echo "\n";

// Test 8: count method
echo "Test 8: count method\n";
$iter = str_iter("12345");
echo "count() function: " . count($iter) . "\n";
echo "count() method: " . $iter->count() . "\n";
echo "\n";

// Test 9: Different modes
echo "Test 9: Different modes\n";
$test_str = "A🎉B";
echo "String: '$test_str'\n";

$grapheme_iter = str_iter($test_str, "grapheme");
echo "Grapheme mode count: " . count($grapheme_iter) . "\n";

$codepoint_iter = str_iter($test_str, "codepoint");  
echo "Codepoint mode count: " . count($codepoint_iter) . "\n";

$byte_iter = str_iter($test_str, "byte");
echo "Byte mode count: " . count($byte_iter) . "\n";
echo "\n";

// Test 10: Multiple iteration
echo "Test 10: Multiple iteration (should work)\n";
$iter = str_iter("XYZ");
echo "First iteration:\n";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char'\n";
}
echo "Second iteration:\n";
foreach ($iter as $index => $char) {
    echo "[$index] => '$char'\n";
}
echo "\n";

echo "Updated basic tests completed!\n";
?>