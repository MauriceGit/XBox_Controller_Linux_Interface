/*
 * mtJoystick.h
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


/* System header */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include "mtJoystick.h"


/* Array of open device streams! */
JoystickDevice g_device;

/**
 * Changes the correction coefficients to calibrate a Joystick.
 * This can also be done, using the Linux programs: jstest-gtk or jscal
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

    /* set correction */
    if (ioctl(g_device.fd, JSIOCSCORR, &corr)) {
        return 0;
    }

    return 1;
}

/**
 * Frees the allocated memory for any device data.
 *
 * @return 0 on failure. Otherwise success.
 */
static int freeDeviceMemory() {

    free(g_device.axisValues);
    free(g_device.buttonValues);

    return (g_device.axisValues == NULL) && (g_device.buttonValues == NULL);
}

/**
 * Opens the joystick device stream for reading.
 * @param devname Path to device (Most common: /dev/input/js0)
 *
 * @return 0 on failure. Otherwise success.
 */
static int openDeviceStream(const char * devname) {
    /*
     * Opens device in blocking mode.
     * If the return value is invalid or no device is connected,
     * -1 is returned as error value.
     */
    g_device.fd = open(devname, O_RDONLY);

    if (g_device.fd == -1) {
        fprintf(stderr, "Error opening device: %s\n", devname);
        return 0;
    }

    /*
     * Changes into a NON-BLOCKING-MODE.
     * A "read" is now put onto the driver stack and doesn't wait
     * for a triggered event.
     */
    fcntl(g_device.fd, F_SETFL, O_NONBLOCK);

    return 1;
}

/**
 * Determines the following device data and saves them internally:
 * - Axis count (IOCTL operation: JSIOCGAXES)
 * - Button count (IOCTL operation: JSIOCGBUTTONS)
 * - Device name (IOCTL operation: JSIOCGNAME(length))
 * - Driver version (IOCTL operation: JSIOCGVERSION)
 *
 * @return 0 on failure. Otherwise success.
 */
static int getJoystickInformation() {
    /* the device must be opened ! */
    if (g_device.fd == -1) {
        fprintf(stderr, "getJoystickInformation: Device not opened!\n");
        return 0;
    }

    /* Read device name and save it */
    if (ioctl (g_device.fd, JSIOCGNAME(JOY_NAME_LENGTH), &g_device.name) < 0) {
        strncpy(g_device.name, "Unknown Device", sizeof(g_device.name));
    }

    /* Read and save available Axis count */
    ioctl(g_device.fd, JSIOCGAXES, &g_device.axisNumber);

    /* Read and save available Button count */
    ioctl(g_device.fd, JSIOCGBUTTONS, &g_device.buttonNumber);

    /* Read and save driver version */
    ioctl(g_device.fd, JSIOCGVERSION, &g_device.driverVersion);

    return 1;
}

/**
 * Allocates memory for the dynamic arrays for the data structure to manage
 * axes and buttons.
 *
 * @return 0 on failure. Otherwise success.
 */
static int allocateDeviceValueMemory() {
    /* dynamic array of axis values */
    g_device.axisValues = (short *)calloc(g_device.axisNumber, sizeof(short));

    if (g_device.axisValues == NULL) {
        fprintf(stderr, "allocateDeviceValueMemory: Error allocating memory for the axis!\n");
        return 0;
    }

    /* dynamic Array for button values */
    g_device.buttonValues = (short *)calloc(g_device.buttonNumber, sizeof(short));

    if (g_device.buttonValues == NULL) {
        fprintf(stderr, "allocateDeviceValueMemory: Error allocating memory for the buttons!\n");
        return 0;
    }

    return 1;
}

/**
 * Starts a connection to the given device and allocates the correspondent memory.
 *
 * @return 0 on failure. Otherwise success.
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
 * Closes the connection to a device and frees memory.
 *
 * @return 0 on failure. Otherwise success.
 */
int endDeviceConnection() {
    close(g_device.fd);
    return freeDeviceMemory();
}

/**
 * Prints the following Joystick information to stdout:
 * - Device name
 * - Driver version
 * - Axis count
 * - Button count
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
 * Handles one Joystick event.
 *
 * @param e the Joystick event.
 */
static void processEvent(struct js_event e) {
    /*
     * Interprets event.
     * The JS_EVENT_INIT bits will be deactivated as we do not want to distinguish
     * between synthetic and real events.
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
 * Reads a joystick event and updates the correspondent values
 * accordingly.
 *
 * Strukture of js_events:
 *
 * time     :   unsigned int    - event timestamp in ms
 * value    :   short           - the new value
 * type     :   char            - the type of the event
 * number   :   char            - number of the relevant axis/button
 *
 * possible values for 'type' are:
 *
 * JS_EVENT_BUTTON (0x01)       - a button event
 * JS_EVENT_AXIS (0x02)         - an axis event
 * JS_EVENT_INIT (0x80)         - Initial status of the device
 */
void handleJoystickEvents() {
    struct js_event jsEvent;

    /* read all events from the driver stack! */
    while (read(g_device.fd, &jsEvent, sizeof(struct js_event)) > 0) {
        processEvent(jsEvent);
    }
}

/**
 * Reads the value of an axis to 'value'.
 *
 * @param axisNumber the relevant axis to be read.
 * @param value the read value of the axis
 *
 * @return 0 on failure. Otherwise success.
 */
int getAxisValue(int axisNumber, short * value) {
    if (axisNumber <= g_device.axisNumber) {
        (*value) = g_device.axisValues[axisNumber];
        return 1;
    }

    fprintf(stderr, "getAxisValue: Invalid axis number\n");
    (*value) = 0;
    return 0;
}


/**
 * Reads the value of a button to 'value'.
 *
 * @param buttonNumber the relevant button to be read.
 * @param value the read button value.
 *
 * @return 0 on failure. Otherwise success.
 */
int getButtonValue(int buttonNumber, short * value) {
    if (buttonNumber <= g_device.buttonNumber) {
        (*value) = g_device.buttonValues[buttonNumber];
        return 1;
    }

    fprintf(stderr, "getButtonValue: Invalid button number\n");
    (*value) = 0;
    return 0;
}

/*
 * MODULE TEST
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
