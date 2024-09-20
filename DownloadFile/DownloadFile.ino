#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the SD card interfaces setting and mounting
#include <addons/SDHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Close"
#define WIFI_PASSWORD "CaputDraconis1977"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDb9BvqNgBXE9FahrDzbSF2KY63jJ1_Xl0"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "fatemahmedkhalil@gmail.com"
#define USER_PASSWORD "123456"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "fota-93301.appspot.com"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

const char* remoteFilePath = "LED.hex";         // Path in Firebase Storage
const char* localFilePath = "/update/LED.hex";  // Path on LittleFS

bool taskCompleted = false;

// Parsing Constants
const int HEX_CONVERTER =                         16;
const int START_OF_RECORD =                       1;
const int BYTE_COUNT_SIZE =                       2;
const int ADDRESS_OFFSET_SIZE =                   4;
const int FLASH_START_SIZE =                      4;
const int DATA_SIZE =                             2;
const int RECORD_TYPE_SIZE =                      2;
const int CHECK_SUM_SIZE =                        2;
const int RECORD_TYPE_DATA =                      0;
const int RECORD_TYPE_END_OF_FILE =               1;
const int RECORD_TYPE_EXTENDED_SEGMENT_ADDRESS =  2;
const int RECORD_TYPE_START_SEGMENT_ADDRESS =     3;
const int RECORD_TYPE_EXTENDED_LINEAR_ADDRESS =   4;
const int RECORD_TYPE_START_LINEAR_ADDRESS =      5;
const int START_SEGMENT_ADDRESS_ERROR =           1;
const int EXTENDED_LINEAR_ADDRESS_ERROR =         2;
const int START_LINEAR_ADDRESS_ERROR =            3;
const int START_SEGMENT_ADDRESS_RECORD_SIZE =     8;
const int EXTENDED_LINEAR_ADDRESS_RECORD_SIZE =   4;
const int START_LINEAR_ADDRESS_RECORD_SIZE =      8;

// Define constants for bootloader commands
#define CBL_GET_VER_CMD             0x10
#define CBL_GET_HELP_CMD            0x11
#define CBL_GET_CID_CMD             0x12
#define CBL_GET_RDP_STATUS_CMD      0x13
#define CBL_GO_TO_ADDR_CMD          0x14
#define CBL_FLASH_ERASE_CMD         0x15
#define CBL_MEM_WRITE_CMD           0x16
#define CBL_ED_W_PROTECT_CMD        0x17
#define CBL_MEM_READ_CMD            0x18
#define CBL_READ_SECTOR_STATUS_CMD  0x19
#define CBL_OTP_READ_CMD            0x20
#define CBL_CHANGE_ROP_Level_CMD    0x21

#define INVALID_SECTOR_NUMBER       0x00
#define VALID_SECTOR_NUMBER         0x01
#define UNSUCCESSFUL_ERASE          0x02
#define SUCCESSFUL_ERASE            0x03

#define FLASH_PAYLOAD_WRITE_FAILED  0x00
#define FLASH_PAYLOAD_WRITE_PASSED  0x01
#define WriteDataUnit                 8

/******  Sector number ******/
typedef enum
{
	SECTOR0_0x08000000,
	SECTOR1_0x08004000,
	SECTOR2_0x08008000,
	SECTOR3_0x0800C000,
	SECTOR4_0x08010000,
	SECTOR5_0x08020000,
	SECTOR6_0x08040000,
	SECTOR7_0x08060000

}FMI_SectorsSelect;

typedef struct FlashInfo
{
  uint8_t StartSector; 
  uint8_t EndSector;
  uint32_t FlashAddress;
} FlashInfo_t;

// Application Flash Info
const FlashInfo_t Application = {
  .StartSector = SECTOR2_0x08008000,
  .EndSector = (SECTOR7_0x08060000 - SECTOR2_0x08008000),
  .FlashAddress = 0x08008000
};

//****************************** Bootloader Commands Control ******************************//
// Check Sum Control
void AppendCRC32(uint8_t* data, uint32_t length) {
  uint32_t crc = 0xFFFFFFFF;
  for (uint32_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 32; j++) {
      if (crc & 0x80000000) {
        crc = (crc << 1) ^ 0x04C11DB7;
      } else {
        crc <<= 1;
      }
    }
  }
  data[length + 0] = (crc >> 0) & 0xFF;
  data[length + 1] = (crc >> 8) & 0xFF;
  data[length + 2] = (crc >> 16) & 0xFF;
  data[length + 3] = (crc >> 24) & 0xFF;
}

// Get Version command
void send_CBL_GET_VER_CMD () {
  uint8_t packet[6];
  packet[0] = 5; // (1 ID) + (4 CRC)
  packet[1] = CBL_GET_VER_CMD;
  AppendCRC32(packet, 2);

  // Send Packet
  Serial.write(packet, 6);
  
  // Response
  Read_Serial_Response();
}

// Get Help command
void send_CBL_GET_HELP_CMD () {
  uint8_t packet[6];
  packet[0] = 5; // (1 ID) + (4 CRC)
  packet[1] = CBL_GET_HELP_CMD;
  AppendCRC32(packet, 2);

  // Send Packet
  Serial.write(packet, 6);
  
  // Response
  Read_Serial_Response();
}

// Get Chip ID command
void send_CBL_GET_CID_CMD () {
  uint8_t packet[6];
  packet[0] = 5; // (1 ID) + (4 CRC)
  packet[1] = CBL_GET_CID_CMD;
  AppendCRC32(packet, 2);

  // Send Packet
  Serial.write(packet, 6);
  
  // Response
  Read_Serial_Response();
}

// Go to Specific Address command
void send_CBL_GO_TO_ADDR_CMD (uint32_t u32Address) {
  uint8_t packet[10];
  packet[0] = 9; // (1 ID) + (4 Address) + (4 CRC)
  packet[1] = CBL_GO_TO_ADDR_CMD;
  packet[2] = (u32Address >> 0 ) & 0xFF;
  packet[3] = (u32Address >> 8 ) & 0xFF;
  packet[4] = (u32Address >> 16) & 0xFF;
  packet[5] = (u32Address >> 24) & 0xFF;
  AppendCRC32(packet, 6);

  // Send Packet
  Serial.write(packet, 10);
  
  // Response
  Read_Serial_Response();
}

// Flash Sectors Erase Command
void send_CBL_FLASH_ERASE_CMD (uint8_t u8SectorNumber, uint8_t u8NumberOfSectors) {
  uint8_t packet[11];
  packet[0] = 10; // (1 ID) + (4 Address) + (1 Number of Sectors)  + (4 CRC)
  packet[1] = CBL_FLASH_ERASE_CMD;
  packet[2] = (u8SectorNumber >> 0 ) & 0xFF;
  packet[3] = (u8SectorNumber >> 8 ) & 0xFF;
  packet[4] = (u8SectorNumber >> 16) & 0xFF;
  packet[5] = (u8SectorNumber >> 24) & 0xFF;
  packet[6] = u8NumberOfSectors;
  AppendCRC32(packet, 7);

  // Send Packet
  Serial.write(packet, 11);

  // Response
  Read_Serial_Response();
}

// Write to Flash command
void send_CBL_MEM_WRITE_CMD(uint32_t u32Address, uint8_t Data[WriteDataUnit]) {
  uint8_t packet[11 + WriteDataUnit];
  packet[0] = 10 + WriteDataUnit;  // (1 ID) + (4 Flash Address) + (Data unit) + (Data Bytes)  + (4 CRC)
  packet[1] = CBL_MEM_WRITE_CMD;
  packet[2] = (u32Address >> 0) & 0xFF;
  packet[3] = (u32Address >> 8) & 0xFF;
  packet[4] = (u32Address >> 16) & 0xFF;
  packet[5] = (u32Address >> 24) & 0xFF;
  packet[6] = WriteDataUnit;
  for (int i = 0; i < WriteDataUnit; i++) {
    packet[7 + i] = Data[i];
  }
  AppendCRC32(packet, 7 + WriteDataUnit);

  Serial.write(packet, 11 + WriteDataUnit);  // Send Packet

  // Response
  Read_Serial_Response();
}

//****************************** Parsing the File and Downloading into STM32 Flash ******************************//
void FlashFile(const char* path) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open the File");
    exit(1);
    return;
  }

  /* Erase Flash to write the new data */
  send_CBL_FLASH_ERASE_CMD (Application.StartSector, Application.EndSector);

  unsigned long flashStartAddress = 0;
  while (file.available()) {
    //***** Start Parsing Hex File *****//
    String line = file.readStringUntil('\n');
    line.trim();
    if (line[0] != ':') {
      Serial.println("START_CODE_ERROR");
      file.close();
      exit(1);  // or any other appropriate action
      return;
    }

    unsigned long startRecordIndex = START_OF_RECORD;
    // Byte count
    unsigned long byteCount = strtoul(line.substring(startRecordIndex, BYTE_COUNT_SIZE + startRecordIndex).c_str(), nullptr, 16);
    startRecordIndex += BYTE_COUNT_SIZE;
    // Address offset
    unsigned long addressOffset = strtoul(line.substring(startRecordIndex, ADDRESS_OFFSET_SIZE + startRecordIndex).c_str(), nullptr, 16);
    startRecordIndex += ADDRESS_OFFSET_SIZE;
    // Record type
    unsigned long recordType = strtoul(line.substring(startRecordIndex, RECORD_TYPE_SIZE + startRecordIndex).c_str(), nullptr, 16);
    startRecordIndex += RECORD_TYPE_SIZE;
    // checksum calculation
    unsigned long checkSum = byteCount;
    checkSum += (addressOffset >> 8) & 0xFF;
    checkSum += (addressOffset >> 0) & 0xFF;
    checkSum += recordType;

    /* Handle record types */
    if (recordType == RECORD_TYPE_DATA) {
      unsigned long address = flashStartAddress + addressOffset;
      // Read the data and append to parsed data
      while (byteCount != 0) {
        uint8_t Data[WriteDataUnit];
        for (int i = 0; i < WriteDataUnit; i++) {
          Data[i] = strtoul(line.substring(startRecordIndex, startRecordIndex + DATA_SIZE).c_str(), nullptr, 16);
          checkSum += Data[i];
          startRecordIndex += DATA_SIZE;
        }
        // Write Data into Flash
        send_CBL_MEM_WRITE_CMD(address, Data);
        byteCount -= WriteDataUnit;
        address += WriteDataUnit;
      }
    }

    else if (recordType == RECORD_TYPE_END_OF_FILE) {
      if (line != String(":00000001FF")) {
        Serial.println("END_OF_FILE_ERROR");
        file.close();
        exit(1); // or any other appropriate action
        return;
      }
    }

    else if (recordType == RECORD_TYPE_EXTENDED_SEGMENT_ADDRESS) {
      if (byteCount != 2) {
        Serial.println("EXTENDED_SEGMENT_ADDRESS_ERROR");
        file.close();
        exit(1); // or any other appropriate action
        return;
      }
      unsigned long segmentAddress = strtoul(line.substring(startRecordIndex, startRecordIndex + 4).c_str(), nullptr, 16);
      checkSum += (segmentAddress >> 8) & 0xFF;
      checkSum += (segmentAddress >> 0) & 0xFF;
      flashStartAddress = segmentAddress << 4;
      startRecordIndex += 4;
    }

    else if (recordType == RECORD_TYPE_START_SEGMENT_ADDRESS) {
      if (byteCount != 4) {
        // The byte count for this mode is always 4
        Serial.println("START_SEGMENT_ADDRESS_ERROR");
        file.close();
        exit(1); // or any other appropriate action
        return;
      }
      // Get the upper 16 bits of the segment address
      unsigned long SegmentAddress = strtoul(line.substring(startRecordIndex, startRecordIndex + START_SEGMENT_ADDRESS_RECORD_SIZE).c_str(), nullptr, 16);
      // Update checksum
      checkSum += (SegmentAddress >> 24) & 0xFF;
      checkSum += (SegmentAddress >> 16) & 0xFF;
      checkSum += (SegmentAddress >> 8) & 0xFF;
      checkSum += (SegmentAddress >> 0) & 0xFF;
      startRecordIndex += START_SEGMENT_ADDRESS_RECORD_SIZE;  // Update Start Record Address
    }

    else if (recordType == RECORD_TYPE_EXTENDED_LINEAR_ADDRESS) {
      if (byteCount != 2) {
        // The byte count for this mode is always 2
        Serial.println("EXTENDED_LINEAR_ADDRESS_ERROR");
        file.close();
        exit(1); // or any other appropriate action
        return;
      }
      // Get the upper 16 bits of the start address
      unsigned long StartAddress = strtoul(line.substring(startRecordIndex, startRecordIndex + EXTENDED_LINEAR_ADDRESS_RECORD_SIZE).c_str(), nullptr, 16);
      // Update checksum
      checkSum += (StartAddress >> 8) & 0xFF;
      checkSum += (StartAddress >> 0) & 0xFF;
      flashStartAddress = StartAddress << 16;
      startRecordIndex += EXTENDED_LINEAR_ADDRESS_RECORD_SIZE;  // Update Start Record Address
    }

    else if (recordType == RECORD_TYPE_START_LINEAR_ADDRESS) {
      if (byteCount != 4) {
        // The byte count for this mode is always 4
        Serial.println("EXTENDESTART_LINEAR_ADDRESS_ERROR");
        file.close();
        exit(1); // or any other appropriate action
        return;
      }
      // Get the upper 16 bits of the start linear address
      unsigned long StartLinearAddress = strtoul(line.substring(startRecordIndex, startRecordIndex + START_LINEAR_ADDRESS_RECORD_SIZE).c_str(), nullptr, 16);
      // Update checksum
      checkSum += (StartLinearAddress >> 24) & 0xFF;
      checkSum += (StartLinearAddress >> 16) & 0xFF;
      checkSum += (StartLinearAddress >> 8) & 0xFF;
      checkSum += (StartLinearAddress >> 0) & 0xFF;
      startRecordIndex += START_LINEAR_ADDRESS_RECORD_SIZE;  // Update Start Record Address
    }

    /* Verify checksum */
    checkSum += strtoul(line.substring(startRecordIndex, startRecordIndex + CHECK_SUM_SIZE).c_str(), nullptr, 16);
    if ((checkSum & 0xFF) != 0) {
      Serial.println("CHECK_SUM_ERROR");
      file.close();
      exit(1); // or any other appropriate action
      return;
    }
  }
  file.close();

  // Jump to App
  send_CBL_GO_TO_ADDR_CMD(Application.FlashAddress);
}

//****************************** Bootloader Response ******************************//
void Read_Serial_Response() {
  uint8_t Length_To_Follow = 0;
  uint8_t BL_ACK = 0xAB;    // Acknowledgement

  while (!Serial.available()) {}
  BL_ACK = Serial.read();
  
  if (BL_ACK == 0xCD) {     // Check if ACK is received (0xCD)
    Length_To_Follow = Serial.read();
    while (!Serial.available()) {}
    uint8_t buffer[Length_To_Follow];
    for (int i = 0; i < Length_To_Follow; i++) {
      buffer[i] = Serial.read();
    }
  }
  else {      // if Received Non-Acknowledgement
    Serial.print ("Recevied Non-Acknowledgement");
    exit(1);  // or any other appropriate action
  }
}

//****************************** Start Flashing ******************************//
void setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Stop Serial temporarily
  Serial.end();
  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Initialize Firebase
  Firebase.begin(&config, &auth);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  /* Assign download buffer size in byte */
  // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
  // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
  config.fcs.download_buffer_size = 2048;

  Firebase.begin(&config, &auth);

  // Start Serial with Baudrate 115200
  Serial.begin(115200);
}

// The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info) {
  if (info.status == firebase_fcs_download_status_init) {
    // Serial.printf("Downloading file %s (%d) to %s\n", info.remoteFileName.c_str(), info.fileSize, info.localFileName.c_str());
  } else if (info.status == firebase_fcs_download_status_download) {
    // Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
  } else if (info.status == firebase_fcs_download_status_complete) {
    // Serial.println("Download completed\n");
  } else if (info.status == firebase_fcs_download_status_error) {
    // Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
  }
}

void loop() {
  if (Firebase.ready() && !taskCompleted) {
    taskCompleted = true;

    // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    // Download into ESP32 Flash as a File.
    if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, 
                                    remoteFilePath /* path of remote file stored in the bucket */, 
                                    localFilePath /* path to local file */, 
                                    mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, 
                                    fcsDownloadCallback /* callback function */)) {
      // Serial.println(fbdo.errorReason());
    }
    FlashFile(localFilePath); // Flashing Over the Air
  }
}