#ifndef BOOTLOADER_USB_BOOTLOADER_USB_INC_BOOTLOADER_CONFIG_H_
#define BOOTLOADER_USB_BOOTLOADER_USB_INC_BOOTLOADER_CONFIG_H_

#define FIRMWARE_FILE_NAME         "flash.bin"
#define FIRMWARE_HASH_FILE_NAME    "flash.ly"


// Uncomment this if hash check for firmware is mandatory.
// By default if hash file missing firmware considered as correct
//#define HASH_CHECK_IS_MANDATORY


// Comment out the following to totally disable UART output
#define UART_ENABLED
#define UART_BAUDRATE        921600



#define USB_WAITING_PERIOD         1000


#endif /* BOOTLOADER_USB_BOOTLOADER_USB_INC_BOOTLOADER_CONFIG_H_ */
