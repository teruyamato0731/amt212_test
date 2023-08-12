#include <mbed.h>

// Const variable

// Function prototype

// IO
BufferedSerial pc{USBTX, USBRX, 115200};
// CAN can{PA_11, PA_12, (int)1e6};
// CAN can{PB_12, PB_13, (int)1e6};
// CANMessage msg;
Timer timer;

// Struct definition
struct Amt21_C {
  static constexpr int rotate = 4096;

  uint8_t address;
  uint16_t pos;
  uint16_t turns;

  bool read_pos(const uint16_t msg) {
    bool res = is_valid(msg);
    if(res) pos = (msg & 0x3fff) >> 2;
    return res;
  }
  bool read_turns(const uint16_t msg) {
    bool res = is_valid(msg);
    if(res) turns = msg & 0x3fff;
    return res;
  }
  static bool is_valid(uint16_t raw_data) {
    bool k1 = raw_data >> 15;
    bool k0 = raw_data >> 14 & 1;
    raw_data <<= 2;
    do {
      k1 ^= raw_data & 0x8000;          // even
      k0 ^= (raw_data <<= 1) & 0x8000;  // odd
    } while(raw_data <<= 1);
    return k0 && k1;
  }
};

// Global variable

/// @brief The application entry point.
int main() {
  // put your setup code here, to run once:
  printf("\nsetup\n");
  timer.start();
  auto pre = timer.elapsed_time();
  while(1) {
    // put your main code here, to run repeatedly:
    auto now = timer.elapsed_time();
    if(now - pre > 20ms) {
      printf("hoge\n");
      pre = now;
    }
  }
}

// Function definition
