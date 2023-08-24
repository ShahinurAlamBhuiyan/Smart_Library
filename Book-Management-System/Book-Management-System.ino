
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10  // Slave Select pin
#define RST_PIN 9  // Reset pin
#define BUZZER_PIN 7  // Buzzer pin

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create an instance of the MFRC522 library

const int maxCards = 10;  // Maximum number of cards to store
String cardUIDs[maxCards];  // Array to store card UIDs
int cardCount = 0;  // Counter for stored card count

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  SPI.begin();  // Initiate SPI bus
  mfrc522.PCD_Init();  // Initiate MFRC522
  Serial.println("Scan an RFID card");
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  String uidStr = "";  // Create an empty string to store the UID
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    
    // Convert each byte of UID to a two-digit hexadecimal string
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidStr += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uidStr += String(mfrc522.uid.uidByte[i], HEX);
    }
    
    // Store the UID string in the array
    if (cardCount < maxCards) {
      cardUIDs[cardCount] = uidStr;
      cardCount++;
      Serial.println("Card UID stored: " + uidStr);
    } else {
      Serial.println("Array is full. Cannot store more card UIDs.");
    }
    
    mfrc522.PICC_HaltA();  // Halt the current card
    mfrc522.PCD_StopCrypto1();  // Stop encryption on the card
  }
  if(uidStr == "dc7fba38"){
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
    digitalWrite(BUZZER_PIN, LOW);
  delay(300);
}