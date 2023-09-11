#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

MFRC522 mfrc522(10, 9);

bool DoorState = false;
byte DoorCloseCNT = 0;

String RFID_Cards = "";

void setup() {
  //EEPROM Setup
  if (EEPROM.read(0) != 1) {
    EEPROM.write(0, 1);
    writeStringToEEPROM(1, " & ");
  } else {
    RFID_Cards = readStringFromEEPROM(1);
  }
  //end
  //RC522 Setup
  SPI.begin();
  //Serial.begin(9600);
  mfrc522.PCD_Init();
  //end
  //pin mode

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  //end
  digitalWrite(6, HIGH);
}

void loop() {
  if (DoorState == false) checkRFIDCard();
  else checkDoorState();
  delay(333);
}

void checkDoorState() {
  if (analogRead(A0) > 10) DoorCloseCNT++;
  if (DoorCloseCNT > 13) {
    DoorState = false;
    DoorCloseCNT = 0;
    digitalWrite(8, LOW);
  }
}

void checkRFIDCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  if (digitalRead(4) == 0) {
    digitalWrite(2, HIGH);
    RFID_Cards.replace("&" + content.substring(1), "");
    writeStringToEEPROM(1, RFID_Cards);
    delay(1500);
    digitalWrite(2, LOW);
  } else if (digitalRead(5) == 0) {
    digitalWrite(3, HIGH);
    RFID_Cards += "&" + content.substring(1) + " ";
    writeStringToEEPROM(1, RFID_Cards);
    delay(1500);
    digitalWrite(3, LOW);
  } else if (RFID_Cards.indexOf(content.substring(1)) > 0) {
    digitalWrite(3, HIGH);
    digitalWrite(8, HIGH);
    //  Serial.println("Door UNLOCK!");
    DoorState = true;
    delay(1500);
    digitalWrite(3, LOW);
  } else {
    digitalWrite(2, HIGH);
    // Serial.println("Access Denied ID");
    delay(5000);
    digitalWrite(2, LOW);
  }
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++) {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\ 0';
  return String(data);
}