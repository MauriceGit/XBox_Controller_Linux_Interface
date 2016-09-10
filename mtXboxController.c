/**
 * Implementation of camera movements controlled by a joystick.
 *
 * @author Maurice Tollmien. Github: MauriceGit
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "mtXboxController.h"
#include "mtVector.h"
#include "mtQuaternions.h"
#include "mtJoystick.h"

MTVec3D G_JoyUpVector;
MTVec3D G_JoyViewVector;
MTVec3D G_JoyTranslation;
MTVec3D G_JoyPosition;

MTVec3D mtGetJoyPosition () {
    return G_JoyPosition;
}

MTVec3D mtGetJoyUp() {
    return G_JoyUpVector;
}

MTVec3D mtGetJoyCenter() {
    return mtAddVectorVector(G_JoyViewVector, G_JoyPosition);
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
 * This is unique to the XBox controller! If posToAngle does not work out on another joystick,
 * get creative.
 */
float posToAngle(short pos, double factor) {

    if (pos < 1000 && pos > -1000) {
        pos = 0;
    }

    float value = log(abs(pos) <= 1 && abs(pos) >= 0 ? 1 : abs(pos)*factor) / MT_XBOX_TURN_NORMALISATION;
    if (pos <= 0) {
        value *= -1;
    }

    return value;
}

/**
 * Returns a normalised Quaternion with the Joystick movements.
 */
MTQuaternion getQuaternion(MTVec3D jawAxis, MTVec3D turnAxis, double minJawAngle, double maxJawAngle, double factor) {
    short a,b;

    /* Right now, we are only interested in these two axes. If you want to use more
     * than these two, just add some short variables and read the axis-values accordingly. Easy. */
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


    MTQuaternion qB = mtCreateQuaternion(jawAxis, angle);
    MTQuaternion qA = mtCreateQuaternion(turnAxis, -posToAngle(a, factor));

    MTQuaternion qRes = mtAddQuaternionQuaternion(&qA, &qB);

    mtNormQuaternion(&qRes);
    return qRes;
}

/**
 * Calculates all relevant joystick/gamepad stuff and rotates, moves the camera accordingly.
 */
void mtCalcJoyMovement (double interval)
{
    // This pulls and handles all usb-stream joystick events.
    handleJoystickEvents();

    MTVec3D sideDirection = mtNormVector3D(mtCrossProduct3D(G_JoyViewVector, G_JoyUpVector));

    double maxAngle = MT_MAX_ANGLE - mtAngleVectorVector(G_JoyViewVector, G_JoyUpVector); // around 90° -- +70°
    double minAngle = MT_MIN_ANGLE - mtAngleVectorVector(G_JoyViewVector, G_JoyUpVector); // around 90° -- -70°

    maxAngle = maxAngle < 0 ? -1.0 : maxAngle;
    minAngle = minAngle > 0 ? 1.0 : minAngle;

    MTQuaternion q = getQuaternion(sideDirection, mtToVector3D(0, 1, 0), minAngle, maxAngle, interval);

    G_JoyViewVector = mtRotatePointWithQuaternion(q, G_JoyViewVector);

    double forwardTranslation = -(getTranslationAxisValue(4) + MT_AXIS_4_OFFSET) / MT_XBOX_TRANS_NORMALISATION;
    MTVec3D forwardVec = mtNormVector3D(mtToVector3D(G_JoyViewVector.x, 0, G_JoyViewVector.z));
    G_JoyTranslation = mtAddVectorVector(G_JoyTranslation, mtMultiplyVectorScalar(forwardVec, forwardTranslation));

    double sideTranslation = (getTranslationAxisValue(3) + MT_AXIS_3_OFFSET) / MT_XBOX_TRANS_NORMALISATION;
    MTVec3D sideVec = mtNormVector3D(mtToVector3D(sideDirection.x, 0, sideDirection.z));
    G_JoyTranslation = mtAddVectorVector(G_JoyTranslation, mtMultiplyVectorScalar(sideVec, sideTranslation));

    MTVec3D upDirection = mtToVector3D(0, 1, 0);
    double upTranslation = (getTranslationAxisValue(2) + MT_AXIS_2_OFFSET) / MT_XBOX_TRANS_NORMALISATION;
    G_JoyTranslation = mtAddVectorVector(G_JoyTranslation, mtMultiplyVectorScalar(upDirection, upTranslation));

    MTVec3D downDirection = mtToVector3D(0, -1, 0);
    double downTranslation = (getTranslationAxisValue(5) + MT_AXIS_5_OFFSET) / MT_XBOX_TRANS_NORMALISATION;
    G_JoyTranslation = mtAddVectorVector(G_JoyTranslation, mtMultiplyVectorScalar(downDirection, downTranslation));

    G_JoyPosition = G_JoyTranslation;

    G_JoyViewVector = mtNormVector3D(G_JoyViewVector);

}

/**
 * Initialisation of the camera and hmd module for js input.
 */
int mtInitJoyControl (char* name)
{
    G_JoyUpVector = mtToVector3D(0, 1, 0);
    G_JoyViewVector = mtToVector3D(-MT_START_POS_X, -MT_START_POS_Y, -MT_START_POS_Z);
    G_JoyViewVector = mtNormVector3D(G_JoyViewVector);

    G_JoyTranslation = mtToVector3D(0,0,0);
    G_JoyPosition = mtToVector3D(MT_START_POS_X, MT_START_POS_Y, MT_START_POS_Z);

    if  (!startDeviceConnection(name)) {
        printf("ERROR: joystick could not be initialized.\n");
        return 0;
    }

    return 1;
}

/**
 * Closes the hmd module connection to the joystick.
 */
int mtFinishJoyControl()
{
    return endDeviceConnection();
}


