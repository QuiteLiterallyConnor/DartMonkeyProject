#ifndef _SERVO_EASING_H
#define _SERVO_EASING_H

#define VERSION_SERVO_EASING "3.2.0"
#define VERSION_SERVO_EASING_MAJOR 3
#define VERSION_SERVO_EASING_MINOR 2
#define VERSION_SERVO_EASING_PATCH 0
#define VERSION_HEX_VALUE(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define VERSION_SERVO_EASING_HEX  VERSION_HEX_VALUE(VERSION_SERVO_EASING_MAJOR, VERSION_SERVO_EASING_MINOR, VERSION_SERVO_EASING_PATCH)
#define MILLIS_IN_ONE_SECOND 1000L
#define START_EASE_TO_SPEED 5 
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

#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
//#define PROVIDE_ONLY_LINEAR_MOVEMENT
#endif

#if !defined(THRESHOLD_VALUE_FOR_INTERPRETING_VALUE_AS_MICROSECONDS)
#define THRESHOLD_VALUE_FOR_INTERPRETING_VALUE_AS_MICROSECONDS  360  // treat values less than 360 as angles in degrees, others are handled as microseconds
#endif

#if !defined(va_arg)
// workaround for STM32
#include <stdarg.h>
#endif

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

#if !defined(MICROSECONDS_FOR_ROTATING_SERVO_STOP)
#define MICROSECONDS_FOR_ROTATING_SERVO_STOP 1500 // Change this value to your servos real stop value
#endif

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
#define EASE_BOUNCE_IN          0x4C // call OUT function inverse
#define EASE_BOUNCE_OUT         0x0C // call OUT function direct
#endif

#if defined(ENABLE_EASE_PRECISION)
#define EASE_PRECISION_IN       0x0D // Negative bounce for movings from above (go in to origin)
#define EASE_PRECISION_OUT      0x4D // Positive bounce for movings from below (go out from origin)
#endif

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
#  endif
#endif
extern const char *const easeTypeStrings[] PROGMEM;
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
#define PCA9685_ACTUAL_CLOCK_FREQUENCY   25000000L // 25 MHz this is the default frequency
#endif
#define PCA9685_PRESCALER_FOR_20_MS ((PCA9685_ACTUAL_CLOCK_FREQUENCY /(4096L * 50)) - 1) // = 121 / 0x79 at 50 Hz
#define START_UPDATE_BY_INTERRUPT           true
#define DO_NOT_START_UPDATE_BY_INTERRUPT    false


class ServoEasing
#if (!defined(USE_PCA9685_SERVO_EXPANDER) || defined(USE_SERVO_LIB)) && !defined(USE_LEIGHTWEIGHT_SERVO_LIB)
        : public Servo
#endif
{
public:

    ServoEasing();

    uint8_t attach(int aPin);
    uint8_t attach(int aPin, int aInitialDegreeOrMicrosecond);
    uint8_t attachWithTrim(int aPin, int aTrimDegreeOrMicrosecond, int aInitialDegreeOrMicrosecond);
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
    int DegreeToMicrosecondsOrUnitsWithTrimAndReverse(int aDegree);
    void print(Print *aSerial, bool doExtendedOutput = true); // Print dynamic and static info
    void printDynamic(Print *aSerial, bool doExtendedOutput = true);
    void printStatic(Print *aSerial);
    static void printEasingType(Print *aSerial, uint_fast8_t aEasingType);
    static float QuadraticEaseIn(float aPercentageOfCompletion);
    static float CubicEaseIn(float aPercentageOfCompletion);
    static float QuarticEaseIn(float aPercentageOfCompletion);
    static float SineEaseIn(float aPercentageOfCompletion);
    static float CircularEaseIn(float aPercentageOfCompletion);
    static float BackEaseIn(float aPercentageOfCompletion);
    static float ElasticEaseIn(float aPercentageOfCompletion);
    static float EaseOutBounce(float aPercentageOfCompletion);
    float LinearWithQuadraticBounce(float aPercentageOfCompletion);
    static bool areInterruptsActive(); // The recommended test if at least one servo is moving yet.
    volatile int mCurrentMicrosecondsOrUnits; ///< set by write() and _writeMicrosecondsOrUnits(). Required as start for next move and to avoid unnecessary writes.
    int mStartMicrosecondsOrUnits;  ///< Only used with millisAtStartMove to compute currentMicrosecondsOrUnits in update()
    int mEndMicrosecondsOrUnits;    ///< Only used once as last value if movement was finished to provide exact end position.
    int mDeltaMicrosecondsOrUnits;  ///< end - start
    uint_fast16_t mSpeed; ///< in DegreesPerSecond - only set by setSpeed(int16_t aSpeed);
#if !defined(PROVIDE_ONLY_LINEAR_MOVEMENT)
    uint8_t mEasingType;
#  if defined(ENABLE_EASE_USER)
    void *UserDataPointer;
    float (*mUserEaseInFunction)(float aPercentageOfCompletion, void *aUserDataPointer);
#  endif
#endif
    volatile bool mServoMoves;
    uint8_t mServoPin; ///< pin number / port number of PCA9685 [0-15] or NO_SERVO_ATTACHED_PIN_NUMBER - at least required for Lightweight Servo Library
    uint8_t mServoIndex; ///< Index in sServoArray or INVALID_SERVO if error while attach() or if detached
    uint32_t mMillisAtStartMove;
    uint_fast16_t mMillisForCompleteMove;
#if !defined(DISABLE_PAUSE_RESUME)
    bool mServoIsPaused;
    uint32_t mMillisAtStopMove;
#endif
    bool mOperateServoReverse; ///< true -> direction is reversed
#if !defined(DISABLE_MIN_AND_MAX_CONSTRAINTS)
    int mMaxMicrosecondsOrUnits; ///< Max value checked at _writeMicrosecondsOrUnits(), before trim and reverse is applied
    int mMinMicrosecondsOrUnits; ///< Min value checked at _writeMicrosecondsOrUnits(), before trim and reverse is applied
#endif
    int mTrimMicrosecondsOrUnits; ///< This value is always added by the function _writeMicrosecondsOrUnits() to the requested degree/units/microseconds value
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

void writeAllServos(int aTargetDegreeOrMicrosecond);
void setSpeedForAllServos(uint_fast16_t aDegreesPerSecond);
#if defined(va_arg)
#endif
#if defined(va_start)
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
void disableServoEasingInterrupt();
int clipDegreeSpecial(uint_fast8_t aDegreeToClip);

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif


#endif // _SERVO_EASING_H
