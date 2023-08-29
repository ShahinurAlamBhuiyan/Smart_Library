#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <Servo.h> // servo motor code
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// -------Sound Sensor-------------
const int soundSensorPin = A2;
const int buzzerPin = 6; 
// ----------------------------

// IR-SENSOR -------
#define IR_SENSOR_PIN 5
#define DOOR_SERVO_PIN 4

// -------PhotoResistor---------------
const int photoresistorPin = A0;
const int bulbPin = 47;


LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD// set the LCD address to 0x27 for a 16 chars and 2 line display

Servo myServo;  // Create a Servo object
Servo mySideServo;  // Create a Servo object
Servo doorServo;  // Create a Servo object

int MAIN_SERVO_PIN = 9;
int SIDE_SERVO_PIN = 7;

SoftwareSerial BTserial(10, 11); // RX | 
#define SS_PIN 53
#define RST_PIN 49
int LEDPin = 13;

// Define constants for button and LED pins
const int takeButton = 2; // Push button connected to digital pin 2
const int giveButton = 3;
const int takeLEDPin = 13;   // LED connected to digital pin 13
const int giveLEDPin = 12;

// Variable to store the button state
int takeButtonState = 0;
int giveButtonState = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

struct BookRFID {
  String uid;  // Change the size if needed
  String bookNo;
};

const int MAX_BOOKS = 100;  // Maximum number of books you want to track
BookRFID books[MAX_BOOKS];
int numBooks = 0;
int totalBookCount = 3;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522
  Serial.println("RFID reading UID");

  myServo.attach(MAIN_SERVO_PIN);
  mySideServo.attach(SIDE_SERVO_PIN);
  doorServo.attach(DOOR_SERVO_PIN);

  doorServo.write(180);
  mySideServo.write(180);
  myServo.write(0);
  BTserial.begin(9600); 
  lcd.init();
  lcd.backlight();


    // Initialize the LED pin as an output
  pinMode(takeLEDPin, OUTPUT);
  pinMode(giveLEDPin, OUTPUT);
  // Initialize the button pin as an input
  pinMode(takeButton, INPUT_PULLUP);
  pinMode(giveButton, INPUT_PULLUP);

  pinMode(bulbPin, OUTPUT);

  // BUZZER
  pinMode(buzzerPin, OUTPUT);
  
}


bool rf_id_scanned = false;
int pressedButton = 0;
String ScannedBook = "";
String BookCollections[] = {"uiu-book-1", "uiu-book-2"};
int bookSlot = -1;
bool storingInProgress = false;
String scannedUID;
void loop() {
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(totalBookCount);
  lcd.print(" books left.");
  lcd.setCursor(0, 1);
  lcd.print("Scan ID Card!");
  if(mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("Rf_id present");
    // getting rf_id name --> stored in scannedUID.
    if (mfrc522.PICC_ReadCardSerial()) {
       scannedUID = ""; // Reset the string before storing new UID
      
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        scannedUID += String(mfrc522.uid.uidByte[i], HEX);
      }
    }
    if(scannedUID){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan Slot QR &");
      lcd.setCursor(0, 1);
      lcd.print("Send book code.");

    }
    ScannedBook = "";
    rf_id_scanned = true;
    Serial.println(rf_id_scanned);
    if(rf_id_scanned){  
      // then scan qrcode --> 
      while(rf_id_scanned && ScannedBook == ""){
        scanQRCode();
        Serial.println(ScannedBook);
      }
    }

    int arraySize = sizeof(BookCollections) / sizeof(BookCollections[0]);

    

    for (int i = 0; i < arraySize; i++) {
        if (BookCollections[i] == ScannedBook) {
            bookSlot = i;
        }
    }

    lcd.setCursor(0, 1);
    if (bookSlot != -1) {
        Serial.print("Slot no: ");
        Serial.println(bookSlot+1);
        if(totalBookCount > 0 && totalBookCount < 3){ // 
          lcd.print("PRESS TAKE/GIVE.");
        }else if(totalBookCount == 0){
          lcd.print("PRESS GIVE.");
        }else if(totalBookCount == 3){
          lcd.print("PRESS TAKE.");
        }

    } else {
        Serial.println("Slot not found");
        lcd.print("QR NOT MATCHED.");
        delay(2500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reset!");
        lcd.setCursor(0, 1);
        lcd.print("Scan Card Again.");
        delay(2500);
        lcd.clear();
    }

   
    if(bookSlot+1 == 1){  // Slot 1 will activated
      Serial.println("Slot one activated!");
      buttonAndLed();
      if(pressedButton == 1){ 
        if(totalBookCount > 0){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Take PRESSED!");
          delay(1500);
          OPENDOOR();
          // if (mfrc522.PICC_ReadCardSerial()) {
          if (numBooks < MAX_BOOKS) {
              books[numBooks].uid =  scannedUID;
              books[numBooks].bookNo = ScannedBook;
              numBooks++;
              storingInProgress = false; // Reset the flag after storing
          }
          totalBookCount--;
          printBooks();
        }else{
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("No Book in Slot!");
        }

      }

      if(pressedButton == 2){
        Serial.println("Give button pressed for slot 1");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Give PRESSED!");
        delay(1500);
        if(totalBookCount < 3){
          totalBookCount++;
          CLOSEDOOR();
          delay(1500);
          for (int i = 0; i < numBooks; i++) {
            if (books[i].uid == scannedUID) {
              lcd.setCursor(0, 1);
              lcd.print("History Deleted.");
              delay(1500);
              // Delete the data by shifting remaining elements
              for (int j = i; j < numBooks - 1; j++) {
                books[j] = books[j + 1];
              }
              numBooks--; 
            }
          }
          lcd.clear();
        }else{
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Slot is full!");
          delay(2500);
          lcd.clear();
        }
       
      }
    }

    if(bookSlot+1 == 2){ // Slot 2 will activated
      if(pressedButton == 3){ 
        Serial.println("Take button pressed for slot 2");
        // ServoWork();
      }
      if(pressedButton == 4){
        Serial.println("Take button pressed for slot 2");
      }
    }

    bookSlot=-1;

    
  }
    // delay(500);
    // Serial.print(totalBookCount);
    // Serial.println(" avaiable!");
    
    // printBooks();
    pressedButton  = 0;
    // Serial.println("----------");


  TableSoundSensor(); // SOUND-SENSOR --> 
  photoResistor();
  MAINDOORIRSENSOR();// door
  TableSoundSensor(); // SOUND-SENSOR --> 
    
}

int servoAngle = 0;
void OPENDOOR(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OPENING...");
  for(int i = servoAngle; i < servoAngle+18; i+=1){
    myServo.write(i);
    delay(50);
  }
  servoAngle+=18;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Take the book.");
  delay(1000);
  for(int i = 180; i >= 50; i-=5){
    mySideServo.write(i);
    delay(20);
  }

  mySideServo.write(180);
  

}

// int servoAngle = 0;
void CLOSEDOOR(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CLOSING...");
  for(int i = servoAngle; i > servoAngle-18; i-=1){
    myServo.write(i);
    delay(50);
  }
  servoAngle-=18;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Give the book.");

}

void scanQRCode() {
  String receivedData = "";
  while(receivedData == "") {
    receivedData = BTserial.readStringUntil(';'); // Read data until semicolon
    receivedData.trim();
    rf_id_scanned = false;
    ScannedBook = receivedData;// Remove leading/trailing spaces
    if(ScannedBook != ""){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sent!");
    }
  }
}


bool isPressed = false;
void buttonAndLed() {
//  bool isPressed = false;
  while(!isPressed){
    takeButtonState = digitalRead(takeButton);
    if(takeButtonState == 0){
      digitalWrite(takeLEDPin, HIGH);
      // rf_id_scaned = false;
      pressedButton =  1; // if take button push it will return 1;
      // Serial.println(pressedButton);
      isPressed = true;
    }
  
    giveButtonState = digitalRead(giveButton);
    if(giveButtonState == 0){
      digitalWrite(giveLEDPin, HIGH);
      pressedButton =  2; 
      // Serial.println(pressedButton);// if give button push it will return 2;
      isPressed = true;
    }
  }
  delay(1000);
  digitalWrite(takeLEDPin, LOW);
  digitalWrite(giveLEDPin, LOW);
  isPressed = false;
  // rf_id_scanned = false;
}

// give book with qr code
void lightUpLED(){
    if (BTserial.available()) {
    String receivedData = BTserial.readStringUntil(';'); // Read data until semicolon
    receivedData.trim(); // Remove leading/trailing spaces
    
    if (receivedData.equals("uiu-book-29")) {
      digitalWrite(LEDPin, HIGH);
      BTserial.println("LED on");
    } else {
      digitalWrite(LEDPin, LOW);
      BTserial.println("LED off");
    }
  }
}

void printBooks() {
  for (int i = 0; i < numBooks; i++) {
    Serial.print("UID: ");
    Serial.println(books[i].uid);
    Serial.print("Book Number: ");
    Serial.println(books[i].bookNo);
  }
}





// -----------SOUND SENSOR CODE------------------
void TableSoundSensor(){
  int soundValue = analogRead(soundSensorPin);
  Serial.print("sound - ");
  Serial.println(soundValue);

  // Check if soundValue is above a certain threshold
  if (soundValue >= 27) { // Adjust the threshold as needed
    // Activate the buzzer for 1 second with a medium-frequency beep
    tone(buzzerPin, 2000); // Adjust the frequency as needed
    delay(500); // Beep duration
    noTone(buzzerPin);
  }

  delay(100); // Add a delay to prevent rapid reading and toggling
}

// -------------------------------------------------------


// -----------------IR SENSOR IN MAIN DOOR ----------------
void MAINDOORIRSENSOR(){
  int irValue = digitalRead(IR_SENSOR_PIN); // Read the value from IR sensor

  if (irValue == LOW) {
    // IR sensor detects an obstacle (object in front)
    Serial.println("401");
    for(int i = 190; i > 90; i--){
      
       doorServo.write(i); // Open the door (rotate servo to 180 degrees)
       delay(10);
    }
    // Wait until the object moves away before closing the door
    while (irValue == LOW) {
      irValue = digitalRead(IR_SENSOR_PIN);
    }
    delay(1000); 
    for(int i = 90; i < 190; i++){
       doorServo.write(i); // Close the door (rotate servo to 90 degrees)
       delay(20);
    }
  }

  delay(100); // Delay for smoother operation
}

// ---------------------------------------------------------------


// ----------PHOTO-RESISTOR----------------------
// bulbPin
void photoResistor(){

int lightIntensity = analogRead(photoresistorPin);
  Serial.print("Light Intensity: ");
  Serial.println(lightIntensity);

  // If the light intensity is below a threshold, turn on the LED
  if (lightIntensity > 60) { // Adjust the threshold value as needed
    digitalWrite(bulbPin, HIGH); // Turn on the LED
  } else {
    digitalWrite(bulbPin, LOW); // Turn off the LED
  }

  delay(1000);



}


