#include <TFT_eSPI.h>
#include <SPI.h>

/*
#########################################################################################################################
##################################################<ALEKSKOD>#############################################################
#########################################################################################################################
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>


const char* host = "esp32";
//const char* ssid = "WiFi2";
//const char* password = "@GrUnWaLdZkA@";
const char* ssid = "MiSiec";
const char* password = "12345678@";
//WebServer server(80);
AsyncWebServer server(80);
/* Style */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex = 
"<form name=loginForm>"
"<h1>ESP32 Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='shox')"
"{window.open('/update')}"
"else"
"{alert('Error Password or Username')}"
"}"
"</script>" + style;
 
/* Server Index Page */
String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

/*
#########################################################################################################################
##################################################</ALEKSKOD>############################################################
#########################################################################################################################
*/

TFT_eSPI tft = TFT_eSPI();

//###Zmienne
unsigned long czas;                     // Czas podczas ostatniego sprawdzania. Używane do wyświetlania kursora
const uint8_t fosi = 2;                 // Wielkość czcionki
uint8_t msz = 6;                        // Mnożnik szerokości znaku (podstawowe wymiary znaku)
uint8_t mwz = 8;                        // Mnożnik wysokości znaku
uint8_t sz = fosi * msz;                // Szerokość jednego znaku w px
uint8_t wz = fosi * mwz;                // Wysokość jednego znaku w px
const uint16_t maxznakow = 1600/fosi;   // STAŁA - Praktycznie wielkość tablicy, w której przechowywane są dane
uint16_t szerek = TFT_HEIGHT;           // Szerokość ekranu w px | Wartosci są odwrócone, bo z jakiegoś powodu ekran domyślnie jest w trybire portretowym
uint16_t wysek = TFT_WIDTH;             // Wysokość ekranu w px | Wartosci są odwrócone, bo z jakiegoś powodu ekran domyślnie jest w trybire portretowym
uint8_t czcionka = 1;                   // Czcionka. Wybiera się rodzaj zmieniając jej numer. Inne wyglądają okropnie
uint16_t domyslnykolor = 07;
uint16_t ostatnikolor = domyslnykolor;
uint16_t domyslnykolorfn = domyslnykolor % 10;
uint16_t domyslnykolorbg = ((domyslnykolor % 100) - domyslnykolorfn)/10;
boolean DEBUGMODE = true;

//###Tablice
uint8_t KOLOR[maxznakow];       // Tablica instrukcji do znaków
char ZNAKI[maxznakow];          // Tablica wszystkich znaków na ekranie
bool WYSWIETLONE[maxznakow];    // czy wyswietlone

//###Funkcje
void processSerial();                                                                   // Wstępne przetwarzania inputu z seriala
uint16_t calcXYtoNR(uint16_t x, uint16_t y);                                            // Zamiana pozycji na ekranie na numer znaku
void calcNRtoXY(int16_t numerznaku, int16_t *zwracanyx, int16_t *zwracanyy);            // Zamiana numeru znaku na jego pozycje na ekranie
void dodajZnak(char x, uint16_t poz);                                                   // Dodawanie znaków do pamięci
void wyswietlZnaki();                                                                   // Wypisywanie znaków z pamięci na ekranie
void kursorBlink();                                                                     // Obsługa wizualnej reprezentacji kursora
void refresh();                                                                         // DEBUG - Wypisanie wszystkiego na ekranie od nowa
void dumpChars();                                                                       // DEBUG - Wypisanie wszystkich znaków z pamięci na serial
void dumpCols();
void pushCur();                                                                         // Przesunięcie kursora po wypisaniu znaku
void backspace();                                                                       // Usuwanie znaków backspace'em
void strzalka(char DIR, uint16_t ile);                                                  // Obsługa klawiszy strzałek (A - góra, B - dół, C - prawo, D - lewo)
void znakiSpecialne();                                                                  // Obsługa znaków specialnych 
void dodajKolor(uint16_t a);
void kolor(uint16_t colorData);
void irTab();
uint16_t colorMan(uint8_t x);
uint8_t reverseColorMan(uint16_t x);
void decodeColor(uint16_t x);
uint8_t calcDATAtoFG(uint16_t colorData);
uint8_t calcDATAtoBG(uint16_t colorData);
uint16_t calcColorAddToData(uint8_t colorToAdd, uint8_t colorPos, uint16_t dataNow);
void clearChars(uint16_t startCh, uint16_t endCh);
void processZnak(char znak, uint16_t xyz);
void carriageReturn();

void setup(){
    Serial.begin(115200);
    tft.init();
    tft.setRotation(3);
    tft.setCursor(0,0,czcionka);
    tft.setTextSize(fosi);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    czas = millis();
    irTab();
}

void loop(){
    kursorBlink();
    processSerial();
    wyswietlZnaki();
}

void processSerial(){
    if(Serial.available() > 0){
        char znak = Serial.read();
        uint16_t xyz = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
        switch((int)znak){
            case 13: //Enter/CR
                if(DEBUGMODE == true)Serial.print("CR;");
                carriageReturn();
                break;
            case 17:
                break;
            case 19:
                break;
            case 27:
                if(DEBUGMODE == true)Serial.print("Znak specialny - ");
                znakiSpecialne();
                break;
            case 255:
                //ERROR (Ten case nie powinien się zdarzyć)
                break;
            case 64: // '@' - odświerza ekran
                if(DEBUGMODE == true)refresh();
                else processZnak(znak, xyz);
                break;
            case 35: // '#' - wypisuje wszystkie znaki z tablicy na serial
                if(DEBUGMODE == true)dumpChars();
                else processZnak(znak, xyz);
                break;
            case 36: // '$' - resetuje tablice/pamięć
                if(DEBUGMODE == true)irTab();
                else processZnak(znak, xyz);
                break;
            case 37: // '%' - dump kolorów na serial
                if(DEBUGMODE == true)dumpCols();
                else processZnak(znak, xyz);
                break;
            case 127: //Backspace
                backspace();
                break;
            default:
                processZnak(znak, xyz);
                break;
        }
    }
}

void processZnak(char znak, uint16_t xyz){
    dodajZnak(znak, xyz);
    pushCur();
    int16_t x, y;
    calcNRtoXY(xyz, &x, &y);
    if(DEBUGMODE == true)Serial.print("| ");
    if(DEBUGMODE == true)Serial.print(znak);
    if(DEBUGMODE == true)Serial.print(" ");
    if(DEBUGMODE == true)Serial.print(x); 
    if(DEBUGMODE == true)Serial.print(" ");
    if(DEBUGMODE == true)Serial.print(y);
    if(DEBUGMODE == true)Serial.print(" | ");
}

bool kursorBW;
uint16_t pamiecKursoraX, pamiecKursoraY;
void kursorBlink(){
    if(pamiecKursoraX != tft.getCursorX() || pamiecKursoraY != tft.getCursorY())czas = 0;
    if((czas + 500) < millis()){
        if(kursorBW == true){
            pamiecKursoraX = tft.getCursorX(); pamiecKursoraY = tft.getCursorY();
            if(pamiecKursoraX>szerek-sz)tft.drawRect(pamiecKursoraX-3, pamiecKursoraY, 2, wz, TFT_WHITE);
            else tft.drawRect(pamiecKursoraX, pamiecKursoraY, 2, wz, TFT_WHITE);
            kursorBW = false;
        }
        else {
            if(tft.getCursorX()>szerek-sz)tft.drawRect(tft.getCursorX()-3, tft.getCursorY(), 2, wz, TFT_BLACK);
            else tft.drawRect(tft.getCursorX(), tft.getCursorY(), 2, wz, TFT_BLACK);
            if(pamiecKursoraX>szerek-sz)tft.drawRect(pamiecKursoraX-3, pamiecKursoraY, 2, wz, TFT_BLACK);
            else tft.drawRect(pamiecKursoraX, pamiecKursoraY, 2, wz, TFT_BLACK);
            WYSWIETLONE[calcXYtoNR(tft.getCursorX(),tft.getCursorY())] = false;
            WYSWIETLONE[calcXYtoNR(pamiecKursoraX,pamiecKursoraY)] = false;
            kursorBW = true;
        }
        czas = millis();
    }
}

uint16_t calcXYtoNR(uint16_t x, uint16_t y){
    if(x>=szerek)x=0;
    if(x<0)x=szerek;
    if(y>=wysek)y=0;
    if(y<0)y=wysek;
    uint16_t xznakowy = x / sz;
    uint16_t yznakowy = y / wz;
    uint16_t ileznakowwlinijce = szerek / sz;

    return (yznakowy * ileznakowwlinijce) + xznakowy;
}

void calcNRtoXY(int16_t numerznaku, int16_t *zwracanyx, int16_t *zwracanyy){
    if(numerznaku >= maxznakow)numerznaku = 0;
    if(numerznaku < 0)numerznaku = (maxznakow-1);
    uint16_t linijka = (numerznaku/(szerek / sz));
    *zwracanyx = sz * (numerznaku % (szerek/sz));
    *zwracanyy = linijka * wz;
}

void wyswietlZnaki(){
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    uint16_t starex = tft.getCursorX();
    uint16_t starey = tft.getCursorY();
    tft.setCursor(0,0,czcionka);
    for (uint16_t i = 0; i < maxznakow; i++){
        if(KOLOR[i] != domyslnykolor)kolor(KOLOR[i]);
        if(!WYSWIETLONE[i]){
            int16_t x, y;
            calcNRtoXY(i, &x, &y);
            tft.setCursor(x,y,czcionka);
            if(ZNAKI[i] != 0){
                tft.print(ZNAKI[i]);
            }
            WYSWIETLONE[i] = true;
        }
    tft.setCursor(starex,starey,czcionka);
    }
}

void dodajZnak(char x, uint16_t poz){
    ZNAKI[poz] = x;
    WYSWIETLONE[poz] = false;
}


void refresh(){
  tft.fillScreen(TFT_BLACK);
  for (uint16_t i = 0; i < maxznakow; i++){
    WYSWIETLONE[i] = false;
  }
  if(DEBUGMODE == true)Serial.println("\nREFRESH!");
}

void dumpChars(){
    Serial.println(); Serial.println("===============================[Dane w tablicy znaków]==========================");
    uint16_t pozkur = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
    for (uint16_t i = 0; i < maxznakow; i++){
        if(i == pozkur){
            Serial.print("\u001b[1m\u001b[45m"); Serial.print(ZNAKI[i]); Serial.print(i); Serial.print("\u001b[0m"); Serial.print(" ");
        }
        else {
            Serial.print(ZNAKI[i]); Serial.print(i); Serial.print(" ");
        }
    }
    Serial.println();
}

void dumpCols(){
    uint16_t laval;
    Serial.println(); Serial.println("===============================[COLOR DEBUG]==========================");
    Serial.print("Default color: "); Serial.print(domyslnykolor); Serial.print(" - | FG:"); Serial.print(domyslnykolorfn); Serial.print(" BG:"); Serial.print(domyslnykolorbg); Serial.println("| ");
    for (uint16_t i = 0; i < maxznakow; i++){
        if(KOLOR[i]!=domyslnykolor){
            Serial.print(KOLOR[i]); Serial.print(" ["); Serial.print(i); Serial.print("] \u001b[0m | ");
            laval = KOLOR[i];
        }
    }
    Serial.println();
    Serial.print("Ostatni kolor ustawiony: "); Serial.print(laval); Serial.print(" - kolor tekstu: [\u001b["); Serial.print(colorMan(calcDATAtoFG(laval))); Serial.print("m"); Serial.print(calcDATAtoFG(laval)); Serial.print("\u001b[0m] kolor tla: [\u001b["); Serial.print(colorMan(calcDATAtoBG(laval))); Serial.print("m"); Serial.print(calcDATAtoBG(laval)); Serial.println("\u001b[0m]");
}


void pushCur(){
    int16_t x,y; calcNRtoXY(calcXYtoNR(tft.getCursorX(),tft.getCursorY())+1, &x, &y);
    tft.setCursor(x,y,czcionka);
}

void backspace(){
    int16_t xyz = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
    xyz--;
    int16_t x,y; calcNRtoXY(xyz,&x,&y);
    tft.setCursor(x,y,czcionka);
    ZNAKI[xyz] = 0;
    WYSWIETLONE[xyz] = false;
    tft.fillRect(tft.getCursorX(), tft.getCursorY(), sz, wz, TFT_BLACK);
}

void strzalka(char DIR, uint16_t ile = 1){                                                    // Obsługa klawiszy strzałek (A - góra, B - dół, C - prawo, D - lewo)
    for(uint16_t i = 0 ; i < ile ; i++){
        uint16_t gdzie = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
        if(DIR == 'A' || DIR == 'a' || DIR == '1')gdzie -= szerek / sz;
        else if(DIR == 'B' || DIR == 'b' || DIR == '2')gdzie += szerek / sz;
        else if(DIR == 'C' || DIR == 'c' || DIR == '3')gdzie++;
        else if(DIR == 'D' || DIR == 'd' || DIR == '4')gdzie--;
        else if(DEBUGMODE == true)Serial.println("CURSOR ERROR - wrong move parameter"); //Po angielsku fajniej błędy brzmią
        int16_t x,y;
        calcNRtoXY(gdzie, &x, &y);
        tft.setCursor(x,y,czcionka);
    }
}

void znakiSpecialne(){
    bool czyDrugi = false;
    uint8_t pierwszy = 0;
    uint8_t drugi = 0;
    uint16_t stary, nowy;

    while(true){
        if(Serial.available() > 0){
            char znak = Serial.read();
            if(isDigit(znak)){  //Wiem że to może był zrobione w jednej linijce, ale z jakiegoś powodu nie działa.
                if(czyDrugi == false){
                    stary = pierwszy;
                    nowy = znak - '0';
                    pierwszy = (stary * 10) + nowy;
                }
                else{
                    stary = drugi;
                    nowy = znak - '0';
                    drugi = (stary * 10) + nowy;
                }
            }
            else if(znak == ';' || (int)znak == 59)czyDrugi = true;
            else if((int)znak == 91){if(DEBUGMODE == true)Serial.print("przetwarzanie znaku specialnego");}
            else if(((int)znak>=65 && (int)znak <= 90) || ((int)znak >= 97 && (int)znak <= 122)){
                switch((int)znak){
                    case 109:
                        dodajKolor(pierwszy);
                        break;
                    case 65:
                        if(pierwszy<1)pierwszy=1;
                        strzalka('A',pierwszy);
                        break;
                    case 66:
                        if(pierwszy<1)pierwszy=1;
                        strzalka('B',pierwszy);
                        break;
                    case 67:
                        if(pierwszy<1)pierwszy=1;
                        strzalka('C',pierwszy);
                        break;
                    case 68:
                        if(pierwszy<1)pierwszy=1;
                        strzalka('D',pierwszy);
                        break;
                    case 69:   //(A - góra, B - dół, C - prawo, D - lewo)   - reusing those   Cursor Next Line	
                        if(pierwszy<1)pierwszy=1;
                        strzalka('B',pierwszy);
                        tft.setCursor(0,tft.getCursorY(), czcionka);
                        break;
                    case 70: //Cursor Previous Line	
                        if(pierwszy<1)pierwszy=1;
                        strzalka('A',pierwszy);
                        tft.setCursor(0,tft.getCursorY(), czcionka);
                        break;
                    case 71:   //Set Column: \u001b[{n}G moves cursor to column n
                        tft.setCursor((pierwszy * sz), tft.getCursorY(), czcionka);
                        break;
                    case 72:   //Set Position: \u001b[{n};{m}H moves cursor to row n column m   -- może nie działać, nie sprawdzałem czy drugi argument działa dobrze
                        tft.setCursor((pierwszy * sz), (drugi * wz), czcionka);
                        break;
                    case 74:   //Clear Screen: \u001b[{n}J clears the screen
                    /*  n=0 clears from cursor until end of screen,             --including the character on cursor or not? I'm gonna assume not
                        n=1 clears from cursor to beginning of screen
                        n=2 clears entire screen                            */
                        if(pierwszy == 0){
                            uint16_t chSnr = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
                            clearChars(chSnr, maxznakow);
                            //refresh();   - może być potrzebny
                        }
                        else if(pierwszy == 1){
                            uint16_t chEnr = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
                            clearChars(0, chEnr);
                        }
                        else if(pierwszy == 2){
                            irTab();
                            //tft.setCursor(0,0,czcionka);  -- chyba nie trzeba przesuwać po wyczyszczeniu, ale idk
                        }
                        break;
                    case 75:  //k
                        if(pierwszy == 0){
                            uint16_t chSnr = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
                            uint16_t chEnr = calcXYtoNR(szerek, tft.getCursorY());
                            clearChars(chSnr, chEnr);
                        }
                        else if(pierwszy == 1){
                            uint16_t chSnr = calcXYtoNR(0, tft.getCursorY());
                            uint16_t chEnr = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
                            clearChars(chSnr, chEnr);
                        }
                        else if(pierwszy == 2){
                            uint16_t chSnr = calcXYtoNR(0, tft.getCursorY());
                            uint16_t chEnr = calcXYtoNR(szerek, tft.getCursorY());
                            clearChars(chSnr, chEnr);
                        }
                }
                break;
            }
        }
    }
}

void clearChars(uint16_t startCh, uint16_t endCh){
    for (uint16_t i = startCh; i < endCh; i++){
        KOLOR[i] = domyslnykolor;
        ZNAKI[i] = 0;
        WYSWIETLONE[i] = false;
    }
    refresh();
}

void dodajKolor(uint16_t a){
    uint16_t nr = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
    //40 - 47 = bg
    //30 - 37 = fg
    if(a>=30 && a<=37){
        a %= 10;
        if(DEBUGMODE == true){Serial.print("FNT SET: "); Serial.print(a); Serial.println(" ");}
        KOLOR[nr] = calcColorAddToData(a,0,KOLOR[nr]);
    }
    else if(a>=40 && a<=47){
        a %= 10;
        if(DEBUGMODE == true){Serial.print("BG SET: "); Serial.print(a); Serial.println(" | ");}
        KOLOR[nr] = calcColorAddToData(a,1,KOLOR[nr]);
    }
    else{
        if(DEBUGMODE == true){Serial.print("ERROR "); Serial.print(100 + rand() % 899); Serial.print(" - Bad ");}
        return;
    }
}

uint16_t calcColorAddToData(uint8_t colorToAdd, uint8_t colorPos, uint16_t dataThen = 0){
    uint8_t FG = calcDATAtoFG(dataThen);
    uint8_t BG = calcDATAtoBG(dataThen);
    uint16_t dataNow;
    // uint8_t FT = calcDATAtoFT(dataNow);    TODO jeżeli chcemy obsługiwać czcionki
    switch (colorPos){
    case 0:
        FG = colorToAdd;
        break;
    case 1:
        BG = colorToAdd;
        break;
    }
    dataNow = FG + (BG*10)/*+ (FT*100) etc...*/;
    return dataNow;
}

void kolor(uint16_t colorData){
    uint16_t fnColor = colorData % 10;
    uint16_t bgColor = ((colorData % 100) - fnColor)/10;
    if(fnColor == domyslnykolorfn)fnColor = (ostatnikolor % 10);
    if(bgColor == domyslnykolorbg)bgColor = (((ostatnikolor % 100) - fnColor)/10);
    fnColor = colorMan(fnColor);
    bgColor = colorMan(bgColor);
    tft.setTextColor(fnColor, bgColor);
    ostatnikolor = colorData;
}

uint8_t calcDATAtoFG(uint16_t colorData){
    return colorData%10;
}

uint8_t calcDATAtoBG(uint16_t colorData){
    return ((colorData%100)-(colorData%10))/10;
}

uint16_t colorMan(uint8_t x){
    x %= 10;
    switch(x){
        case 0: //Black
            return 0x0000;
        case 1: //Red
            return 0xF800;
        case 2: //Green
            return 0x07E0;
        case 3: //Yellow
            return 0xFFE0;
        case 4: //Blue
            return 0x001F;
        case 5: //Magenta
            return 0xF81F;
        case 6: //Cyan
            return 0x07FF;
        case 7: //White
            return 0xFFFF;
        default:
            if(DEBUGMODE == true){Serial.print("ERROR 100 - bad var: "); Serial.println(x);}
            return 0;
    }
}

void irTab(){
    for (uint16_t i = 0; i < maxznakow; i++){
        KOLOR[i] = domyslnykolor;
        ZNAKI[i] = 0;
        WYSWIETLONE[i] = false;
    }
}

void carriageReturn(){
    uint16_t gdzie = calcXYtoNR(tft.getCursorX(), tft.getCursorY());
    int16_t x,y;
    calcNRtoXY(gdzie, &x, &y);
    tft.setCursor(1,(y+wz),czcionka);
}

// uint8_t reverseColorMan(uint16_t x){
//     x = x % 10;
//     switch(x){
//         case 0x0000: //Black
//             return 0;
//         case 0xF800: //Red
//             return 1;
//         case 0x07E0: //Green
//             return 2;
//         case 0xFFE0: //Yellow
//             return 3;
//         case 0x001F: //Blue
//             return 4;
//         case 0xF81F: //Magenta
//             return 5;
//         case 0x07FF: //Cyan
//             return 6;
//         case 0xFFFF: //White
//             return 7;
//         default:
//             Serial.print("ERROR 101");
//             return 0;
//     }
// }



/* Bugi:
- this program is flawless

TODO:
- OBSŁUGA TYCH DODATKOWYCH FUNKCJI


- obsługa entera
- obsługa znaków specjalnych, --kolorów--, przesunięć kursora
- zmiana czcionki
- logo
*/





/*
void robiRzeczy(){
  bool czyDrugiParametr = 0;
  uint16_t uno = 0;
  uint16_t dos = 0;
  while(true){
    if(Serial.available() > 0){
      char analizowanyZnak = Serial.read();
      if (isDigit(analizowanyZnak)){ //test czy to parametr
        //tft.print("<");tft.print(analizowanyZnak);tft.print(">");tft.print("[");tft.print(uno);tft.print("]");
        if(czyDrugiParametr == 0){
          uint16_t old = uno;
          int16_t nowy = analizowanyZnak - '0';
          uno = (old*10)+nowy;
        } //Okazuje się że trzeba to rozbić tak na zmienne bo nie działa. Fajnie, godzina stracona
        else if(czyDrugiParametr == 1){
          //dos = (dos*10)+(analizowanyZnak + '0');      <--- tak nie działa z jakiegoś powodu
          uint16_t old = dos;
          int16_t nowy = analizowanyZnak - '0';
          dos = (old*10)+nowy;
        }
      }
      else if((int)analizowanyZnak == 59){
        czyDrugiParametr = 1;
      }
      else if((int)analizowanyZnak == 91);
      else{
        processComm(analizowanyZnak,uno,dos);
        break;
      }
    }
  }
}
*/


/*
Reset(color/style): \u001b[0m
Reversed(bg and txt colors): \u001b[7m
Bold: \u001b[1m
Underline: \u001b[4m
Black: \u001b[30m
Red: \u001b[31m
Green: \u001b[32m
Yellow: \u001b[33m
Blue: \u001b[34m
Magenta: \u001b[35m
Cyan: \u001b[36m
White: \u001b[37m
Background Black: \u001b[40m
Background Red: \u001b[41m
Background Green: \u001b[42m
Background Yellow: \u001b[43m
Background Blue: \u001b[44m
Background Magenta: \u001b[45m
Background Cyan: \u001b[46m
Background White: \u001b[47m



Up: \u001b[{n}A moves cursor up by n
Down: \u001b[{n}B moves cursor down by n
Right: \u001b[{n}C moves cursor right by n
Left: \u001b[{n}D moves cursor left by n



Clear Screen: \u001b[{n}J
    n=0 clears from cursor until end of screen,        
    n=1 clears from cursor to beginning of screen
    n=2 clears entire screen
Clear Line: \u001b[{n}K
    n=0 clears from cursor to end of line          !!!!
    n=1 clears from cursor to start of line
    n=2 clears entire line                         !!!!
Next Line: \u001b[{n}E moves cursor to beginning of line n lines down
Prev Line: \u001b[{n}F moves cursor to beginning of line n lines down
Set Column: \u001b[{n}G moves cursor to column n
Set Position: \u001b[{n};{m}H moves cursor to row n column m     (MATH ERROR?)

Save Position: \u001b[{s} saves the current cursor position
Save Position: \u001b[{u} restores the cursor to the last saved position
*/