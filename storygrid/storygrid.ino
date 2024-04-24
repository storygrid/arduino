#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Hashtable.h>


// RFID GLOBALS
#define SS_PIN 53
#define RST_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
byte nuidPICC[3]; // Init array that will store new NUID
// RFID GLOBALS

int FIGURE = -1;
int highest_fig_id = -1;
Hashtable<int, int> figIDs;

// Define the keymap for your 4x4 charlieplex array
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    { 'A', 'B', 'C', 'D' },
    { 'E', 'F', 'G', 'H' },
    { 'I', 'J', 'K', 'L' },
    { 'M', 'N', 'O', 'P' }
};

// Define the row and column pins for your charlieplex array
byte rowPins[ROWS] = { 37, 35, 33, 31 };  // Cathodes || { 53, 51, 49, 47 }
byte colPins[COLS] = { 23, 25, 27, 29 };  // Anodes   || { 39, 41, 43, 45 }

// Create a Keypad object
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
    Serial.begin(9600);
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522 
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
  
    customKeypad.setDebounceTime(500);
    customKeypad.setHoldTime(500);
    customKeypad.addEventListener(keypadEvent);  // Add an event listener for key presses
}

void loop() {
    customKeypad.getKeys();
    
    // Look for new cards
    if ( ! rfid.PICC_IsNewCardPresent())
        return;

    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
        return;

    // Check is the PICC of Classic MIFARE type
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return;
    }

    int uid = (int) getRFIDID(rfid);
    // Serial.print("Card detected, UID: ");
    // Serial.println(uid);
    // Serial.println();
    if (uid == 0) {
        return;
    }
    if(!figIDs.containsKey(uid)) {
        highest_fig_id += 1;
        figIDs.put(uid, highest_fig_id);
    }
    FIGURE = figIDs.getElement(uid);
}

unsigned long getRFIDID(MFRC522 rfid){
    unsigned long hex_num;
    hex_num =  rfid.uid.uidByte[0] << 24;
    hex_num += rfid.uid.uidByte[1] << 16;
    hex_num += rfid.uid.uidByte[2] <<  8;
    hex_num += rfid.uid.uidByte[3];
    rfid.PICC_HaltA(); // Stop reading
    return hex_num;
}

// Function to handle key press events
void keypadEvent(KeypadEvent key) {
    char strBuf[50] = "";
    
    switch (customKeypad.getState()) {
        case PRESSED:
            sprintf(strBuf, "%d @ %c - DOWN", FIGURE, key);
            Serial.println(strBuf);
            break;
        case RELEASED:
            sprintf(strBuf, "%d @ %c - UP", FIGURE, key);
            Serial.println(strBuf);
            break;
        case IDLE:
            // No action needed for IDLE state
            break;
    }
}
