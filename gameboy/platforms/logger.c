#include "../non_core/logger.h"

#include <string.h>
#include <stdarg.h>

#include <grub/misc.h>
#include <grub/term.h>

static LogLevel current_log_level = LOG_OFF;

void set_log_level(LogLevel ll) {
    
    if (ll >= LOG_OFF) {
        current_log_level = LOG_OFF;
    } else {
        current_log_level = ll;
    }
}
/*
static unsigned uint_to_str_padded(unsigned i, char *buf, unsigned max_digits) {
    unsigned int digits = 0;
    
    do {
        buf[digits] = (i % 10) + '0';
        i /= 10;
        digits++;
     } while (i != 0);

    while (digits < max_digits) {
        buf[digits] = '0';
        digits++;
    }

     // Digits in string are back-to-front, swap them round
     for (unsigned int j = 0; j < digits/2; j++) {
        char temp = buf[j];
        buf[j] = buf[digits - j - 1];
        buf[digits - j - 1] = temp;
     }

     return digits;
}
 
static unsigned uint_to_str(unsigned i, char *buf) {
    int digits = 0;
    
    do {
        buf[digits] = (i % 10) + '0';
        i /= 10;
        digits++;
     } while (i != 0);

     // Digits in string are back-to-front, swap them round
     for (int j = 0; j < digits/2; j++) {
        char temp = buf[j];
        buf[j] = buf[digits - j - 1];
        buf[digits - j - 1] = temp;
     }

     return digits;
}

 
static unsigned uint_to_hex_str(unsigned i, char *buf, int uppercase) {
    int digits = 0;
    
    do {
        int value = i & (0x10 - 1);
        char letter = uppercase ? 'A' : 'a';
        buf[digits] = value < 0xA ? value + '0' : value - 10 + letter;
        i >>= 4;
        digits++;
     } while (i != 0);

     // Digits in string are back-to-front, swap them round
     for (int j = 0; j < digits/2; j++) {
        char temp = buf[j];
        buf[j] = buf[digits - j - 1];
        buf[digits - j - 1] = temp;
     }

     return digits;
}*/

void log_message(LogLevel ll __attribute__ ((unused)), const char *fmt, ...) {
	va_list ap;
	va_start (ap, fmt);
	grub_vprintf (fmt, ap);
	va_end (ap);
}
