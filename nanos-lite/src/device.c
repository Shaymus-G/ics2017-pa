#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

static char dispinfo[128] __attribute__((used));

size_t events_read(void *buf, size_t len) {
  static char event_buf[32];
  static size_t event_len = 0;
  static size_t event_pos = 0;
  static unsigned long last_time = 0;

  if (len == 0) {
    return 0;
  }

  if (event_pos < event_len) {
    size_t n = event_len - event_pos;
    if (n > len) {
      n = len;
    }

    memcpy(buf, event_buf + event_pos, n);
    event_pos += n;

    if (event_pos >= event_len) {
      event_pos = 0;
      event_len = 0;
    }

    return n;
  }

  while(1) {
    int key = _read_key();

    if (key != _KEY_NONE) {
      const char *type;
      int keycode;

      if (key & 0x8000) {
        type = "kd";
	keycode = key & ~0x8000;
      } else {
	type = "ku";
	keycode = key;
      }

      if (keycode > 0 && keycode < 256 && keyname[keycode] != NULL) {
	event_len = snprintf(event_buf, sizeof(event_buf), "%s %s\n", type, keyname[keycode]);

	Log("events_read return event: %s", event_buf);
	break;
      }
    }

  //int key = _read_key();

  //if (key != _KEY_NONE) {
    //const char *type;
    //int keycode;

    //if (key & 0x8000) {
      //type = "kd";
      //keycode = key & ~0x8000;
    //} else {
      //type = "ku";
      //keycode = key;
    //}

    //Log("events_read: raw_key=0x%x, type=%s, keycode=%d, keyname=%s", key, type, keycode, (keycode > 0 && keycode <256 && keyname[keycode] != NULL) ? keyname[keycode] : "UNKNOWN");

    //if (keycode > 0 && keycode < 256 && keyname[keycode] != NULL) {
      //event_len = snprintf(event_buf, sizeof(event_buf), "%s %s\n", type, keyname[keycode]);
      //Log("events_read return event: %s", event_buf);
    //} else {
      //event_len = snprintf(event_buf, sizeof(event_buf), "t %u\n", (unsigned)_uptime());
    //}
  //} else {
    //event_len = snprintf(event_buf, sizeof(event_buf),  "t %u\n", (unsigned)_uptime());
  //}

  //event_pos = 0;

  //size_t n = event_len;
  //if (n > len) {
    //n = len;
  //}

  //memcpy(buf, event_buf, n);
  //event_pos += n;

  //if (event_pos >= event_len) {
    //event_pos = 0;
    //event_len = 0;
  //}

  //return n;
//}

    unsigned long now = _uptime();
    if (now - last_time >= 100) {
      last_time = now;
      event_len = snprintf(event_buf, sizeof(event_buf), "t %d\n", (int)now);
      break;
    }
  }

  event_pos = 0;

  size_t n = event_len;
  if (n > len) {
    n = len;
  }

  memcpy(buf, event_buf, n);
  event_pos += n;
    
  if (event_pos >= event_len) {
    event_pos = 0;
    event_len = 0;
  }

  return n;
}

  //return 0;
  //int n =snprintf((char *)buf, len, "t %lu\n", _uptime());
  //return n < len ? n : len;
//}

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
