#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h> 
#include <RFID.h>
#include <Servo.h> 

RFID rfid(10, 5);       //D10:pin of tag reader SDA. D9:pin of tag reader RST 
unsigned char str[MAX_LEN]; //MAX_LEN is 16: size of the array 
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
SoftwareSerial mySerial(8, 9);
String accessGranted [1] = {"20131558215"};  //RFID serial numbers to grant access to
int accessGrantedSize = 1;                                //The number of serial numbers

Servo lockServo;                //Servo for locking mechanism
int lockPos = 0;               //Locked position limit
int unlockPos = 90;             //Unlocked position limit
boolean locked = true;

const int red = 3;
const int green = 4;
const int buzzer = 7;
String alertMsg="DOOR LOCKED";
int x=2;
String mob="+917491981734";           // Enter second mobile number with country code

void setup()
{
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buzzer, OUTPUT);
  SPI.begin();            //Start SPI communication with reader
  rfid.init();            //initialization 
  
  lcd.init();                      // initialize the lcd 
  lcd.clear();  
  lcd.backlight();

  mySerial.begin(9600);   // Setting the baud rate of GSM Module  
  Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)

  lockServo.attach(6);
  lockServo.write(lockPos);         //Move servo into locked position
  Serial.println("Place card/tag near reader...");
}

void loop()
{
  if (rfid.findCard(PICC_REQIDL, str) == MI_OK)   //Wait for a tag to be placed near the reader
  { 
    Serial.println("Card found"); 
    String temp = "";                             //Temporary variable to store the read RFID number
    if (rfid.anticoll(str) == MI_OK)              //Anti-collision detection, read tag serial number 
    { 
      Serial.print("The card's ID number is : "); 
      for (int i = 0; i < 4; i++)                 //Record and display the tag serial number 
      { 
        temp = temp + (0x0F & (str[i] >> 4)); 
        temp = temp + (0x0F & str[i]); 
      } 
      Serial.println (temp);
      checkAccess (temp);     //Check if the identified tag is an allowed to open tag
    } 
    rfid.selectTag(str); //Lock card to prevent a redundant read, removing the line will make the sketch read cards continually
  }
  rfid.halt();

  lcd.setCursor(1,0);   
  lcd.print("SCAN YOUR RFID");
  lcd.setCursor(x,1);   
  lcd.print(alertMsg);
  delay(500);
  lcd.clear();
}
void checkAccess (String temp)    //Function to check if an identified tag is registered to allow access
{
  boolean granted = false;
  for (int i=0; i <= (accessGrantedSize-1); i++)    //Runs through all tag ID numbers registered in the array
  {
    if(accessGranted[i] == temp)            //If a tag is found then open/close the lock
    {
      Serial.println ("Access Granted");
      digitalWrite(green, HIGH);    //Green LED sequence
      digitalWrite(red, LOW);
      digitalWrite(buzzer, LOW);   
      granted = true;
      if (locked == true)         //If the lock is closed then open it
      {
          SendMessage("Access Granted for Tag Id: "+temp+" - DOOR UNLOCKED",mob);
          lockServo.write(unlockPos);
          x=1;   
          alertMsg="DOOR UNLOCKED";
          locked = false;
      }
      else if (locked == false)   //If the lock is open then close it
      {
          SendMessage("Access Granted for Tag Id: "+temp+" - DOOR LOCKED",mob);
          lockServo.write(lockPos);
          x=2; 
          alertMsg="DOOR LOCKED";
          locked = true;
      }
    }
  }
  if (granted == false)     //If the tag is not found
  {
    Serial.println ("Access Denied");
    SendMessage("Security Breached",mob);
    x=1;    
    alertMsg="ACCESS DENIED";
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);      //Red LED sequence
    digitalWrite(buzzer, HIGH);      //Red LED sequence
  }
  
}
 void SendMessage(String msg, String mob)
{
  Serial.println(msg);  //Message sent to Mobile
  
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\""+mob+"\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println(msg);// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);

}
 
