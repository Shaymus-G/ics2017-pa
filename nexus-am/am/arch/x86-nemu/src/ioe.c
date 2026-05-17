#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int i;
  //for (i = 0; i < _screen.width * _screen.height; i++) {
    //fb[i] = i;
  //}
  
  for (i = 0; i < h; i++) {
    int yy = y + i;
    
    if (yy < 0 || yy >= _screen.height) {
      continue;
    }

    int xx = x;
    int ww = w;
    const uint32_t *src = pixels + i * w;

    if (xx < 0) {
      src += -xx;
      ww += xx;
      xx = 0;
    }

    if (xx + ww > _screen.width) {
      ww = _screen.width - xx;
    }

    if (ww <= 0) {
      continue;
    }

    memcpy(&fb[yy * _screen.width + xx], src, ww * sizeof(uint32_t));
  }
}

void _draw_sync() {
}

#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
#define I8042_STATUS_HASKEY_MASK 0x1

int _read_key() {
  if ((inb(I8042_STATUS_PORT) & I8042_STATUS_HASKEY_MASK) == 0) {
    return _KEY_NONE;
  }

  return inl(I8042_DATA_PORT);
}
