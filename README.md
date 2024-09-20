# FOTA (Firmware Over-The-Air) Project with STM32F401CCU6 and ESP32

This project focuses on enabling efficient and secure **Firmware Over-The-Air (FOTA)** updates for IoT devices, leveraging the power of **STM32F401CCU6** and **ESP32** microcontrollers. This solution is crucial for remotely maintaining devices by eliminating the need for physical access during firmware updates.

## Hardware Setup
- **STM32F401CCU6**: The target microcontroller that receives firmware updates.
- **ESP32**: Acts as the host, managing file downloads and UART communication.
- **UART Communication**: 
  - Data is transferred via **Serial0**.
  - **GPIO15** of ESP32 is connected to the ground to initiate **bootloader mode** on STM32 for flashing operations.

## Wi-Fi & Firebase Integration
- **ESP32** connects to Wi-Fi and retrieves the latest firmware hex file from **Firebase Storage**.
- **STM32’s Flash Memory** is erased at the application start address (`0x8008000`) before flashing the new firmware to ensure a clean memory space.

## Flash Driver Development
A customized **Flash Driver** was developed for **ARM microcontrollers** (specifically STM32) to handle the firmware updates efficiently. The driver is optimized for memory management, which is critical for ensuring smooth and reliable firmware updates.

## Bootloader Functionality
The STM32 bootloader plays a critical role in receiving and flashing new firmware. The following commands are sent from ESP32 to STM32 over UART:

1. **`send_CBL_GO_TO_ADDR_CMD(uint32_t u32Address)`**: 
   - Instructs STM32 to jump to the application address after flashing.
   
2. **`send_CBL_FLASH_ERASE_CMD(uint8_t u8SectorNumber, uint8_t u8NumberOfSectors)`**: 
   - Erases specific flash memory sectors before writing the new firmware.
   
3. **`send_CBL_MEM_WRITE_CMD(uint32_t u32Address, uint8_t Data[WriteDataUnit])`**: 
   - Writes the parsed firmware data from ESP32 to STM32’s memory.

This bootloader was implemented with a strong focus on **compatibility** and **system reliability**, ensuring that updates do not corrupt the device and that the microcontroller can always recover from interrupted updates.

## File Format Handling
The project adeptly handles **Hexadecimal (Intel Hex)** and **Motorola S-record** file formats, both commonly used in firmware updates for ARM microcontrollers. **JavaScript** was utilized to parse and manage these file formats, which are essential for loading firmware data onto the microcontroller.

## Wi-Fi Communication
The **ESP8266 Wi-Fi module** was integrated into the FOTA system to establish **robust wireless communication**. This guarantees **secure and reliable transmission** of firmware updates over Wi-Fi, ensuring that the updates are both safe and timely.

## Parsing the Hex File
ESP32 downloads the firmware hex file from Firebase, parses it into binary data, and writes **8 bytes at a time** to STM32, ensuring accurate flashing over UART.

## Overcoming Serial Communication Issues
Initially, there were issues with **false data transmission** during the serial setup, leading to unexpected behavior.

### Solution:
- I used **`Serial.end()`** to temporarily halt the Serial communication.
- **`Serial.begin(115200)`** is restarted just before actual data transfer, ensuring no erroneous data is sent.

## Process Flow
1. **ESP32 connects to Wi-Fi** and downloads the latest hex file from Firebase.
2. **STM32’s flash memory is erased** using `send_CBL_FLASH_ERASE_CMD`.
3. **ESP32 parses and writes the hex file** to STM32 using `send_CBL_MEM_WRITE_CMD`.
4. **STM32 is instructed to jump** to the new firmware's address using `send_CBL_GO_TO_ADDR_CMD`.
5. **STM32 resets**, and the new firmware is executed seamlessly without interruption.

---

## Future Improvements
- Add support for secure firmware downloads to further improve system integrity.
- Implement real-time progress monitoring for firmware updates.

---

## Contributing
Feel free to submit issues or pull requests if you'd like to contribute!

