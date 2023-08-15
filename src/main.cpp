#include <mbed.h>

#include "Amt21.h"
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
struct : Amt21 {
  // request
  bool send_read_pos() {
    rs485.uart_transmit({address});
    if(uint16_t receive; rs485.uart_receive(&receive, sizeof(receive), 10ms)) {
      read_pos(receive);
      return true;
    }
    return false;
  }
  bool send_read_turns() {
    rs485.uart_transmit({address + 1});
    if(uint16_t receive; rs485.uart_receive(&receive, sizeof(receive), 10ms)) {
      read_turns(receive);
      return true;
    }
    return false;
  }
  void send_reset() {
    rs485.uart_transmit({address + 2, 0x75});
  }
} amt[] = {{0x50}, {0x54}, {0x58}, {0x5C}};

/// @brief The application entry point.
int main() {
  // put your setup code here, to run once:
  printf("\nsetup\n");
  timer.start();
  auto pre = timer.elapsed_time();
  for(auto& e: amt) e.send_reset();
  while(1) {
    // put your main code here, to run repeatedly:
    auto now = timer.elapsed_time();
    if(now - pre > 20ms) {
      printf("hoge\n");

      for(auto& e: amt) {
        e.send_read_pos();
        e.send_read_turns();
        printf("% 6d ", e.get_pos());
      }

      pre = now;
    }
  }
}

// Function definition
