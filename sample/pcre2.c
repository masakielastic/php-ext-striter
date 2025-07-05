#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

int count_utf8_chars_pcre2(const char *utf8_string) {
    pcre2_code *re;
    pcre2_match_data *match_data;
    PCRE2_SPTR pattern = (PCRE2_SPTR)".";  // 任意の1文字（UTF-8モードで個別のコードポイント）
    PCRE2_SPTR subject = (PCRE2_SPTR)utf8_string;
    PCRE2_SIZE subject_length = strlen(utf8_string);
    int errorcode;
    PCRE2_SIZE erroroffset;
    int count = 0;
    PCRE2_SIZE start_offset = 0;
    int rc;

    if (subject_length == 0) {
        return 0;
    }

    // UTF-8オプションでパターンをコンパイル
    re = pcre2_compile(
        pattern,               // パターン文字列（\\X = Unicode拡張書記素クラスタ）
        PCRE2_ZERO_TERMINATED, // パターンの長さ
        PCRE2_UTF | PCRE2_UCP, // UTF-8とUnicodeプロパティを有効
        &errorcode,            // エラーコード
        &erroroffset,          // エラー位置
        NULL                   // コンパイルコンテキスト
    );

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset, buffer);
        return -1;
    }

    // マッチデータを作成
    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    if (match_data == NULL) {
        printf("マッチデータ作成エラー\n");
        pcre2_code_free(re);
        return -1;
    }

    // 文字列全体をスキャンして各拡張書記素クラスタをカウント
    while (start_offset < subject_length) {
        rc = pcre2_match(
            re,                    // コンパイル済み正規表現
            subject,               // 対象文字列
            subject_length,        // 対象文字列の長さ
            start_offset,          // 開始位置
            0,                     // オプション
            match_data,            // マッチデータ
            NULL                   // マッチコンテキスト
        );

        if (rc < 0) {
            if (rc == PCRE2_ERROR_NOMATCH) {
                // マッチしない場合は終了
                break;
            } else {
                // その他のエラー
                printf("マッチエラー: %d (オフセット: %zu)\n", rc, start_offset);
                break;
            }
        }

        // 文字数をカウント
        count++;

        // 次の開始位置を設定
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        PCRE2_SIZE match_start = ovector[0];
        PCRE2_SIZE match_end = ovector[1];
        
        // デバッグ情報（オプション）
        // printf("マッチ: %zu-%zu (", match_start, match_end);
        // for (PCRE2_SIZE i = match_start; i < match_end; i++) {
        //     printf("%02X ", (unsigned char)utf8_string[i]);
        // }
        // printf(")\n");
        
        start_offset = match_end;

        // 無限ループを防ぐ（マッチが進んでいない場合）
        if (match_end == match_start) {
            if (start_offset >= subject_length) {
                break;
            }
            start_offset++;
        }
    }

    // メモリを解放
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return count;
}

int main() {
    // テスト用のUTF-8文字列
    const char *test_strings[] = {
        "Hello",                    // ASCII文字のみ
        "こんにちは",               // 日本語ひらがな
        "Hello世界",                // ASCII + 日本語
        "🌸🌺🌻",                  // 絵文字
        "café naïve résumé",       // アクセント記号付きラテン文字
        "",                        // 空文字列
        "A",                       // 1文字
        "Test文字列🎉"             // 混合文字列
    };
    
    int num_tests = sizeof(test_strings) / sizeof(test_strings[0]);
    
    printf("PCRE2を使用したUTF-8文字数カウントのテスト\n");
    printf("=====================================\n");
    
    for (int i = 0; i < num_tests; i++) {
        int char_count = count_utf8_chars_pcre2(test_strings[i]);
        printf("文字列: \"%s\"\n", test_strings[i]);
        printf("文字数: %d\n", char_count);
        printf("バイト数: %zu\n", strlen(test_strings[i]));
        printf("---------------------\n");
    }
    
    return 0;
}
