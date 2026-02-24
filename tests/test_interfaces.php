<?php
/**
 * Test script for _StrIterIterator with IteratorAggregate and Countable interfaces
 */

echo "=== _StrIterIterator IteratorAggregate & Countable Test ===\n\n";

// Test 1: Basic creation and count
echo "Test 1: Basic creation and count\n";
$iterator = str_iter("Hello");
echo "Class: " . get_class($iterator) . "\n";
echo "Count using count(): " . count($iterator) . "\n";
echo "Count using method: " . $iterator->count() . "\n";
echo "\n";

// Test 2: IteratorAggregate - getIterator method
echo "Test 2: IteratorAggregate - getIterator\n";
$iterator = str_iter("ABC");
$iter = $iterator->getIterator();
echo "getIterator() class: " . get_class($iter) . "\n";
echo "Same object? " . ($iter === $iterator ? "Yes" : "No") . "\n";
echo "\n";

// Test 3: Check interfaces
echo "Test 3: Interface check\n";
$iterator = str_iter("Test");
echo "implements Iterator: " . (($iterator instanceof Iterator) ? "Yes" : "No") . "\n";
echo "implements IteratorAggregate: " . (($iterator instanceof IteratorAggregate) ? "Yes" : "No") . "\n";
echo "implements Countable: " . (($iterator instanceof Countable) ? "Yes" : "No") . "\n";
echo "implements Traversable: " . (($iterator instanceof Traversable) ? "Yes" : "No") . "\n";
echo "\n";

// Test 4: foreach with IteratorAggregate
echo "Test 4: foreach iteration (using IteratorAggregate)\n";
$iterator = str_iter("XYZ");
echo "Iterating over 'XYZ':\n";
foreach ($iterator as $index => $char) {
    echo "  [$index] => '$char'\n";
}
echo "\n";

// Test 5: Multibyte string count
echo "Test 5: Multibyte string count\n";
$iterator = str_iter("こんにちは");
echo "String: 'こんにちは'\n";
echo "Count: " . count($iterator) . "\n";
echo "\n";

// Test 6: Empty string
echo "Test 6: Empty string\n";
$iterator = str_iter("");
echo "Empty string count: " . count($iterator) . "\n";
echo "\n";

// Test 7: Different modes
echo "Test 7: Different iteration modes\n";
$test_string = "A🎉B";
echo "String: '$test_string'\n";

$grapheme_iter = str_iter($test_string, "grapheme");
echo "Grapheme mode count: " . count($grapheme_iter) . "\n";

$codepoint_iter = str_iter($test_string, "codepoint");
echo "Codepoint mode count: " . count($codepoint_iter) . "\n";

$byte_iter = str_iter($test_string, "byte");
echo "Byte mode count: " . count($byte_iter) . "\n";
echo "\n";

echo "=== Test Complete ===\n";
?>