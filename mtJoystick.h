/*
 * mtJoystic.h
 *
 * Implements Joystick control for Linux newer than 2.2.x.
 * This implementation can handle a maxium of:
 * - 256 axes
 * - 256 Buttons
 *
 * Allows to read the following attributes:
 * - Device name
 * - Axis count
 * - Button count
 * - Axis status (-32768/32767) (signed short)
 * - Button status (0/1)
 *
 * Reading of those values is event based.
 *
 * Author: Maurice Tollmien, Mervyn McCreight
 *
 * Last change: 10.09.2016
 */

#define JOY_NAME_LENGTH 80


/**
 * Internal data structure for complete joystick information.
 */
typedef struct {
    int fd;                                 /* Data stream */
    int driverVersion;                      /* Driver version */
    char axisNumber;                        /* Axis count */
    char buttonNumber;                      /* Button count */
    char name[JOY_NAME_LENGTH];             /* Device name */
    short * axisValues;                     /* Dynamic array for axis values */
    short * buttonValues;                   /* Dynamic array for button values */
} JoystickDevice;


/**
 * Prints the following Joystick information to stdout:
 * - Device name
 * - Driver version
 * - Axis count
 * - Button count
 */
void printJoystickInformation();

/**
 * Starts the device connection and allocates memory for the internal data.
 * Returns 0 on failure. Otherwise success.
 */
int startDeviceConnection(const char * devname);

/**
 * Nicely ends the device connection and frees memory.
 * Returns 0 on failure. Otherwise success.
 */
int endDeviceConnection();

/**
 * Reads and interprets the Joystick events.
 * This should be called continiously !
 */
void handleJoystickEvents();

/**
 * Reads one axis parameter and writes it into 'value'.
 * Returns 0 on failure. Otherwise success.
 */
int getAxisValue(int axisNumber, short * value);

/**
 * Reads one button value and writes it into 'value'.
 * Returns 0 on failure. Otherwise success.
 */
int getButtonValue(int buttonNumber, short * value);

/**
 * Calibrates the joystick itself.
 * This can also be done, using the Linxu programs: jstest-gtk or jscal
 * Returns 0 on failure. Otherwise success.
 */
int setCalibrationCoefficients(int a,int b,int c,int d,int t,int prec);
