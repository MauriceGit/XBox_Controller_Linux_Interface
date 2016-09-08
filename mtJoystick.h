/*
 * joystick.h
 *
 * Implementiert die Behandlung von Joysticks ab linux 2.2.x.
 * In dieser Implementierung sind maximal:
 * - 256 Achsen
 * - 256 Buttons
 * auslesbar.
 *
 * Ermöglicht das Auslesen von:
 * - Namen des Device
 * - Anzahl der Achsen
 * - Anzahl der Buttons
 * - Status der Achsen (-32768/32767) (signed short)
 * - Status der Buttons (0/1)
 *
 * Das Auslesen ist eventbasiert.
 *
 * Author: Mervyn McCreight, Maurice Tollmien
 *
 * zuletzt geändert: 4.4.2013
 */

/* Konstanten */
#define JOY_NAME_LENGTH 80


/**
 * interne Datenstruktur zur Speicherung der Joystick-Daten
 */
typedef struct {
    int fd;                                 /* Datenstrom */
    int driverVersion;                      /* Treiberversion */
    char axisNumber;                        /* Anzahl der Achsen */
    char buttonNumber;                      /* Anzahl der Buttons */
    char name[JOY_NAME_LENGTH];             /* Name des Device */
    short * axisValues;                     /* Dynamisches Array für die Achsenwerte */
    short * buttonValues;                   /* Dynamisches Array für die Buttonwerte */
} JoystickDevice;


/**
 * Gibt die ausgelesenen Joystickinformationen aus.
 */
void printJoystickInformation();

/**
 * Startet eine Verbindung zum Device.
 * Reserviert Speicher für die interne Datenverwaltung.
 */
int startDeviceConnection(const char * devname);

/**
 * Beendet die Verbindung zum Device.
 * Gibt reservierten Speicher wieder frei.
 */
int endDeviceConnection();

/**
 * Auslesen und interpretieren der Joystick-Events.
 * Sollte kontinuierlich ausgeführt werden.
 */
void handleJoystickEvents();

/**
 * Gibt den Wert einer Achse in einem Variablenparameter aus.
 */
int getAxisValue(int axisNumber, short * value);

/**
 * Gibt den Wert eines Button in einem Variablenparameter aus.
 */
int getButtonValue(int buttonNumber, short * value);

int setCalibrationCoefficients(int a,int b,int c,int d,int t,int prec);
