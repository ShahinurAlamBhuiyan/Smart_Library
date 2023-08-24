
#include <Wire.h> // Wire library
#include <Keypad.h> // keypad library
#include <LiquidCrystal_I2C.h>  // LCD Library


LiquidCrystal_I2C lcd(0x3F, 16, 2); // LCD

// keypad constants -->
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// _____________________________________________________________________


char MAINPASSWORD[4]={};
char MATCHEDPASSWORD[4]={};
int passwordIndex=0;

void setup(){
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
    // startWork();

}

void startWork(){
if(isPassExist()){
      ReEnterPassword();
      isPasswordCorrect();
      tenSecondCounting();
    }else{
      TakePassword();
      tenSecondCounting();
    }
}



void loop(){
  char key = keypad.getKey();
  if(key == 'D'){
    if(isPassExist()){ // if true, that means pass already set last time;
      ReEnterPassword(); 
      if(isPasswordCorrect()){ 
        lcd.setCursor(0, 0);
        lcd.print("Password Correct!");
        delay(1500);
        lcd.clear();
        tenSecondCounting();
        PassSetToEmpty(MAINPASSWORD); // when pass correct, set password will be reset.
      }else{
        lcd.setCursor(0, 0);
        lcd.print("Password In-correct!");
        delay(1500);
        lcd.clear();
        PassSetToEmpty(MATCHEDPASSWORD); // when pass incorrect, make the input again.
      }
    }

    if(!isPassExist()){
      Serial.println("No Exist");
      TakePassword();
      tenSecondCounting();
    }
  }

  // if(isDoorOpen){
  //   startWork();
  // }
  

}


// setup components function.




// Checked if password is already exists -->
bool isPassExist(){
  if(MAINPASSWORD[0] == '\0'){
    return false;
  }else{
    return true;
  }
}

// Taking password from keypad -->
void TakePassword(){
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Set Pass:");
    lcd.setCursor(0,1);

  // taking password
  while(passwordIndex < 4){
    char key = keypad.getKey();
    if(key){
      MAINPASSWORD[passwordIndex] = key;
      lcd.print("T");
      // Serial.print(key);
      passwordIndex++;
    }
  }
  passwordIndex=0;
  // Serial.println();
}

// Re-enter the password -->
void ReEnterPassword(){
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("re-enter pass:");
    lcd.setCursor(0,1);

  while(passwordIndex < 4){
    char key = keypad.getKey();
    if(key){
      MATCHEDPASSWORD[passwordIndex] = key;
      lcd.print("R");
      passwordIndex++;
    }
  }

  lcd.clear();
  passwordIndex=0;
}


// comparing re-enter password -->
bool isPasswordCorrect(){
  int size = sizeof(MAINPASSWORD); 

  bool areEqual = (memcmp(MAINPASSWORD, MATCHEDPASSWORD, size) == 0);
  if(areEqual){
    return true;
  }else{
    return false;
  }
}


// printing password arrays
void printPass(char pass[]){
  for(int i = 0; i < strlen(pass); i++){
    Serial.print(pass[i]);
  }
  Serial.println();
}


void PassSetToEmpty(char arr[]){
  for (int i = 0; i < 4; i++) {
        arr[i] = '\0';
    }
}



// SERVO
void tenSecondCounting(){
  int i;
  for(i = 3; i > 0; i--){
      lcd.setCursor(0, 0);
      lcd.print("Time end in: " );
      lcd.print(i);
      lcd.cursor();
      delay(1000);
      lcd.clear();
  }
   if(i == 0){
      lcd.setCursor(0, 0);
      lcd.print("Time's up!");
      lcd.setCursor(0, 1);
      lcd.print("Door Closing!");
      delay(2000);
      lcd.clear();
    }
}