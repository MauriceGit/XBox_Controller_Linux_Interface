#ifndef __JOYSTICKCAMERA_H__
#define __JOYSTICKCAMERA_H__
/**
 * Implementation of a Camera controlled by an X-Box controller (And possibly other joysticks).
 *
 * All operations are prefixed with 'mt' to avoid name clashes and get an
 * attempt for a unique prefix.
 *
 * This module is tested exclusively for an X-Box controller and can be used out-of-the-box.
 * It might work with other joysticks.
 * If the camera is moving by its own, consider calibrating it (jscal / jstest-gtk).
 *
 * @author Maurice Tollmien
 */

#include "mtVector.h"

#define MT_CAMERA_X        -30.0
#define MT_CAMERA_Y        0.0
#define MT_CAMERA_Z        70.0

#define MT_MAX_ANGLE       179.0
#define MT_MIN_ANGLE       1.0

#define MT_XBOX_NORMALISATION 500000.0

MTVec3D mtGetJoyPosition ();
MTVec3D mtGetJoyUp();
MTVec3D mtGetJoyCenter();

void mtCalcJoyMovement(double interval);

int mtInitJoyControl (char* name);
int mtFinishJoyControl();

#endif
