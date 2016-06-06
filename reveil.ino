//We always have to include the library
#include "LedControlMS.h"
#include <Wire.h>
#include "RTClib.h"
#include <ClickEncoder.h>
#include <TimerOne.h>

ClickEncoder *encoder;

//0: heure
//1: réglage heure
//2: alarme
//3: réglage alarme
//9: menu
int mode = 0; 

int afficheOption = 0;
int intensite = 0; //0..15
int intensitePrec = 0; //0..15

//0: rien
//1: cliqué
//2: doublecliqué
ClickEncoder::Button btnStatus;

int16_t last, value;

void timerIsr() {
  encoder->service();
}
 
RTC_DS1307 RTC;


/*
 * Affichage :
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 + nombre d'écrans
 */
 int NB_AFFICHAGES = 4;
LedControl lc=LedControl(12,11,10,NB_AFFICHAGES);

//TODO à sortir dans un .h
//Matrices d'affichage
byte chiffreD[][8]={
{B00000000,
B00011100,
B00100010,
B00100010,
B00100010,
B00100010,
B00100010,
B00011100},

{B00000000,
B00000010,
B00000110,
B00001010,
B00000010,
B00000010,
B00000010,
B00000010},

{B00000000,
B00011100,
B00100010,
B00000010,
B00000100,
B00001000,
B00010000,
B00111110},

{B00000000,
B00011100,
B00100010,
B00000010,
B00001100,
B00000010,
B00100010,
B00011100},

{B00000000,
B00000100,
B00001000,
B00010000,
B00100100,
B00111110,
B00000100,
B00000100},

{B00000000,
B00111110,
B00100000,
B00100000,
B00111100,
B00000010,
B00100010,
B00011100},

{B00000000,
B00011100,
B00100010,
B00100000,
B00111100,
B00100010,
B00100010,
B00011100},

{B00000000,
B00111110,
B00000010,
B00000100,
B00001000,
B00010000,
B00010000,
B00010000},

{B00000000,
B00011100,
B00100010,
B00100010,
B00011100,
B00100010,
B00100010,
B00011100},

{B00000000,
B00011100,
B00100010,
B00100010,
B00011110,
B00000010,
B00100010,
B00011100}
};

byte chiffreG[][8]={
{B00000000,
B00111000,
B01000100,
B01000100,
B01000100,
B01000100,
B01000100,
B00111000},

{B00000000,
B00000100,
B00001100,
B00010100,
B00000100,
B00000100,
B00000100,
B00000100},

{B00000000,
B00111000,
B01000100,
B00000100,
B00001000,
B00010000,
B00100000,
B01111100},

{B00000000,
B00111000,
B01000100,
B00000100,
B00011000,
B00000100,
B01000100,
B00111000},

{B00000000,
B00001000,
B00010000,
B00100000,
B01001000,
B01111100,
B00001000,
B00001000},

{B00000000,
B01111100,
B01000000,
B01000000,
B01111000,
B00000100,
B01000100,
B00111000},

{B00000000,
B00111000,
B01000100,
B01000000,
B01111000,
B01000100,
B01000100,
B00111000},

{B00000000,
B01111100,
B00000100,
B00001000,
B00010000,
B00100000,
B00100000,
B00100000},

{B00000000,
B00111000,
B01000100,
B01000100,
B00111000,
B01000100,
B01000100,
B00111000},

{B00000000,
B00111000,
B01000100,
B01000100,
B00111100,
B00000100,
B01000100,
B00111000}
};

byte vide[8]=
{B00000000,//affichage vide
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000};

byte lettres[][8]={
{B00000000,//A
B00011000,
B00100100,
B00111100,
B01000010,
B01000010,
B10000001,
B00000000},

{B00000000,
B11111110,//E
B10000000,
B10000000,
B11111000,
B10000000,
B10000000,
B11111110},

{B00000000,//G
B00111100,
B01000010,
B10000000,
B10011110,
B10000100,
B01000100,
B01111100},

{B00000000,//H
B10000010,
B10000010,
B10000010,
B11111110,
B10000010,
B10000010,
B10000010},

{B00000000,//I
B00111000,
B00010000,
B00010000,
B00010000,
B00010000,
B00010000,
B00111000},

{B00000000,//L
B10000000,
B10000000,
B10000000,
B10000000,
B10000000,
B10000000,
B11111110},

{B00000000,//M
B11000110,
B10101010,
B10010010,
B10000010,
B10000010,
B10000010,
B10000010},


{B00000000,//N
B10000010,
B11000010,
B10100010,
B10010010,
B10001010,
B10000110,
B10000010},

{B00000000,//R
B11111100,
B10000010,
B10000010,
B11111100,
B10001000,
B10000100,
B10000010},

{B00000000,//T
B11111110,
B00010000,
B00010000,
B00010000,
B00010000,
B00010000,
B00010000},

{B00000000,//U
B10000010,
B10000010,
B10000010,
B10000010,
B10000010,
B11000110,
B00111000},

{B00000000,//X
B10000010,
B01000100,
B00101000,
B00010000,
B00101000,
B01000100,
B10000010}

};

/* we always wait a bit between updates of the display */
int sec = 0;



/* 
 This time we have more than one device. 
 But all of them have to be initialized 
 individually.
 */
void setup() {
  Serial.begin(9600);

  pinMode(3, INPUT_PULLUP);
  
  encoder = new ClickEncoder(A1, A0, A2);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  last = -1;
  //we have to init all devices in a loop
  for(int address=0;address<NB_AFFICHAGES;address++) {
    /*The MAX72XX is in power-saving mode on startup*/
    lc.shutdown(address,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(address,intensite);
    /* and clear the display */
    lc.clearDisplay(address);
  }

  Wire.begin();
  RTC.begin();
  encoder->setAccelerationEnabled(false);
  //RTC.adjust (DateTime (__ DATE__, __TIME__)); 
  //RTC.setAlarm (Alarm (0, 0));
}



void choixMenu()
{
  if(afficheOption==0)
  {
    mode = 0;
    afficheHeure();
  }
  else if(afficheOption==1)
  {
    mode = 1; // le mode = 2 est réservé au réglage des minutes
    regleHeures();
  }
  else if(afficheOption==2)
  {
    mode = 3; //Le mode = 4 est réservé au réglage des minutes
    regleAlarmeHeures();
  }
  else if(afficheOption==3)
  {
    mode = 5;
    regleLight();
  }
}

void regleHeures()
{ 
  Serial.println("Regle heure"); 
  DateTime now = RTC.now();

  afficheHeure();
  entoureHeures();
  
  int heure = now.hour();
  value = encoder->getValue();
  Serial.println(value);
  if (value != 0)
  {
    heure=heure+value;
    Serial.println(heure);
    if(heure<0)heure=23;
    if(heure>23)heure=0;
    RTC.adjust(DateTime(now.year(), now.month(), now.day(), heure, now.minute(), now.second()));
  }
}

void regleMinutes()
{  
  Serial.println("Regle minutes");
  DateTime now = RTC.now();
  
  afficheHeure();
  entoureMinutes();
  
  int minutes = now.minute();
  value = encoder->getValue();
  if (value != 0)
  {
    minutes=minutes+value;
    Serial.println(minutes);
    if(minutes<0)minutes=59;
    if(minutes>59)minutes=0;
    RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), minutes, now.second()));
  }
}

void regleAlarmeMinutes()
{  
  Alarm alarm = RTC.alarm();

  afficheChiffres(alarm.hour()/10, alarm.hour()%10,alarm.minute()/10, alarm.minute()%10);
  entoureMinutes();
  
  int minutes = alarm.minute();
  value = encoder->getValue();
  Serial.println(value);
  if (value != 0)
  {
    minutes=minutes+value;
    Serial.println(minutes);
    if(minutes<0)minutes=59;
    if(minutes>59)minutes=0;
    RTC.setAlarm(Alarm(alarm.hour(), minutes));
  }
}

void entoure(int ecran)
{
    lc.setLed(ecran,0,0,true);
    lc.setLed(ecran+1,0,7,true);

    lc.setLed(ecran,7,0,true);
    lc.setLed(ecran+1,7,7,true);
}

void entoureHeures(){entoure(0);}

void entoureMinutes(){entoure(2);}

void regleAlarmeHeures()
{
  Serial.println("Règle alarme");
  Alarm alarm = RTC.alarm();

  afficheChiffres(alarm.hour()/10, alarm.hour()%10,alarm.minute()/10, alarm.minute()%10);
  entoureHeures();
  
  int heure = alarm.hour();
  value = encoder->getValue();
  Serial.println(value);
  if (value != 0)
  {
    heure=heure+value;
    Serial.println(heure);
    if(heure<0)heure=23;
    if(heure>23)heure=0;
    RTC.setAlarm(Alarm(heure, alarm.minute()));
  }
}

void regleLight()
{
  if(intensitePrec!=intensite)
  {
    for(int address=0;address<NB_AFFICHAGES;address++) 
    {
      lc.setIntensity(address,intensite);
    }
  }
  afficheChiffres(-1, -1, (((intensite/10)==0)?(-1):(intensite/10)), intensite%10);
  value = encoder->getValue();
  if (value != 0)
  {
    intensitePrec = intensite;
    intensite=intensite+value;
    if(intensite<0)intensite=0;
    if(intensite>15)intensite=15;
    Serial.println(intensite);
  }
  delay(100);
}

void afficheMot()
{
  if(afficheOption==0)
  {
    afficheLettres(1,11,4,9); //EXIT
  }
  else if(afficheOption==1)
  {
    afficheLettres(9,4,6,1); //TIME
  }
  else if(afficheOption==2)
  {
    afficheLettres(0,5,8,6); //ALRM
  }
  else if(afficheOption==3)
  {
    afficheLettres(5,2,3,9); //LGHT
  }
}

void afficheLettres(int D1,int D2,int D3,int D4)
{
  for(int row=0;row<8;row++) {
        lc.setRow(0,row,lettres[D1][row]);
  }
  for(int row=0;row<8;row++) {
        lc.setRow(1,row,lettres[D2][row]);
  }
  for(int row=0;row<8;row++) {
        lc.setRow(2,row,lettres[D3][row]);
  }
  for(int row=0;row<8;row++) {
        lc.setRow(3,row,lettres[D4][row]);
  }
}

void afficheChiffres(int D1,int D2,int D3,int D4)
{
  for(int row=0;row<8;row++) 
  {
    if(D1!=-1)
    {
      lc.setRow(0,row,chiffreD[D1][row]);
    }
    else
    {
      lc.setRow(0,row,vide[row]);
    }

    if(D2!=-1)
    {
      lc.setRow(1,row,chiffreG[D2][row]);
    }
    else
    {
      lc.setRow(1,row,vide[row]);
    }
  
    if(D3!=-1)
    {
      lc.setRow(2,row,chiffreD[D3][row]);
    }
    else
    {
      lc.setRow(2,row,vide[row]);
    }
 
    if(D4!=-1)
    {
      lc.setRow(3,row,chiffreG[D4][row]);
    }
    else
    {
      lc.setRow(3,row,vide[row]);
    }
  }
}

void afficheHeure()
{
    DateTime now = RTC.now();
    afficheChiffres(now.hour()/10,
                    now.hour()%10,
                    now.minute()/10,
                    now.minute()%10);
    
}

void afficheMenu()
{
    afficheMot();
    value = encoder->getValue();
  
    if (value != 0)
    {
      //last = value;
      //Serial.println(last);
      afficheOption=(afficheOption+value)%4;
      //choixMenu();
    }
    delay(100);
}

void choixDoubleClick()
{
  //On était dans le menu, on retourne à l'heure
  if(mode==9)
  {
    mode = 0;
  }
  //On n'était pas dans le menu et on y retourne
  else
  {
    mode = 9;
    afficheOption=0;
  }
}

void choixClick()
{
  //On était dans le menu, on rentre dans un sous menu
  if(mode==9)
  {
    choixMenu();
  }
  //On était dans le réglage du time(heure), on va vers le time(minutes)
  else if(mode==1)
  {
    mode = 2;
  }
  //On était dans le réglage du time(minutes), on va vers le menu
  else if(mode==2)
  {
    mode = 9;
    afficheOption=0;
  }
  //On était dans le réglage du alarm(heure), on va vers le alarm(minutes)
  else if(mode==3)
  {
    mode = 4;
  }
  //On était dans le réglage du alarm(minutes), on va vers le menu
  else if(mode==4)
  {
    mode = 9;
    afficheOption=0;
  }
  //On était dans le réglage de la luminosité, on en sort direction le menu
  else if(mode==5)
  {
    mode = 9;
    afficheOption=0;
  }
  else //?
  {
    mode = 0;
  }
}

void loop() { 
  encoder->setAccelerationEnabled(false);
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    #define VERBOSECASE(label) case label: Serial.println(#label); break;
    switch (b) {
      case ClickEncoder::Pressed:
        Serial.println("ClickEncoder::Pressé");
        break;
      case ClickEncoder::Held:
        Serial.println("ClickEncoder::Maintenu");
      break;
      case ClickEncoder::Released:
        Serial.println("ClickEncoder::Relaché");
      break;
      case ClickEncoder::Clicked:
        choixClick();
        Serial.println("ClickEncoder::Cliqué");
      break;
      case ClickEncoder::DoubleClicked:
          Serial.println("ClickEncoder::DoubleClicked");
          choixDoubleClick();
        break;
    }
  }

  if(mode==9)
  {
    afficheMenu();
  }
  else if(mode==1)
  {
    regleHeures();
  }
  else if(mode==2)
  {
    regleMinutes();
  }
  else if(mode==3)
  {
    regleAlarmeHeures();
  }
  else if(mode==4)
  {
    regleAlarmeMinutes();
  }
  else if(mode==5)
  {
    regleLight();
  }
  else
  {
    afficheHeure();
  }  
}
