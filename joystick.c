/*
 * joystick.c
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


/* benötigte system-header einbinden */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

/* eigene header einbinden */
#include "joystick.h"


/* Dynamisches Array an Devices mit geöffneten Verbindungen. */
JoystickDevice g_device;

/**
 * Veraendert die Correction Koeffizienten, um
 * einen Joystick zu kalibrieren.
 */
int setCalibrationCoefficients(int a,int b,int c,int d,int t,int prec) {
    struct js_corr corr[8];

    int i = 0;

    for (i=0; i<8; ++i) {

        corr[i].type = t;
        corr[i].prec = prec;
        corr[i].coef[0] = a;
        corr[i].coef[1] = b;
        corr[i].coef[2] = c;
        corr[i].coef[3] = d;

        printf("corr.type = %d, corr.prec = %d, corr.a = %d, corr.b = %d, corr.c = %d, corr.d = %d\n", corr[i].type, corr[i].prec, corr[i].coef[0], corr[i].coef[1], corr[i].coef[2], corr[i].coef[3]);
    }

    /* correction setzen */
    if (ioctl(g_device.fd, JSIOCSCORR, &corr)) {
        return 0;
    }

    return 1;
}

/**
 * Gibt den für die interne
 * Datenstruktur zur Speicherung
 * der Joystickdaten allokierten
 * Speicher wieder frei.
 *
 * @return 0 = Fehler / 1 = hat funktioniert.
 */
static int freeDeviceMemory() {

    /* dynamische Arrays zur Achsen/Tastenwertspeicherung freigeben. */
    free(g_device.axisValues);
    free(g_device.buttonValues);

    return (g_device.axisValues == NULL) && (g_device.buttonValues == NULL);
}

/**
 * Öffnet den Joystick-Datenstrom zum Lesen.
 * @param devname der Pfad zum Device
 *
 * @return 0 = Fehler / 1 = hat funktioniert
 */
static int openDeviceStream(const char * devname) {
    /*
     * Öffnet das Device im Blocking-Mode.
     * Ist kein Device angeschlossen, bzw. sendet das Device
     * keine valide Antwort, wird "-1" als Fehlerwert zurückgegeben.
     */
    g_device.fd = open(devname, O_RDONLY);

    if (g_device.fd == -1) {
        fprintf(stderr, "Error opening device: %s\n", devname);
        return 0;
    }

    /* Wechselt in den NON-BLOCKING-MODE.
     * Ein "read" wartet nun nicht mehr unendlich lange auf ein
     * triggerndes Event, sondern liest das oberste Event auf dem
     * Treiber-Stack.
     */
    fcntl(g_device.fd, F_SETFL, O_NONBLOCK);

    return 1;
}

/**
 * Ermittelt folgende Daten des Device
 * und speichert diese intern.
 * - Anzahl der Achsen (IOCTL Operation: JSIOCGAXES)
 * - Anzahl der Buttons (IOCTL Operation: JSIOCGBUTTONS)
 * - Name des Device (IOCTL Operation: JSIOCGNAME(length))
 * - Treiberversion (IOCTL Operation: JSIOCGVERSION)
 *
 * @return 0 = Fehler / 1 = alles OK.
 */
static int getJoystickInformation() {
    /* das Device muss geöffnet sein. */
    if (g_device.fd == -1) {
        fprintf(stderr, "getJoystickInformation: Device not opened!\n");
        return 0;
    }

    /* Namen des Device auslesen und speichern. */
    if (ioctl (g_device.fd, JSIOCGNAME(JOY_NAME_LENGTH), &g_device.name) < 0) {
        strncpy(g_device.name, "Unknown Device", sizeof(g_device.name));
    }

    /* Anzahl der verfügbaren Achsen auslesen */
    ioctl(g_device.fd, JSIOCGAXES, &g_device.axisNumber);

    /* Anzahl der verfügbaren Buttons auslesen */
    ioctl(g_device.fd, JSIOCGBUTTONS, &g_device.buttonNumber);

    /* Treiberversion auslesen */
    ioctl(g_device.fd, JSIOCGVERSION, &g_device.driverVersion);

    return 1;
}

/**
 * Reserviert entsprechend Speicher für
 * die dynamischen Arrays der internen
 * Datenstruktur zur Verwaltung der Achsen/Button-Werte.
 *
 * @return 0 = Fehler / 1 = hat funktioniert
 */
static int allocateDeviceValueMemory() {
    /* dynamisches Array für die Achsenwerte */
    g_device.axisValues = (short *)calloc(g_device.axisNumber, sizeof(short));

    if (g_device.axisValues == NULL) {
        fprintf(stderr, "allocateDeviceValueMemory: Error allocating memory for the axis!\n");
        return 0;
    }

    /* dynamisches Array für die Buttonwerte */
    g_device.buttonValues = (short *)calloc(g_device.buttonNumber, sizeof(short));

    if (g_device.buttonValues == NULL) {
        fprintf(stderr, "allocateDeviceValueMemory: Error allocating memory for the buttons!\n");
        return 0;
    }

    return 1;
}

/**
 * Startet eine Verbindung zum Device.
 * Reserviert Speicher für die interne Datenverwaltung.
 *
 * @return 0 = Fehler / 1 = hat funktioniert.
 */
int startDeviceConnection(const char * devname) {
    if (!openDeviceStream(devname)) {
        return 0;
    }

    if (!getJoystickInformation()) {
        return 0;
    }

    if (!allocateDeviceValueMemory()) {
        return 0;
    }

    return 1;
}

/**
 * Beendet die Verbindung zum Device.
 * Gibt reservierten Speicher wieder frei.
 *
 * @return 0 = Fehler / 1 = hat funktioniert.
 */
int endDeviceConnection() {
    close(g_device.fd);
    return freeDeviceMemory();
}

/**
 * Gibt die ausgelesenen Joystickinformationen aus.
 */
void printJoystickInformation() {
    fprintf(stdout, "-------------------------------------\n");
    fprintf(stdout, "Device Name:\t%s\n", g_device.name);
    fprintf(stdout, "Driver Version:\t%i\n", g_device.driverVersion);
    fprintf(stdout, "-------------------------------------\n");
    fprintf(stdout, "Number of Axis:\t%d\n", g_device.axisNumber);
    fprintf(stdout, "Number of Buttons:\t%d\n", g_device.buttonNumber);
}

/**
 * Verarbeitet ein Joystick-Event.
 *
 * e das Joystick-Event.
 */
static void processEvent(struct js_event e) {
    /* Event interpretieren
     * die JS_EVENT_INIT Bits werden deaktiviert,
     * da nicht zwischen synthetic und real events unterschieden
     * werden soll.
     */
    switch (e.type & ~JS_EVENT_INIT) {
        case JS_EVENT_AXIS:
            //printf("axis: %i, value: %i\n", e.number, e.value);
            g_device.axisValues[e.number] = e.value;

            break;

        case JS_EVENT_BUTTON:
            //printf("button: %i, value: %i\n", e.number, e.value);
            g_device.buttonValues[e.number] = e.value;

            break;
        default:
            break;
    }
}

/**
 * Liest ein JoystickEvent aus und aktualisiert die Werte
 * entsprechend in der internen JoystickDatenstruktur.
 *
 * Struktur eines js_events:
 *
 * time     :   unsigned int    - event Zeitstempel in ms
 * value    :   short           - der neue Wert
 * type     :   char            - die Art des Events
 * number   :   char            - die Nummer der Achse/des Buttons
 *
 * die möglichen Werte von type sind:
 *
 * JS_EVENT_BUTTON (0x01)       - ein Buttonevent
 * JS_EVENT_AXIS (0x02)         - ein Achsenevent
 * JS_EVENT_INIT (0x80)         - Initialstatus des Device
 */
void handleJoystickEvents() {
    struct js_event jsEvent;

    /* all Events auf dem Treiber-Stack auslesen. */
    while (read(g_device.fd, &jsEvent, sizeof(struct js_event)) > 0) {
        processEvent(jsEvent);
    }
}

/**
 * Gibt den Wert einer Achse in einem Variablenparameter aus.
 *
 * @param axisNumber die Achse, die ausgelesen werden soll
 * @param value der ausgelesene Wert
 *
 * @return 0 = Fehler / 1 = hat funktioniert.
 */
int getAxisValue(int axisNumber, short * value) {
    if (axisNumber <= g_device.axisNumber) {
        (*value) = g_device.axisValues[axisNumber];
        return 1;
    }

    fprintf(stderr, "getAxisValue: Ungueltige Achsenanhabe.\n");
    (*value) = 0;
    return 0;
}


/**
 * Gibt den Wert eines Button in einem Variablenparameter aus.
 *
 * @param buttonNumber der Button, der ausgelesen werden soll
 * @param value der ausgelesene Wert
 *
 * @return 0 = Fehler / 1 = hat funktioniert.
 */
int getButtonValue(int buttonNumber, short * value) {
    if (buttonNumber <= g_device.buttonNumber) {
        (*value) = g_device.buttonValues[buttonNumber];
        return 1;
    }

    fprintf(stderr, "getButtonValue: Ungueltige Buttonanhabe.\n");
    (*value) = 0;
    return 0;
}

/*
 * MODUL-TEST
int main(void) {

    short testvalue = 2000;

    if (!startDeviceConnection("/dev/input/js0")) exit(1);
    printJoystickInformation();

    setCalibrationCoefficients(24617, 40917, 21844, 21844, 1, 255);

    while (1) {
        int i = 0;
        handleJoystickEvents();

        for (i=0; i<g_device.axisNumber; ++i) {
            getAxisValue(i, &testvalue);
            printf("[%d]: %d; ", i, testvalue);
        }
        printf("\n");
    }

    endDeviceConnection();

    return 0;
} */
