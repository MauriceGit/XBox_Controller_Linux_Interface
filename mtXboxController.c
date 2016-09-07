/**
 * Implementation of camera movements controlled by a joystick.
 *
 * @author Maurice Tollmien. Github: MauriceGit
 */

#include <stdio.h>
#include "mtXboxController.h"
#include "mtVector.h"
#include "hmd.h"
#include "mtQuaternions.h"

MTVec3D G_JoyUpVector;
MTVec3D G_JoyViewVector;
MTVec3D G_JoyCameraTranslation;
MTVec3D G_JoyCameraPosition;

MTVec3D mtGetJoyCameraPosition () {
    return G_JoyCameraPosition;
}

/**
 * Just for convenience.
 */
MTVec3D mtGetJoyPosition () {
    return mtGetJoyCameraPosition();
}

MTVec3D mtGetJoyUp() {
    return G_JoyUpVector;
}

MTVec3D mtGetJoyCenter() {
    return mtAddVectorVector(G_JoyViewVector, G_JoyCameraPosition);
}

/**
 * Calculates all relevant joystick/gamepad stuff and rotates, moves the camera accordingly.
 */
void mtCalcJoyCameraMovement (double interval)
{

    handleHMDEvent();

    MTVec3D sideDirection = mtNormVector3D(mtCrossProduct3D(G_JoyViewVector, G_JoyUpVector));

    double maxAngle = MT_MAX_ANGLE - mtAngleVectorVector(G_JoyViewVector, G_JoyUpVector); // around 90° -- +70°
    double minAngle = MT_MIN_ANGLE - mtAngleVectorVector(G_JoyViewVector, G_JoyUpVector); // around 90° -- -70°

    maxAngle = maxAngle < 0 ? -1.0 : maxAngle;
    minAngle = minAngle > 0 ? 1.0 : minAngle;

    MTQuaternion q = getMTQuaternion(sideDirection, {.x=0, .y=1, .z=0}, minAngle, maxAngle, interval);

    G_JoyViewVector = mtRotatePointWithMTQuaternion(q, G_JoyViewVector);

    double forwardTranslation = -getTranslationAxisValue(4) / MT_XBOX_NORMALISATION;
    MTVec3D forwardVec = mtNormVector3D({.x=G_JoyViewVector.x, .y=0, .z=G_JoyViewVector.z});
    G_JoyCameraTranslation = mtAddVectorVector(G_JoyCameraTranslation, mtMultiplyVectorScalar(forwardVec, forwardTranslation));

    double sideTranslation = getTranslationAxisValue(3) / MT_XBOX_NORMALISATION;
    MTVec3D sideVec = mtNormVector3D({.x=sideDirection.x, .y=0, .z=sideDirection.z});
    G_JoyCameraTranslation = mtAddVectorVector(G_JoyCameraTranslation, mtMultiplyVectorScalar(sideVec, sideTranslation));

    MTVec3D upDirection = {.x=0, .y=1, .z=0};
    double upTranslation = (getTranslationAxisValue(2) + 32768) / MT_XBOX_NORMALISATION;
    G_JoyCameraTranslation = mtAddVectorVector(G_JoyCameraTranslation, mtMultiplyVectorScalar(upDirection, upTranslation));

    MTVec3D downDirection = {.x=0, .y=-1, .z=0};
    double downTranslation = (getTranslationAxisValue(5) + 32768) / MT_XBOX_NORMALISATION;
    G_JoyCameraTranslation = mtAddVectorVector(G_JoyCameraTranslation, mtMultiplyVectorScalar(downDirection, downTranslation));

    G_JoyCameraPosition = G_JoyCameraTranslation;

    G_JoyViewVector = mtNormVector3D(G_JoyViewVector);

}

/**
 * Initialisation of the camera and hmd module for js input.
 */
int mtInitJoyCamera (char* name)
{
    G_JoyUpVector = {.x=0, .y=1, .z=0};
    G_JoyViewVector = {.x=-MT_CAMERA_X, .y=-MT_CAMERA_Y, .z=-MT_CAMERA_Z};
    G_JoyViewVector = mtNormVector3D(G_JoyViewVector);

    G_JoyCameraTranslation = {.x=0, .y=0, .z=0};
    G_JoyCameraPosition = {.x=MT_CAMERA_X, .y=MT_CAMERA_Y, .z=MT_CAMERA_Z};

    if (!initializeHMD(name)) {
        printf("ERROR: hmd could not be initialized.\n");
        return 0;
    }
    return 1;
}

/**
 * Closes the hmd module connection to the joystick.
 */
int mtFinishJoyControl()
{
    return closeHMD();
}


