#include <mbed.h>

// Const variable

// Function prototype
void uart_transmit(const uint8_t *send, const int len);
template<int N>
void uart_transmit(const uint8_t (&send)[N]);
bool uart_receive(void *buf, const int len, const std::chrono::milliseconds timeout);
template<int N>
bool uart_receive(uint8_t (&buf)[N], const std::chrono::milliseconds timeout);

// IO
BufferedSerial pc{USBTX, USBRX, 115200};
// 2Mbps-8-N-1
BufferedSerial enc_bus{PB_6, PA_10, (int)2e6};
DigitalOut de{PC_0};
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

  int get_pos() const {
    return rotate * get_turns() + pos;
  }
  int16_t get_turns() const {
    int16_t sign_bit = turns & 0x2000;
    sign_bit = sign_bit << 2 | sign_bit << 1;
    return sign_bit | turns;
  }
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
struct MyAmt21_C : Amt21_C {
  // request
  bool send_read_pos() {
    uart_transmit({address});
    if(uint16_t receive; uart_receive(&receive, sizeof(receive), 10ms)) {
      read_pos(receive);
      return true;
    }
    return false;
  }
  bool send_read_turns() {
    uart_transmit({address + 1});
    if(uint16_t receive; uart_receive(&receive, sizeof(receive), 10ms)) {
      read_turns(receive);
      return true;
    }
    return false;
  }
  void send_reset() {
    uart_transmit({address + 2, 0x75});
  }
} amt{0x54};

/// @brief The application entry point.
int main() {
  // put your setup code here, to run once:
  printf("\nsetup\n");
  timer.start();
  auto pre = timer.elapsed_time();
  enc_bus.set_blocking(0);
  amt.send_reset();
  while(1) {
    // put your main code here, to run repeatedly:
    auto now = timer.elapsed_time();
    if(now - pre > 20ms) {
      printf("hoge\n");

      amt.send_read_pos();
      amt.send_read_turns();
      printf("% 4d % 4d % 6d % 4d ", amt.pos, amt.turns, amt.get_pos(), amt.get_turns());

      pre = now;
    }
  }
}

// Function definition
void uart_transmit(const uint8_t *send, const int len) {
  de = 1;
  enc_bus.sync();
  enc_bus.write(send, len);
  wait_us(3);
  de = 0;
}
template<int N>
void uart_transmit(const uint8_t (&send)[N]) {
  uart_transmit(send, sizeof(send));
}
bool uart_receive(void *buf, const int len, const std::chrono::milliseconds timeout) {
  auto now = timer.elapsed_time();
  auto pre = now;
  uint8_t *p = reinterpret_cast<uint8_t *>(buf);
  const uint8_t *end = p + len;
  do {
    if(enc_bus.read(p, 1) > 0 && ++p == end) {
      return (wait_ns(275), true);
    }
    now = timer.elapsed_time();
  } while(now - pre < timeout);
  return false;
}
template<int N>
bool uart_receive(uint8_t (&buf)[N], const std::chrono::milliseconds timeout) {
  return uart_receive(buf, sizeof(buf), timeout);
}
