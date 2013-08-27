
#include "SPI.h"
#include "DriverWS2801.h"

#define pgm_read_byte(x) (*(x))
#define NUM_PIXELS 100
#define FRAMES_PER_SECOND 60
#define FADE_FRAMES 256   //number of frames to crossfade within
#define STAY_FRAMES 2000 //number of frames to show a program - if multiple of half num pixels, then some programs won't run into each other (eg random strobe fade)
#define MAX_LEVEL 256 //max R,G,B or S, V levels (note: need to subtract one to make 0 based)

#define PI 3.14159265

// Simplex Noise
//(inspired by happyinmotion: http://happyinmotion.livejournal.com/278357.html)
// Simplex noise for whole strip of LEDs.
// (Well, it's simplex noise for 6 nodes and cubic interpolation between those nodes.)
#define onethird 0.333333333
#define onesixth 0.166666667
#define numSpacing 10 //was 4
#define FULL_ONE_MINUS 255 //level range
int ii, jj, kk, AA[] = {0, 0, 0};
float uu, vv, ww, ss;
int TT[] = {0x15, 0x38, 0x32, 0x2c, 0x0d, 0x13, 0x07, 0x2a};

// Membrane 4x4 Keypad setup:
const byte KEYPAD_ROWS = 4; //four KEYPAD_ROWS
const byte KEYPAD_COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char keyTable[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}};
char matrixJustPressed = ' ';
byte rowPins[KEYPAD_ROWS] = {39, 41, 43, 45}; //connect to the row pinouts of the keypad
byte colPins[KEYPAD_COLS] = {47, 49, 51, 53}; //connect to the column pinouts of the keypad


//buttons
#define BUTTON_PROG_UP 0
#define BUTTON_RANDOM 6
#define BUTTON_ONOFF 7

//states
bool justPressedOnOff, desireOff, turningOff, turningOn, isOff;
bool desireRandom = true;
bool desireProgChangeUp = false;
bool desireProgChangeDn = false;
bool desirePattChangeUp = false;
bool desirePattChangeDn = false;
bool desireFadeChangeUp = false;
bool desireFadeChangeDn = false;
int nextProgram = 1;
int nextPattern = 0; //ToDo
int nextFade = 0; //ToDo
int brightness = 8; //ToDo
int speed = 5; //ToDo


//table of powers for damping used in ProgramRandomSplash [pow(damping, frame) - damping^frame]
#define damping 0.90
float dampingTable[100];

int numPrograms = 0;

// You can also use hardware SPI for ultra-fast writes by omitting the data
// and clock pin arguments.  This is faster, but the data and clock are then
// fixed to very specific pin numbers: on Arduino 168/328, data = pin 11,
// clock = pin 13.  On Mega, data = pin 51, clock = pin 52. SPI 3(c) & 4(d) on Due
DriverWS2801 pixelString = DriverWS2801(NUM_PIXELS);

// Principle of operation: at any given time, the LEDs depict an image or
// animation effect (referred to as the "back" image throughout this code).
// Periodically, a transition to a new image or animation effect (referred
// to as the "front" image) occurs.  During this transition, a third buffer
// (the "alpha channel") determines how the front and back images are
// combined; it represents the opacity of the front image.  When the
// transition completes, the "front" then becomes the "back," a new front
// is chosen, and the process repeats.
byte imgData[2][NUM_PIXELS * 3],	// Data for 2 pixelStrings worth of imagery
	alphaMask[NUM_PIXELS],			// Alpha channel for compositing images
	backImgIdx = 0,					// Index of 'back' image (always 0 or 1)
	fxIdx[3];						// Effect # for back & front images + alpha
bool fxInitialised[3];			// Whether to set initialisation variables, or already fxInitialised
int fxIntVars[3][11],				// Effect instance variables (explained later)
	fxArrVars[3][2][10],			// MEO: 2 x Array
	fxFrameCount[3],				// MEO: current overall frame count of single effect
	fxFrameDelay[3],			// MEO: if too fast - can set number of frames to pause
	fxFrameDelayCount[3],		// MEO: counter for fxFrameDelay
	tCounter   = -1,				// Countdown to next transition
	transitionTime;					// Duration (in frames) of current transition
float fxFltVars[3][6];				// MEO: float variables
bool fxBlnVars[3][3];				// MEO: bool variables

bool randProg = true;
bool lightsOn = true;

// Chaser functions
void SetChaserColor(int bulb, int color, byte idx);
void FillChaserSeq(int count, int sequence,
                               int span_size, int startColor, int (*sequence_func)(int sequence, int startColor), byte idx);

// Crossfade functions
void crossfadeSimple(void);
void crossfadeWipe(void);
void crossfadeDither(void);

//Programs
void ProgramOff(byte idx);
void ProgramLarsonScanner(byte idx);
void ProgramRandomSplash(byte idx);
void ProgramStrobeFade(byte idx);

volatile void CheckSwitches(void);

byte getGamma(byte x);
int HSVtoRGB(int h, byte s, byte v, int wheelLine);
char FixedSine(int angle);
char FixedCosine(int angle);

int testcounter;
	
// ---------------------------------------------------------------------------

void setup() {
	// Open serial communications and wait for port to open:
	Serial.begin(115200);

	// Start up the LED pixelString.  Note that pixelString.show() is NOT called here --
	// the LightFrame function will be invoked immediately when attached, and
	// the first thing the calback does is update the pixelString.
	pixelString.begin();
       // pixelString.show();
       
	// Initialize random number generator from a floating analog input.
	randomSeed(analogRead(0));
	memset(imgData, 0, sizeof(imgData)); // Clear image data

        testcounter = 0;
}

void loop() {
	//required by Ardunino compiler - but all my code in interrupt handlers
        if (testcounter >= 50)
        {
            testcounter = 0;
        }
        pixelString.setPixelColor(testcounter++, random(255), random(255), random(255));
}


