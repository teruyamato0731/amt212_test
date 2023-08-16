#include <mbed.h>

#include "Rs485.h"

// Const variable

// Function prototype

// IO
BufferedSerial pc{USBTX, USBRX, 115200};
Rs485 rs485{PB_6, PA_10, (int)2e6, PC_0};
// CAN can{PA_11, PA_12, (int)1e6};
// CAN can{PB_12, PB_13, (int)1e6};
// CANMessage msg;
Timer timer;

// Struct definition

// Global variable
struct Amt21 {
  static constexpr int rotate = 4096;

  uint8_t address;
  int32_t pos;
  uint16_t pre_pos;

  bool request_pos() {
    rs485.uart_transmit({address});
    if(uint16_t now_pos; rs485.uart_receive(&now_pos, sizeof(now_pos), 10ms) && is_valid(now_pos)) {
      now_pos = (now_pos & 0x3fff) >> 2;
      int16_t diff = now_pos - pre_pos;
      if(diff > rotate / 2) {
        diff -= rotate;
      } else if(diff < -rotate / 2) {
        diff += rotate;
      }
      pos += diff;
      pre_pos = now_pos;
      return true;
    }
    return false;
  }
  void request_reset() {
    rs485.uart_transmit({address + 2, 0x75});
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
} amt[] = {{0x50}, {0x54}, {0x58}, {0x5C}};

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

      for(auto& e: amt) {
        e.request_pos();
        printf("% 12ld ", e.pos);
      }

      pre = now;
    }
  }
}

// Function definition
