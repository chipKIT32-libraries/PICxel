/************************************************************************/
/*  PICxel.h  - PIC32 Neopixel Library                                  */
/*                                                                      */
/*  A simple to use library for addressable LEDs like the WS2812 for    */
/*  the PIC32 line of microcontrollers.                                 */
/*                                                                      */
/*  tested supported boards:                                            */
/*    - Digilent UNO32                                                  */
/*    - Digilent UC32                                                   */
/*                                                                      */
/*  This library is protected under the GNU GPL v3.0 license            */
/*  http://www.gnu.org/licenses/                                        */
/*                                                                      */
/*  See PICexl.c for change log                                         */
/************************************************************************/
#ifndef PICxel_H
#define PICxel_H

#include <WProgram.h>
#include <stdint.h>

#define BYTE uint8_t

/* You can choose either GRB (green red blue) or HSV (Hue Saturation Value) */
enum color_mode_t {GRB, HSV};
/* You can choose to have the library allocate pixel color memory with malloc ,
 * (alloc) or allocate them yourself and pass pointers in (noalloc) */
enum memory_mode_t {alloc, noalloc};
/* You can choose to use per-pixel brightness or not. Per-pixel brightness will 
 * require 2.3 times the RAM for the pixel color memory compared to not having
 * per pixel brightness */
enum brightness_mode_t {globalOnly, perPixel};

class PICxel {

private:
//colorArray variables
  /* Binary to keep track of if we're using per-pixel brightness or not */
  brightness_mode_t brightnessMode;
  /* Which color mode (GRB vs HSV) for this object */
  color_mode_t colorMode;
  /* Which memory allocation mode for this object */
  memory_mode_t memoryMode;
  /* Number of LEDs in the strip */
  uint16_t numberOfLEDs;
  /* Size of colorArray (the bits to shift out) in bytes */
  uint16_t colorArraySizeBytes;
  /* Stores the global brightness value, applied to all pixels globally */
  uint8_t globalBrightness;
  /* Stores the R, G and B values that are sent directly out to string of LEDs */
  uint8_t *colorArray;
  /* Stores the original R, G and B values before any brightness values are applied */
  uint8_t *originalColorArray;
  /* Stores the per-pixel brightness values */
  uint8_t *pixelBrightnessArray;

  //pin control variables 
  uint8_t pin;
  volatile uint32_t *portSet;
  volatile uint32_t *portClr;
  uint32_t pinMask;

public:
  //PICxel constructor
  PICxel(
    uint16_t pixelCount,
    uint8_t pin,
    color_mode_t newColorMode = GRB,
    memory_mode_t newMemoryMode = alloc,
    brightness_mode_t newBrightnessMode = globalOnly,
    uint8_t* colorArrayPtr = NULL,
    uint8_t* originalColorArrayPtr = NULL,
    uint8_t* pixelBrightnessArrayPtr = NULL
  );

  // PICxel destructor
  ~PICxel(void);

  //PICxel control functions
  void begin(void);
  void refreshLEDs(void);
  void GRBrefreshLEDs(void);
  void HSVrefreshLEDs(void);
  
  void GRBsetLEDColor(
    uint16_t pixelNumber, 
    uint8_t green, 
    uint8_t red, 
    uint8_t blue, 
    uint8_t pixelBrightness = 0xFF);
  void GRBsetLEDColor(uint16_t pixelNumber, uint32_t color, uint8_t pixelBrightness = 0xFF);
  
  void setPixelBrightness(uint16_t pixelNumber, uint8_t pixelBrightness);
  uint8_t getPixelBrightness(uint16_t pixelNumber);
  
  void HSVsetLEDColor(uint16_t pixelNumber, uint16_t hue, uint8_t sat, uint8_t val);
  void HSVsetLEDColor(uint16_t pixelNumber, uint32_t color);

  void clear();
  void clear(uint8_t num);

  uint32_t colorToScaledColor(uint32_t color);
  uint32_t HSVToColor(unsigned int HSV);
  void updatePixelColorValues(uint16_t pixelNumber);
  
//set class variable functions 
  /* Sets global brightness, applied to all pixels in the string */
  void setBrightness(uint8_t b);

//get class variable functions
  uint16_t getNumberOfLEDs(void);
  bool setNumberOfLEDs(
    uint16_t pixelCount,
    uint8_t* colorArrayPtr = NULL,
    uint8_t* originalColorArrayPtr = NULL,
    uint8_t* pixelBrightnessArrayPtr = NULL
  );
  void setArrayPointer(
    uint8_t* colorArrayPtr,
    uint8_t* originalColorArrayPtr = NULL,
    uint8_t* pixelBrightnessArrayPtr = NULL
  );
  uint8_t* getColorArray(void);
  /* Returns the global brightness value */
  uint8_t getBrightness(void);


  };
#endif // PICxel

/* Hard coded delays
 * See the GRBrefreshLEDs() function in PICxel.cpp for how these are used.
 * Even though it is very bad form to hard code assembly nop delays like this, there just aren't enough 
 * cycles (even at 40MHz - slowest chipKIT board) to do anything else and still meet timing.
 * The key time for Neopixels is T0H, the time of a 0 bit high pulse. That has to be
 * between 200ns and 500ns, no more. We need it to be on the short side of that because
 * when doing hardware debugging in MPALB X, the debugger appears to steal some cycles
 * every now and then, and will extend this pulse a bit. 
 * Now, at 40MHz, the core timer runs at 20MHz, and so 200ns is only 4 core timer
 * ticks, or 8 instructions. That's not enough time to make the decision, check the
 * core timer, and decide when to exit the loop waiting for the core timer to be
 * equal to a value. So hard coded nops it is.
 * Of course, this means there are different numbers of nops that are needed for
 * each possible chipKIT core speed. At this point, I believe we have 40MHz, 48MHz,
 * 80MHz and 200Mhz chipKIT boards. So those values will be used, at compile time,
 * to decide which group of nop macros get used. Unfortunately this does not take
 * into account changes to the core speed at runtime. Maybe that can be a future
 * upgrade to the PICxel library.
 * 
 * It's counter intuitive that some of these delays have no instructions in them.
 * The reason it works out is because of the other overhead in the loop code that
 * takes up time, so the delays don't need to really be as long.
 * 
 * See https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/
 * for a reference on this timing topic.
 * 
 * The datasheet limts for our timing are as follows:
 * T0H must be between 200 and 500 ns
 * T0L must be between 650 and 950 ns
 * T1H must be between 550 and 850 ns
 * T1L must be between 450 and 750 ns
 *
 * As descrbed in the above link, those  limits are more restrive than actually necessary,
 * but they are a good place to aim for.
 *
 * Since the two types of LEDs (WS2812/WS2812S and WS2812B) have slightly different
 * timing requirements, we try to split the difference here. Our goals are:
 * T0H =  220 ns (on the short side so we can debug - debugger adds some extra time every now and then)
 * T0L = 1000 ns (450 minimum, 5000 max)
 * T1H =  800 ns (550 minimum)
 * T1L =  500 ns (450 minimum, 5000 max)
 * 
 * All of these require the optimization level to be at -O2 (default for Arduino IDE)
 *
 * A nice optimization would be to determine the minimum F_CPU frequency where the
 * core timer method could be used, and use it for any frequency above that, and use
 * the hard coded nops for all frequencies below.
 *
 * The number of nops in the defines below are tuned to create the necessary delays in the
 * LED bitstream. However, they take into account the other code that is running before and
 * after.
 */
#define GRB_DELAY(XX) {asm volatile("li $a0,"#XX" \n GRB_delay_%=: \n addi $a0,$a0,-1 \n bnez $a0, GRB_delay_%= \n nop \n " : : : "a0");}

#if F_CPU == 40000000L
  #define GRB_DELAY_T0H   GRB_DELAY(2)    // Goal 220ns,  actual 274ns
  #define GRB_DELAY_T0L   GRB_DELAY(10)   // Goal 1000ns, actual 1052ns
  #define GRB_DELAY_T1H   GRB_DELAY(9)    // Goal 800ns,  actual 800ns
  #define GRB_DELAY_T1L   GRB_DELAY(4)    // Goal 500ns,  actual 600ns
#elif F_CPU == 48000000L
  #define GRB_DELAY_T0H   GRB_DELAY(2)    // Goal 220ns,  actual 226ns
  #define GRB_DELAY_T0L   GRB_DELAY(12)   // Goal 1000ns, actual 1002ns
  #define GRB_DELAY_T1H   GRB_DELAY(11)   // Goal 800ns,  actual 790ns
  #define GRB_DELAY_T1L   GRB_DELAY(5)    // Goal 500ns,  actual 654ns
#elif F_CPU == 80000000L
  #define GRB_DELAY_T0H   GRB_DELAY(5)    // Goal 220ns,  actual 248ns
  #define GRB_DELAY_T0L   GRB_DELAY(21)   // Goal 1000ns, actual 940ns
  #define GRB_DELAY_T1H   GRB_DELAY(20)   // Goal 800ns,  actual 810ns
  #define GRB_DELAY_T1L   GRB_DELAY(10)   // Goal 500ns,  actual 526ns
#elif F_CPU == 200000000L
  #define GRB_DELAY_T0H   GRB_DELAY(14)   // Goal 220ns,  actual 238ns
  #define GRB_DELAY_T0L   GRB_DELAY(63)   // Goal 1000ns, actual 1002ns
  #define GRB_DELAY_T1H   GRB_DELAY(52)   // Goal 800ns,  actual 808ns
  #define GRB_DELAY_T1L   GRB_DELAY(30)   // Goal 500ns,  actual 512ns
#else
  #warning F_CPU is not defined to a known value. PICxcel library not able to create delays properly.
  #define GRB_DELAY_T0H
  #define GRB_DELAY_T0L
  #define GRB_DELAY_T1H
  #define GRB_DELAY_T1L
#endif

/* Note, these need to be measured and converted so F_CPU selects the right set, 
 * as we do above. */
#define HSV_bit_0_delay_0 "nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_0_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_0_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_0_delay_3 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"

#define HSV_bit_1_delay_0 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_1_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_1_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"

#define HSV_bit_2_delay_0 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_2_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_2_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"

#define HSV_bit_3_delay_0 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_3_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_3_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"

#define HSV_bit_4_delay_0 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_4_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_4_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"

#define HSV_bit_5_delay_0 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_5_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_bit_5_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"

#define HSV_universal_delay_1 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_universal_delay_2 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
#define HSV_universal_delay_3 "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
