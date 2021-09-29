/*  
 Test the tft.print() viz embedded tft.write() function

 This sketch used font 2, 4, 7

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 */
#include "SD.h"
#include "JPEGDecoder.h"
#include "FS.h"
#include "FS.h"
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include <sstream>

#define TFT_GREY 0x5AEB // New colour
#define CALIBRATION_FILE "/calibrationData"

TFT_eSPI tft = TFT_eSPI(); // Invoke library

void ansiTrans(String komenda);

void setup(void)
{

  tft.init();
  tft.setRotation(3);
  Serial.begin(115200);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 1);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
}

void loop()
{
  String litera;
  char litera2;
  // Fill screen with grey so we can see the effect of printing with and without
  // a background colour defined

  //tft.fillScreen(i);

  //Serial.println(i);
  //Serial.println("\u001b[31mHelloWorld");
  //tft.println("\u001b[31mHelloWorld");
  if (Serial.available() > 0)
  {
    //litera = Serial.readString();
    litera2 = Serial.read();

    Serial.print(litera2);
    //tft.print(litera2,HEX);
    int a = (int)litera2;
    String ciag;

    switch (a)
    {
    case 127: // Backspace
      tft.fillRect(tft.getCursorX() - 6, tft.getCursorY(), 6, 8, TFT_BLACK);
      tft.setCursor(tft.getCursorX() - 6, tft.getCursorY(), 1);
      if (tft.getCursorX() < 0)
      {
        tft.setCursor(0, tft.getCursorY(), 1);
      }
      break;
    case 13: //Enter
      Serial.println();
      tft.println();
      break;
    case 92:
      ciag = Serial.readString();
      Serial.print(ciag);
      ansiTrans(ciag);
      // Serial.print("mamy to ");

      break;
    default:
     
      tft.print(litera2);
     break;
    }
  }
}

void ansiTrans(String komenda)
{
  uint32_t n = komenda.length();
  char tab[n + 1];
  strcpy(tab, komenda.c_str());
 // if (tab[0] == 'u' && tab[1] == '0' && tab[2] == '0' && tab[3] == '1' && tab[4] == 'b' && tab[5] == '[')

  if (tab[2] == 'u' && tab[3] == '0' && tab[4] == '0' && tab[5] == '1' && tab[6] == 'b' && tab[7] == '[')
  {
    uint16_t poz = 7;
    bool koniec = false;
    do
    {
      poz++;
      int spr = (int)tab[poz];
      if (spr >= 65 && spr <= 75)
      {
        koniec = true;
      }
      else if (spr == 109)
      {
        koniec = true;
      }

    } while (koniec == true || poz >= 33);

    if (koniec == true)
    {
      komenda = "";
    Serial.print(komenda);
      for (uint8_t i = 8; i < poz; i++)
      {
        komenda += tab[i];
      }
Serial.print(komenda);
      n = komenda.toInt();
      switch (tab[poz])
      {
      case 'A':
        tft.setCursor(tft.getCursorX(), tft.getCursorY() - 8 * n);
        if (tft.getCursorY() < 0)
        {
          tft.setCursor(tft.getCursorX(), 0);
        }
        break;
      case 'B':
        tft.setCursor(tft.getCursorX(), tft.getCursorY() + 8 * n);
        if (tft.getCursorY() > TFT_HEIGHT)
        {
          tft.setCursor(tft.getCursorX(), TFT_HEIGHT - 8);
        }
        break;
      case 'C':
        tft.setCursor(tft.getCursorX() + 6 * n, tft.getCursorY());
        if (tft.getCursorX() < 0)
        {
          tft.setCursor(0, tft.getCursorY());
        }
        break;
      case 'D':
        tft.setCursor(tft.getCursorX() - 6 * n, tft.getCursorY());
        if (tft.getCursorX() > TFT_WIDTH)
        {
          tft.setCursor(TFT_WIDTH - 6, tft.getCursorY());
        }
        break;
      case 'E':
        tft.setCursor(0, tft.getCursorY()+8*n);
        if (tft.getCursorY() > TFT_HEIGHT)
        {
          tft.setCursor(0, TFT_HEIGHT - 8);
        }
        break;
      case 'F':
        tft.setCursor(0, tft.getCursorY()-8*n);
        if (tft.getCursorY() > TFT_HEIGHT)
        {
          tft.setCursor(0, 0);
        }
        break;
      case 'G':
        tft.setCursor(7*n, tft.getCursorY());
        if (tft.getCursorY() > TFT_HEIGHT)
        {
          tft.setCursor(TFT_WIDTH-6, tft.getCursorY());
        }
        break;
      case 'H':
        /* code */
        break;
      case 'J':
        if(n==0)
        {
        tft.fillRect(tft.getCursorX(),tft.getCursorY(),TFT_WIDTH-tft.getCursorX(),8,TFT_BLACK);
        tft.fillRect(0,tft.getCursorY()+8,TFT_WIDTH,TFT_HEIGHT-tft.getCursorY(),TFT_BLACK);
        }
        else if(n==1)
        {
        tft.fillRect(0,tft.getCursorY(),tft.getCursorX(),8,TFT_BLACK);
        tft.fillRect(0,0,TFT_WIDTH,tft.getCursorY(),TFT_BLACK);
        }
        else
        {
          tft.fillScreen(TFT_BLACK);
        }
        
        break;
      case 'K':
        if(n==0)
        {
            tft.fillRect(tft.getCursorX(),tft.getCursorY(),TFT_WIDTH-tft.getCursorX(),8,TFT_BLACK);
        }
        else if(n==1)
        {
          tft.fillRect(0,tft.getCursorY(),tft.getCursorX(),8,TFT_BLACK);
        }
        else
        {
          tft.fillRect(tft.getCursorX(),tft.getCursorY(),TFT_WIDTH,8,TFT_BLACK);
        }
        break;
      case 'm':
        switch (n)
        {
        case 30:
          tft.setTextColor(TFT_BLACK, TFT_BLACK);
          break;
        case 31:
          tft.setTextColor(TFT_RED, TFT_BLACK);
          break;
        case 32:
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          break;
        case 33:
          tft.setTextColor(TFT_YELLOW, TFT_BLACK);
          break;
        case 34:
          tft.setTextColor(TFT_BLUE, TFT_BLACK);
          break;
          case 35:
          tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
          break;
          case 36:
          tft.setTextColor(TFT_CYAN, TFT_BLACK);
          break;
          case 37:
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
          break;
          case 0:
         tft.setTextColor(TFT_WHITE, TFT_BLACK);
          break;
        
        default:
          break;
        }
        break;

      default:
        break;
      }
    }
    else
    {
    }
  }
  else
  {
    tft.print("nie to samo");
  }
}
