#include <FastLED.h>


// =======================
// Arduino Pin Definitions
// =======================

// Digital Clock Pin For APA102 (Dotstar) Timing
/* #define CLOCK_PIN 23  // SPI Clock Pin (Arduino Zero) */
#define CLOCK_PIN 13  // SPI Clock Pin (Teensy 3.2)

// Digital Data Pin For APA102 (Dotstar) Data
/* #define DATA_PIN 24  //SPI Data Pin (Arduino Zero) */
#define DATA_PIN 11  //SPI Data Pin (Teensy 3.2)

// Analog Pin used to add entropy to random seed.
// NOTE: Nothing should be plugged into this pin.
#define RANDOM_ANALOG_PIN 0


// ================
// APA102/Dotstar Definitions
// ================

// RGB Order for LED strips
#define RGB_ORDER BGR


// ===============
// HSV Definitions
// ===============

// Brightness/Value
#define DEFAULT_BRIGHTNESS 191
#define MIN_BRIGHTNESS 63
#define MAX_BRIGHTNESS 255

// Saturation
#define DEFAULT_SAT 223
#define MIN_SAT 191
#define MAX_SAT 255


// ==========================
// LED Strip Size Definitions
// ==========================

// Number of LED's On Strip
#define NUM_LEDS 58

// Number of Color Groups throughout LED Strip
// Works best if it cleanly divides into NUM_LEDS or has a small remainder
#define NUM_GROUPS 5

// Size of a color group for certain animations.
#define LED_GROUP_SIZE ((NUM_LEDS) / (NUM_GROUPS))


// ========================
// Time Syncing Definitions
// ========================

// Frames Per Second
#define FPS 120

// Amount of time to show each pattern
#define PATTERN_SECS 24

// Amount of time to show each palette
#define PALETTE_SECS ((PATTERN_SECS) / 4)

// Amount of time between each hue in rainbow
#define RAINBOW_MILLIS 20


// ========
// Typedefs
// ========

// Typedef to make function pointer array easier to type.
typedef void (*PatternArray[])();

// Typedef for defining the animation type for a given pattern function.
// Generally used in a switch statement to add variety to existing animations.
typedef enum {
    RAINBOW_ANIM, GROUP_ANIM, RGB_PALETTE_ANIM, HSV_PALETTE_ANIM
} AnimationType;


// ================
// Global Variables
// ================

// The LED's, man!
CRGB leds[NUM_LEDS];

// Designates whether the group size is even or odd.
const boolean IS_GROUP_SIZE_EVEN = (LED_GROUP_SIZE) % 2 == 0;

// Designates the "center" index of a given group.
// For even groups sizes, this is "to the right" of where the center would be.
const uint16_t GROUP_CENTER = (LED_GROUP_SIZE) / 2;

uint8_t currentPattern;     // Index of currently selected pattern
uint8_t currentHSVPalette;  // Index of currently selected HSV Palette
uint8_t currentRGBPalette;  // Index of currently selected RGB Palette
uint8_t rainbowHue = 0;     // Global value for cycling through hues


// ============
// HSV Palettes
// ============

/*
 * Chez Cargot Colors
 *
 * A variety of dynamic colors that move across the color hue wheel in 1/6ths
 * with a few colors wrapping around, having a mixed sorting order that
 * encourages similar colors to be spread out.
 */
const CHSVPalette16 ChezCargotColorsPalette_p(
    CHSV(0, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(127, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(42, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(170, DEFAULT_SAT, MAX_BRIGHTNESS),

    CHSV(234, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(191, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(64, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(191, DEFAULT_SAT, MAX_BRIGHTNESS),

    CHSV(106, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(234, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(42, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(255, DEFAULT_SAT, MAX_BRIGHTNESS),

    CHSV(128, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(255, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(170, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(42, DEFAULT_SAT, MAX_BRIGHTNESS)
);

/*
 * SK Sunset Palette
 *
 * A variety of colors picked from a photograph of a sunset.
 */
const CHSVPalette16 SKSunsetPalette_p(
    CHSV(6, 161, 223),
    CHSV(217, 178, 141),
    CHSV(8, 156, 247),
    CHSV(163, 226, 71),
    CHSV(251, 127, 231),
    CHSV(163, 226, 100),
    CHSV(233, 152, 197),
    CHSV(169, 217, 121),
    CHSV(220, 173, 154),
    CHSV(207, 196, 112),
    CHSV(203, 207, 96),
    CHSV(253, 120, 237),
    CHSV(163, 255, 89),
    CHSV(242, 135, 216),
    CHSV(169, 217, 121),
    CHSV(233, 156, 218)
);

/*
 * SK Saturated Sunset Palette
 *
 * Features the same colors as the sunset palette, but with higher saturation.
 */
const CHSVPalette16 SKSaturatedSunsetPalette_p(
    CHSV(6, DEFAULT_SAT, 223),
    CHSV(217, DEFAULT_SAT, 141),
    CHSV(8, DEFAULT_SAT, 247),
    CHSV(163, DEFAULT_SAT, 71),
    CHSV(251, DEFAULT_SAT, 231),
    CHSV(163, DEFAULT_SAT, 100),
    CHSV(233, DEFAULT_SAT, 197),
    CHSV(169, DEFAULT_SAT, 121),
    CHSV(220, DEFAULT_SAT, 154),
    CHSV(207, DEFAULT_SAT, 112),
    CHSV(203, DEFAULT_SAT, 96),
    CHSV(253, DEFAULT_SAT, 237),
    CHSV(163, DEFAULT_SAT, 89),
    CHSV(242, DEFAULT_SAT, 216),
    CHSV(169, DEFAULT_SAT, 121),
    CHSV(233, DEFAULT_SAT, 218)
);


/*
 * SK Blue, Purple, and Red Palette
 *
 * Features a variety of blue and purple colors, with notes of de-saturated
 * reds. Was initially inspired by a photo of a flower, and was worked
 * extensively from there.
 */
const CHSVPalette16 SKBPRPalette_p(
    CHSV(142, 236, 254),
    CHSV(146, 236, 254),
    CHSV(151, 236, 254),
    CHSV(154, 236, 254),
    CHSV(159, 236, 254),
    CHSV(166, 236, 254),
    CHSV(173, 236, 254),
    CHSV(184, 236, 254),
    CHSV(193, 236, 254),
    CHSV(204, 170, 242),
    CHSV(210, 170, 242),
    CHSV(210, 156, 242),
    CHSV(234, 156, 242),
    CHSV(239, 163, 241),
    CHSV(246, 188, 248),
    CHSV(251, 188, 248)
);


/*
 * SK Grass Palette
 *
 * Features a variety of colors found on grass, including many pigments of
 * green with hints of yellow.
 */
const CHSVPalette16 SKGrassPalette_p(
    CHSV(112, 236, 109),
    CHSV(112, 221, 109),
    CHSV(112, 178, 70),
    CHSV(62, 151, 62),
    CHSV(50, 255, 70),
    CHSV(80, 222, 93),
    CHSV(90, 252, 132),
    CHSV(70, 252, 161),
    CHSV(65, 210, 180),
    CHSV(80, 190, 208),
    CHSV(66, 190, 208),
    CHSV(66, 211, 232),
    CHSV(66, 172, 249),
    CHSV(56, 172, 249),
    CHSV(46, 187, 255),
    CHSV(46, 187, 255)
);


/*
 * SK Candy Land Palette
 *
 * A variety of bright and saturated colors that were very bright and candy
 * like - specifically targeting high saturation to look good with the LED's.
 */
const CHSVPalette16 SKFiveHSVPalette_p(
    CHSV(43, 255, 255),
    CHSV(85, 255, 255),
    CHSV(124, 255, 255),
    CHSV(158, 255, 255),
    CHSV(117, 204, 255),
    CHSV(190, 204, 255),
    CHSV(43, 255, 255),
    CHSV(85, 255, 255),
    CHSV(124, 255, 255),
    CHSV(158, 255, 255),
    CHSV(117, 204, 255),
    CHSV(190, 204, 255),
    CHSV(43, 255, 255),
    CHSV(85, 255, 255),
    CHSV(124, 255, 255),
    CHSV(158, 255, 255)
);


// ============
// RGB Palettes
// ============

/*
 * Lava Colors (No Blacks) Palette
 *
 * A modification of FastLED's LavaColors palette, except replacing blacks with
 * other shades of red and orange.
 */
const CRGBPalette16 LavaNoBlackColorsPalette_p(
    CRGB::Crimson,
    CRGB::Maroon,
    CRGB::Red,
    CRGB::Maroon,

    CRGB::DarkRed,
    CRGB::Maroon,
    CRGB::DarkRed,
    CRGB::Orange,

    CRGB::DarkRed,
    CRGB::DarkRed,
    CRGB::Red,
    CRGB::Orange,

    CRGB::White,
    CRGB::Orange,
    CRGB::Red,
    CRGB::DarkRed
);


// The collection of available HSV Palettes
CHSVPalette16 hsvPalettes[] = {
    ChezCargotColorsPalette_p, SKSunsetPalette_p, SKSaturatedSunsetPalette_p,
    SKBPRPalette_p, SKGrassPalette_p, SKFiveHSVPalette_p
};

// The collection of available RGB Palettes
CRGBPalette16 rgbPalettes[] = {
    ChezCargotColorsPalette_p, LavaNoBlackColorsPalette_p, PartyColors_p,
    OceanColors_p, SKSunsetPalette_p, SKSaturatedSunsetPalette_p,
    SKBPRPalette_p, SKGrassPalette_p, SKFiveHSVPalette_p
};

// Length of hsvPalettes[]
const uint16_t hsvPalettesLength = (
    sizeof(hsvPalettes) / sizeof(hsvPalettes[0]));

// Length of rgbPalettes[]
const uint16_t rgbPalettesLength = (
    sizeof(rgbPalettes) / sizeof(rgbPalettes[0]));


// =================
// Utility Functions
// =================


/*
 * Returns a random amount of entropy using an unused analogPin's input.
 *
 * Takes the read value of an unused analog pin, and scales it to a 16 byte
 * range. This allows the order of animations and palettes to be more random
 * upon first turning on the lights.
 */
uint16_t generate_entropy() {
    return (analogRead(RANDOM_ANALOG_PIN) * 65535) / 1023;
}

/*
 * Returns a hue index that corresponds with the given LED index.
 *
 * This function smoothly interpolates over the range of [0, 255].
 *
 * Pre: 0 <= led < NUM_LEDS
 * Returns: 0 <= gradientHue <= 255
 */
uint8_t getGradientHue(uint16_t led) {
    return (led * 255) / (NUM_LEDS - 1);
}

/*
 * Returns a hue index that corresponds to the group index of the given LED.
 *
 * There are NUM_GROUPS groups, and each group number corresponds to a value
 * over [0, 255]; this function returns the discrete value that corresponds
 * with a given LED's group. For example, if LED_GROUP_SIZE == 15, led
 * indexes [0, 14] would all return the same value, and the next hue starts at
 * led index 15.
 *
 * Pre: 0 <= led < NUM_LEDS
 * Returns: 0 <= groupHue <= 255
 */
uint8_t getGroupHue(uint16_t led) {
    return ((led / LED_GROUP_SIZE) * 255) / (NUM_GROUPS - 1);
}


// ============
// LED Patterns
// ============


/*
 * Notes For LED Patterns
 *
 * Each LED Pattern should be defined as one iteration of the loop, preserving
 * any state as necessary between subsequent calls (since each pattern function
 * is called in every iteration of the Arduino hardware's loop() function).
 *
 * General Steps For Creating A New Pattern:
 * - Create a function with no parameters, ideally in the format of *Pattern.
 * - Add the function to the global PatternArray patterns (which essentially is
 *   an array of function pointers that change automatically).
 *
 * Other Tips:
 * - Use rgbPalettes with currentRGBPalette, and hsvPalettes with
 *   currentHSVPalette to easily add dynamic color changes to your pattern
 *   using FastLED's ColorFromPalette.
 * - If you want to make variations of a pattern, create master functinos that
 *   take parameters, and add small parameter-less functions that call your
 *   master pattern function in different states.
 */


/*
 * Pattern that goes the entire strip in 16 off beat sin waves.
 */
void beatSyncMultiplesPattern() {
    fadeToBlackBy(leds, NUM_LEDS, 20);
    CRGBPalette16 palette = rgbPalettes[currentRGBPalette];
    for(uint16_t i = 0; i < 16; ++i) {
        uint16_t index = beatsin16(i * 2, 0, NUM_LEDS);
        leds[index] |= ColorFromPalette(palette, i * 16, MAX_BRIGHTNESS);
    }
}


/*
 * Pattern which has multiple trails of LED "comets" moving along the strip.
 */
void cometPattern(AnimationType animType) {
    static const uint64_t animationTimeMillis = 50;
    static uint64_t lastTime = 0;
    static uint16_t offset = 0;

    uint64_t currentTime = millis();

    fadeToBlackBy(leds, NUM_LEDS, 31);

    if (lastTime + animationTimeMillis <= currentTime) {
        for (uint16_t i = offset; i < NUM_LEDS; i += LED_GROUP_SIZE) {
            switch (animType) {
                case HSV_PALETTE_ANIM:
                    {
                        CHSV hsv = ColorFromPalette(
                            hsvPalettes[currentHSVPalette], getGradientHue(i),
                            MAX_BRIGHTNESS);
                        hsv.hue += 16 * offset;
                        leds[i] = hsv;
                    }
                    break;
                case RGB_PALETTE_ANIM:
                default:
                    leds[i] = ColorFromPalette(
                        rgbPalettes[currentRGBPalette], getGradientHue(i),
                        MAX_BRIGHTNESS);
                    break;
            }
        }
        offset = (offset + 1) % LED_GROUP_SIZE;
        lastTime = currentTime;
    }
}


/*
 * See cometPattern, uses RGB Palettes.
 */
void cometPatternRGB() {
    cometPattern(RGB_PALETTE_ANIM);
}


/*
 * See cometPattern, uses HSV Palettes.
 */
void cometPatternHSV() {
    cometPattern(HSV_PALETTE_ANIM);
}


/*
 * Pattern that starts from individual points and converges with others.
 */
void convergePattern(AnimationType animType) {
    static const uint64_t animationTimeMillis = 75;
    static const uint16_t maxDist = (
        IS_GROUP_SIZE_EVEN ? GROUP_CENTER - 1: GROUP_CENTER);

    static uint64_t lastTime = 0;
    static uint16_t dist = 0;
    static boolean goingOut = true;

    uint64_t currentTime = millis();
    uint16_t start = maxDist - dist;

    if (lastTime + animationTimeMillis <= currentTime) {
        for (uint16_t i = 0; i < NUM_LEDS; i += LED_GROUP_SIZE) {
            for (uint16_t j = i + start;
                    j <= i + GROUP_CENTER + dist && j < NUM_LEDS; ++j) {
                if (goingOut) {
                    switch (animType) {
                        case HSV_PALETTE_ANIM:
                            {
                                CHSV hsv = ColorFromPalette(
                                    hsvPalettes[currentHSVPalette],
                                    getGradientHue(j), DEFAULT_BRIGHTNESS);
                                hsv.sat = beatsin8(30, MIN_SAT, MAX_SAT);
                                hsv.hue += 48 * dist;
                                leds[j] = hsv;
                            }
                            break;
                        case RGB_PALETTE_ANIM:
                        default:
                            leds[j] = ColorFromPalette(
                                rgbPalettes[currentRGBPalette],
                                getGradientHue(j), DEFAULT_BRIGHTNESS);
                            break;
                    }
                } else {
                    leds[j] = CRGB::Black;
                }
            }
        }

        dist = (dist + 1) % (maxDist + 1);
        if (dist == 0) {
            goingOut = !goingOut;
        }
        lastTime = currentTime;
    }
}


/*
 * See convergePattern, uses RGB Palettes.
 */
void convergePatternRGB() {
    convergePattern(RGB_PALETTE_ANIM);
}


/*
 * See convergePattern, uses HSV Palettes.
 */
void convergePatternHSV() {
    convergePattern(HSV_PALETTE_ANIM);
}


/*
 * Pattern that goes through ChezCargot Colors with glittering lights and
 * "morphing" like quality.
 */
void glitterPattern() {
    uint8_t bpm = 30;
    uint8_t beat = beatsin8(bpm, 127, MAX_BRIGHTNESS);
    CRGBPalette16 palette = ChezCargotColorsPalette_p;

    for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette(palette, rainbowHue + (i * 3), beat);
    }
    if(random8() < 64) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}


/*
 * Beat syncing pattern that pulses in both directions with a varying hue and
 * brightness factor.
 */
void pulsingPattern() {
    uint8_t bpm = 30;
    uint8_t beat = beatsin8(bpm, 64, MAX_BRIGHTNESS);
    uint8_t slowBeat = beatsin8(bpm / 10, 64, 255);

    CRGBPalette16 palette = ChezCargotColorsPalette_p;

    for(uint16_t i = 0; i < NUM_LEDS; ++i) {
        uint8_t brightness = beat - (rainbowHue + (i * 8));
        leds[i] = ColorFromPalette(
            palette, slowBeat - (rainbowHue + (i * 2)), brightness);
    }
}


/*
 * Turns on LED's at random, creating a sparkling pattern where LED's slowly
 * fade to black.
 */
void randomSparklesPattern(AnimationType animType) {
    uint16_t pos = random16(NUM_LEDS);
    uint8_t hue;

    fadeToBlackBy(leds, NUM_LEDS, 10);
    switch(animType) {
        case GROUP_ANIM:
            hue = getGroupHue(pos);
            break;
        case RAINBOW_ANIM:
        default:
            hue = rainbowHue;
            break;
    }
    leds[pos] += CHSV(hue + random8(64), DEFAULT_SAT, MAX_BRIGHTNESS);
}


/*
 * See randomSparklesPattern using group index hues.
 */
void randomSparklesGroupPattern() {
    randomSparklesPattern(GROUP_ANIM);
}


/*
 * See randomSparklesPattern using rainbow hues.
 */
void randomSparklesRainbowPattern() {
    randomSparklesPattern(RAINBOW_ANIM);
}


/*
 * Pattern that twinkles on even and odd lights with changing saturation.
 */
void twinklePattern() {
    uint8_t bpm = 30;
    uint8_t beat = beatsin8(bpm, 0, 255);
    CHSVPalette16 palette = hsvPalettes[currentHSVPalette];
    for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        CHSV hsv = ColorFromPalette(
            palette, getGroupHue(i), DEFAULT_BRIGHTNESS);
        hsv.sat = lerp8by8(MIN_SAT, MAX_SAT, beat);
        if (i % 2 == 0) {
            hsv.val = lerp8by8(MAX_BRIGHTNESS, MIN_BRIGHTNESS, beat);
        } else {
            hsv.val = lerp8by8(MIN_BRIGHTNESS, MAX_BRIGHTNESS, beat);
        }
        leds[i] = hsv;
    }
}


// =======================
// Patterns Initialization
// =======================


/*
 * Global list of pattern functions.
 *
 * Each should be the name of a function that has return type void and takes no
 * parameters. Some patterns are repeated in order to increase their chances of
 * showing up when the next pattern is randomly chosen.
 */
PatternArray patterns = {
    beatSyncMultiplesPattern,
    beatSyncMultiplesPattern,
    cometPatternHSV,
    cometPatternRGB,
    convergePatternHSV,
    convergePatternRGB,
    glitterPattern,
    glitterPattern,
    pulsingPattern,
    pulsingPattern,
    randomSparklesGroupPattern,
    randomSparklesRainbowPattern,
    twinklePattern,
    twinklePattern
};


// Length of patterns[]
const uint16_t patternsLength = sizeof(patterns) / sizeof(patterns[0]);


// ========================
// State Changing Functions
// ========================

/*
 * Randomly changes the index on both the HSV and RGB palette arrays. This
 * should be called in a EVERY_N_SECONDS macro provided by FastLED in order
 * to dynamically change color palettes in patterns.
 *
 * NOTE: It always chooses a unique palette (no repetitions), so as a result
 * you have to have at least 2 palettes to not encounter an infinite loop.
 */
void nextPalette() {
    uint8_t newRGBPalette = random8(rgbPalettesLength);
    while (newRGBPalette == currentRGBPalette) {
        newRGBPalette = random8(rgbPalettesLength);
    }
    currentRGBPalette = newRGBPalette;

    uint8_t newHSVPalette = random8(hsvPalettesLength);
    while (newHSVPalette == currentHSVPalette) {
        newHSVPalette = random8(hsvPalettesLength);
    }
    currentHSVPalette = newHSVPalette;
}


/*
 * Randomly changes the index of the patterns array in order to switch to the
 * next pattern. This should be called in an EVERY_N_SECONDS macro provided by
 * FastLED in order to dynamically change patterns.
 *
 * NOTE: It always chooses a unique pattern (no repetitions), so as a result
 * you have to have at least 2 patterns to not encounter an infinite loop.
 */
void nextPattern() {
    uint8_t newPattern = random8(patternsLength);
    while (newPattern == currentPattern) {
        newPattern = random8(patternsLength);
    }
    currentPattern = newPattern;
}


// ======================
// Main Arduino Functions
// ======================


void setup() {
    // Enable LED 5V Buffers on Teensy 3.2 Prop Shield
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    // Add some entropy to initial random FastLED seed
    random16_add_entropy(generate_entropy());

    // Initialize starting patterns and palettes
    currentPattern = random8(patternsLength);
    currentHSVPalette = random8(hsvPalettesLength);
    currentRGBPalette = random8(rgbPalettesLength);

    // For Use With Explicit Clock and Data
    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB_ORDER, DATA_RATE_MHZ(8)>(
        leds, NUM_LEDS).setCorrection(TypicalSMD5050);

    // Set the color temperature
    FastLED.setTemperature(CarbonArc);

    // Set the global brightness
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}


void loop() {
    patterns[currentPattern](); // Setup one frame of a pattern
    FastLED.show(); // Show the LED's
    FastLED.delay(1000 / FPS);  // Add a global delay at the frame rate.

    // Switch patterns every PATTERN_SECS
    EVERY_N_SECONDS(PATTERN_SECS) { nextPattern(); }

    // Switch palettes every PALETTE_SECS
    EVERY_N_SECONDS(PALETTE_SECS) { nextPalette(); }

    // Increment the "rainbow hue" every RAINBOW_MILLIS milli's.
    EVERY_N_MILLISECONDS(RAINBOW_MILLIS) { ++rainbowHue; }
}
