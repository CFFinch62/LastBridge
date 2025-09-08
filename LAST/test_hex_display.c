#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

// Copy of the fixed function for testing
char* format_data_for_display(const char *data, size_t data_len, gboolean hex_mode) {
    if (!hex_mode) {
        // In text mode, create a null-terminated copy of the data
        char *text_copy = malloc(data_len + 1);
        memcpy(text_copy, data, data_len);
        text_copy[data_len] = '\0';
        return text_copy;
    }

    // Convert to hex display - use actual byte count, not strlen
    char *hex_str = malloc(data_len * 3 + 1); // Each byte becomes "XX "
    hex_str[0] = '\0';

    for (size_t i = 0; i < data_len; i++) {
        char hex_byte[4];
        snprintf(hex_byte, sizeof(hex_byte), "%02X ", (unsigned char)data[i]);
        strcat(hex_str, hex_byte);
    }

    return hex_str;
}

int main() {
    // Test data with CR+LF (0x0D 0x0A)
    char test_data[] = "Hello\r\nWorld\r\n";
    size_t data_len = strlen(test_data);
    
    printf("Original data length: %zu bytes\n", data_len);
    printf("Original data (with escapes): ");
    for (size_t i = 0; i < data_len; i++) {
        if (test_data[i] == '\r') printf("\\r");
        else if (test_data[i] == '\n') printf("\\n");
        else printf("%c", test_data[i]);
    }
    printf("\n\n");
    
    // Test text mode
    char *text_result = format_data_for_display(test_data, data_len, FALSE);
    printf("Text mode result: %s\n", text_result);
    free(text_result);
    
    // Test hex mode
    char *hex_result = format_data_for_display(test_data, data_len, TRUE);
    printf("Hex mode result: %s\n", hex_result);
    free(hex_result);
    
    // Test with just CR+LF
    char crlf_data[] = "\r\n";
    size_t crlf_len = 2;
    
    printf("\nTesting just CR+LF:\n");
    char *crlf_hex = format_data_for_display(crlf_data, crlf_len, TRUE);
    printf("CR+LF in hex: %s\n", crlf_hex);
    free(crlf_hex);
    
    return 0;
}
