#include <dmx-debug-lib.h>
#include <stddef.h>

// We can't use the real usbRequest_t since we would need to know which usbconfig.h is used.
// So, we duplicate the struct.

typedef uint8_t uchar;

typedef union usbWord{
  unsigned    word;
  uchar       bytes[2];
} dbg_usbWord_t;

typedef struct usbRequest{
  uchar         bmRequestType;
  uchar         bRequest;
  dbg_usbWord_t wValue;
  dbg_usbWord_t wIndex;
  dbg_usbWord_t wLength;
} dbg_usbRequest_t;

void dbg_usbrequest(void *request_raw) {
  dbg_usbRequest_t *rq = (dbg_usbRequest_t*) request_raw;

  dbg_hexdump(request_raw, 8);
  dbg_print(F(". req type "));
  dbg_print(rq->bmRequestType);
  dbg_print(F(", req "));
  dbg_print(rq->bRequest);
  dbg_print(F(", value "));
  dbg_print(rq->wValue.word);
  dbg_print(F(", wIndex "));
  dbg_print(rq->wIndex.word);
  dbg_print(F(", length "));
  dbg_print(rq->wLength.word);
}

const char hexdigit_to_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

inline void dbg_print_hexdump_byte(uint8_t val, size_t count) {
  dbg_print(' ');
  dbg_print(hexdigit_to_char[val >> 4]);
  dbg_print(hexdigit_to_char[val & 0xF]);
  if(count > 1) {
    dbg_print(F("x"));
    dbg_print(count);
  }
}

void dbg_hexdump(void *data, size_t byte_len) {
  if(byte_len == 0)
    return;

  uint8_t *data_uint8 = (uint8_t *) data;
  uint8_t last = data_uint8[0];
  size_t last_count = 1;
  for (size_t i = 1; i < byte_len; i ++) {
    uint8_t val = data_uint8[i];
    if(val == last)
      last_count ++;
    else {
      dbg_print_hexdump_byte(last, last_count);
      last = val;
      last_count = 1;
    }
  }
  dbg_print_hexdump_byte(last, last_count);
}
