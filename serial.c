#include <stdio.h>
#include <stdlib.h>
#include  "pico/stdlib.h"
#include "hardware/pio.h"
#include "serial.h"
#include "serial.pio.h"

uint8_t serial_rx_buf[SERIAL_BUFSIZE];
uint serial_rx_buf_start = 0;
uint serial_rx_buf_end = 0;
void serial_read_into_buf();

void serial_init() {
  // Calculate the clock divisor needed to get the requested baud rate
  float div = (float)clock_get_hz(clk_sys) / (8 * SERIAL_BAUDRATE);

  // Initialise the RX pin
  pio_sm_set_consecutive_pindirs(SERIAL_PIO, SERIAL_PIO_RX_SM, SERIAL_PIN_RX, 1, false);
  pio_gpio_init(SERIAL_PIO, SERIAL_PIN_RX);
  gpio_pull_up(SERIAL_PIN_RX);
  uint offset_rx = pio_add_program(SERIAL_PIO, &serial_rx_program);
  pio_sm_config c_rx = serial_rx_program_get_default_config(offset_rx);
  sm_config_set_in_pins(&c_rx, SERIAL_PIN_RX);
  sm_config_set_jmp_pin(&c_rx, SERIAL_PIN_RX);
  sm_config_set_in_shift(&c_rx, true, true, 8);
  sm_config_set_fifo_join(&c_rx, PIO_FIFO_JOIN_RX);
  sm_config_set_clkdiv(&c_rx, div);
  pio_sm_init(SERIAL_PIO, SERIAL_PIO_RX_SM, offset_rx, &c_rx);
  pio_sm_set_enabled(SERIAL_PIO, SERIAL_PIO_RX_SM, true);

  // Initialise the TX pin
  pio_sm_set_consecutive_pindirs(SERIAL_PIO, SERIAL_PIO_TX_SM, SERIAL_PIN_TX, 1, true);
  pio_gpio_init(SERIAL_PIO, SERIAL_PIN_TX);
  uint offset_tx = pio_add_program(SERIAL_PIO, &serial_tx_program);
  pio_sm_config c_tx = serial_tx_program_get_default_config(offset_tx);
  sm_config_set_out_pins(&c_tx, SERIAL_PIN_TX, 1);
  sm_config_set_sideset_pins(&c_tx, SERIAL_PIN_TX);
  sm_config_set_out_shift(&c_tx, true, false, 32);
  sm_config_set_fifo_join(&c_tx, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&c_tx, div);
  pio_sm_init(SERIAL_PIO, SERIAL_PIO_TX_SM, offset_tx, &c_tx);
  pio_sm_set_enabled(SERIAL_PIO, SERIAL_PIO_TX_SM, true);

  // Set up interrupts on the RX pin
  switch (SERIAL_PIO_RX_SM) {
    case 0:
      pio_set_irq0_source_enabled(SERIAL_PIO, pis_sm0_rx_fifo_not_empty, true);
      break;
    case 1:
      pio_set_irq0_source_enabled(SERIAL_PIO, pis_sm1_rx_fifo_not_empty, true);
      break;
    case 2:
      pio_set_irq0_source_enabled(SERIAL_PIO, pis_sm2_rx_fifo_not_empty, true);
      break;
    case 3:
      pio_set_irq0_source_enabled(SERIAL_PIO, pis_sm3_rx_fifo_not_empty, true);
      break;
  }
  uint irq = pio_get_index(SERIAL_PIO) == 0 ? PIO0_IRQ_0 : PIO1_IRQ_0;
  irq_set_exclusive_handler(irq, serial_read_into_buf);
  irq_set_enabled(irq, true);
}

static inline void serial_putc(char c) {
  // Send one byte (blocking)
  pio_sm_put_blocking(SERIAL_PIO, SERIAL_PIO_TX_SM, (uint32_t)c);
}

static inline bool serial_ready() {
  // Returns true if there is data in the receive buffer
  return !(serial_rx_buf_start == serial_rx_buf_end);
}

void serial_read_into_buf() {
  // Read data from the RX pin into the receive buffer
  io_rw_8 *rxfifo_shift = (io_rw_8*)&SERIAL_PIO->rxf[SERIAL_PIO_RX_SM] + 3;
  while (!pio_sm_is_rx_fifo_empty(SERIAL_PIO, SERIAL_PIO_RX_SM)) {
    serial_rx_buf[serial_rx_buf_end] = (uint8_t)*rxfifo_shift;
    serial_rx_buf_end = (serial_rx_buf_end + 1) % SERIAL_BUFSIZE;
  }
  pio_interrupt_clear(SERIAL_PIO, 0);
}

char serial_getc() {
  // Read one byte from the receive buffer (blocking)
  while (!serial_ready()) tight_loop_contents();
  uint8_t byte = serial_rx_buf[serial_rx_buf_start];
  serial_rx_buf_start = (serial_rx_buf_start + 1) % SERIAL_BUFSIZE;
  return byte;
}
