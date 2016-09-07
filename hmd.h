/**
 * hmd.h
 *
 *  Author: Maurice Tollmien
 */

#include "joystick.h"
#include "mtQuaternions.h"

#define HMD_PI 3.141592654

/**
 * Liefert das MTQuaternion vom HMD normalisiert zurück.
 * factor entspricht dem Zeitinterval zwischen jedem Frame.
 */
MTQuaternion getMTQuaternion(MTVec3D jawAxis, MTVec3D turnAxis, double minJawAngle, double maxJawAngle, double factor);

/**
 * Gibt den absoluten Wert, den der Joystick liefert.
 */
short getTranslationAxisValue(int axis);

/**
 * Initialisiert/Überprüft die Verbindung mit dem HMD.
 * @return 1 = okay; 0 = fehler
 */
int initializeHMD(char* name);

/**
 * Beendet die Verbindung zum HMD.
 */
int closeHMD();

/**
 * Liest die Events von dem HMD aus der Treiber-Queue.
 * Sollte kontinuierlich ausgeführt werden.
 */
void handleHMDEvent();
