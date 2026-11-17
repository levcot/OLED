
#include "OLED.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <IR.h>

#include "IMU.h"


// --------------------------------------------------
// OLED
// --------------------------------------------------

static Adafruit_SH1106G display(128, 64, &Wire2);

// --------------------------------------------------
// KY-040
// --------------------------------------------------

static constexpr uint8_t ENCODER_CLK = 26;
static constexpr uint8_t ENCODER_DT  = 27;
static constexpr uint8_t ENCODER_SW  = 28;

// --------------------------------------------------
// MENUE
// --------------------------------------------------

static constexpr uint8_t MENU_COUNT = 4;

static const char* menuItems[MENU_COUNT] = {
    "Programm starten",
    "IMU-Daten",
    "IR-Daten",
    "IR kalibrieren"
};

static uint8_t selection = 0;
static uint8_t lastCLK = HIGH;

static bool lastButtonReading = HIGH;
static bool stableButtonState = HIGH;
static bool clickEvent = false;

static unsigned long lastButtonChange = 0;
static constexpr unsigned long DEBOUNCE_MS = 30;

// --------------------------------------------------
// INTERNE HILFSFUNKTIONEN
// --------------------------------------------------

static void OLED_drawMenu()
{
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("WRO ROBOT");
    display.drawLine(0, 10, 127, 10, SH110X_WHITE);

    // Vier Menuepunkte auf zwei Seiten anzeigen:
    // Seite 1: Punkte 1 und 2
    // Seite 2: Punkte 3 und 4

    uint8_t firstItem = (selection / 2) * 2;

    for (uint8_t i = firstItem; i < firstItem + 2; i++)
    {
        uint8_t y = 19 + (i - firstItem) * 21;

        display.setCursor(0, y);

        if (i == selection)
        {
            display.print(">");
        }
        else
        {
            display.print(" ");
        }

        display.setCursor(12, y);
        display.println(menuItems[i]);
    }

    display.setCursor(0, 55);
    display.print("Drehen: Auswahl");

    display.display();
}

// --------------------------------------------------
// INITIALISIERUNG
// --------------------------------------------------

void OLED_begin()
{
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);

    Wire2.begin();

    if (!display.begin(0x3C, true))
    {
        // Display nicht gefunden.
        // Hier koennte eine Fehleranzeige ueber Serial folgen.
        return;
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WRO ROBOT");
    display.println();
    display.println("System startet...");
    display.display();

    lastCLK = digitalRead(ENCODER_CLK);

    delay(500);

    OLED_goHome();
}

// --------------------------------------------------
// DRUCKKNOPF ERKENNEN
// --------------------------------------------------

bool OLED_click()
{
    bool reading = digitalRead(ENCODER_SW);
    unsigned long now = millis();

    if (reading != lastButtonReading)
    {
        lastButtonChange = now;
        lastButtonReading = reading;
    }

    if (now - lastButtonChange >= DEBOUNCE_MS)
    {
        if (reading != stableButtonState)
        {
            stableButtonState = reading;

            // INPUT_PULLUP: LOW bedeutet gedrueckt.
            if (stableButtonState == LOW)
            {
                clickEvent = true;
            }
        }
    }

    if (clickEvent)
    {
        clickEvent = false;
        return true;
    }

    return false;
}

// --------------------------------------------------
// MENUE AKTUALISIEREN
// --------------------------------------------------

void OLED_update()
{
    // Drehencoder auslesen
    uint8_t clk = digitalRead(ENCODER_CLK);

    if (clk != lastCLK && clk == LOW)
    {
        if (digitalRead(ENCODER_DT) != clk)
        {
            selection++;

            if (selection >= MENU_COUNT)
            {
                selection = 0;
            }
        }
        else
        {
            if (selection == 0)
            {
                selection = MENU_COUNT - 1;
            }
            else
            {
                selection--;
            }
        }

        OLED_drawMenu();
    }

    lastCLK = clk;

    // Druckknopf wird separat mit OLED_click()
    // im Hauptprogramm abgefragt.
}

uint8_t OLED_getSelection()
{
    return selection;
}

// --------------------------------------------------
// ZURUECK ZUM HAUPTMENUE
// --------------------------------------------------

void OLED_goHome()
{
    OLED_drawMenu();
}

// --------------------------------------------------
// PROGRAMM LAEUFT
// --------------------------------------------------

void OLED_programmRun()
{
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("PROGRAMM LAEUFT");

    display.drawLine(0, 11, 127, 11, SH110X_WHITE);

    display.setCursor(0, 20);
    display.println("Roboter in Bewegung");

    display.setCursor(0, 40);
    display.println("Taste: STOPP");

    display.display();
}

// --------------------------------------------------
// IMU ANZEIGEN
// --------------------------------------------------

void OLED_showIMU()
{
    // Voraussetzung:
    // IMU_getPosition() liefert einen float-Winkel.
    float winkel = IMU_getPosition();

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("IMU-DATEN");

    display.drawLine(0, 11, 127, 11, SH110X_WHITE);

    display.setCursor(0, 20);
    display.print("Winkel: ");
    display.print(winkel, 1);
    display.println(" deg");

    display.setCursor(0, 48);
    display.println("Taste: Zurueck");

    display.display();
}

// --------------------------------------------------
// IR ANZEIGEN
// --------------------------------------------------

void OLED_showIR()
{
    // Voraussetzung:
    // IR_getPosition() liefert die Linienposition.
    int position = IR_getPosition(); 

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("IR-SENSORDATEN");

    display.drawLine(0, 11, 127, 11, SH110X_WHITE);

    display.setCursor(0, 20);
    display.print("Linienposition:");

    display.setCursor(0, 32);
    display.println(position);

    display.setCursor(0, 48);
    display.println("Taste: Zurueck");

    display.display();
}

// --------------------------------------------------
// IR KALIBRIEREN
// --------------------------------------------------

void OLED_calibrate()
{
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("IR-KALIBRIERUNG");

    display.drawLine(0, 11, 127, 11, SH110X_WHITE);

    display.setCursor(0, 22);
    display.println("Kalibriere Sensoren");

    display.setCursor(0, 38);
    display.println("Bitte bewegen...");

    display.display();

    IR_calibrate(5000);

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("IR-KALIBRIERUNG");

    display.drawLine(0, 11, 127, 11, SH110X_WHITE);

    display.setCursor(0, 25);
    display.println("Kalibrierung beendet");

    display.setCursor(0, 48);
    display.println("Taste: Zurueck");

    display.display();
}
