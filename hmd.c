

/* - System Header einbinden */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* - eigene Header einbinden */
#include "hmd.h"
#include "mtQuaternions.h"

float posToAngle(short pos, double factor) {

    if (pos < 1000 && pos > -1000) {
        pos = 0;
    }

    // This is unique to the XBox controller! If posToAngle does not work out on another joystick,
    // get creative.
    float value = log(abs(pos) <= 1 && abs(pos) >= 0 ? 1 : abs(pos)*factor) / 2000.0;
    if (pos <= 0) {
        value *= -1;
    }

    return value;
}

/**
 * Liefert das MTQuaternion vom Controller normalisiert zurück.
 */
MTQuaternion getMTQuaternion(MTVec3D jawAxis, MTVec3D turnAxis, double minJawAngle, double maxJawAngle, double factor) {
    short a,b;

    /* im Fehlerfall gib 0 zurück. */
    if (!(
         getAxisValue(0, &a)
      && getAxisValue(1, &b)
      )) {
          printf("ERROR reading an axis value!\n");
          MTQuaternion q;
          return q;
    }

    double angle = posToAngle(b, factor);
    angle = (angle > 0.0 && angle < minJawAngle) ? 0.0 : angle;
    angle = (angle < 0.0 && angle > maxJawAngle) ? 0.0 : angle;


    MTQuaternion qB = mtCreateMTQuaternion(jawAxis, angle);
    MTQuaternion qA = mtCreateMTQuaternion(turnAxis, -posToAngle(a, factor));

    MTQuaternion qRes = mtAddMTQuaternionMTQuaternion(&qA, &qB);

    mtNormMTQuaternion(&qRes);
    return qRes;
}

short getTranslationAxisValue(int axis) {
    if (axis <= 5) {
        short v;
        if (!getAxisValue(axis, &v)) {
            printf("ERROR reading a translation axis value!\n");
            return 0;
        }
        return v;
    }
    printf("ERROR wrong axis value\n");
    return 0;
}

/**
 * Initialisiert/Überprüft die Verbindung mit dem HMD.
 * @return 1 = okay; 0 = fehler
 */
int initializeHMD(char* name) {
    if  (!startDeviceConnection(name)) {
        return 0;
    }

    return 1;
}

/**
 * Beendet die Verbindung zum HMD.
 */
int closeHMD() {
    return endDeviceConnection();
}

/**
 * Liest die Events von dem HMD aus der Treiber-Queue.
 * Sollte kontinuierlich ausgeführt werden.
 */
void handleHMDEvent() {
    handleJoystickEvents();
}
