#include <Wire.h>                       // Library for I2C communication
#include <LiquidCrystal_I2C.h>          // Library for I2C LCD
#include <SPI.h>                        // Library for SPI communication
#include <MFRC522.h>                    // Library for RFID reader
#include <Keypad.h>                     // Library for keypad input
#include <Servo.h>                      // Library for servo control

// Initialize LCD with I2C address 0x27 and size 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);     // Create an LCD object with I2C address 0x27 and 16x2 display

// Initialize a Servo object
Servo servo;                           // Create a Servo object
int servoPos = 0;                      // Variable to store servo position

// Define sensor, buzzer, and LED pins
#define sensorPin1 A2                   // Define analog pin A2 as sensorPin1
#define sensorPin2 A3                   // Define analog pin A3 as sensorPin2
#define buzzerPin A6                    // Define analog pin A6 as buzzerPin
#define Green_Led 12                    // Define digital pin 12 as Green_Led
#define red_Led 13                      // Define digital pin 13 as red_Led

// Variables to store sensor readings
int senVal1 = 0;                       // Variable to store sensor 1 value
int senVal2 = 0;                       // Variable to store sensor 2 value

// Define RFID pins
#define RST_PIN 11                     // Define pin 11 for RFID RST
#define SS_PIN 53                      // Define pin 53 for RFID SS

// Balance for two cards
int card1Balance = 5000;               // Initial balance for card 1
int card2Balance = 300;                // Initial balance for card 2

// Keypad configuration
#define num 7                          // Define the number of characters for input
char Data[num];                        // Array to store keypad input
byte data_count = 0;                   // Counter for number of input characters
String num1, num2, card, card2;        // Strings to store input amounts and card IDs
int a, b;                              // Unused variables
char Key;                              // Variable to store keypad key
bool recharge = true;                  // Flag to indicate recharge mode

// Initialize RFID object
MFRC522 mfrc522(SS_PIN, RST_PIN);      // Create MFRC522 object for RFID

// State variable to track servo position
int state = 0;                         // Variable to store system state

// Keypad size and key mapping
const byte ROWS = 4;                   // Number of rows on the keypad
const byte COLS = 4;                   // Number of columns on the keypad
char keys[ROWS][COLS] = {              // Keypad layout
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};     // Define the row pins for the keypad
byte colPins[COLS] = {5, 4, 3, 2};     // Define the column pins for the keypad

// Initialize Keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);  // Create keypad object

void setup () {
  // Initialize LCD
  lcd.init();                          // Initialize the LCD
  lcd.backlight();                     // Turn on the LCD backlight
  
  // Start serial communication
  Serial.begin(9600);                  // Start serial communication at 9600 baud
  
  // Attach servo to pin 23 and set initial position
  servo.attach(23);                    // Attach the servo to pin 23
  servo.write(120);                    // Set initial servo position to 120 degrees
  
  // Configure pin modes
  pinMode(sensorPin1, INPUT);          // Set sensorPin1 as input
  pinMode(sensorPin2, INPUT);          // Set sensorPin2 as input
  pinMode(buzzerPin, OUTPUT);          // Set buzzerPin as output
  pinMode(Green_Led, OUTPUT);          // Set Green_Led as output
  pinMode(red_Led, OUTPUT);            // Set red_Led as output
  
  // Turn on green LED
  digitalWrite(Green_Led, HIGH);       // Turn on Green_Led
  
  // Initialize SPI and RFID
  SPI.begin();                         // Initialize SPI communication
  mfrc522.PCD_Init();                  // Initialize the RFID reader
  
  // Display initial message on LCD
  lcd.setCursor(0, 0);                 // Set LCD cursor to the first row, first column
  lcd.print(" Automatic toll");        // Print message on LCD
  lcd.setCursor(0, 1);                 // Set LCD cursor to the second row, first column
  lcd.print("collection system");      // Print message on LCD
  delay(3000);                         // Wait for 3 seconds
  lcd.clear();                         // Clear the LCD
}

void loop() {
  if (recharge == 0) {                 // Check if in recharge mode
    // If in recharge mode, handle recharging
    reCharge();                        // Call the recharge function
  } else {
    // Display welcome message
    lcd.setCursor(0, 0);               // Set LCD cursor to the first row, first column
    lcd.print("   Welcome!!!");        // Print welcome message
    
    // Read sensor values and check RFID
    sensorRead();                      // Call the sensor read function
    rfid();                            // Call the RFID read function
    KeyPad();                          // Call the keypad read function
    
    // If vehicle is detected
    if (senVal1 == 0) {                // Check if sensor 1 is activated
      lcd.clear();                     // Clear the LCD
      lcd.setCursor(0, 0);             // Set LCD cursor to the first row, first column
      lcd.print("Vehicle detected");   // Print vehicle detected message
      delay(1000);                     // Wait for 1 second
      lcd.clear();                     // Clear the LCD
      lcd.setCursor(0, 0);             // Set LCD cursor to the first row, first column
      lcd.print("Put your card to");   // Print message
      lcd.setCursor(0, 1);             // Set LCD cursor to the second row, first column
      lcd.print("the reader......");   // Print message
      delay(2000);                     // Wait for 2 seconds
      lcd.clear();                     // Clear the LCD
    }

    // If vehicle has passed the gate
    else if (senVal2 == 0 && state == 1) { // Check if sensor 2 is activated and state is 1
      lcd.clear();                     // Clear the LCD
      lcd.setCursor(0, 0);             // Set LCD cursor to the first row, first column
      lcd.print("Have a safe");        // Print message
      lcd.setCursor(0, 1);             // Set LCD cursor to the second row, first column
      lcd.print("journey");            // Print message
      delay(4000);                     // Wait for 4 seconds
      lcd.clear();                     // Clear the LCD
      servoDown();                     // Call the function to lower the servo
      state = 0;                       // Reset state to 0
    }
  }
}

// Function to lower the servo
void servoDown() {
  servo.attach(23);                    // Attach the servo to pin 23
  for (servoPos = 30; servoPos <= 120; servoPos += 1) { // Move servo from 30 to 120 degrees
    servo.write(servoPos);             // Write servo position
    delay(5);                          // Wait for 5 milliseconds
  }
}

// Function to raise the servo
void servoUp() {
  servo.attach(23);                    // Attach the servo to pin 23
  for (servoPos = 120; servoPos >= 30; servoPos -= 1) { // Move servo from 120 to 30 degrees
    servo.write(servoPos);             // Write servo position
    delay(5);                          // Wait for 5 milliseconds
  }
}

// Function to read sensor values
void sensorRead() {
  senVal1 = digitalRead(sensorPin1);   // Read value from sensorPin1
  senVal2 = digitalRead(sensorPin2);   // Read value from sensorPin2
}

// Function to handle RFID card reading and balance checking
void rfid() {
  if (!mfrc522.PICC_IsNewCardPresent()) { // Check if a new card is present
    return;                             // If not, return
  }
  if (!mfrc522.PICC_ReadCardSerial()) { // Check if card data can be read
    return;                             // If not, return
  }
  
  // Read RFID card UID
  String content = "";                  // Create a string to hold card content
  for (byte i = 0; i < mfrc522.uid.size; i++) { // Loop through card UID
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); // Add leading zero if needed
    content.concat(String(mfrc522.uid.uidByte[i], HEX)); // Add card UID byte in HEX
  }
  content.toUpperCase();                // Convert content to uppercase
  
   // Check if the card is known and has sufficient balance
  if (content.substring(1) == "D3 0F 99 34") { // Check if card UID matches card 1
    if (card1Balance >= 500) {          // Check if card 1 has enough balance
      lcdPrint();                       // Call function to print LCD message
      card1Balance = card1Balance - 500; // Deduct 500 from card 1 balance
      lcd.setCursor(9, 1);              // Set LCD cursor to the second row, tenth column
      lcd.print(card1Balance);          // Print new card 1 balance
      delay(2000);                      // Wait for 2 seconds
      lcd.clear();                      // Clear the LCD
      state = 1;                        // Set state to 1
      servoUp();                        // Call function to raise the servo
    } else {
      card = content.substring(1);      // Store card UID in card variable
      LcdPrint();                       // Call function to print insufficient balance message
      lcd.setCursor(9, 1);              // Set LCD cursor to the second row, tenth column
      lcd.print(card1Balance);          // Print card 1 balance
      lcd.print(" USD");                // Print USD
      delay(2000);                      // Wait for 2 seconds
      lcd.clear();                      // Clear the LCD
      lcd.setCursor(0, 0);              // Set LCD cursor to the first row, first column
      lcd.print("Please Recharge");     // Print recharge message
      delay(1000);                      // Wait for 1 second
      lcd.clear();                      // Clear the LCD
      state = 0;                        // Reset state to 0
    }
  } else if (content.substring(1) == "53 84 32 14") { // Check if card UID matches card 2
    if (card2Balance >= 500) {          // Check if card 2 has enough balance
      lcdPrint();                       // Call function to print LCD message
      card2Balance = card2Balance - 500; // Deduct 500 from card 2 balance
      lcd.setCursor(9, 1);              // Set LCD cursor to the second row, tenth column
      lcd.print(card2Balance);          // Print new card 2 balance
      delay(2000);                      // Wait for 2 seconds
      lcd.clear();                      // Clear the LCD
      state = 1;                        // Set state to 1
      servoUp();                        // Call function to raise the servo
    } else {
      card = content.substring(1);      // Store card UID in card variable
      LcdPrint();                       // Call function to print insufficient balance message
      lcd.setCursor(9, 1);              // Set LCD cursor to the second row, tenth column
      lcd.print(card2Balance);          // Print card 2 balance
      lcd.print(" USD");                // Print USD
      delay(2000);                      // Wait for 2 seconds
      lcd.clear();                      // Clear the LCD
      lcd.setCursor(0, 0);              // Set LCD cursor to the first row, first column
      lcd.print("Please Recharge");     // Print recharge message
      lcd.clear();                      // Clear the LCD
      delay(1000);                      // Wait for 1 second
      state = 0;                        // Reset state to 0
    }
  } else {
    // Handle unknown card
    digitalWrite(buzzerPin, HIGH);      // Turn on buzzer
    digitalWrite(red_Led, HIGH);        // Turn on red LED
    lcd.setCursor(0, 0);                // Set LCD cursor to the first row, first column
    lcd.print("Unknown Vehicle");       // Print unknown vehicle message
    lcd.setCursor(0, 1);                // Set LCD cursor to the second row, first column
    lcd.print("Access denied");         // Print access denied message
    delay(1500);                        // Wait for 1.5 seconds
    lcd.clear();                        // Clear the LCD
    digitalWrite(buzzerPin, LOW);       // Turn off buzzer
    digitalWrite(red_Led, LOW);         // Turn off red LED
  }
}


// Function to handle keypad input
void KeyPad() {
  Key = keypad.getKey();                // Get key from keypad
  if (Key) {                            // Check if a key is pressed
    if (Key == 'A') {                   // Check if key 'A' is pressed
      lcd.clear();                      // Clear the LCD
      lcd.setCursor(0, 0);              // Set LCD cursor to the first row, first column
      lcd.print("Recharging Mode.");    // Print recharging mode message
      lcd.setCursor(0, 1);              // Set LCD cursor to the second row, first column
      lcd.print("................");    // Print dots
      delay(1500);                      // Wait for 1.5 seconds
      lcd.clear();                      // Clear the LCD
      recharge = 0;                     // Set recharge flag to 0
    }
  }
}

// Function to clear input data
void clearData() {
  while (data_count != 0) {             // Loop until data_count is 0
    Data[data_count--] = 0;             // Clear data in Data array
  }
  return;                               // Return from function
}

// Function to handle recharging
void reCharge() {
  lcd.setCursor(0, 0);                  // Set LCD cursor to the first row, first column
  lcd.print ("Enter the amount");       // Print enter the amount message
  Key = keypad.getKey();                // Get key from keypad
  if (Key) {                            // Check if a key is pressed
    if (Key == 'D') {                   // Check if key 'D' is pressed
      if (card == "D3 0F 99 34") {      // Check if card matches card 1
        num1 = Data;                    // Store Data in num1
        card1Balance = num1.toInt() + card1Balance; // Add amount to card 1 balance
        lcd.clear();                    // Clear the LCD
        lcd.setCursor(0, 0);            // Set LCD cursor to the first row, first column
        lcd.print("Your current");      // Print message
        lcd.setCursor(0, 1);            // Set LCD cursor to the second row, first column
        lcd.print("balance: ");         // Print message
        lcd.setCursor(9, 1);            // Set LCD cursor to the second row, tenth column
        lcd.print (card1Balance);       // Print new card 1 balance
        lcd.print(" USD");              // Print USD
        delay(3000);                    // Wait for 3 seconds
        clearData();                    // Clear Data array
        lcd.clear();                    // Clear the LCD
        recharge = 1;                   // Set recharge flag to 1
      } else if (card == "53 84 32 14") { // Check if card matches card 2
        num2 = Data;                    // Store Data in num2
        card2Balance = num2.toInt() + card2Balance; // Add amount to card 2 balance
        lcd.clear();                    // Clear the LCD
        lcd.setCursor(0, 0);            // Set LCD cursor to the first row, first column
        lcd.print("Your current");      // Print message
        lcd.setCursor(0, 1);            // Set LCD cursor to the second row, first column
        lcd.print("balance: ");         // Print message
        lcd.setCursor(9, 1);            // Set LCD cursor to the second row, tenth column
        lcd.print (card2Balance);       // Print new card 2 balance
        lcd.print(" USD");              // Print USD
        delay(3000);                    // Wait for 3 seconds
        clearData();                    // Clear Data array
        lcd.clear();                    // Clear the LCD
        recharge = 1;                   // Set recharge flag to 1
      }
    } else {
      Data[data_count] = Key;           // Store key in Data array
      lcd.setCursor(data_count, 1);     // Set LCD cursor to second row and data_count column
      lcd.print(Data[data_count]);      // Print key on LCD
      data_count++;                     // Increment data_count
    }
  }
}

// Function to display successful payment message
void lcdPrint() {
  digitalWrite(buzzerPin, HIGH);        // Turn on buzzer
  delay(200);                           // Wait for 200 milliseconds
  digitalWrite(buzzerPin, LOW);         // Turn off buzzer
  delay(100);                           // Wait for 100 milliseconds
  lcd.clear();                          // Clear the LCD
  lcd.setCursor(0, 0);                  // Set LCD cursor to the first row, first column
  lcd.print("  Successfully");          // Print message
  lcd.setCursor(0, 1);                  // Set LCD cursor to the second row, first column
  lcd.print(" paid your bill");         // Print message
  delay(1500);                          // Wait for 1.5 seconds
  lcd.clear();                          // Clear the LCD
  lcd.setCursor(0, 0);                  // Set LCD cursor to the first row, first column
  lcd.print("Your Remaining");          // Print message
  lcd.setCursor(0, 1);                  // Set LCD cursor to the second row, first column
  lcd.print("balance: ");               // Print message
}

// Function to display insufficient balance message
void LcdPrint() {
  digitalWrite(buzzerPin, HIGH);        // Turn on buzzer
  delay(200);                           // Wait for 200 milliseconds
  digitalWrite(buzzerPin, LOW);         // Turn off buzzer
  delay(100);                           // Wait for 100 milliseconds
  lcd.clear();                          // Clear the LCD
  lcd.setCursor(0, 0);                  // Set LCD cursor to the first row, first column
  lcd.print("  Your balance");          // Print message
  lcd.setCursor(0, 1);                  // Set LCD cursor to the second row, first column
  lcd.print(" is insufficient");        // Print message
  delay(1500);                          // Wait for 1.5 seconds
  lcd.clear();                          // Clear the LCD
  lcd.setCursor(0, 0);                  // Set LCD cursor to the first row, first column
  lcd.print("Your Remaining");          // Printmessage
  lcd.setCursor(0, 1);                  // Set LCD cursor to the second row, first column
  lcd.print("balance:");                // Print message
}
