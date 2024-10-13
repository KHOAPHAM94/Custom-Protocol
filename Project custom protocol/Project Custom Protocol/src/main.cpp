#include <Arduino.h>
#include "driver/uart.h"
#include "esp_log.h"

#define IN_SIZE 1
#define COIL_SIZE 1
#define IN_REG_SIZE 4
#define WRITE_REG_SIZE 4

typedef enum {
  ERROR,
  READ_INPUT,
  READ_REGISTER,
  WRITE_COIL,
  WRITE_REGISTER,
} Func_code_t;

#define ESP32_Addr 0x10

class communication {
protected:
  uint8_t dis_in [IN_SIZE] = {0x05};
  uint8_t coil_out [COIL_SIZE] = {0x05};
  uint8_t reg_in [IN_REG_SIZE] = {0x01, 0x05, 0x07, 0xCA};
  uint8_t reg_out [WRITE_REG_SIZE] = {0x01, 0x05, 0x00, 0x01};
public:
  //Constructor
  communication (){
  };
  uint8_t *receive_ptr = nullptr;
  uint8_t *send_data_ptr = nullptr;
  uart_port_t port;

  bool get_in (uint8_t *ptr){
    memcpy (ptr, dis_in, IN_SIZE);
    return true;
  }
  bool get_reg (uint8_t *ptr){
    memcpy (ptr, reg_in, IN_REG_SIZE);
    return true;
  }
  bool write_coil (uint8_t *ptr){
    memcpy(coil_out, ptr, COIL_SIZE);
    return true;
  }
  bool feedback_write_coil (uint8_t *ptr){
    memcpy (ptr, coil_out, COIL_SIZE);
    return true;
  } 
  bool write_reg (uint8_t *ptr){
    memcpy(reg_out, ptr, WRITE_REG_SIZE);
    return true;
}
  bool feedback_write_reg (uint8_t *ptr){
    memcpy (ptr, reg_out, WRITE_REG_SIZE);
    return true;
  }
};

communication my_com;
void handleReadInput ();
void handleReadRegister();
void feedbackWriteCoil();
void feedbackWriteReg();
void handleeror();
void handleCRCmismatch ();
// CRC16 Calculation
uint16_t calculateCRC(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)data[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

//Initiate UART communication
void uart_init (uart_port_t _port, int _baudrate, uint8_t tx_pin, uint8_t rx_pin, int _buf_size){
  const uart_config_t uart_config = {
        .baud_rate = _baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
      };
  uart_param_config(_port, &uart_config);
  uart_set_pin(_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(_port, _buf_size*2, _buf_size*2, 0, NULL, 0);
  my_com.port = _port;
}

/**
 * @brief Send the data to Uart transmittion FIFO
 *        Add slave address at the first byte
 *        Add function code at the second byte
 *        Add data into the middle of the message
 *        Calculate and add CRC in
 * @param _func 
 * @param _data 
 * @param _len 
 * @return true 
 * @return false 
 */
bool send (Func_code_t _func, uint8_t *_data, size_t _len){
  uint8_t total_len = _len + 4;
  uint8_t send_buffer [total_len];
  send_buffer [0] = ESP32_Addr;
  send_buffer[1] = _func;
  for (int i = 0; i < _len; i++){
    send_buffer[i+2] = _data[i];
  }
  uint16_t crc = calculateCRC (send_buffer, _len + 2);

  send_buffer[total_len - 2] = (uint8_t) ((crc) & 0xFF);        
  send_buffer[total_len - 1] = (uint8_t) ((crc >> 8) & 0xFF);

  if (uart_write_bytes(my_com.port, send_buffer, _len + 4) == -1){
    return false;
  }
  else{
    return true;
  };
  vTaskDelay(100/portTICK_PERIOD_MS);
}
/**
 * @brief Check uart buffer
 *        Calculate CRC of the message
 *        Compare received_CRC and calculated_CRC
 * 
 * @return uint8_t 
 */
uint8_t receive (){
  size_t len = 0;
  uart_get_buffered_data_len (my_com.port, &len);
  
  if (len > 0){
  my_com.receive_ptr = (uint8_t*) malloc(len);
  uart_read_bytes (my_com.port, my_com.receive_ptr, len, 100/portTICK_RATE_MS);
  uint16_t CRC_received = my_com.receive_ptr [len - 1] << 8 | my_com.receive_ptr [len-2]; 
  uint16_t CRC_calculated = calculateCRC (my_com.receive_ptr, len-2);
  //uart_write_bytes (UART_NUM_0, &CRC_calculated, 2);
    if (CRC_received == CRC_calculated)
    {
      return len;
    }
    else {
      handleCRCmismatch ();
    }
  }
  else{
        return 0;
      }
    return 0;
}
/**
 * @brief Tear down recieving message into: Adrr + Func + Data + CRC
 *        Adrr: Slave Adrr
 *        Func: 0x01, 0x02, 0x03, 0x04
 *        Data: only for write coils/registers
 * @param buf_ptr 
 * @param _len 
 * @return true 
 * @return false 
 */
bool decoder (uint8_t *buf_ptr, size_t _len){
      //verify function code
      uint8_t sub_buffer [_len-4];
      if (buf_ptr[0] == ESP32_Addr){
        switch (buf_ptr[1]){
          case READ_INPUT:
          handleReadInput();
          break;
          case READ_REGISTER:
          handleReadRegister();
          return true;
          break;
          case WRITE_COIL:
          memcpy (sub_buffer, buf_ptr + 2, _len-4);
          if ((_len - 4) == COIL_SIZE){
            my_com.write_coil(sub_buffer);
            feedbackWriteCoil();
           }
          return true;
          break;
          case WRITE_REGISTER:
          memcpy (sub_buffer, buf_ptr + 2, _len-4);
          if ((_len - 4) == WRITE_REG_SIZE){
            my_com.write_reg(sub_buffer);
            feedbackWriteReg();
           }
          return true;
          break;
          default:
          handleeror();
          return false;
        }
      }
    else{
      //Wrong message return 0x01
      return false;
    }
    return false;
  }

/**
 * @brief 
 * 
 * @param _func_code function code
 * @param _len the length of the message
 * @param _data the pointer to received message
 * @return * void 
 */
/* Handle with read discrete input message */
void handleReadInput (){
  uint8_t *ptr = (uint8_t*) malloc(IN_SIZE);
  my_com.get_in(ptr);
  send (READ_INPUT, ptr, IN_SIZE);
  free(ptr);
}
/* Handle with read holding register message */
void handleReadRegister(){
  uint8_t *ptr = (uint8_t*) malloc(IN_REG_SIZE);
  my_com.get_reg(ptr);
  send (READ_REGISTER, ptr, IN_REG_SIZE);
  free(ptr);
}
/* Feedback current state of coil after being written */
void feedbackWriteCoil(){
  uint8_t *ptr = (uint8_t*) malloc(COIL_SIZE);
  my_com.feedback_write_coil(ptr);
  send (WRITE_COIL, ptr, COIL_SIZE);
  free(ptr);   
}
/* Feedback current state of register after being written */
void feedbackWriteReg(){
  uint8_t *ptr = (uint8_t*) malloc(WRITE_REG_SIZE);
  my_com.feedback_write_reg(ptr);
  send (WRITE_REGISTER, ptr, WRITE_REG_SIZE);
  free(ptr);

}
/* Send error message to requester */
void handleeror(){
  uint8_t exception_error = 0x01; //Invalid Function Code
  send(ERROR, &exception_error, 1);
}

void handleCRCmismatch (){
 uint8_t exception_error = 0x00; //Invalid Function Code
send(ERROR, &exception_error, 1);
}
void setup() {
uart_init(UART_NUM_0, 9600, 1, 3, 256);
}

void loop() {
  // put your main code here, to run repeatedly:
uint8_t len = receive ();

if (len > 0) {
  decoder(my_com.receive_ptr, len);
  vTaskDelay (100/portTICK_PERIOD_MS);
  free(my_com.receive_ptr);
}
}