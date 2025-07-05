<?php
// Test for PCRE2 known bug with consecutive emoji

echo "Test: PCRE2 consecutive emoji bug\n";
echo "=================================\n\n";

// Test cases for consecutive emoji sequences
$test_cases = [
    "🌸🌺",           // Two flower emoji
    "🌸🌺🌻",         // Three flower emoji  
    "👋🏽👋🏽",         // Repeated emoji with skin tone modifier
    "🏴󠁧󠁢󠁥󠁮󠁧󠁿🏴󠁧󠁢󠁳󠁣󠁴󠁿",      // Two flag emoji (complex sequences)
    "👨‍👩‍👧‍👦👨‍👩‍👧‍👦",   // Family emoji sequences
    "🇯🇵🇺🇸",         // Country flag emoji
];

foreach ($test_cases as $index => $emoji_str) {
    echo "Test case " . ($index + 1) . ": '$emoji_str'\n";
    echo "Byte length: " . strlen($emoji_str) . "\n";
    
    echo "Grapheme mode:\n";
    $iter_g = str_iter($emoji_str, "grapheme");
    $count_g = 0;
    foreach ($iter_g as $key => $char) {
        echo "  [$key] => '$char' (bytes: " . strlen($char) . ", hex: " . bin2hex($char) . ")\n";
        $count_g++;
    }
    echo "  Total grapheme clusters: $count_g\n";
    
    echo "Codepoint mode:\n";
    $iter_c = str_iter($emoji_str, "codepoint");
    $count_c = 0;
    foreach ($iter_c as $key => $char) {
        echo "  [$key] => '$char' (bytes: " . strlen($char) . ", hex: " . bin2hex($char) . ")\n";
        $count_c++;
    }
    echo "  Total codepoints: $count_c\n";
    
    echo "  Difference: " . ($count_c - $count_g) . " codepoints vs graphemes\n";
    echo "  " . str_repeat("-", 50) . "\n\n";
}

echo "Note: If PCRE2 has bugs with consecutive emoji, you may see\n";
echo "unexpected results in grapheme mode for some test cases.\n";
echo "This is a known limitation of PCRE2's \\X pattern.\n\n";

echo "Consecutive emoji bug tests completed!\n";
?>