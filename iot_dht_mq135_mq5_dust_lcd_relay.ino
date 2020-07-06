#include <SoftwareSerial.h>
// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
int mq5;
int mq135;
int timesTosend=1;
int count=0, count1=0;

#include <stdlib.h>
#include <dht.h>
#define dht_dpin A1

char buf1[16];
char buf2[16];
char buf3[16];
char buf4[16];
char buf5[16];
int i, j;
dht DHT;
String stri;
String strj;
String strden;
String strmq135;
String strmq5;
// replace with your channel's thingspeak API key
String apiKey = "QB00AA8EIKLU7GR9";


SoftwareSerial ser(2, 3); // RX, TX

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000           


/*
I/O define
*/
const int iled = 6;                                            //drive the led of sensor
const int vout = 0;                                            //analog input

/*
variable
*/
float density, voltage;
int   adcvalue;

/*
private function
*/
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

// this runs once
void setup() {                
  // initialize the digital pin as an output.
   lcd.begin(20, 4);
lcd.setCursor(0, 0);
 lcd.print("Air Quality Filter");
 lcd.setCursor(0, 1);
 lcd.print("Team TRIDENT");
 lcd.setCursor(0, 2);
 lcd.print("Abhishek K,Mohd Faiz");
  lcd.setCursor(0, 3);
 lcd.print("Abhishek Vashistha");
 delay(5000);
 
 lcd.clear();
 pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);                                     //iled default closed
  
 lcd.setCursor(0, 0);
 lcd.print("");
  lcd.setCursor(0, 1);
 lcd.print("");
 //delay(3000);
 lcd.clear();
analogReference(DEFAULT);
  // enable debug serial
  Serial.begin(9600);
  // enable software serial
  Serial.println("AT+CMGF=1");
  ser.begin(115200);

  // reset ESP8266
  ser.println("AT+RST");
   delay(500);
  ser.println("AT+CWMODE=3");
   delay(500);
  ser.println("AT+CWJAP=\"project\",\"12345678\"");
  delay(500);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); //relay
}


// the loop
void loop() {
  
     DHT.read11(dht_dpin);
      

 i=DHT.humidity;
      j =DHT.temperature;
 /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
    
  /*
  display the result
  */
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n");  
  


mq135 = analogRead(A2); //mq135
mq5= analogRead(A4);
  lcd.setCursor(0, 2);
    lcd.print("H: ");
 lcd.print(i);
 lcd.print(" % ");

  lcd.setCursor(10, 2);
   lcd.print("T: ");
 lcd.print(j);
  lcd.print(" C ");
    lcd.setCursor(0, 0);
    lcd.print("MQ135: ");
 lcd.print(mq135);
 lcd.setCursor(0, 1);
    lcd.print("Dust: ");
 lcd.print(density);
lcd.print(" ug/m3 ");
 lcd.setCursor(0, 3);
    lcd.print("MQ5: ");
 lcd.print(mq5);
lcd.print("   ");
if(i>60 || j>40 || density> 200 || mq5>500 || mq135>500)
{
   digitalWrite(4, LOW);
}
else
{
   digitalWrite(4, HIGH);
}
  strmq135 = dtostrf(mq135, 4, 1, buf1);
strmq5 = dtostrf(mq5, 4, 1, buf5);
// convert to string
 
 strden = dtostrf(density, 4, 1, buf2);
 stri = dtostrf(i, 4, 1, buf3);
strj = dtostrf(j, 4, 1, buf4);
 Serial.print(strmq5);
  Serial.print(" ");
  Serial.print(strmq135);
  Serial.print(" ");
   Serial.print(strden);
     Serial.print(" ");
    Serial.print(stri);
  Serial.print(" ");
   Serial.print(strj);
  Serial.print(" ");
  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);

  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }

  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strmq135);
  getStr += "\r\n\r\n";



  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);

   

  if(ser.find(">")){
    ser.print(getStr);
  }
   else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSED");
  }
  // thingspeak needs 6 sec delay between updates
  delay(6000);
 
     DHT.read11(dht_dpin);
      

 i=DHT.humidity;
      j =DHT.temperature;
 /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
    
  /*
  display the result
  */
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n");  
  


mq135 = analogRead(A2); //mq135
mq5= analogRead(A4);
  lcd.setCursor(0, 2);
    lcd.print("H: ");
 lcd.print(i);
 lcd.print(" % ");

  lcd.setCursor(10, 2);
   lcd.print("T: ");
 lcd.print(j);
  lcd.print(" C ");
    lcd.setCursor(0, 0);
    lcd.print("MQ135: ");
 lcd.print(mq135);
 lcd.setCursor(0, 1);
    lcd.print("Dust: ");
 lcd.print(density);
lcd.print(" ug/m3 ");
 lcd.setCursor(0, 3);
    lcd.print("MQ5: ");
 lcd.print(mq5);
lcd.print("   ");
if(i>60 || j>40 || density> 200 || mq5>500 || mq135>500)
{
   digitalWrite(4, LOW);
}
else
{
   digitalWrite(4, HIGH);
}
  strmq135 = dtostrf(mq135, 4, 1, buf1);
strmq5 = dtostrf(mq5, 4, 1, buf5);
// convert to string
 
 strden = dtostrf(density, 4, 1, buf2);
 stri = dtostrf(i, 4, 1, buf3);
strj = dtostrf(j, 4, 1, buf4);
 Serial.print(strmq5);
  Serial.print(" ");
  Serial.print(strmq135);
  Serial.print(" ");
   Serial.print(strden);
     Serial.print(" ");
    Serial.print(stri);
  Serial.print(" ");
   Serial.print(strj);
  Serial.print(" ");
      
   // TCP connection
  String cmd1 = "AT+CIPSTART=\"TCP\",\"";
  cmd1 += "184.106.153.149"; // api.thingspeak.com
  cmd1 += "\",80";
  ser.println(cmd1);

  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }


 // prepare GET string
  String getStr1 = "GET /update?api_key=";
  getStr1 += apiKey;
  getStr1 +="&field2=";
  getStr1 += String(strden);
  getStr1 += "\r\n\r\n";




// send data length
  cmd1 = "AT+CIPSEND=";
  cmd1 += String(getStr1.length());
  ser.println(cmd1);


   if(ser.find(">")){
    ser.print(getStr1);
  }
   else{
   ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSED");
  }



  // thingspeak needs 6 sec delay between updates
  delay(6000);
    
     DHT.read11(dht_dpin);
      

 i=DHT.humidity;
      j =DHT.temperature;
 /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
    
  /*
  display the result
  */
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n");  
  


mq135 = analogRead(A2); //mq135
mq5= analogRead(A4);
  lcd.setCursor(0, 2);
    lcd.print("H: ");
 lcd.print(i);
 lcd.print(" % ");

  lcd.setCursor(10, 2);
   lcd.print("T: ");
 lcd.print(j);
  lcd.print(" C ");
    lcd.setCursor(0, 0);
    lcd.print("MQ135: ");
 lcd.print(mq135);
 lcd.setCursor(0, 1);
    lcd.print("Dust: ");
 lcd.print(density);
lcd.print(" ug/m3 ");
 lcd.setCursor(0, 3);
    lcd.print("MQ5: ");
 lcd.print(mq5);
lcd.print("   ");
if(i>60 || j>40 || density> 200 || mq5>500 || mq135>500)
{
   digitalWrite(4, LOW);
}
else
{
   digitalWrite(4, HIGH);
}
  strmq135 = dtostrf(mq135, 4, 1, buf1);
strmq5 = dtostrf(mq5, 4, 1, buf5);
// convert to string
 
 strden = dtostrf(density, 4, 1, buf2);
 stri = dtostrf(i, 4, 1, buf3);
strj = dtostrf(j, 4, 1, buf4);
 Serial.print(strmq5);
  Serial.print(" ");
  Serial.print(strmq135);
  Serial.print(" ");
   Serial.print(strden);
     Serial.print(" ");
    Serial.print(stri);
  Serial.print(" ");
   Serial.print(strj);
  Serial.print(" ");
 // TCP connection
  String cmd3 = "AT+CIPSTART=\"TCP\",\"";
  cmd3 += "184.106.153.149"; // api.thingspeak.com
  cmd3 += "\",80";
  ser.println(cmd3);

  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }


 // prepare GET string
  String getStr3 = "GET /update?api_key=";
  getStr3 += apiKey;
  getStr3 +="&field4=";
  getStr3 += String(strj);
  getStr3 += "\r\n\r\n";



// send data length
  cmd3 = "AT+CIPSEND=";
  cmd3 += String(getStr3.length());
  ser.println(cmd3);


   if(ser.find(">")){
    ser.print(getStr3);
  }
   else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSED");
  }



  // thingspeak needs 15 sec delay between updates

delay(6000);


     DHT.read11(dht_dpin);
      

 i=DHT.humidity;
      j =DHT.temperature;
 /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
    
  /*
  display the result
  */
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n");  
  


mq135 = analogRead(A2); //mq135
mq5= analogRead(A4);
  lcd.setCursor(0, 2);
    lcd.print("H: ");
 lcd.print(i);
 lcd.print(" % ");

  lcd.setCursor(10, 2);
   lcd.print("T: ");
 lcd.print(j);
  lcd.print(" C ");
    lcd.setCursor(0, 0);
    lcd.print("MQ135: ");
 lcd.print(mq135);
 lcd.setCursor(0, 1);
    lcd.print("Dust: ");
 lcd.print(density);
lcd.print(" ug/m3 ");
 lcd.setCursor(0, 3);
    lcd.print("MQ5: ");
 lcd.print(mq5);
lcd.print("   ");
if(i>60 || j>40 || density> 200 || mq5>500 || mq135>500)
{
   digitalWrite(4, LOW);
}
else
{
   digitalWrite(4, HIGH);
}
  strmq135 = dtostrf(mq135, 4, 1, buf1);
strmq5 = dtostrf(mq5, 4, 1, buf5);
// convert to string
 
 strden = dtostrf(density, 4, 1, buf2);
 stri = dtostrf(i, 4, 1, buf3);
strj = dtostrf(j, 4, 1, buf4);
 Serial.print(strmq5);
  Serial.print(" ");
  Serial.print(strmq135);
  Serial.print(" ");
   Serial.print(strden);
     Serial.print(" ");
    Serial.print(stri);
  Serial.print(" ");
   Serial.print(strj);
  Serial.print(" ");
   // TCP connection
  String cmd2 = "AT+CIPSTART=\"TCP\",\"";
  cmd2 += "184.106.153.149"; // api.thingspeak.com
  cmd2 += "\",80";
  ser.println(cmd2);

  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }


 // prepare GET string
  String getStr2 = "GET /update?api_key=";
  getStr2 += apiKey;
  getStr2 +="&field3=";
  getStr2 += String(stri);
  getStr2 += "\r\n\r\n";




// send data length
  cmd2 = "AT+CIPSEND=";
  cmd2 += String(getStr2.length());
  ser.println(cmd2);


   if(ser.find(">")){
    ser.print(getStr2);
  }
   else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSED");
  }
delay(6000);

     DHT.read11(dht_dpin);
      

 i=DHT.humidity;
      j =DHT.temperature;
 /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
    
  /*
  display the result
  */
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n");  
  


mq135 = analogRead(A2); //mq135
mq5= analogRead(A4);
  lcd.setCursor(0, 2);
    lcd.print("H: ");
 lcd.print(i);
 lcd.print(" % ");

  lcd.setCursor(10, 2);
   lcd.print("T: ");
 lcd.print(j);
  lcd.print(" C ");
    lcd.setCursor(0, 0);
    lcd.print("MQ135: ");
 lcd.print(mq135);
 lcd.setCursor(0, 1);
    lcd.print("Dust: ");
 lcd.print(density);
lcd.print(" ug/m3 ");
 lcd.setCursor(0, 3);
    lcd.print("MQ5: ");
 lcd.print(mq5);
lcd.print("   ");
if(i>60 || j>40 || density> 200 || mq5>500 || mq135>500)
{
   digitalWrite(4, LOW);
}
else
{
   digitalWrite(4, HIGH);
}
  strmq135 = dtostrf(mq135, 4, 1, buf1);
strmq5 = dtostrf(mq5, 4, 1, buf5);
// convert to string
 
 strden = dtostrf(density, 4, 1, buf2);
 stri = dtostrf(i, 4, 1, buf3);
strj = dtostrf(j, 4, 1, buf4);
 Serial.print(strmq5);
  Serial.print(" ");
  Serial.print(strmq135);
  Serial.print(" ");
   Serial.print(strden);
     Serial.print(" ");
    Serial.print(stri);
  Serial.print(" ");
   Serial.print(strj);
  Serial.print(" ");
   // TCP connection
  String cmd5 = "AT+CIPSTART=\"TCP\",\"";
  cmd5 += "184.106.153.149"; // api.thingspeak.com
  cmd5 += "\",80";
  ser.println(cmd5);

  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }


 // prepare GET string
  String getStr5 = "GET /update?api_key=";
  getStr5 += apiKey;
  getStr5 +="&field5=";
  getStr5 += String(strmq5);
  getStr5 += "\r\n\r\n";




// send data length
  cmd5 = "AT+CIPSEND=";
  cmd5 += String(getStr5.length());
  ser.println(cmd5);


   if(ser.find(">")){
    ser.print(getStr5);
  }
   else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSED");
  }
delay(6000);
if(i>60 || j>40 || density> 200 || mq5>500 || mq135>500)
{
   digitalWrite(4, LOW);
}
else
{
   digitalWrite(4, HIGH);
}
}
