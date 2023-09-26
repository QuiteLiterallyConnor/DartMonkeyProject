#ifndef _SERVO_EASING_H
#define _SERVO_EASING_H

#define VERSION_SERVO_EASING "3.2.0"
#define VERSION_SERVO_EASING_MAJOR 3
#define VERSION_SERVO_EASING_MINOR 2
#define VERSION_SERVO_EASING_PATCH 0
// The change log is at the bottom of the file


#define VERSION_HEX_VALUE(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define VERSION_SERVO_EASING_HEX  VERSION_HEX_VALUE(VERSION_SERVO_EASING_MAJOR, VERSION_SERVO_EASING_MINOR, VERSION_SERVO_EASING_PATCH)

#define MILLIS_IN_ONE_SECOND 1000L

#define START_EASE_TO_SPEED 5 // If not specified use 5 degree per second. It is chosen so low in order to signal that it was forgotten to specify by program.

#if defined(USE_LEIGHTWEIGHT_SERVO_LIB) && !(defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__))
#error USE_LEIGHTWEIGHT_SERVO_LIB can only be activated for the Atmega328 CPU
#endif

#if defined(ENABLE_EXTERNAL_SERVO_TIMER_HANDLER)
__attribute__((weak)) extern void handleServoTimerInterrupt();
#endif

#if !( defined(__AVR__) || defined(ESP8266) || defined(ESP32) || defined(STM32F1xx) || defined(__STM32F1__) || defined(__SAM3X8E__) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_APOLLO3) || defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040) || defined(TEENSYDUINO))
#warning No periodic timer support existent (or known) for this platform. Only blocking functions and simple example will run!
#endif

/*
 * Include of the appropriate Servo.h file
 */
#if !defined(USE_PCA9685_SERVO_EXPANDER) || defined(USE_SERVO_LIB)
#  if defined(ESP32)
// This does not work in Arduino IDE for step "Generating function prototypes..."
//#    if ! __has_include("ESP32Servo.h")
//#error This ServoEasing library requires the "ESP32Servo" library for running on an ESP32. Please install it via the Arduino library manager.
//#    endif
#include <ESP32Servo.h>

#  elif defined(MEGATINYCORE)
#include <Servo_megaTinyCore.h>

#  elif defined(MEGACOREX)
#    if __has_include("ServoMegaCoreX.h")
#include <ServoMegaCoreX.h> // since Version 1.1.1 of MEGACOREX
#    else
#include <Servo.h>
#    endif

#  else // defined(ESP32)
#    if defined(USE_LEIGHTWEIGHT_SERVO_LIB)
#include "LightweightServo.h"
#      if !defined(MAX_EASING_SERVOS)
#define MAX_EASING_SERVOS 2 // default value for UNO etc.
#      endif
#    else
#include <Servo.h>
#    endif // !defined(USE_LEIGHTWEIGHT_SERVO_LIB)
#  endif // defined(ESP32)
#endif // defined(USE_SERVO_LIB)

#if defined(ARDUINO_ARCH_MBED) // Arduino Nano 33 BLE
#include "mbed.h"
#endif



#if defined(USE_PCA9685_SERVO_EXPANDER)
#  if !defined(MAX_EASING_SERVOS)
#define MAX_EASING_SERVOS 16 // One PCA9685 has 16 outputs. You must MODIFY this, if you have more than one PCA9685 attached!
#  endif // defined(USE_PCA9685_SERVO_EXPANDER)
   #include <Wire.h>
// PCA9685 works with up to 1 MHz I2C frequency
#  if defined(ESP32)
// The ESP32 I2C interferes with the Ticker / Timer library used.
// Even with 100 kHz clock we have some dropouts / NAK's because of sending address again instead of first data.
# define I2C_CLOCK_FREQUENCY 100000 // 200000 does not work for my ESP32 module together with the timer even with external pullups :-(
#  elif defined(ESP8266)
#define I2C_CLOCK_FREQUENCY 400000 // 400000 is the maximum for 80 MHz clocked ESP8266 (I measured real 330000 Hz for this setting)
#  else
#define I2C_CLOCK_FREQUENCY 800000 // 1000000 does not work for my Arduino Nano, maybe because of parasitic breadboard capacities
#  endif
#endif // defined(USE_PCA9685_SERVO_EXPANDER)


#if !defined(MAX_EASING_SERVOS)
#  if defined(MAX_SERVOS)
#define MAX_EASING_SERVOS MAX_SERVOS // =12 use default value from Servo.h for UNO etc.
#  else
#define MAX_EASING_SERVOS 12 // just take default value from Servo.h for UNO etc.
#  endif
#endif // !defined(MAX_EASING_SERVOS)

#if !defined(DEFAULT_PULSE_WIDTH)
#define DEFAULT_PULSE_WIDTH 1500     // default pulse width when servo is attached (from Servo.h)
#endif
#if !defined(REFRESH_INTERVAL)
#define REFRESH_INTERVAL 20000   // // minimum time to refresh servos in microseconds (from Servo.h)
#endif
#if !defined(INVALID_SERVO)
#define INVALID_SERVO    255     // flag indicating an invalid servo index (from Servo.h)
#endif
#define REFRESH_INTERVAL_MICROS REFRESH_INTERVAL         // 20000
#define REFRESH_INTERVAL_MILLIS (REFRESH_INTERVAL/1000)  // 20 - used for delay()
#define REFRESH_FREQUENCY (MILLIS_IN_ONE_SECOND/REFRESH_INTERVAL_MILLIS) // 50


#if !defined(DISABLE_COMPLEX_FUNCTIONS)
//#define DISABLE_COMPLEX_FUNCTIONS
#endif

/*
 * If you need only the linear movement you may define `PROVIDE_ONLY_LINEAR_MOVEMENT`. This saves additional 1540 bytes program memory.
 */
#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
//#define PROVIDE_ONLY_LINEAR_MOVEMENT
#endif

/*
 * If you do not require passing microsecond values as parameter instead of degree values. This saves 128 bytes program memory.
 */
//#define DISABLE_MICROS_AS_DEGREE_PARAMETER

#if !defined(THRESHOLD_VALUE_FOR_INTERPRETING_VALUE_AS_MICROSECONDS)
#define THRESHOLD_VALUE_FOR_INTERPRETING_VALUE_AS_MICROSECONDS  360  // treat values less than 360 as angles in degrees, others are handled as microseconds
#endif

#if !defined(va_arg)
// workaround for STM32
#include <stdarg.h>
#endif

// @formatter:on
// Approximately 10 microseconds per degree
#define DEFAULT_MICROSECONDS_FOR_0_DEGREE     544
#define DEFAULT_MICROSECONDS_FOR_45_DEGREE   (544 + ((2400 - 544) / 4)) // 1008
#define DEFAULT_MICROSECONDS_FOR_90_DEGREE   (544 + ((2400 - 544) / 2)) // 1472
#define DEFAULT_MICROSECONDS_FOR_135_DEGREE (2400 - ((2400 - 544) / 4)) // 1936
#define DEFAULT_MICROSECONDS_FOR_180_DEGREE  2400

// Approximately 2 units per degree
#define DEFAULT_PCA9685_UNITS_FOR_0_DEGREE    111 // 111.411 = 544 us
#define DEFAULT_PCA9685_UNITS_FOR_45_DEGREE  (111 + ((491 - 111) / 4)) // 206
#define DEFAULT_PCA9685_UNITS_FOR_90_DEGREE  (111 + ((491 - 111) / 2)) // 301 = 1472 us
#define DEFAULT_PCA9685_UNITS_FOR_135_DEGREE (491 - ((491 - 111) / 4)) // 369
#define DEFAULT_PCA9685_UNITS_FOR_180_DEGREE  491 // 491.52 = 2400 us

/*
 * Definitions for continuous rotating servo - Values are taken from the Parallax Continuous Rotation Servo manual
 * and rely on a stop value of exactly 1500 microseconds.
 * If the stop value of your servo is NOT exactly 1500 microseconds, you must change the value of MICROSECONDS_FOR_ROTATING_SERVO_STOP.
 * My modified MG90 servo has 1630 and 1400 as max.
 *
 * Use attach(PIN, MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_MAX, MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_MAX, 100, -100);
 * Use write(100) for maximum clockwise and write(-100) for maximum counter clockwise rotation.
 */
#if !defined(MICROSECONDS_FOR_ROTATING_SERVO_STOP)
#define MICROSECONDS_FOR_ROTATING_SERVO_STOP 1500 // Change this value to your servos real stop value
#endif
/*
 * Definitions here are only for convenience. You may freely modify them.
 */
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_MAX (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 200)
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_HALF (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 100)
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_QUARTER (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 50)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_MAX (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 200)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_HALF (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 100)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_QUARTER (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 50)

#if (!(defined(ENABLE_EASE_QUADRATIC) || defined(ENABLE_EASE_CUBIC) || defined(ENABLE_EASE_QUARTIC) \
|| defined(ENABLE_EASE_SINE) || defined(ENABLE_EASE_CIRCULAR) || defined(ENABLE_EASE_BACK) \
|| defined(ENABLE_EASE_USER) \
|| defined(ENABLE_EASE_ELASTIC) || defined(ENABLE_EASE_BOUNCE)|| defined(ENABLE_EASE_PRECISION) \
))
#define ENABLE_EASE_QUADRATIC
#define ENABLE_EASE_CUBIC
#define ENABLE_EASE_QUARTIC
#define ENABLE_EASE_USER
#  if !defined(DISABLE_COMPLEX_FUNCTIONS)
#define ENABLE_EASE_SINE
#define ENABLE_EASE_CIRCULAR
#define ENABLE_EASE_BACK
#define ENABLE_EASE_ELASTIC
#define ENABLE_EASE_BOUNCE
#define ENABLE_EASE_PRECISION
#  endif
#endif

// Offset to decide if the user function returns degrees instead of 0.0 to 1.0.
#define EASE_FUNCTION_DEGREE_INDICATOR_OFFSET       200 // Returns 20 for -180�, 110 for -90�, 200 for 0� and 380 for 180�.
#define EASE_FUNCTION_DEGREE_THRESHOLD              (EASE_FUNCTION_DEGREE_INDICATOR_OFFSET - 180) // allows -180�.
#define EASE_FUNCTION_MICROSECONDS_INDICATOR_OFFSET (EASE_FUNCTION_DEGREE_INDICATOR_OFFSET + 200) // Offset to decide if the user function returns microseconds instead of 0.0 to 1.0. => returns 256 for 0 degree.

/*
 * Values for provided EaseTypes
 * The call style is coded in the upper 2 bits
 */
#define CALL_STYLE_DIRECT       0x00 // == IN
#define CALL_STYLE_IN           0x00
#define CALL_STYLE_OUT          0x40
#define CALL_STYLE_IN_OUT       0x80
#define CALL_STYLE_BOUNCING_OUT_IN  0xC0 // Bouncing has double movement, so double time (half speed) is taken for this modes

#define CALL_STYLE_MASK         0xC0
#define EASE_TYPE_MASK          0x0F

#define EASE_LINEAR             0x00 // No bouncing available

#if defined(ENABLE_EASE_QUADRATIC)
#define EASE_QUADRATIC_IN       0x01
#define EASE_QUADRATIC_OUT      0x41
#define EASE_QUADRATIC_IN_OUT   0x81
#define EASE_QUADRATIC_BOUNCING 0xC1
#endif

#if defined(ENABLE_EASE_CUBIC)
#define EASE_CUBIC_IN           0x02
#define EASE_CUBIC_OUT          0x42
#define EASE_CUBIC_IN_OUT       0x82
#define EASE_CUBIC_BOUNCING     0xC2
#endif

#if defined(ENABLE_EASE_QUARTIC)
#define EASE_QUARTIC_IN         0x03
#define EASE_QUARTIC_OUT        0x43
#define EASE_QUARTIC_IN_OUT     0x83
#define EASE_QUARTIC_BOUNCING   0xC3
#endif

#if defined(ENABLE_EASE_USER)
#define EASE_USER_DIRECT        0x06
#define EASE_USER_IN            0x06
#define EASE_USER_OUT           0x46
#define EASE_USER_IN_OUT        0x86
#define EASE_USER_BOUNCING      0xC6
#endif

#define EASE_DUMMY_MOVE         0x07 // can be used as delay

#if defined(ENABLE_EASE_SINE)
#define EASE_SINE_IN            0x08
#define EASE_SINE_OUT           0x48
#define EASE_SINE_IN_OUT        0x88
#define EASE_SINE_BOUNCING      0xC8
#endif

#if defined(ENABLE_EASE_CIRCULAR)
#define EASE_CIRCULAR_IN        0x09
#define EASE_CIRCULAR_OUT       0x49
#define EASE_CIRCULAR_IN_OUT    0x89
#define EASE_CIRCULAR_BOUNCING  0xC9
#endif

#if defined(ENABLE_EASE_BACK)
#define EASE_BACK_IN            0x0A
#define EASE_BACK_OUT           0x4A
#define EASE_BACK_IN_OUT        0x8A
#define EASE_BACK_BOUNCING      0xCA
#endif

#if defined(ENABLE_EASE_ELASTIC)
#define EASE_ELASTIC_IN         0x0B
#define EASE_ELASTIC_OUT        0x4B
#define EASE_ELASTIC_IN_OUT     0x8B
#define EASE_ELASTIC_BOUNCING   0xCB
#endif

#if defined(ENABLE_EASE_BOUNCE)
// the coded function is an OUT function
#define EASE_BOUNCE_IN          0x4C // call OUT function inverse
#define EASE_BOUNCE_OUT         0x0C // call OUT function direct
#endif

#if defined(ENABLE_EASE_PRECISION)
#define EASE_PRECISION_IN       0x0D // Negative bounce for movings from above (go in to origin)
#define EASE_PRECISION_OUT      0x4D // Positive bounce for movings from below (go out from origin)
#endif

// !!! Must be without comment and closed by @formatter:on !!!
// @formatter:off
extern const char easeTypeLinear[]     PROGMEM;
#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
extern const char easeTypeQuadratic[]  PROGMEM;
extern const char easeTypeCubic[]      PROGMEM;
extern const char easeTypeQuartic[]    PROGMEM;
extern const char easeTypePrecision[]  PROGMEM;
extern const char easeTypeUser[]       PROGMEM;
extern const char easeTypeDummy[]      PROGMEM;
#  if !defined(DISABLE_COMPLEX_FUNCTIONS)
extern const char easeTypeSine[]       PROGMEM;
extern const char easeTypeCircular[]   PROGMEM;
extern const char easeTypeBack[]       PROGMEM;
extern const char easeTypeElastic[]    PROGMEM;
extern const char easeTypeBounce[]     PROGMEM;
#  endif // !defined(DISABLE_COMPLEX_FUNCTIONS)
#endif // !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
// @formatter:on
extern const char *const easeTypeStrings[] PROGMEM;

// some PCA9685 specific constants
#define PCA9685_GENERAL_CALL_ADDRESS 0x00
#define PCA9685_SOFTWARE_RESET          6
#define PCA9685_DEFAULT_ADDRESS      0x40
#define PCA9685_MAX_CHANNELS           16 // 16 PWM channels on each PCA9685 expansion module
#define PCA9685_MODE1_REGISTER        0x0
#define PCA9685_MODE_1_RESTART          7
#define PCA9685_MODE_1_AUTOINCREMENT    5
#define PCA9685_MODE_1_SLEEP            4
#define PCA9685_FIRST_PWM_REGISTER   0x06
#define PCA9685_PRESCALE_REGISTER    0xFE
#if !defined(PCA9685_ACTUAL_CLOCK_FREQUENCY)
// See chapter 2 and 5 of the PCA9685 Datasheet "25 MHz typical internal oscillator requires no external components"
#define PCA9685_ACTUAL_CLOCK_FREQUENCY   25000000L // 25 MHz this is the default frequency
#endif

#define PCA9685_PRESCALER_FOR_20_MS ((PCA9685_ACTUAL_CLOCK_FREQUENCY /(4096L * 50)) - 1) // = 121 / 0x79 at 50 Hz

// to be used as values for parameter bool aStartUpdateByInterrupt
#define START_UPDATE_BY_INTERRUPT           true
#define DO_NOT_START_UPDATE_BY_INTERRUPT    false

/*
 * Size is 46 bytes RAM per servo
 */
class ServoEasing
#if (!defined(USE_PCA9685_SERVO_EXPANDER) || defined(USE_SERVO_LIB)) && !defined(USE_LEIGHTWEIGHT_SERVO_LIB)
        : public Servo
#endif
{
public:

#if defined(USE_PCA9685_SERVO_EXPANDER)
#  if defined(ARDUINO_SAM_DUE)
    ServoEasing(uint8_t aPCA9685I2CAddress, TwoWire *aI2CClass = &Wire1);
#  else
#    if defined(USE_SOFT_I2C_MASTER)
    ServoEasing(uint8_t aPCA9685I2CAddress);
#    else
    ServoEasing(uint8_t aPCA9685I2CAddress, TwoWire *aI2CClass = &Wire);
#    endif
#  endif
    void I2CInit();
    void PCA9685Reset();
    void PCA9685Init();
    void I2CWriteByte(uint8_t aAddress, uint8_t aData);
    void setPWM(uint16_t aPWMOffValueAsUnits);
    void setPWM(uint16_t aPWMOnStartValueAsUnits, uint16_t aPWMPulseDurationAsUnits);
    // main mapping functions for us to PCA9685 Units (20000/4096 = 4.88 us) and back
    int MicrosecondsToPCA9685Units(int aMicroseconds);
    int PCA9685UnitsToMicroseconds(int aPCA9685Units);
#endif // defined(USE_PCA9685_SERVO_EXPANDER)

    ServoEasing();

    uint8_t attach(int aPin);
    uint8_t attach(int aPin, int aInitialDegreeOrMicrosecond);
    uint8_t attachWithTrim(int aPin, int aTrimDegreeOrMicrosecond, int aInitialDegreeOrMicrosecond);
    // Here no units accepted, only microseconds!
    uint8_t attach(int aPin, int aMicrosecondsForServo0Degree, int aMicrosecondsForServo180Degree);
    uint8_t attach(int aPin, int aInitialDegreeOrMicrosecond, int aMicrosecondsForServo0Degree, int aMicrosecondsForServo180Degree);
    uint8_t attachWithTrim(int aPin, int aTrimDegreeOrMicrosecond, int aInitialDegreeOrMicrosecond,
            int aMicrosecondsForServo0Degree, int aMicrosecondsForServo180Degree);
    uint8_t attach(int aPin, int aMicrosecondsForServoLowDegree, int aMicrosecondsForServoHighDegree, int aServoLowDegree,
            int aServoHighDegree);
    uint8_t attach(int aPin, int aInitialDegreeOrMicrosecond, int aMicrosecondsForServoLowDegree,
            int aMicrosecondsForServoHighDegree, int aServoLowDegree, int aServoHighDegree);
    uint8_t attachWithTrim(int aPin, int aTrimDegreeOrMicrosecond, int aInitialDegreeOrMicrosecond,
            int aMicrosecondsForServoLowDegree, int aMicrosecondsForServoHighDegree, int aServoLowDegree, int aServoHighDegree);

    void detach();
    void setReverseOperation(bool aOperateServoReverse); // You should call it before using setTrim, or better use attach function with 6 parameters

    void setTrim(int aTrimDegreeOrMicrosecond, bool aDoWrite = false);
    void _setTrimMicrosecondsOrUnits(int aTrimMicrosecondsOrUnits, bool aDoWrite = false);

#if !defined(DISABLE_MIN_AND_MAX_CONSTRAINTS)
    void setMaxConstraint(int aMaxDegreeOrMicrosecond);
    void setMinConstraint(int aMinDegreeOrMicrosecond);
    void setMinMaxConstraint(int aMinDegreeOrMicrosecond, int aMaxDegreeOrMicrosecond);
#endif

#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
    void setEasingType(uint_fast8_t aEasingType);
    uint_fast8_t getEasingType();

    float callEasingFunction(float aPercentageOfCompletion);            // used in update()

#  if defined(ENABLE_EASE_USER)
    void registerUserEaseInFunction(float (*aUserEaseInFunction)(float aPercentageOfCompletion, void *aUserDataPointer),
            void *aUserDataPointer = NULL);
    void setUserDataPointer(void *aUserDataPointer);
#  endif
#endif

    void write(int aTargetDegreeOrMicrosecond);     // Apply trim and reverse to the value and write it direct to the Servo library.
    void _writeMicrosecondsOrUnits(int aTargetDegreeOrMicrosecond);

    void easeTo(int aTargetDegreeOrMicrosecond);                                   // blocking move to new position using mLastSpeed
    void easeTo(int aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond);  // blocking move to new position using speed
    void easeToD(int aTargetDegreeOrMicrosecond, uint_fast16_t aMillisForMove);    // blocking move to new position using duration

    bool setEaseTo(int aTargetDegreeOrMicrosecond);                                     // shortcut for startEaseTo(..,..,DO_NOT_START_UPDATE_BY_INTERRUPT)
    bool setEaseTo(int aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond);    // shortcut for startEaseTo(..,..,DO_NOT_START_UPDATE_BY_INTERRUPT)
    bool startEaseTo(int aTargetDegreeOrMicrosecond);                             // shortcut for startEaseTo(aDegree, mSpeed, START_UPDATE_BY_INTERRUPT)
    bool startEaseTo(int aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond, bool aStartUpdateByInterrupt =
    START_UPDATE_BY_INTERRUPT);
    bool setEaseToD(int aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond);   // shortcut for startEaseToD(..,..,DO_NOT_START_UPDATE_BY_INTERRUPT)
    bool startEaseToD(int aTargetDegreeOrMicrosecond, uint_fast16_t aMillisForMove, bool aStartUpdateByInterrupt =
    START_UPDATE_BY_INTERRUPT);

    // The float versions
    void write(float aTargetDegreeOrMicrosecond);   // Apply trim and reverse to the value and write it direct to the Servo library.

    void easeTo(float aTargetDegreeOrMicrosecond);                                 // blocking move to new position using mLastSpeed
    void easeTo(float aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond);  // blocking move to new position using speed
    void easeToD(float aTargetDegreeOrMicrosecond, uint_fast16_t aMillisForMove);    // blocking move to new position using duration

    bool setEaseTo(float aTargetDegreeOrMicrosecond);                                     // shortcut for startEaseTo(..,..,DO_NOT_START_UPDATE_BY_INTERRUPT)
    bool setEaseTo(float aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond);    // shortcut for startEaseTo(..,..,DO_NOT_START_UPDATE_BY_INTERRUPT)
    bool startEaseTo(float aTargetDegreeOrMicrosecond);                           // shortcut for startEaseTo(aDegree, mSpeed, START_UPDATE_BY_INTERRUPT)
    bool startEaseTo(float aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond, bool aStartUpdateByInterrupt =
    START_UPDATE_BY_INTERRUPT);
    bool setEaseToD(float aTargetDegreeOrMicrosecond, uint_fast16_t aDegreesPerSecond);   // shortcut for startEaseToD(..,..,DO_NOT_START_UPDATE_BY_INTERRUPT)
    bool startEaseToD(float aTargetDegreeOrMicrosecond, uint_fast16_t aMillisForMove, bool aStartUpdateByInterrupt =
    START_UPDATE_BY_INTERRUPT);

    bool noMovement(uint_fast16_t aMillisToWait);                                       // stay at the position for aMillisToWait

    void setSpeed(uint_fast16_t aDegreesPerSecond);                            // This speed is taken if no speed argument is given.
    uint_fast16_t getSpeed();

    void stop();
    void pause();
    void resumeWithInterrupts();
    void resumeWithoutInterrupts();
    bool update();

    void setTargetPositionReachedHandler(void (*aTargetPositionReachedHandler)(ServoEasing*));

    int getCurrentAngle();
    int getCurrentMicroseconds();
    int getEndMicrosecondsOrUnits();
    int getEndMicrosecondsOrUnitsWithTrim();
    int getDeltaMicrosecondsOrUnits();
    int getMillisForCompleteMove();
    bool isMoving();
    bool isMovingAndCallYield() __attribute__ ((deprecated ("Replaced by isMoving(). Often better to use areInterruptsActive() instead.")));

    int MicrosecondsOrUnitsToDegree(int aMicrosecondsOrUnits);
    int MicrosecondsToDegree(int aMicroseconds);
    int MicrosecondsOrUnitsToMicroseconds(int aMicrosecondsOrUnits);
    int DegreeOrMicrosecondToMicrosecondsOrUnits(int aDegreeOrMicrosecond);
    int DegreeOrMicrosecondToMicrosecondsOrUnits(float aDegreeOrMicrosecond);
//    int DegreeToMicrosecondsOrUnits(float aDegree);
    int DegreeToMicrosecondsOrUnitsWithTrimAndReverse(int aDegree);

    void synchronizeServosAndStartInterrupt(bool doUpdateByInterrupt);

    void print(Print *aSerial, bool doExtendedOutput = true); // Print dynamic and static info
    void printDynamic(Print *aSerial, bool doExtendedOutput = true);
    void printStatic(Print *aSerial);

    static void printEasingType(Print *aSerial, uint_fast8_t aEasingType);

    /*
     * Included easing functions
     */
    static float QuadraticEaseIn(float aPercentageOfCompletion);
    static float CubicEaseIn(float aPercentageOfCompletion);
    static float QuarticEaseIn(float aPercentageOfCompletion);
    static float SineEaseIn(float aPercentageOfCompletion);
    static float CircularEaseIn(float aPercentageOfCompletion);
    static float BackEaseIn(float aPercentageOfCompletion);
    static float ElasticEaseIn(float aPercentageOfCompletion);
    // Non symmetric function
    static float EaseOutBounce(float aPercentageOfCompletion);
    // Special non static function
    float LinearWithQuadraticBounce(float aPercentageOfCompletion);

    /*
     * Convenience function
     */
#if defined(__AVR__)
    bool InitializeAndCheckI2CConnection(Print *aSerial); // Using Print class saves 95 bytes flash
#else
    bool InitializeAndCheckI2CConnection(Stream *aSerial); // Print class has no flush() here
#endif

    /*
     * Static functions
     */
    static bool areInterruptsActive(); // The recommended test if at least one servo is moving yet.

    /**
     * Internally only microseconds (or units (= 4.88 us) if using PCA9685 expander) and not degree are used to speed up things.
     * Other expander or libraries can therefore easily be added.
     */
    volatile int mCurrentMicrosecondsOrUnits; ///< set by write() and _writeMicrosecondsOrUnits(). Required as start for next move and to avoid unnecessary writes.
    int mStartMicrosecondsOrUnits;  ///< Only used with millisAtStartMove to compute currentMicrosecondsOrUnits in update()
    int mEndMicrosecondsOrUnits;    ///< Only used once as last value if movement was finished to provide exact end position.
    int mDeltaMicrosecondsOrUnits;  ///< end - start

    /**
     * max speed is 450 degree/sec for SG90 and 540 degree/second for MG90 servos -> see speedTest.cpp
     */
    uint_fast16_t mSpeed; ///< in DegreesPerSecond - only set by setSpeed(int16_t aSpeed);

#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
    uint8_t mEasingType; // EASE_LINEAR, EASE_QUADRATIC_IN_OUT, EASE_CUBIC_IN_OUT, EASE_QUARTIC_IN_OUT
#  if defined(ENABLE_EASE_USER)
    void *UserDataPointer;
    float (*mUserEaseInFunction)(float aPercentageOfCompletion, void *aUserDataPointer);
#  endif
#endif

    volatile bool mServoMoves;

#if defined(USE_PCA9685_SERVO_EXPANDER)
#  if defined(USE_SERVO_LIB)
    // Here we can have both types of servo connections
    bool mServoIsConnectedToExpander; // to distinguish between different using microseconds or PWM units and appropriate write functions
#  endif
    uint8_t mPCA9685I2CAddress;
#  if !defined(USE_SOFT_I2C_MASTER)
    TwoWire *mI2CClass;
#  endif
#endif
    uint8_t mServoPin; ///< pin number / port number of PCA9685 [0-15] or NO_SERVO_ATTACHED_PIN_NUMBER - at least required for Lightweight Servo Library

    uint8_t mServoIndex; ///< Index in sServoArray or INVALID_SERVO if error while attach() or if detached

    uint32_t mMillisAtStartMove;
    uint_fast16_t mMillisForCompleteMove;
#if !defined(DISABLE_PAUSE_RESUME)
    bool mServoIsPaused;
    uint32_t mMillisAtStopMove;
#endif

    /**
     * Reverse means, that values for 180 and 0 degrees are swapped by: aValue = mServo180DegreeMicrosecondsOrUnits - (aValue - mServo0DegreeMicrosecondsOrUnits)
     * Be careful, if you specify different end values, it may not behave, as you expect.
     * For this case better use the attach function with 5 parameter.
     */
    bool mOperateServoReverse; ///< true -> direction is reversed
#if !defined(DISABLE_MIN_AND_MAX_CONSTRAINTS)
    int mMaxMicrosecondsOrUnits; ///< Max value checked at _writeMicrosecondsOrUnits(), before trim and reverse is applied
    int mMinMicrosecondsOrUnits; ///< Min value checked at _writeMicrosecondsOrUnits(), before trim and reverse is applied
#endif
    int mTrimMicrosecondsOrUnits; ///< This value is always added by the function _writeMicrosecondsOrUnits() to the requested degree/units/microseconds value

    /**
     * Values contain always microseconds except for servos connected to a PCA9685 expander, where they contain PWM units.
     * Values are set exclusively by attach(), and here it is determined if they contain microseconds or PWM units.
     */
    int mServo0DegreeMicrosecondsOrUnits;
    int mServo180DegreeMicrosecondsOrUnits;

    void (*TargetPositionReachedHandler)(ServoEasing*);  ///< Is called any time when target servo position is reached


    static volatile bool sInterruptsAreActive; ///< true if interrupts are still active, i.e. at least one Servo is moving with interrupts.
    static uint_fast8_t sServoArrayMaxIndex; ///< maximum index of an attached servo in sServoArray[]
    static ServoEasing *ServoEasingArray[MAX_EASING_SERVOS];
    static float ServoEasingNextPositionArray[MAX_EASING_SERVOS];

#define areInterruptsActive() ServoEasing::areInterruptsActive()
#define sServoArray ServoEasing::ServoEasingArray
#define sServoNextPositionArray ServoEasing::ServoEasingNextPositionArray

};

/*
 * Functions working on all servos in the list
 */
void writeAllServos(int aTargetDegreeOrMicrosecond);
void setSpeedForAllServos(uint_fast16_t aDegreesPerSecond);
#if defined(va_arg)
void setIntegerDegreeForAllServos(uint_fast8_t aNumberOfValues, va_list *aDegreeValues);
void setFloatDegreeForAllServos(uint_fast8_t aNumberOfValues, va_list *aDegreeValues);
#endif
#if defined(va_start)
void setDegreeForAllServos(uint_fast8_t aNumberOfValues, ...) __attribute__ ((deprecated ("Please use setIntegerDegreeForAllServos().")));
void setIntegerDegreeForAllServos(uint_fast8_t aNumberOfValues, ...);
void setFloatDegreeForAllServos(uint_fast8_t aNumberOfValues, ...);
#endif

bool setEaseToForAllServos();
bool setEaseToForAllServos(uint_fast16_t aDegreesPerSecond);
bool setEaseToDForAllServos(uint_fast16_t aMillisForMove);
void setEaseToForAllServosSynchronizeAndStartInterrupt();
void setEaseToForAllServosSynchronizeAndStartInterrupt(uint_fast16_t aDegreesPerSecond);
void synchronizeAllServosAndStartInterrupt(bool aStartUpdateByInterrupt = START_UPDATE_BY_INTERRUPT);

#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
void setEasingTypeForAllServos(uint_fast8_t aEasingType);
void setEasingTypeForMultipleServos(uint_fast8_t aNumberOfServos, uint_fast8_t aEasingType);
#endif

// blocking wait functions
void updateAndWaitForAllServosToStop();
bool delayAndUpdateAndWaitForAllServosToStop(unsigned long aMillisDelay, bool aTerminateDelayIfAllServosStopped = false);
void setEaseToForAllServosSynchronizeAndWaitForAllServosToStop();
void setEaseToForAllServosSynchronizeAndWaitForAllServosToStop(uint_fast16_t aDegreesPerSecond);
void synchronizeAndEaseToArrayPositions() __attribute__ ((deprecated ("Please use setEaseToForAllServosSynchronizeAndWait().")));
void synchronizeAndEaseToArrayPositions(uint_fast16_t aDegreesPerSecond) __attribute__ ((deprecated ("Please use setEaseToForAllServosSynchronizeAndWait().")));
void synchronizeAllServosStartAndWaitForAllServosToStop();

void printArrayPositions(Print *aSerial);
bool isOneServoMoving();
void stopAllServos();
void resumeWithInterruptsAllServos();
void resumeWithoutInterruptsAllServos();
bool updateAllServos();

void enableServoEasingInterrupt();
#if defined(__AVR_ATmega328P__)
void setTimer1InterruptMarginMicros(uint16_t aInterruptMarginMicros);
#endif
void disableServoEasingInterrupt();

int clipDegreeSpecial(uint_fast8_t aDegreeToClip);

//extern float (*sEaseFunctionArray[])(float aPercentageOfCompletion);

// Static convenience function
#if defined(__AVR__)
bool checkI2CConnection(uint8_t aI2CAddress, Print *aSerial); // Using Print class saves 95 bytes flash
#else
bool checkI2CConnection(uint8_t aI2CAddress, Stream *aSerial); // Print class has no flush() here
#endif

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif


#if !defined(_SERVO_EASING_HPP) && !defined(SUPPRESS_HPP_WARNING)
#warning You probably must change the line #include "ServoEasing.h" to #include "ServoEasing.hpp" in your ino file or define SUPPRESS_HPP_WARNING before the include to suppress this warning.
#endif

#endif // _SERVO_EASING_H
