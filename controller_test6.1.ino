/* Working prototype
 *  password screen working, clr reseting entered passwords, enter advances to password confirmation screen
 *  
 *  remaining todo: bar under selected unit (hrs, mins, or seconds). on countdown scree, put "target time" on top, print out target time
 *  put remaining time below, and then the countdown below that. create a back button and a logout button on the completed screen.
*/

//libraries
#include <Adafruit_GFX.h> //main graphics library
#include <TouchScreen.h>  //supporting library
#include <MCUFRIEND_kbv.h> //main tft library
#include <Adafruit_ILI9341.h>
#include <SPI.h> //for display
#include <Wire.h> //needed for FT6206
#include <Adafruit_FT6206.h>

//default for adafruit shield
#define TFT_DC 9
#define TFT_CS 10

//FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

//use hardware SPI (on Uno, 13, 12, 11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//colors defined
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//button placement
#define BUTTON   60 //changable button size
#define FIRSTR   35
#define SECONDR  105
#define THIRDR   175
#define FOURTHR  245
#define FIRSTC   20
#define SECONDC  95
#define THIRDC   170

int counter = 0;
int i = 0;
int myPassword[4] = {1, 1, 1, 1};
int guess[4];
int secOn = 0;
int secRemaining = secOn;
int minOn = 0;
int minRemaining = minOn;
int hrOn = 0;
int hrRemaining = hrOn;
boolean sec = true, minutes = false, hours = false;
boolean loopflag = true;  //stops reloading pages each refresh
//int burntime = secOn + minOn*60 + hrOn*3600;
int burntimeRemaining; //recalculate down below = secOn + minOn*60 + hrOn*3600; // = burntime
boolean pause = false;
unsigned long currentMillis;
unsigned long nextMillis;

void button(int x, int y, String s)
{
  tft.fillRect(x, y, BUTTON, BUTTON, BLUE);
  tft.setCursor(x + 6, y + 6);
  tft.setTextColor(WHITE);
  tft.println(s);
}

void buttonRed(int x, int y, String s)
{
  tft.fillRect(x, y, BUTTON, BUTTON, RED);
  tft.setCursor(x + 6, y + 6);
  tft.setTextColor(WHITE);
  tft.println(s);
  delay(200);
}

void loginScreen()
{
  tft.fillScreen(BLACK);
  tft.setCursor(20, 10);
  tft.setTextColor(WHITE);
  tft.println("Enter passcode");
  button(FIRSTC, FIRSTR, "1");
  button(SECONDC, FIRSTR, "2");
  button(THIRDC, FIRSTR, "3");
  button(FIRSTC, SECONDR, "4");
  button(SECONDC, SECONDR, "5");
  button(THIRDC, SECONDR, "6");
  button(FIRSTC, THIRDR, "7");
  button(SECONDC, THIRDR, "8");
  button(THIRDC, THIRDR, "9");
  button(FIRSTC, FOURTHR, "clr");
  button(SECONDC, FOURTHR, "0");
  button(THIRDC, FOURTHR, "ent");
}

void resetGuess()
{
  for(int i = 0; i<4; i++)
  {
    guess[i] = 0;
  }
}

boolean checkGuess()
{
  int correctGuesscounter = 0;
  for (int i=0; i<4; i++)
  {
    if(myPassword[i]==guess[i])
      correctGuesscounter++;
  }
  if(correctGuesscounter==4)
    return true;
  else
    return false;
}

void wrongPassword() //wrong password entered
{
  resetGuess();
  tft.fillScreen(BLACK);
  tft.setCursor(20, 10);
  tft.setTextColor(WHITE);
  tft.println("Incorrect password");
  delay(1500);
  counter = 0;
  loginScreen();
}

void entPassword()
{
  tft.fillScreen(BLACK);
  tft.setCursor(20, 10);
  tft.setTextColor(WHITE);
  tft.println("You entered:");
  tft.setCursor(20, 40);
  for(int i = 0; i<4; i++)
  {
    tft.print(guess[i]);
    tft.print(" ");
  }
  button(FIRSTC, THIRDR, "clr");
  button(THIRDC, THIRDR, "ent");
}

void setTimeScreen()
{
  tft.fillScreen(BLACK);
  tft.setCursor(20, 10);
  tft.setTextColor(WHITE);
  tft.print(hrOn);
  tft.print(" h");
  tft.setCursor(85, 10);
  tft.print(minOn);
  tft.print(" m");
  tft.setCursor(150, 10);
  tft.print(secOn);
  tft.print(" s");
  //up
  tft.fillRect(FIRSTC, THIRDR, BUTTON, BUTTON, BLUE);
  tft.fillTriangle(50, 180, 25, 230, 75, 230, WHITE);
  //next
  tft.fillRect(SECONDC, THIRDR, BUTTON, BUTTON, BLUE);
  tft.fillTriangle(100, 180, 100, 230, 150, 205, WHITE);
  //down
  tft.fillRect(THIRDC, THIRDR, BUTTON, BUTTON, BLUE);
  tft.fillTriangle(200, 230, 175, 180, 225, 180, WHITE);
  //on
  tft.fillRect(FIRSTC, FOURTHR, BUTTON, BUTTON, BLUE);
  tft.setCursor(FIRSTC + 6, FOURTHR + 6);
  tft.println("ON");
  //back
  tft.fillRect(THIRDC, FOURTHR, BUTTON, BUTTON, BLUE);
  tft.setCursor(THIRDC + 6, FOURTHR + 6);
  tft.print("BACK");
  if(hours)
    tft.fillRect(20, 25, 25, 10, WHITE);
  if(minutes)
    tft.fillRect(85, 25, 25, 10, WHITE);
  if(sec)
    tft.fillRect(150, 25, 25, 10, WHITE);
}

void countdownScreen()
{
  //set up time & time remaining, only adjust the time remaining
  tft.fillScreen(BLACK);
  tft.setCursor(20, 10);
  tft.setTextColor(WHITE);
  tft.println("Target time");
  tft.setCursor(20, 30);
  tft.print(hrOn);
  tft.print(" h");
  tft.setCursor(85, 30);
  tft.print(minOn);
  tft.print(" m");
  tft.setCursor(150, 30);
  tft.print(secOn);
  tft.print(" s");
  tft.setCursor(20, 60);
  tft.print("Time remaining");
  tft.setCursor(20, 80);
  tft.print(hrOn);
  tft.print(" h");
  tft.setCursor(85, 80);
  tft.print(minOn);
  tft.print(" m");
  tft.setCursor(150, 80);
  tft.print(secOn);
  tft.print(" s");
  button(SECONDC, FOURTHR, "pause");
}

void finalScreen()
{
  pause = true;
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setCursor(20, 10);
  tft.print(hrOn);
  tft.print(" h");
  tft.setCursor(85, 10);
  tft.print(minOn);
  tft.print(" m");
  tft.setCursor(150, 10);
  tft.print(secOn);
  tft.print(" s");
  tft.setCursor(30, 40);
  tft.println("completed");
  button(FIRSTC, FOURTHR, "back");
  button(THIRDC, FOURTHR, "log");
  tft.setCursor(THIRDC+6, FOURTHR+20);
  tft.print("out");  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("setup");

  tft.begin();

  if (!ctp.begin(40)) {
    Serial.println("Can't start");
  }
  else {
    Serial.println("Started touchscreen");
  }
  currentMillis = millis();
  nextMillis = currentMillis;
  tft.setTextSize(2);
  loginScreen();
  pinMode(13, OUTPUT); //sets digital pin 13 as output
}

void loop() {
  TS_Point p = ctp.getPoint();
  delay(200);

  p.x = map(p.x, 0, 240, 240, 0);
  p.y = map(p.y, 0, 320, 320, 0);

  if(counter<4)
  {
    if(p.y>FIRSTR && p.y<FIRSTR+BUTTON) //first row
    {
      if(p.x>FIRSTC && p.x<FIRSTC+BUTTON) //1
      {
        buttonRed(FIRSTC, FIRSTR, "1");
        guess[counter] = 1;
        button(FIRSTC, FIRSTR, "1");
        counter++;
      }
      if(p.x>SECONDC && p.x<SECONDC+BUTTON) //2
      {
        buttonRed(SECONDC, FIRSTR, "2");
        guess[counter] = 2;
        button(SECONDC, FIRSTR, "2");
        counter++;
      }
      if(p.x>THIRDC && p.x<THIRDC+BUTTON) //3
      {
        buttonRed(THIRDC, FIRSTR, "3");
        guess[counter] = 3;
        button(THIRDC, FIRSTR, "3");
        counter++;
      }
    }
    if(p.y>SECONDR && p.y<SECONDR+BUTTON) //second row
    {
      if(p.x>FIRSTC && p.x<FIRSTC+BUTTON) //4
      {
        buttonRed(FIRSTC, SECONDR, "4");
        guess[counter] = 4;
        button(FIRSTC, SECONDR, "4");
        counter++;
      }
      if(p.x>SECONDC && p.x<SECONDC+BUTTON) //5
      {
        buttonRed(SECONDC, SECONDR, "5");
        guess[counter] = 5;
        button(SECONDC, SECONDR, "5");
        counter++;
      }
      if(p.x>THIRDC && p.x<THIRDC+BUTTON) //6
      {
        buttonRed(THIRDC, SECONDR, "6");
        guess[counter] = 6;
        button(THIRDC, SECONDR, "6");
        counter++;
      }      
    }
    if(p.y>THIRDR && p.y<THIRDR+BUTTON) //third row
    {
      if(p.x>FIRSTC && p.x<FIRSTC+BUTTON) //7
      {
        buttonRed(FIRSTC, THIRDR, "7");
        guess[counter] = 7;
        button(THIRDC, SECONDR, "7");
        counter++;
      }
      if(p.x>SECONDC && p.x<SECONDC+BUTTON) //8
      {
        buttonRed(SECONDC, THIRDR, "8");
        guess[counter] = 8;
        button(SECONDC, THIRDR, "8");
        counter++;
      }
      if(p.x>THIRDC && p.x<THIRDC+BUTTON) //9
      {
        buttonRed(THIRDC, THIRDR, "9");
        guess[counter] = 9;
        button(THIRDC, THIRDR, "9");
        counter++;
      }      
    }
    if(p.y>FOURTHR && p.y<FOURTHR+BUTTON) //fourth row
    {
      if(p.x>FIRSTC && p.x<FIRSTC+BUTTON) //clr
      {
        buttonRed(FIRSTC, FOURTHR, "clr");
        resetGuess();
        button(FIRSTC, FOURTHR, "clr");
        counter = 0;
      }
      if(p.x>SECONDC && p.x<SECONDC+BUTTON) //0
      {
        buttonRed(SECONDC, FOURTHR, "0");
        guess[counter] = 0;
        button(SECONDC, FOURTHR, "0");
        counter++;
      }
      if(p.x>THIRDC && p.x<THIRDC+BUTTON) //ent
      {
        loopflag = true;
        counter = 4;
      }      
    }
  }

  if(counter == 4) //ent password page
  {
    if (loopflag)
    {
      entPassword();
      loopflag = false;
    }
    if(p.y>THIRDR && p.y<THIRDR+BUTTON) //third row, clr and ent
    {
      if(p.x>FIRSTC && p.x<FIRSTC+BUTTON) //clr
      {
        resetGuess();
        loopflag = true;
        loginScreen();
        counter = 0;
      }
      if(p.x>THIRDC && p.x<THIRDC+BUTTON)  //ent
      {
        loopflag = true;
        if(checkGuess())
        {
          counter = 5;
        }
        else //wrong password
        {
          wrongPassword();
          counter = 0;
        }
      }
    }
  }

  if (counter == 5) //timer set page
  {
    if (loopflag)
    {
      setTimeScreen();
      loopflag = false;
    }
    if (p.y > THIRDR && p.y < THIRDR + BUTTON)
    {
      if (p.x > FIRSTC && p.x < FIRSTC + BUTTON) //up
      {
        if (hours)
        {
          tft.setCursor(20, 10);
          tft.setTextColor(BLACK);
          tft.println(hrOn);
          hrOn++;
          tft.setTextColor(WHITE);
          tft.setCursor(20, 10);
          tft.print(hrOn);
        }//hours
        else if (minutes)
        {
          if (minOn < 59)
          {
            tft.setCursor(85, 10);
            tft.setTextColor(BLACK);
            tft.print(minOn);
            minOn++;
            tft.setTextColor(WHITE);
            tft.setCursor(85, 10);
            tft.print(minOn);
          }
          else //minOn = 59, add an hour
          {
            tft.setCursor(20, 10);
            tft.setTextColor(BLACK);
            tft.print(hrOn);
            tft.setCursor(85, 10);
            tft.print(minOn);
            hrOn++;
            minOn = 0;
            tft.setTextColor(WHITE);
            tft.setCursor(20, 10);
            tft.print(hrOn);
            tft.setCursor(85, 10);
            tft.print(minOn);
          }
        }//minutes
        else if (sec)
        {
          if (secOn < 59)
          {
            tft.setCursor(150, 10);
            tft.setTextColor(BLACK);
            tft.print(secOn);
            secOn++;
            tft.setCursor(150, 10);
            tft.setTextColor(WHITE);
            tft.print(secOn);
          }
          else //secOn = 59
          {
            if (minOn < 59)
            {
              tft.setTextColor(BLACK);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
              minOn++;
              secOn = 0;
              tft.setTextColor(WHITE);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
            }
            else //minOn = 59, secOn = 59
            {
              tft.setTextColor(BLACK);
              tft.setCursor(20, 10);
              tft.print(hrOn);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
              hrOn++;
              minOn = 0;
              secOn = 0;
              tft.setTextColor(WHITE);
              tft.setCursor(20, 10);
              tft.print(hrOn);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
            }
          }
        }//seconds
      }
      if (p.x > SECONDC && p.x < SECONDC + BUTTON) //next
      {
        if (sec)
        {
          hours = true;
          sec = false;
          Serial.println("Hours");
          tft.fillRect(20, 25, 25, 10, WHITE); //hours
          tft.fillRect(150, 25, 25, 10, BLACK); //seconds
          //black box beneath seconds
          //white box between hours
        }
        else if (hours)
        {
          minutes = true;
          hours = false;
          Serial.println("minutes");
          tft.fillRect(20, 25, 25, 10, BLACK); // hours
          tft.fillRect(85, 25, 25, 10, WHITE); //minutes
          //black box beneath hours
          //white box beneath minutes
        }
        else if (minutes)
        {
          sec = true;
          minutes = false;
          Serial.println("Seconds");
          tft.fillRect(85, 25, 25, 10, BLACK); //minutes
          tft.fillRect(150, 25, 25, 10, WHITE); //seconds
          //black box beneath minutes
          //white box beneath seconds 
        }
      }
      if (p.x > THIRDC && p.x < THIRDC + BUTTON) //down
      {
        if(sec)
        {
          if(secOn>0)
          {
            tft.setCursor(150, 10);
            tft.setTextColor(BLACK);
            tft.print(secOn);
            secOn--;
            tft.setCursor(150, 10);
            tft.setTextColor(WHITE);
            tft.print(secOn);
          }
          else if(secOn == 0)
          {
            if(minOn>0)
            {
              tft.setCursor(85, 10);
              tft.setTextColor(BLACK);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
              minOn--;
              secOn = 59;
              tft.setTextColor(WHITE);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
            }
            else if(hrOn>0) //minOn = 0, secOn = 0
            {
              tft.setTextColor(BLACK);
              tft.setCursor(20, 10);
              tft.print(hrOn);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
              hrOn--;
              minOn = 59;
              secOn = 59;
              tft.setTextColor(WHITE);
              tft.setCursor(20, 10);
              tft.print(hrOn);
              tft.setCursor(85, 10);
              tft.print(minOn);
              tft.setCursor(150, 10);
              tft.print(secOn);
            }
            else
            {
              tft.setCursor(30, 40);
              tft.setTextColor(WHITE);
              tft.print("Time at 0");
              delay(1500);
              tft.setTextColor(BLACK);
              tft.setCursor(30, 40);
              tft.print("Time at 0");
            }
          }
        }
        if(minutes)
        {
          if(minOn>0)
          {
            tft.setTextColor(BLACK);
            tft.setCursor(85, 10);
            tft.print(minOn);
            minOn--;
            tft.setTextColor(WHITE);
            tft.setCursor(85, 10);
            tft.print(minOn);
          }
          else if(hrOn>0)
          {
            tft.setTextColor(BLACK);
            tft.setCursor(20, 10);
            tft.print(hrOn);
            tft.setCursor(85, 10);
            tft.print(minOn);
            hrOn--;
            minOn = 59;
            tft.setTextColor(WHITE);
            tft.setCursor(20, 10);
            tft.print(hrOn);
            tft.setCursor(85, 10);
            tft.print(minOn);
          }
          else //hours and minutes at 0
          {
            tft.setCursor(30, 40);
            tft.setTextColor(WHITE);
            tft.print("Can't be negative");
            delay(1500);
            tft.setTextColor(BLACK);
            tft.setCursor(30, 40);
            tft.print("Can't be negative");
          }
        }
        if(hours)
        {
          if(hrOn>0)
          {
            tft.setTextColor(BLACK);
            tft.setCursor(20, 10);
            tft.print(hrOn);
            hrOn--;
            tft.setTextColor(WHITE);
            tft.setCursor(20, 10);
            tft.print(hrOn);
          }
          else //hrOn = 0
          {
            tft.setCursor(30, 40);
            tft.setTextColor(WHITE);
            tft.print("Can't be negative");
            delay(1500);
            tft.setTextColor(BLACK);
            tft.setCursor(30, 40);
            tft.print("Can't be negative");
          }
        }
      }
    } //up, next, down
    if (p.y > FOURTHR && p.y < FOURTHR + BUTTON) //on or back
    {
      if (p.x > FIRSTC && p.x < FIRSTC + BUTTON) //on
      {
        burntimeRemaining = secOn + minOn * 60 + hrOn * 3600;
        secRemaining = secOn;
        minRemaining = minOn;
        hrRemaining = hrOn;
        //flash red briefly
        delay(200);
        counter++;
        loopflag = true;
      }
      if (p.x > THIRDC && p.x < THIRDC + BUTTON) //back
      {
        buttonRed(THIRDC, FOURTHR, "back");
        resetGuess();
        delay(200);
        loginScreen();
        loopflag = true;
        counter = 0;
      }
    }//buttons complete
  }

  if (counter == 6)
  {
    
    if (loopflag)
    {
      countdownScreen();
      currentMillis = millis();
      nextMillis = currentMillis;
      loopflag = false;
      //digitalWrite(13, HIGH); //turn output on
    }
    if (p.y > FOURTHR && p.y < FOURTHR + BUTTON)
    {
      if (p.x > SECONDC && p.x < SECONDC + BUTTON)
      {
        Serial.println("Paused");
        pause = !pause;
        if (pause)
        {
          //digitalWrite(13, LOW); //turn output off
          tft.setCursor(30, 40);
          tft.setTextColor(WHITE);
          tft.println("paused");
          tft.setCursor(SECONDC + 6, FOURTHR + 6);
          tft.setTextColor(BLACK);
          tft.println("pause");
          buttonRed(SECONDC, FOURTHR, "resume");
        }
        else
        {
          //digitalWrite(13, HIGH); //turn output on
          tft.setCursor(30, 40);
          tft.setTextColor(BLACK);
          tft.println("paused");
          tft.setCursor(SECONDC + 6, FOURTHR + 6);
          tft.println("resume");
          button(SECONDC, FOURTHR, "pause");
        }
      }
    }
    nextMillis = millis();
    if (currentMillis + 1000 <= nextMillis && !pause)
    {
      if (secRemaining > 0)
      {
        tft.setCursor(150, 80);
        tft.setTextColor(BLACK);
        tft.print(secRemaining);
        secRemaining--;
        tft.setCursor(150, 80);
        tft.setTextColor(WHITE);
        tft.print(secRemaining);
      }
      else //secRemaining = 0
      {
        if (minRemaining > 0)
        {
          tft.setCursor(150, 80);
          tft.setTextColor(BLACK);
          tft.println("0");
          tft.setCursor(85, 80);
          tft.print(minRemaining);
          secRemaining = 59;
          tft.setCursor(150, 80);
          tft.setTextColor(WHITE);
          //tft.setTextSize(2);
          tft.println(secRemaining);
          tft.setCursor(85, 80);
          minRemaining--;
          tft.print(minRemaining);
        }
        else if (hrRemaining > 0)
        {
          tft.setTextColor(BLACK);
          tft.setCursor(20, 80);
          tft.print(hrRemaining);
          tft.setCursor(85, 80);
          tft.print(minRemaining);
          tft.setCursor(150, 80);
          tft.print(minRemaining);
          minRemaining = 59;
          secRemaining = 59;
          hrRemaining--;
          tft.setTextColor(WHITE);
          tft.setCursor(20, 80);
          tft.print(hrRemaining);
          tft.setCursor(85, 80);
          tft.print(minRemaining);
          tft.setCursor(150, 80);
          tft.print(secRemaining);
        }
      }
      currentMillis = millis();
      nextMillis = currentMillis;
      burntimeRemaining--;
      if(burntimeRemaining == 0)
        loopflag = true;
    }
    if (burntimeRemaining == 0)
    {
      if(loopflag)
      {
        finalScreen();
        loopflag = false;
      }
      if(p.y>FOURTHR && p.y<FOURTHR+BUTTON)
      {
        if(p.x>FIRSTC && p.x<FIRSTC+BUTTON) //back to time screen
        {
          loopflag = true;
          pause = false;
          counter = 5;
          hrOn = minOn = secOn = 0;
        }
        if(p.x>THIRDC && p.x<THIRDC+BUTTON) //log out, back to pin screen
        {
          resetGuess();
          loopflag = true;
          pause = false;
          counter = 0;
          loginScreen();
          hrOn = minOn = secOn = 0;
        }
      }
    }
  }
}
