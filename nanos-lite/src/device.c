#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

static char dispinfo[128] __attribute__((used));

size_t events_read(void *buf, size_t len) {
  //static char event_buf[32];
  //static size_t event_len = 0;
  //static size_t event_pos = 0;

  if (buf == NULL || len == 0) {
    return 0;
  }

  int key_code_raw = _read_key();
  bool is_keydown = false;

  if (key_code_raw & 0x8000) {
    key_code_raw ^= 0x8000;
    is_keydown = true;
  }

  int key_code = key_code_raw;

  if (key_code == _KEY_NONE) {
    unsigned long current_time = _uptime();
    snprintf(buf, len, "t %u\n", (unsigned)current_time);
  } else {
    snprintf(buf, len, "%s %s\n", is_keydown ? "kd" : "ku", keyname[key_code]);
  }

  return strlen(buf);
}

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int pixel_offset = offset / sizeof(uint32_t);

  int x = pixel_offset % _screen.width;
  int y = pixel_offset / _screen.width;

  int pixels = len / sizeof(uint32_t);
  const uint32_t *p = (const uint32_t *)buf;

  while (pixels > 0) {
    int w = _screen.width - x;
    if (w > pixels) {
      w = pixels;
    }

    _draw_rect(p, x, y, w, 1);

    p += w;
    pixels -= w;
    x = 0;
    y++;
  }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
