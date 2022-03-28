#include <Arduino.h>

void dbg_usbrequest(void *request_raw);
void dbg_hexdump(void *data, size_t byte_len);

#define dbg_print Serial.print
