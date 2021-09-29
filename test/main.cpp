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

#define CALIBRATION_FILE "/TouchCalData3"
#define REPEAT_CAL false
#define X_CENTER 0 
#define Y_CENTER 0

TFT_eSPI tft = TFT_eSPI(); // Invoke library
TFT_eSprite DACC = TFT_eSprite(&tft);

void ansiTrans(String komenda);
void touch_calibrate();
void drawSdJpeg(const char *filename, int xpos, int ypos);
void jpegRender(int xpos, int ypos);
void showTime(uint32_t msTime);

void setup(void)
{

  delay(1);
  tft.init();
  tft.setRotation(3);
  Serial.begin(115200);
  tft.fillScreen(TFT_BLACK);
  touch_calibrate();
  tft.setCursor(0, 0, 1);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(5, HIGH);
  if (!SD.begin())
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println("initialisation done.");

  DACC.setColorDepth(16);
  DACC.createSprite(80,64);
  
}

void loop()
{
  String litera;
  char litera2;
  /*
  uint16_t x, y;
  if (tft.getTouch(&x, &y))
  {
    tft.setCursor(x, y);
  }
  */

  tft.drawRect(tft.getCursorX(), tft.getCursorY(), 2, 8, TFT_WHITE); // cursor
  delay(5);
  tft.drawRect(tft.getCursorX(), tft.getCursorY(), 2, 8, TFT_BLACK);

  if (Serial.available() > 0) //odczytywanie z Serialia i pisaniea ekranie
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

        tft.fillRect(tft.getCursorX() - 6, tft.getCursorY() - 8, 12, 8, TFT_BLACK);
        tft.setCursor(TFT_HEIGHT - 6, tft.getCursorY() - 8, 1);
        if (tft.getCursorY() < 0)
        {
          tft.setCursor(0, 0, 1);
        }
      }
      break;
    case 13: //Enter
      Serial.println();
      tft.println();
      break;
    case 92:
      ciag = Serial.readString();
      ansiTrans(ciag);
      break;
    case 91:
      ciag = Serial.readString();

      //tft.println(ciag);
      ansiTrans(ciag);
      break;
    default:

      tft.print(litera2);
      break;
    }
  }
  delay(5000);
  tft.setRotation(2);
  drawSdJpeg("/DAC.jpg", X_CENTER, Y_CENTER);
  delay(5000);
  tft.setRotation(3);
}

void touch_calibrate() // kalibracja touch screeena
{

  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin())
  {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE))
  {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f)
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL)
  {
    // calibration data valid
    tft.setTouch(calData);
  }
  else
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL)
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
void ansiTrans(String komenda) // dekodowanie komend ANSII
{
  uint32_t n = komenda.length();
  char tab[n + 1];
  strcpy(tab, komenda.c_str());
  // if (tab[0] == 'u' && tab[1] == '0' && tab[2] == '0' && tab[3] == '1' && tab[4] == 'b' && tab[5] == '[')

  if (tab[2] == 'u' && tab[3] == '0' && tab[4] == '0' && tab[5] == '1' && tab[6] == 'b' && tab[7] == '[')
  {
    uint16_t poz = 7;
    uint8_t koniec = 0;
    do
    {
      poz++;
      int spr = (int)tab[poz];
      if (spr >= 65 && spr <= 75)
      {
        koniec = 1;
      }
      if (spr == 109)
      {
        koniec = 1;
      }

    } while (koniec != 1 && poz <= 33);

    if (koniec == 1)
    {
      komenda = "";

      for (uint8_t i = 8; i < poz; i++)
      {
        komenda += tab[i];
      }

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
        if (tft.getCursorY() > TFT_WIDTH)
        {
          tft.setCursor(tft.getCursorX(), TFT_WIDTH - 8);
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
        if (tft.getCursorX() > TFT_HEIGHT)
        {
          tft.setCursor(TFT_HEIGHT - 6, tft.getCursorY());
        }
        break;
      case 'E':
        tft.setCursor(0, tft.getCursorY() + 8 * n);
        if (tft.getCursorY() > TFT_WIDTH)
        {
          tft.setCursor(0, TFT_WIDTH - 8);
        }
        break;
      case 'F':
        tft.setCursor(0, tft.getCursorY() - 8 * n);
        if (tft.getCursorY() > TFT_WIDTH)
        {
          tft.setCursor(0, 0);
        }
        break;
      case 'G':
        tft.setCursor(7 * n, tft.getCursorY());
        if (tft.getCursorY() > TFT_WIDTH)
        {
          tft.setCursor(TFT_HEIGHT - 6, tft.getCursorY());
        }
        break;
      case 'H':
        /* code */
        break;
      case 'J':
        if (n == 0)
        {
          tft.fillRect(tft.getCursorX(), tft.getCursorY(), TFT_HEIGHT - tft.getCursorX(), 8, TFT_BLACK);
          tft.fillRect(0, tft.getCursorY() + 8, TFT_HEIGHT, TFT_WIDTH - tft.getCursorY(), TFT_BLACK);
        }
        else if (n == 1)
        {
          tft.fillRect(0, tft.getCursorY(), tft.getCursorX(), 8, TFT_BLACK);
          tft.fillRect(0, 0, TFT_HEIGHT, tft.getCursorY(), TFT_BLACK);
        }
        else
        {
          tft.fillScreen(TFT_BLACK);
        }

        break;
      case 'K':
        if (n == 0)
        {
          tft.fillRect(tft.getCursorX(), tft.getCursorY(), TFT_HEIGHT - tft.getCursorX(), 8, TFT_BLACK);
        }
        else if (n == 1)
        {
          tft.fillRect(0, tft.getCursorY(), tft.getCursorX(), 8, TFT_BLACK);
        }
        else
        {
          tft.fillRect(tft.getCursorX(), tft.getCursorY(), TFT_HEIGHT, 8, TFT_BLACK);
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
  }
  else
  {
    switch (tab[0])
    {
      tft.println(tab[0]);
    case 'A':
      tft.setCursor(tft.getCursorX(), tft.getCursorY() - 8);
      break;
    case 'B':
      tft.setCursor(tft.getCursorX(), tft.getCursorY() + 8);
      break;
    case 'C':
      tft.setCursor(tft.getCursorX() + 6, tft.getCursorY());
      break;
    case 'D':
      tft.setCursor(tft.getCursorX() - 6, tft.getCursorY());
      break;

    default:
      break;
    }
  }
}

void drawSdJpeg(const char *filename, int xpos, int ypos)
{

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open(filename, FILE_READ); // or, file handle reference for SD library

  if (!jpegFile)
  {
    Serial.print("ERROR: File \"");
    Serial.print(filename);
    Serial.println("\" not found!");
    return;
  }

  Serial.println("===========================");
  Serial.print("Drawing file: ");
  Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded = JpegDec.decodeSdFile(jpegFile); // Pass the SD file handle to the decoder,
  //bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded)
  {
    // print information about the image to the serial port
   
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else
  {
    Serial.println("Jpeg file format not supported!");
  }
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos)
{

  //jpegInfo(); // Print information from the JPEG file (could comment this line out)

  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read())
  {                        // While there is more data in the file
    pImg = JpegDec.pImage; // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x)
      win_w = mcu_w;
    else
      win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y)
      win_h = mcu_h;
    else
      win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
   // uint32_t mcu_pixels = win_w * win_h;

    // draw image MCU block only if it will fit on the screen
    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
    //  DACC.pushImage(mcu_x, mcu_y, win_w, win_h, (uint16_t *)pImg);
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }
  /*
  DACC.pushSprite(0,0);
  DACC.pushRotated(45);
  delay(100);
  DACC.pushRotated(90);
  delay(100);
  DACC.pushRotated(180);
*/
  tft.setSwapBytes(swapBytes);

  showTime(millis() - drawTime); // These lines are for sketch testing only
}

void showTime(uint32_t msTime)
{
  //tft.setCursor(0, 0);
  //tft.setTextFont(1);
  //tft.setTextSize(2);
  //tft.setTextColor(TFT_WHITE, TFT_BLACK);
  //tft.print(F(" JPEG drawn in "));
  //tft.print(msTime);
  //tft.println(F(" ms "));
  Serial.print(F(" JPEG drawn in "));
  Serial.print(msTime);
  Serial.println(F(" ms "));
}
