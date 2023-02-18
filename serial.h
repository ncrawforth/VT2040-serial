// Set serial parameters
// Changing the baud rate will break the receive code. Comment out the first
// 5 lines of the serial_rx PIO program in lcd.pio.
#define SERIAL_BAUDRATE 115200
#define SERIAL_BUFSIZE 4096

// Select which GPIO pins to use
#define SERIAL_PIN_RX 1
#define SERIAL_PIN_TX 2

// Select which PIO and state machines to use
#define SERIAL_PIO pio1
#define SERIAL_PIO_RX_SM 0
#define SERIAL_PIO_TX_SM 1

void serial_init();
void serial_putc(char c);
bool serial_ready();
char serial_getc();
