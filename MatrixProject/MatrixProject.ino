#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
/* About Section

  ################## Matrix Project ##################

  Task List:
    - Matrix Control
    - Lcd Control
    - Menu Logic
    - Game Logic

  Checkpoint #1 : LCD Base Functionalities
    - Intro Message + animation
    - Menu :
      - Start game
      - Settings - mandatory :
        - LCD brightness control in EEPROM
        - Matrix brightness control in EEPROM
      - About
    - End Message
    - During gameplay logic

  Checkpoint #2 : Game Logic - TBC

  Extra to add:
    - Photocell for brightness

  TO DO:
    - Game logic function - The homework for #1
    - Buzzer Logic

  DOING:
    - During Gameplay LDC Display function - common messages for #1


  DONE:
    - Matrix logic - God help us
    - Joystick logic
    - Player attributes
    - Player movement in fog of war
    - LCD logic
    - Menu Messages + Scroll with joystick
    - Menu Logic : Start game
    - Menu Logic : Settings functions
    - Menu Logic : about with automatic scrolling
    - Intro : Message, animation on matrix, song
    - End Game Message + animation + song

  ################## Matrix Project - Part 2 ##################

  Define game logic

  Shooting game :
    - static enemies that shoot one bullet at a time in player direction
    - player can shoot n(1/2) bullets at a time
    - Limited lives
    - Win when all enemies have been destroyed
    - Lose if lives == 0

  Tasks :
    - Enemy logic - Mostly on
    - Bullet logic - dynamic, limited - 1 per enemy at time, limited number for player - blink fast


  Menu Updates : 
    - HighScore view
    - Setting :
      - add name - when you start
      - sound on/off
      - reset highscores
    - update about
    - add tutorial

  TO DO:
    - Fix bullet collision - BUG
    - Code review

  DOING:

  DONE:
    - Add lose/win music
    - More maps
    - Update about
    - Add buzzer and sounds
    - Insert more maps
    - Make enemies and bullets static in memory and adapt code
    - Display enemies left
    - Handle two bullets collisions
    - Define score system
    - Update menu
    - Highscore - display, update, enter name, in eeprom
    - Make tutorial option

*/



/// ----------------------------------- Declarations Section --------------------------------------




/// Matrix control variables - constants, initialize lc
  const byte dinPin = 12;
  const byte clockPin = 11;
  const byte loadPin = 10;
  const byte matrixSize = 8;
  const byte virtualMatrixSize = 16;
  LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
  byte matrixBrightness = 2;
  byte matrixChanged = HIGH;
  byte currRow = 0;  /// Upper left corner to display
  byte currCol = 0;
  const byte numOfBrightLevelsMatrix = 16;
  const byte moveIndent = 2;

unsigned short matrix[virtualMatrixSize] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
/// Joystick consts
  const int xPin = A0;  // col
  const int yPin = A1;
  const int minThreshold = 200;
  const int maxThreshold = 600;
/// Button variables
  const byte buttonPin = A5;
  byte buttonReading = LOW;
  byte buttonLastReading = LOW;
  byte buttonState = LOW;
  unsigned long long lastDebounceTime = 0;
  const byte debounceDelay = 50;

/// Player variables - position, direction of movement, blink, movement
  byte playerPosX = 5;
  byte playerPosY = 5;
  byte playerLastPosX = 5;
  byte playerLastPosY = 5;
  short playerDirectionX = 1;
  short playerDirectionY = 0;
  short playerLastDirectionX = 1;
  short playerLastDirectionY = 0;
  const byte moveCheckIntervalPlayer = 200;
  unsigned long long lastMovedJoystick = 0;
  const short blinkIntervalPlayer = 1097;
  unsigned long long lastBlinkPlayer = 0;
  const byte moveCheckInterval = 200;
  short lives = 5;
  int score = 0;

/// LCD
  const byte rs = 9;
  const byte en = 8;
  const byte d4 = 7;
  const byte d5 = 3;
  const byte d6 = 5;
  const byte d7 = 4;
  const byte lcdBrightnessPin = 6;
  // const byte minBrightnessLcd = 0;
  // const byte maxBrightnessLcd = 255;
  const byte numOfBrightLevelsLcd = 6;
  const byte lcdBrightnessValues[numOfBrightLevelsLcd] = {
    10, 50, 100, 150, 200, 255
  };
  byte lcdBrightnessId = 3;
  bool updateLcd = false;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/// States
byte state = 0;
/*
  - 0 : Intro
  - 1 : Menu
  - 2 : in Game
  - 3 : After Game
  - 4 : In Settings
  - 5 : set lcd brightness
  - 6 : set matrix brightness
  - 7 : about
  - 8 : Choosing port
  - 9 : Back to port
  - 10 : Shop
  - 11 : highscores
  - 12 : wait for press to enter name
  - 13 : enter name
  - 15 : Tutorial - ups

*/

//// Menu variables
  byte menuState = 0;
  const byte numOfMenuStates = 5;
  const byte numOfSettings = 6;
  const short aboutDelay = 400;
  unsigned long long lastAboutDisplay = 0;
  byte scrollTextPosition = 0;
  short aboutCounter = 0;
  String message = "              Hello! This is ShootAndRun, Made by Diaconescu Teodora, @Teo0o0 on Github.               ";
  const byte messageLength = 100;

  unsigned short highscores[3] = {0, 0, 0};
  char names[3][3] = {"abc", "def", "ghi"};
  char name[3] = "aaa";
  byte currPosName = 0;

  const short animation1Delay = 200;
  unsigned long long lastAnimation = 0;
  short animationCounter = -1;

/// Custom Characters
  byte scrollBoth[8] = {
    0b00100,
    0b01110,
    0b11111,
    0b00000,
    0b00000,
    0b11111,
    0b01110,
    0b00100
  };

  byte scrollUp[8] = {
    0b00100,
    0b01110,
    0b11111,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
  };

  byte scrollDown[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b11111,
    0b01110,
    0b00100
  };

  byte brightnessBar[8] = {
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110
  };

  byte heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
  };

  byte enemy[8] = {
    0b00100,
    0b01110,
    0b00100,
    0b11111,
    0b10101,
    0b00100,
    0b01010,
    0b10001
  };

  byte coin[8] = {
    0b00000,
    0b00000,
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
  };

  byte scoreChar[8] = {
    0b01010,
    0b01010,
    0b11111,
    0b01010,
    0b01010,
    0b11111,
    0b01010,
    0b01010
  };



//// --------------------------------------------- Sounds -------------------------------------------



bool soundState = HIGH;

int currNote = -1;
unsigned long long startNote = 0;
int soundDuration = 0;
byte song1 = HIGH;
byte song2 = LOW;

const short scrollSound = 1568;
const byte outOfBoundsSound = 98;
const short loseLifeSound = 98;
const short shootSound = 1245;
const short killEnemySound = 1568;
const byte noteDuration = 2;
bool isSinging = false;

const short song1Duration = 5100;
const unsigned short song2Duration = 15600;
unsigned long long soundStart = 0;
short maxNote = 0;
const byte maxNoteSong1 = 16;
const byte maxNoteSong2 = 52;

const int musicNorm = 500;
const int pauseFactor = 1.3;
const int buzzerPin = 13;
int buzzerTone = 2000;
bool inGameMusic = true;





//// ---------------------------------------- Structures and headers of used functions ----------------------------------




void SetBitValue(byte row, byte col, byte value, bool flag = true);
inline byte GetBitValue(byte row, byte col, bool flag = true);
bool CheckPos(byte row, byte col, byte isBullet, bool flag = true); 

/// bullet global consts
  const short bulletMoveInterval = 800;
  const byte blinkIntervalBullet = 200;
  unsigned long long lastBlinkBullet = 0;
  bool bulletState = 0;
  

struct Bullet { //// Structure for bullet handle
  byte posX, posY;
  short directionX, directionY;
  bool shot = false;
  unsigned long long lastMove = 0;
  bool CheckMove() { //// Auto handle moving and blinking
    if(shot == false) return false;
    SetBitValue(posX, posY, bulletState, false);
    if(millis() - lastMove > bulletMoveInterval)
    {
      if(Move() == false) {
        shot = false;
        return false;
      }
      lastMove = millis();
    }
    return true;
  }

  bool Move() { //// Auto handle moving and collision detection
    if(shot == false) return false;
    SetBitValue(posX, posY, 0, false);
    matrixChanged = HIGH;
    if(posX + directionX >= virtualMatrixSize || posY + directionY >= virtualMatrixSize) { shot = false; return false; }
    if(CheckPos(posX + directionX, posY + directionY, true, false) == 1) { shot = false; return false; }
    posX += directionX;
    posY += directionY;
    SetBitValue(posX, posY, 1, false);
    return true;
  }

  bool Shoot(byte VposX, byte VposY, short VdirectionX, short VdirectionY, byte flag = true) { //// as constructor
    Serial.println("Shot!");
    directionX = VdirectionX;
    directionY = VdirectionY;
    posX = VposX;
    posY = VposY;
    Serial.print(posX + (currRow * flag) + directionX);
    Serial.print(" ");
    Serial.print(posY);
    Serial.print(" ");
    Serial.print((currCol * flag));
    Serial.print(" ");
    Serial.print(directionY);
    Serial.print(" ");
    Serial.println(posY + (currCol * flag) + directionY);
    if(posX + (currRow * flag) + directionX >= virtualMatrixSize || posY + (currCol * flag) + directionY >= virtualMatrixSize) return false;
    Serial.println("P2");
    if(CheckPos(posX + (currRow * flag) + directionX, posY + (currCol * flag) + directionY, true, false)) return false;
    Serial.println("P3");
    posX = VposX + (currRow * flag) + directionX;
    posY = VposY + (currCol * flag) + directionY;
    SetBitValue(posX, posY, 1, false);
    Serial.print(posX);
    Serial.print(" ");
    Serial.println(posY);
    lastMove = millis();
    matrixChanged = HIGH;
    shot = true;
    return true;
  }

};

//// Enemy variables
  const short blinkIntervalEnemyOn = 907;
  const short blinkIntervalEnemyOff = 181;
  unsigned long long lastBlinkEnemy = 0;
  short shootIntervalEnemy = 1000;
  byte enemyState = 0;
  byte viewDistanceForEnemies = 4;
  const byte maxNumOfEnemies = 10;
  byte numOfEnemies = 5;
  short spawnAux = 7907;

struct Enemy { //// Structure for enemy handle
  byte posX, posY;
  bool isAlive = false;
  Bullet bullet;
  byte viewDistance;
  unsigned long long lastShotEnemy = 0;
  void CheckForPlayer() { //// if player is on the same coordinate, try to shoot
    if(bullet.shot == true || isAlive == false || millis() - lastShotEnemy < shootIntervalEnemy) return;
    byte distance = 0;
    if(posX == playerPosX + currRow) {
      distance = posY - playerPosY - currCol;
      if(distance <= viewDistance) {
        bullet.Shoot(posX, posY, 0, -1, false);
        lastShotEnemy = millis();
      }
      else if(distance >= 255 - viewDistance + 1) {
        bullet.Shoot(posX, posY, 0, 1, false);
        lastShotEnemy = millis();
      }
    }
    else if (posY == playerPosY + currCol) {
      byte distance = posX - playerPosX - currRow;
      if(distance <= viewDistance) {
        bullet.Shoot(posX, posY, -1, 0, false);
        lastShotEnemy = millis();
      }
      else if(distance >= 255 - viewDistance + 1) {
        bullet.Shoot(posX, posY, 1, 0, false);
        lastShotEnemy = millis();
      }
    }
    
  }
  void MoveBullet() { //// Handle its own bullet
    if(bullet.shot == true) {
      if(bullet.CheckMove() == false) {
        bullet.shot = false; /////// redundant
      }
    }
  }
};

Enemy enemyList[maxNumOfEnemies];
Bullet playerBullet1, playerBullet2;

unsigned long long lastPortBlink = 0;
byte portBlinkInterval = 200;
byte levelValue = 0;
short numOfCoins = 0;

byte shopState = 1;



//// ------------------------------ Maps Section + variables initialization --------------------------------



byte numOfMaps = 1;
byte currMap = 1;
const byte mulLevel[13] = {
  1, 2, 3, 3, 4, 5, 10, 3, 5, 6, 5, 10, 12
};

/// PortMap
  // unsigned short map1[virtualMatrixSize] = {
  //   56, 12348, 28700, 14364, 
  //   8192, 0, 0, 0, 
  //   0, 0, 3080, 3608, 
  //   15384, 2076, 16, 0
  // };
void InitializeMapPort() {
  matrix[0] = 56; matrix[4] = 8192; matrix[8] = 0; matrix[12] = 15384;
  matrix[1] = 12348; matrix[5] = 0; matrix[9] = 0; matrix[13] = 2076;
  matrix[2] = 28700; matrix[6] = 0; matrix[10] = 3080; matrix[14] = 16;
  matrix[3] = 14364; matrix[7] = 0; matrix[11] = 3608; matrix[15] = 0;
  playerPosX = 5;
  playerPosY = 5;
  playerLastPosX = 5;
  playerLastPosY = 5;
  currCol = 4;
  currRow = 3;
  numOfEnemies = 5;
}

void InitializeMapPort2() {
  matrix[0] = 0; matrix[4] = 28672; matrix[8] = 4; matrix[12] = 15840;
  matrix[1] = 14336; matrix[5] = 14350; matrix[9] = 4; matrix[13] = 16352;
  matrix[2] = 61440; matrix[6] = 15; matrix[10] = 256; matrix[14] = 16320;
  matrix[3] = 28672; matrix[7] = 12; matrix[11] = 12736; matrix[15] = 0;
  playerPosX = 5;
  playerPosY = 5;
  playerLastPosX = 5;
  playerLastPosY = 5;
  currCol = 4;
  currRow = 3;
  numOfEnemies = 5;
}

void InitializeMapPort3() {
  matrix[0] = 0; matrix[4] = 63488; matrix[8] = 0; matrix[12] = 61680;
  matrix[1] = 12; matrix[5] = 24576; matrix[9] = 0; matrix[13] = 36960;
  matrix[2] = 31; matrix[6] = 8192; matrix[10] = 0; matrix[14] = 33008;
  matrix[3] = 12303; matrix[7] = 0; matrix[11] = 61584; matrix[15] = 32768;
  playerPosX = 5;
  playerPosY = 5;
  playerLastPosX = 5;
  playerLastPosY = 5;
  currCol = 4;
  currRow = 3;
  numOfEnemies = 5;
}

void InitializeMapPort4() {
  matrix[0] = 0; matrix[4] = 49344; matrix[8] = 57344; matrix[12] = 112;
  matrix[1] = 352; matrix[5] = 57344; matrix[9] = 16384; matrix[13] = 120;
  matrix[2] = 448; matrix[6] = 63488; matrix[10] = 16448; matrix[14] = 124;
  matrix[3] = 448; matrix[7] = 57344; matrix[11] = 96; matrix[15] = 64;
  playerPosX = 2;
  playerPosY = 2;
  playerLastPosX = 2;
  playerLastPosY = 2;
  currCol = 4;
  currRow = 3;
  numOfEnemies = 5;
}

//// Map1
  // unsigned short map1[virtualMatrixSize] = {
  //   2056, 2056, 3592, 8, 
  //   57576, 8320, 128, 128, 
  //   14, 8, 7688, 4192, 
  //   4128, 32, 0, 0
  // };

void InitializeMap1() {
  matrix[0] = 16; matrix[4] = 2032; matrix[8] = 0; matrix[12] = 9184;
  matrix[1] = 16; matrix[5] = 0; matrix[9] = 8223; matrix[13] = 8704;
  matrix[2] = 16; matrix[6] = 0; matrix[10] = 8704; matrix[14] = 8704;
  matrix[3] = 16; matrix[7] = 0; matrix[11] = 8704; matrix[15] = 8704;
  playerPosX = 4;
  playerPosY = 0;
  playerLastPosX = 4;
  playerLastPosY = 0;
  currCol = 0;
  currRow = 5;
  viewDistanceForEnemies = 4;
  numOfEnemies = 5;
  levelValue = mulLevel[0];
}
/// Map 2
  // unsigned short map2[virtualMatrixSize] = {
  //   16, 16, 16, 16, 
  //   2032, 0, 0, 0, 
  //   0, 8223, 8704, 8704, 
  //   9184, 8704, 8704, 8704
  // };


void InitializeMap2() {
  matrix[0] = 2056; matrix[4] = 57576; matrix[8] = 14; matrix[12] = 4128;
  matrix[1] = 2056; matrix[5] = 8320; matrix[9] = 8; matrix[13] = 32;
  matrix[2] = 3592; matrix[6] = 128; matrix[10] = 7688; matrix[14] = 0;
  matrix[3] = 8; matrix[7] = 128; matrix[11] = 4192; matrix[15] = 0;
  playerPosX = 5;
  playerPosY = 5;
  playerLastPosX = 5;
  playerLastPosY = 5;
  currCol = 0;
  currRow = 0;
  viewDistanceForEnemies = 4;
  numOfEnemies = 5;
  levelValue = mulLevel[1];
}
//// Map 3
  // unsigned short map3[virtualMatrixSize] = {
  //   128, 15496, 9224, 1276, 
  //   9344, 15504, 144, 65439, 
  //   4112, 4624, 4624, 1008, 
  //   3, 64528, 144, 144
  // };
void InitializeMap3() {
  matrix[0] = 128; matrix[4] = 9344; matrix[8] = 4112; matrix[12] = 3;
  matrix[1] = 15496; matrix[5] = 15504; matrix[9] = 4624; matrix[13] = 64528;
  matrix[2] = 9224; matrix[6] = 144; matrix[10] = 4624; matrix[14] = 144;
  matrix[3] = 1276; matrix[7] = 65439; matrix[11] = 1008; matrix[15] = 144;
  playerPosX = 3;
  playerPosY = 4;
  playerLastPosX = 3;
  playerLastPosY = 4;
  currCol = 0;
  currRow = 0;
  viewDistanceForEnemies = 5;
  numOfEnemies = 7;
  levelValue = mulLevel[2];
}

void InitializeMap4() {
  matrix[0] = 576; matrix[4] = 2112; matrix[8] = 512; matrix[12] = 528;
  matrix[1] = 576; matrix[5] = 2112; matrix[9] = 512; matrix[13] = 64;
  matrix[2] = 576; matrix[6] = 7759; matrix[10] = 627; matrix[14] = 64;
  matrix[3] = 51320; matrix[7] = 512; matrix[11] = 58896; matrix[15] = 576;
  playerPosX = 5;
  playerPosY = 3;
  playerLastPosX = 5;
  playerLastPosY = 3;
  currCol = 0;
  currRow = 8;
  viewDistanceForEnemies = 5;
  numOfEnemies = 7;
  levelValue = mulLevel[3];
}

void InitializeMap5() {
  matrix[0] = 1028; matrix[4] = 163; matrix[8] = 14340; matrix[12] = 2180;
  matrix[1] = 1028; matrix[5] = 53152; matrix[9] = 3972; matrix[13] = 3324;
  matrix[2] = 15520; matrix[6] = 2108; matrix[10] = 135; matrix[14] = 2048;
  matrix[3] = 160; matrix[7] = 2084; matrix[11] = 49284; matrix[15] = 2048;
  playerPosX = 3;
  playerPosY = 3;
  playerLastPosX = 3;
  playerLastPosY = 3;
  currCol = 5;
  currRow = 3;
  viewDistanceForEnemies = 6;
  numOfEnemies = 7;
  levelValue = mulLevel[4];
}

void InitializeMap6() {
  matrix[0] = 128; matrix[4] = 2184; matrix[8] = 256; matrix[12] = 256;
  matrix[1] = 128; matrix[5] = 2056; matrix[9] = 256; matrix[13] = 499;
  matrix[2] = 2184; matrix[6] = 2056; matrix[10] = 256; matrix[14] = 256;
  matrix[3] = 2184; matrix[7] = 65532; matrix[11] = 63488; matrix[15] = 256;
  playerPosX = 3;
  playerPosY = 6;
  playerLastPosX = 3;
  playerLastPosY = 6;
  currCol = 8;
  currRow = 5;
  viewDistanceForEnemies = 5;
  numOfEnemies = 7;
  levelValue = mulLevel[5];
}

void InitializeMap7() {
  matrix[0] = 72; matrix[4] = 24628; matrix[8] = 9290; matrix[12] = 16380;
  matrix[1] = 520; matrix[5] = 8265; matrix[9] = 10184; matrix[13] = 4101;
  matrix[2] = 4106; matrix[6] = 9288; matrix[10] = 8200; matrix[14] = 512;
  matrix[3] = 16328; matrix[7] = 9288; matrix[11] = 40968; matrix[15] = 32;
  playerPosX = 3;
  playerPosY = 3;
  playerLastPosX = 3;
  playerLastPosY = 3;
  currCol = 3;
  currRow = 2;
  viewDistanceForEnemies = 5;
  numOfEnemies = 10;
  levelValue = mulLevel[6];
}

void InitializeMap8() {
  matrix[0] = 544; matrix[4] = 3968; matrix[8] = 144; matrix[12] = 528;
  matrix[1] = 544; matrix[5] = 2048; matrix[9] = 144; matrix[13] = 8816;
  matrix[2] = 57856; matrix[6] = 14336; matrix[10] = 53148; matrix[14] = 8704;
  matrix[3] = 527; matrix[7] = 240; matrix[11] = 528; matrix[15] = 8704;
  playerPosX = 3;
  playerPosY = 4;
  playerLastPosX = 3;
  playerLastPosY = 4;
  currCol = 2;
  currRow = 6;
  viewDistanceForEnemies = 4;
  numOfEnemies = 7;
  levelValue = mulLevel[7];
}

void InitializeMap9() {
  matrix[0] = 1056; matrix[4] = 10212; matrix[8] = 7168; matrix[12] = 8196;
  matrix[1] = 1056; matrix[5] = 58407; matrix[9] = 1024; matrix[13] = 8196;
  matrix[2] = 8196; matrix[6] = 32; matrix[10] = 58407; matrix[14] = 1056;
  matrix[3] = 8196; matrix[7] = 56; matrix[11] = 10212; matrix[15] = 1056;
  playerPosX = 6;
  playerPosY = 1;
  playerLastPosX = 6;
  playerLastPosY = 1;
  currCol = 0;
  currRow = 8;
  viewDistanceForEnemies = 4;
  numOfEnemies = 7;
  levelValue = mulLevel[8];
}

void InitializeMap10() {
  matrix[0] = 12176; matrix[4] = 2064; matrix[8] = 51443; matrix[12] = 2080;
  matrix[1] = 12176; matrix[5] = 16156; matrix[9] = 2064; matrix[13] = 2080;
  matrix[2] = 51347; matrix[6] = 2064; matrix[10] = 2064; matrix[14] = 49412;
  matrix[3] = 2064; matrix[7] = 2064; matrix[11] = 16380; matrix[15] = 260;
  playerPosX = 2;
  playerPosY = 2;
  playerLastPosX = 2;
  playerLastPosY = 2;
  currCol = 0;
  currRow = 0;
  viewDistanceForEnemies = 5;
  numOfEnemies = 10;
  levelValue = mulLevel[9];
}

void InitializeMap11() {
  matrix[0] = 128; matrix[4] = 1040; matrix[8] = 57472; matrix[12] = 1040;
  matrix[1] = 128; matrix[5] = 9232; matrix[9] = 1052; matrix[13] = 1052;
  matrix[2] = 15516; matrix[6] = 10239; matrix[10] = 1040; matrix[14] = 8320;
  matrix[3] = 1168; matrix[7] = 8320; matrix[11] = 65523; matrix[15] = 8320;
  playerPosX = 5;
  playerPosY = 2;
  playerLastPosX = 5;
  playerLastPosY = 2;
  currCol = 0;
  currRow = 8;
  viewDistanceForEnemies = 4;
  numOfEnemies = 8;
  levelValue = mulLevel[10];
}

void InitializeMap12() {
  matrix[0] = 288; matrix[4] = 256; matrix[8] = 2048; matrix[12] = 8320;
  matrix[1] = 288; matrix[5] = 256; matrix[9] = 2048; matrix[13] = 15504;
  matrix[2] = 2080; matrix[6] = 504; matrix[10] = 9103; matrix[14] = 144;
  matrix[3] = 63527; matrix[7] = 14336; matrix[11] = 8320; matrix[15] = 144;
  playerPosX = 3;
  playerPosY = 4;
  playerLastPosX = 3;
  playerLastPosY = 4;
  currCol = 8;
  currRow = 0;
  viewDistanceForEnemies = 4;
  numOfEnemies = 8;
  levelValue = mulLevel[11];
}

void InitializeMap13() {
  matrix[0] = 1040; matrix[4] = 4080; matrix[8] = 512; matrix[12] = 258;
  matrix[1] = 1040; matrix[5] = 4168; matrix[9] = 8711; matrix[13] = 49410;
  matrix[2] = 16515; matrix[6] = 57412; matrix[10] = 4616; matrix[14] = 2080;
  matrix[3] = 16512; matrix[7] = 64; matrix[11] = 4080; matrix[15] = 2080;
  playerPosX = 2;
  playerPosY = 2;
  playerLastPosX = 2;
  playerLastPosY = 2;
  currCol = 0;
  currRow = 0;
  viewDistanceForEnemies = 4;
  numOfEnemies = 10;
  levelValue = mulLevel[12];
}



///// ---------------------------------------- Running Section -------------------------------------



void setup() { //// -------------------- Program Setup : Menu, pinModes, LCD, Matrix ----------------------
  // put your setup code here, to run once:
  Serial.begin(9600);
  randomSeed(analogRead(A3)); /// increase randomness if nothing connected

  lc.shutdown(0, false);
  matrixBrightness = EEPROM.read(1);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);

  pinMode(buttonPin, INPUT_PULLUP);

  lcd.begin(16, 2);
  pinMode(lcdBrightnessPin, OUTPUT);
  lcdBrightnessId = EEPROM.read(0);
  analogWrite(lcdBrightnessPin, lcdBrightnessValues[lcdBrightnessId]);

  soundState = EEPROM.read(2);



  lcd.createChar(0, scrollDown);
  lcd.createChar(1, scrollBoth);
  lcd.createChar(2, scrollUp);
  lcd.createChar(3, brightnessBar);
  lcd.createChar(4, heart);
  lcd.createChar(5, enemy);
  lcd.createChar(6, coin);
  lcd.createChar(7, scoreChar);
  InitMessage();
  for(byte i = 0; i < 3; i++) {
    unsigned short x1 = ((unsigned short)EEPROM.read(3 + i * 2)) << 8;
    unsigned short x2 = (unsigned short)EEPROM.read(3 + i * 2 + 1);
    highscores[i] = x1 + x2;
  }
  for(byte i = 0; i < 3; i++) {
    for(byte j = 0; j < 3; j++) names[i][j] = EEPROM.read(9 + i * 3 + j);
  }

}

void loop() { ///// -------------------- Handle the state of the program ---------------------
  // put your main code here, to run repeatedly:
  if (matrixChanged == HIGH) {  /// only refresh matrix if sommething has changed
    UpdateMatrix();
    matrixChanged = LOW;
  }
  switch (state) {
    case 8:
      BlinkPorts();
      CheckPlayer();
      break;
    case 0: 
      Animation1(0);
      if(song1) maxNote = maxNoteSong1, Song1();
      break;
    case 1:
      MenuUpdate(numOfMenuStates);
      break;
    case 4 :
      MenuUpdate(numOfSettings);
      break;
    case 2:
      if(updateLcd) {
        DuringGameDisplay();
        updateLcd = false;
      }
      if(lives <= 0) {
          ExitGameOnLose();
          lives = 5;
          return;
        }
      if(numOfEnemies == 0) {
        ExitGameOnWin();
        ///lives = 5;
        return;
      }
      CheckPlayer();
      CheckEnemies();
      
      break;
    case 3:
      Animation1(3);
      break;
    case 9:
      Animation1(9);
      break;
    case 5:
      MenuUpdate(numOfBrightLevelsLcd);
      break;
    case 6: 
      MenuUpdate(numOfBrightLevelsMatrix);
      break;
    case 7:
      DisplayAbout();
      break;
    case 10:
      MoveInShop();
      break;
    case 11:
      MenuUpdate(4);
      break;
    case 13:
      EnterName();
      break;
    case 15:
      MenuUpdate(13);
      if(menuState == 1 || menuState == 3) CheckPlayer();
      if(menuState == 4) CheckEnemies();
      break;
  }
  CheckButton();
  if(!song1 && !song2 && isSinging && soundState) {
    if(millis() - soundStart > soundDuration) {noTone(buzzerPin); isSinging = false;}
  }
  if(song2 && soundState && inGameMusic) Song2();
  if(song1 && soundState) Song1();

}



/// ----------------------------------------- Matrix Display + Operations ----------------------------------------------



void UpdateMatrix() {  /// function called when there is a change in the matrix to refresh
  unsigned short mask = ((1 << (virtualMatrixSize - currCol)) - 1) - ((1 << (virtualMatrixSize - currCol - matrixSize)) - 1);
  for (int row = 0; row < matrixSize; row++) {
    lc.setRow(0, row, (matrix[row + currRow] & mask) >> (matrixSize - currCol));
  }
  //PrintVirtualMatrix();
}

void PrintVirtualMatrix() {  //// for debugging purposes
  for (int i = 0; i < virtualMatrixSize; i++) Serial.println(matrix[i]);
  Serial.println("--------");
  Serial.print(currRow);
  Serial.print(" ");
  Serial.print(currCol);
  Serial.print(" ");
  Serial.print(playerPosX);
  Serial.print(" ");
  Serial.println(playerPosY);
  Serial.println("--------");
}

void SetBitValue(byte row, byte col, byte value, bool flag = true) { /// Set the value of row, col in a relative(flag) to display 
  byte currBit = (matrix[row + currRow * flag] >> (virtualMatrixSize - col - 1 - currCol * flag)) & 1;
  if (currBit == value)
    ;
  else if (currBit == 0) {
    matrix[row + currRow * flag] = matrix[row + currRow * flag] + (1 << (virtualMatrixSize - 1 - col - currCol * flag));
  } else {
    matrix[row + currRow * flag] = matrix[row + currRow * flag] - (1 << (virtualMatrixSize - 1 - col - currCol * flag));
  }
}

inline byte GetBitValue(byte row, byte col, bool flag = true) { /// Get the value of row, col in a relative(flag) to display
  if(currRow * flag + row >= virtualMatrixSize || currCol * flag + col >= virtualMatrixSize) return 1; 
  return (matrix[row + currRow * flag] >> (virtualMatrixSize - 1 - col - currCol * flag)) & 1;
}



//// --------------------------------------- Joystick operations : Move + press ----------------------------------------



void CheckButton() {  /// Check if button was pressed - and handle every state
  buttonReading = digitalRead(buttonPin);
  if (buttonReading != buttonLastReading) lastDebounceTime = millis();
  if (millis() - lastDebounceTime > debounceDelay && buttonReading != buttonState) {
    buttonState = buttonReading;
    if (buttonReading == LOW
    ) {
      ////////////////////////// Insert Button Logic ////////////////////////
      if(state == 0) { //// skip introductory
        currNote = -1;
        song1 = LOW;
        noTone(buzzerPin);
        state = 1;
        ClearMatrix();
        InitMenu();
      }
      else if(state == 2) /// game logic
      {
        if(playerBullet1.shot == false) {
          playerBullet1.Shoot(playerPosX, playerPosY, playerDirectionX, playerDirectionY, true);
          soundDuration = musicNorm / noteDuration;
          soundStart = millis();
          if(soundState) tone(buzzerPin, shootSound, soundDuration);
          isSinging = true;
        }
      }
      else if(state == 8) {
        if(CheckLevel()) StartLevel();
      }
      else if(state == 3) { /// exit lose messages
        currNote = -1;
        song1 = LOW;
        maxNote = maxNoteSong1;
        state = 1;
        ClearMatrix();
        InitMenu();
      }
      else if(state == 13) {
        currNote = -1;
        song1 = LOW;
        maxNote = maxNoteSong1;
        state = 1;
        ClearMatrix();
        InitMenu();
        UpdateHighscores();
      }
      else if(state == 9) {
        currNote = -1;
        song1 = LOW;
        maxNote = maxNoteSong1;
        StartGame();
      }
      else if(state == 1) { /// for menu cases
        if(menuState == 0) { lives = 5; StartGame(); }
        else if(menuState == 1) EnterSettings();
        else if(menuState == 2) {
          lcd.clear();
          //lcd.setCursor(16, 1);
          //lcd.print(message);
          //lcd.autoscroll();
          aboutCounter = 0;
          DisplayAbout();
        }
        else if(menuState == 3) EnterHighScoreDisplay();
        else if(menuState == 4) EnterTutorial();
      }
      else if(state == 4) { //// for settings cases
        if(menuState == 0) LcdBrightnessDisplay();
        else if(menuState == 1) MatrixBrightnessDisplay();
        else if(menuState == 2) { ResetHighscores(); EnterSettings();}
        else if(menuState == 3) 
        {
          soundState = !soundState; 
          EEPROM.update(2, soundState);
          EnterSettings();
        }
        else if(menuState == 4) 
        {
          inGameMusic = !inGameMusic; 
          EnterSettings();
        }
        else if(menuState == 5) InitMenu();
      }
      else if(state == 5) { //// lcd brightness save
        EEPROM.update(0, lcdBrightnessId);
        EnterSettings();
      }
      else if(state == 6) { /// matrix brightness save
        EEPROM.update(1, matrixBrightness);
        ClearMatrix();
        EnterSettings();
      }
      else if(state == 7) { //// end about
        lcd.noAutoscroll();
        lcd.clear();
        InitMenu();
      }
      else if(state == 10) {
        if(shopState == 1) BuyLife();
        else if(shopState == 2) BuyFatigue();
        else if(shopState == 3) BuyMap();
        else if(shopState == 4) StartGame();
      }
      else if(state == 11 && menuState == 3) InitMenu(); 
      else if(state == 12) { EnterName(); EnterNameDisplay(); }
      else if(state == 15) {
        if(menuState == 12) InitMenu();
        if(menuState == 3) 
          if(playerBullet1.shot == false) {
            playerBullet1.Shoot(playerPosX, playerPosY, playerDirectionX, playerDirectionY, true);
          }
      }
    }
  }
  buttonLastReading = buttonReading;
}

byte CheckMove() { //// return joystick state
  int valueX = analogRead(xPin);
  int valueY = analogRead(yPin);

  if (valueX < minThreshold) return 1;
  if (valueX > maxThreshold) return 2;
  if (valueY > maxThreshold) return 3;
  if (valueY < minThreshold) return 4;

  return 0;
}



/// -------------------------------------- Program Intro + Animations Segment ----------------------------------------



void InitMessage() { ///// Print of initial message at the start of the program
  animationCounter = -1;
  state = 0;
  lcd.setCursor(0, 0);
  lcd.print(F("Hello there and "));
  lcd.setCursor(0, 1);
  lcd.print(F("welcome!        "));
}

void Animation1(short afterState) { ////// Animation on matrix of a line moving horizontally
  if(millis() - lastAnimation > animation1Delay) {
    if(animationCounter == -1);
    else matrix[animationCounter] = 0;
    matrix[++animationCounter] = (1 << virtualMatrixSize) - 1;
    if(animationCounter == matrixSize) {
      animationCounter = -1;
      state = afterState;
      if(state == 0 && millis() - soundStart > song1Duration)
        InitMenu();
    }
    matrixChanged = HIGH;
    lastAnimation = millis();
  }
}



//// ------------------------------- During gameplay operations - player, enemies --------------------------------------



void CheckPlayer() { /// every "interval" check or change for player and  ++++++++++++ handle bullet ++++++++++++
  if (millis() - lastMovedJoystick > moveCheckIntervalPlayer && state != 15) {  /// movement of player
    CheckMovePlayer(CheckMove());
    lastMovedJoystick = millis();
  }
  
  if(millis() - lastBlinkBullet > blinkIntervalBullet) {
    bulletState = !bulletState;
    matrixChanged = HIGH;
    lastBlinkBullet = millis();
  }

  if (millis() - lastBlinkPlayer > blinkIntervalPlayer) {  /// blink of player
    byte value = GetBitValue(playerPosX, playerPosY);
    if (value == 0) value = 1;
    else value = 0;
    SetBitValue(playerPosX, playerPosY, value);
    lastBlinkPlayer = millis();
    matrixChanged = HIGH;
  }
  if(playerBullet1.shot == true)  {
    playerBullet1.CheckMove();
  }
}

void CheckMovePlayer(byte move) {  /// Movement of player based on joystick input

  playerLastPosX = playerPosX;
  playerLastPosY = playerPosY;
  playerLastDirectionX = playerDirectionX;
  playerLastDirectionY = playerDirectionY;

  if (move == 1) {
    Serial.println("dreapta");
    if (playerPosX < matrixSize - 1 - moveIndent) playerPosX++;
    else if (currRow == virtualMatrixSize - matrixSize) {
      if (playerPosX != matrixSize - 1) playerPosX++;
    } else if(GetBitValue(playerPosX + 1, playerPosY) != 1) {
      SetBitValue(playerLastPosX, playerLastPosY, 0);
      currRow++;
      SetBitValue(playerPosX, playerPosY, 1);
      matrixChanged = true;
      if(state == 8) DisplayLevelMultiplier();
    }

    playerDirectionX = 1;
    playerDirectionY = 0;
  } else if (move == 2) {
    Serial.println("stanga");
    if (playerPosX > moveIndent) playerPosX--;
    else if (currRow == 0) {
      if (playerPosX != 0) playerPosX--;
    } else if(GetBitValue(playerPosX - 1, playerPosY) != 1) {
      SetBitValue(playerLastPosX, playerLastPosY, 0);
      currRow--;
      SetBitValue(playerPosX, playerPosY, 1);
      matrixChanged = true;
      if(state == 8) DisplayLevelMultiplier();
    }
    playerDirectionX = -1;
    playerDirectionY = 0;
  }

  else if (move == 3) {
    Serial.println("jos");
    if (playerPosY < matrixSize - 1 - moveIndent) playerPosY++;
    else if (currCol == virtualMatrixSize - matrixSize) {
      if (playerPosY != matrixSize - 1) playerPosY++;
    } else if(GetBitValue(playerPosX, playerPosY + 1) != 1) {
      SetBitValue(playerLastPosX, playerLastPosY, 0);
      currCol++;
      SetBitValue(playerPosX, playerPosY, 1);
      matrixChanged = true;
      if(state == 8) DisplayLevelMultiplier();
    }
    playerDirectionX = 0;
    playerDirectionY = 1;
  } else if (move == 4) {
    Serial.println("sus");
    if (playerPosY > moveIndent) playerPosY--;
    else if (currCol == 0) {
      if (playerPosY != 0) playerPosY--;
    } else if(GetBitValue(playerPosX, playerPosY - 1) != 1) {
      SetBitValue(playerLastPosX, playerLastPosY, 0);
      currCol--;
      SetBitValue(playerPosX, playerPosY, 1);
      matrixChanged = true;
      if(state == 8) DisplayLevelMultiplier();
    }
    Serial.println(playerPosY);
    playerDirectionX = 0;
    playerDirectionY = -1;
  }

  if (((playerPosX != playerLastPosX || playerPosY != playerLastPosY) && CheckPos(playerPosX, playerPosY, false) != 1)
      // && (bulletShot == LOW || (bulletPosX != playerPosX || bulletPosY != playerPosY))
  ) {  /// can't move on top of wall, bullet
    matrixChanged = true;
    SetBitValue(playerLastPosX, playerLastPosY, 0);
    SetBitValue(playerPosX, playerPosY, 1);
    lastMovedJoystick = millis();
    if(state == 8) DisplayLevelMultiplier();
  } else {  /// if the movement is illegal, take previous position
    playerPosX = playerLastPosX;
    playerPosY = playerLastPosY;
  }
  //if(playerLastDirectionX != playerDirectionX || playerLastDirectionY != playerDirectionY) DirectionLedLight(); /// only update direction leds if the direction has changed
}

void CheckEnemies() { ///// every "interval" check or change for enemy and handle bullets if needed
  if((millis() - lastBlinkEnemy > blinkIntervalEnemyOff && enemyState == 0) || 
  (millis() - lastBlinkEnemy > blinkIntervalEnemyOn && enemyState == 1) ) {
    enemyState = !enemyState;
    matrixChanged = HIGH;
    lastBlinkEnemy = millis();
  }

  for(byte iterator = 0; iterator < maxNumOfEnemies; iterator++) {
    if(enemyList[iterator].isAlive) {
      SetBitValue(enemyList[iterator].posX, enemyList[iterator].posY, enemyState, false);
      if(state != 15) enemyList[iterator].CheckForPlayer();
    }
    if(state != 15) enemyList[iterator].MoveBullet();
  }

}



//// -------------------------------------- Menu Display on Lcd functions for each state ----------------------------------------



void InitMenu() { //// initialize menu display and state
  lcd.clear();
  state = 1;
  lcd.setCursor(0, 0);
  lcd.print(F("Welcome!"));
  menuState = 0;
  MenuEntriesPrint();
  lastMovedJoystick = millis();
}

void MenuEntriesPrint() { //// first set of options 
  lcd.setCursor(0, 1);
  if (menuState == 0) {
    lcd.print(F("Start game!    "));
    MatrixStart();
  } else if(menuState == 1) {
    lcd.print("Settings       ");
    MatrixSettings();
  } else if(menuState == 2) {
    lcd.print(F("About          "));
    MatrixAbout();
  } else if(menuState == 3) {
    lcd.print("Highscores     ");
    MatrixHighscore();
  } else if(menuState == 4) {
    lcd.print(F("Tutorial       "));
    MatrixTutorial();
  }
  lcd.setCursor(15, 1);
  if (menuState == 0) {
    lcd.write(byte(0));
  } else if (menuState == numOfMenuStates - 1) {
    lcd.write(2);
  } else {
    lcd.write(byte(1));
  }
}

void MenuUpdate(byte maxStates) { //// Check if joystick moved and update lcd and menuState
  if(millis() - lastMovedJoystick > moveCheckInterval) { 
    bool moved = false;
      byte move = CheckMove();
      if(move == 0) {lastMovedJoystick = millis(); return;}
      else if(move == 1){
        if(state == 5) {
          if(lcdBrightnessId < maxStates - 1) {
            lcdBrightnessId++;
            LcdBrightnessDisplay();
            moved = true;
          }
        }
        else if(state == 6) {
          if(matrixBrightness < maxStates - 2) {
            matrixBrightness += 2;
            MatrixBrightnessDisplay();
            moved = true;
          }
        }
      }
      else if(move == 2) {
        if(state == 5) {
          if(lcdBrightnessId > 0) {
            lcdBrightnessId--;
            LcdBrightnessDisplay();
            moved = true;
          }
        }
        else if(state == 6) {
          if(matrixBrightness > 1) {
            matrixBrightness -= 2;
            MatrixBrightnessDisplay();
            moved = true;
          }
        }
      }
      else {
        if(move == 3 && menuState < maxStates - 1 
        && (state == 1 || state == 4 || state == 11 || state == 15)
        ) {
          moved = true;
          menuState += 1;
          if(state == 1) MenuEntriesPrint();
          else if(state == 4) DisplaySettings();
          else if(state == 11) DisplayHighscores();
          else if(state == 15) DisplayTutorial();
        }
        else if(move == 4 && menuState > 0
        && (state == 1 || state == 4 || state == 11)
        ) {
          moved = true;
          menuState -= 1;
          if(state == 1) MenuEntriesPrint();
          else if(state == 4) DisplaySettings();
          else if(state == 11) DisplayHighscores();
        }
      }
      if(moved == true) {
        soundDuration = musicNorm / noteDuration;
        soundStart = millis();
        if(soundState) tone(buzzerPin, scrollSound, soundDuration);
        isSinging = true;
      }
      else {
        soundDuration = musicNorm / noteDuration;
        soundStart = millis();
        if(soundState) tone(buzzerPin, outOfBoundsSound, soundDuration);
        isSinging = true;
      }
      lastMovedJoystick = millis();
    }
}

void EnterSettings() { ///// initialize settings option
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Settings"));
  MatrixSettings();
  state = 4;
  menuState = 0;
  DisplaySettings();
  lastMovedJoystick = millis();
}

void DisplaySettings() { ///// second set of options - for settings
  lcd.setCursor(0, 1);
  if(menuState == 0) lcd.print(F("LCD brightness   "));
  else if(menuState == 1) lcd.print("Matrix brightns ");
  else if(menuState == 2) lcd.print(F("Reset Highscore"));
  else if(menuState == 3) { 
    lcd.print("Sound : ");
    if(soundState == HIGH) lcd.print("ON      ");
    else lcd.print("OFF       ");
  }
  else if(menuState == 4) { 
    lcd.print("Music : ");
    if(inGameMusic == HIGH) lcd.print("ON       ");
    else lcd.print("OFF        ");
  }
  else if(menuState == 5) lcd.print(F("Back            "));
  lcd.setCursor(15, 1);
  if (menuState == 0) {
    lcd.write(byte(0));
  } else if (menuState == numOfSettings - 1) {
    lcd.write(2);
  } else {
    lcd.write(byte(1));
  }
}

void LcdBrightnessDisplay() { //// Lcd update for brightness setting of lcd
  state = 5;
  lcd.clear();
  analogWrite(lcdBrightnessPin, lcdBrightnessValues[lcdBrightnessId]);
  lcd.setCursor(0, 0);
  lcd.print("LCD:");
  lcd.setCursor(0, 1);
  lcd.print("    -");
  for(int level = 0; level < 6; level++) {
    if(level <= lcdBrightnessId) lcd.write(3);
    else lcd.write(" ");
  }
  lcd.print("+    ");
}

void MatrixBrightnessDisplay() { //// Lcd update for brightness setting of matrix
  state = 6;
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Matrix:");
  lc.setIntensity(0, matrixBrightness);
  lcd.setCursor(0, 1);
  lcd.print("   -");
  for(int level = 0; level < 8; level++) {
    if(level*2 <= matrixBrightness) lcd.write(3);
    else lcd.print(" ");
  }
  lcd.print("+    ");
  int maxValue = (1 << virtualMatrixSize) - 1;
  for(int row = 0; row < virtualMatrixSize; row++) matrix[row] = maxValue;
  UpdateMatrix();
}

void ResetHighscores() { //// Puts all highscores on 0, leaving the names
  for(byte i = 0; i < 3; i++) highscores[i] = 0;
  HighscoresToEEPROM();
}

void DisplayAbout() { ///// lcd updates for about display
  state = 7;
  if(millis() - lastAboutDisplay > aboutDelay) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message.substring(aboutCounter, aboutCounter + 16));
    lcd.setCursor(0, 1);
    lcd.print("     About         ");
    aboutCounter++;
    if(aboutCounter == message.length() - 15) aboutCounter = 0;
    lastAboutDisplay = millis();
  }
}

void EnterHighScoreDisplay() { ///// initialize settings option
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Highscores");
  state = 11;
  menuState = 0;
  DisplayHighscores();
  lastMovedJoystick = millis();
}

void DisplayHighscores() { //// Display each highscore on lcd
  lcd.setCursor(0, 1);
  if(menuState != 3) {
    lcd.print(names[menuState]);
    lcd.setCursor(3, 1);
    lcd.print(" ");
    lcd.print(highscores[menuState]);
    lcd.print(F("            "));
  }
  else {
    lcd.print(F("Back            "));
  }
  lcd.setCursor(15, 1);
  if (menuState == 0) {
    lcd.write(byte(0));
  } else if (menuState == 3) {
    lcd.write(2);
  } else {
    lcd.write(byte(1));
  }
}

void EnterTutorial() { //// Tutorial initialization
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("How to play     ");
  menuState = 0;
  state = 15;
  DisplayTutorial();
}

void DisplayTutorial() { /// Display each step of the tutorial
  lcd.setCursor(0, 1);
  switch(menuState) {
    case 0:
      lcd.print(F("The elements:  "));
      break;
    case 1: 
      ClearMatrix();
      lcd.print(F("That's you!    "));
      currRow = 0;
      currCol = 0;
      playerPosX = 3;
      playerPosY = 3;
      break;
    case 2: 
      ClearMatrix();
      lcd.print(F("You can move   "));
      break;
    case 3: 
      lcd.print(F("press to shoot "));
      break;
    case 4:
      ClearMatrix();
      playerBullet1.shot = false;
      enemyList[0].isAlive = true;
      enemyList[0].posX = 3;
      enemyList[0].posY = 3;
      enemyList[0].bullet.shot = false;
      lcd.write(5);
      lcd.setCursor(1, 1);
      lcd.print(F(" : enemy         "));
      break;
    case 5:
      ClearMatrix();
      enemyList[0].isAlive = false;
      lcd.write(4);
      lcd.setCursor(1, 1);
      lcd.print(F(" : lives left      "));
      break;
    case 6:
      lcd.write(6);
      lcd.setCursor(1, 1);
      lcd.print(F(" : coins          "));
      break;
    case 7:
      lcd.write(7);
      lcd.setCursor(1, 1);
      lcd.print(F(" : your score      "));
      break;
    case 8:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("You must defeat"));
      lcd.setCursor(0, 1);
      lcd.print(F("all enemies"));
      break;
    case 9:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("to get coins and"));
      lcd.setCursor(0, 1);
      lcd.print(F("exit level."));
      break;
    case 10:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("From the ports"));
      lcd.setCursor(0, 1);
      lcd.print(F("you can choose"));
      break;
    case 11:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("an island and"));
      lcd.setCursor(0, 1);
      lcd.print(F("enter a level."));
      break;
    case 12:
      lcd.clear();
      lcd.print(F("Press for menu"));
      break;
  }
  lcd.setCursor(15, 1);
  if (menuState == 12);
  else {
    lcd.write(byte(0));
  }
}



//// ------------------------------------ Matrix Display for Menu States -------------------------------


void MatrixHighscore() { //// Matrix Initialization for highscore
  matrix[0] = 0; matrix[4] = 32512; matrix[8] = 0; matrix[12] = 0;
  matrix[1] = 24576; matrix[5] = 30976; matrix[9] = 0; matrix[13] = 0;
  matrix[2] = 30976; matrix[6] = 24576; matrix[10] = 0; matrix[14] = 0;
  matrix[3] = 32512; matrix[7] = 0; matrix[11] = 0; matrix[15] = 0;
  currCol = 0;
  currRow = 0;
  matrixChanged = HIGH;
}

void MatrixAbout() { //// Matrix Initialization for about
  matrix[0] = 0; matrix[4] = 1024; matrix[8] = 0; matrix[12] = 0;
  matrix[1] = 2048; matrix[5] = 9216; matrix[9] = 0; matrix[13] = 0;
  matrix[2] = 9216; matrix[6] = 2048; matrix[10] = 0; matrix[14] = 0;
  matrix[3] = 1024; matrix[7] = 0; matrix[11] = 0; matrix[15] = 0;
  currCol = 0;
  currRow = 0;
  matrixChanged = HIGH;
}

void MatrixStart() { //// Matrix Initialization for start game
  matrix[0] = 4096; matrix[4] = 65280; matrix[8] = 0; matrix[12] = 0;
  matrix[1] = 64512; matrix[5] = 65280; matrix[9] = 0; matrix[13] = 0;
  matrix[2] = 1024; matrix[6] = 30464; matrix[10] = 0; matrix[14] = 0;
  matrix[3] = 30464; matrix[7] = 0; matrix[11] = 0; matrix[15] = 0;
  currCol = 0;
  currRow = 0;
  matrixChanged = HIGH;
}

void MatrixTutorial() { //// Matrix Initialization for tutorial
  matrix[0] = 0; matrix[4] = 36096; matrix[8] = 0; matrix[12] = 0;
  matrix[1] = 0; matrix[5] = 28672; matrix[9] = 0; matrix[13] = 0;
  matrix[2] = 16384; matrix[6] = 0; matrix[10] = 0; matrix[14] = 0;
  matrix[3] = 32768; matrix[7] = 0; matrix[11] = 0; matrix[15] = 0;
  currCol = 0;
  currRow = 0;
  matrixChanged = HIGH;
}

void MatrixSettings() { //// Matrix Initialization for settings
  matrix[0] = 512; matrix[4] = 63488; matrix[8] = 0; matrix[12] = 0;
  matrix[1] = 1792; matrix[5] = 31744; matrix[9] = 0; matrix[13] = 0;
  matrix[2] = 11776; matrix[6] = 14336; matrix[10] = 0; matrix[14] = 0;
  matrix[3] = 29696; matrix[7] = 4096; matrix[11] = 0; matrix[15] = 0;
  currCol = 0;
  currRow = 0;
  matrixChanged = HIGH;
}


//// ----------------------------------------- Ports Segment ------------------------------------------



void StartGame() { //// start game initialization
  playerDirectionX = 1;
  playerDirectionY = 0;
  playerLastDirectionX = 1;
  playerLastDirectionY = 0;
  state = 8;
  ClearMatrix();
  if(currMap == 1) InitializeMapPort();
  if(currMap == 2) InitializeMapPort2();
  if(currMap == 3) InitializeMapPort3();
  if(currMap == 4) InitializeMapPort4();
  SetBitValue(playerPosX, playerPosY, 1);
  matrixChanged = HIGH;
  InitDisplayGame();
  if(song2 == LOW)
  {
    song2 = HIGH;
    currNote = -1;
  }
}

void BlinkPorts() { ///// Function that handle port blinking on mpa
  //// 2 4, 11 3, 2 10, 12 13
  if(millis() - lastPortBlink > portBlinkInterval) {
    if(currMap == 1) {
      SetBitValue(2, 4, !GetBitValue(2, 4, false), false);
      SetBitValue(11, 3, !GetBitValue(11, 3, false), false);
      SetBitValue(2, 10, !GetBitValue(2, 10, false), false);
      SetBitValue(12, 13, !GetBitValue(12, 13, false), false);
      if(numOfMaps > 1) {
        SetBitValue(15, 6, !GetBitValue(15, 6, false), false);
      }
      if(numOfMaps >= 4) {
        SetBitValue(6, 0, !GetBitValue(6, 0, false), false);
      }
    }
    else if(currMap == 2) {
      SetBitValue(3, 4, !GetBitValue(3, 4, false), false);
      SetBitValue(12, 6, !GetBitValue(12, 6, false), false);
      SetBitValue(8, 12, !GetBitValue(8, 12, false), false);
      SetBitValue(0, 6, !GetBitValue(0, 6, false), false);
      if(numOfMaps >= 3) {
        SetBitValue(6, 0, !GetBitValue(6, 0, false), false);
      }
    }
    else if(currMap == 3) {
      SetBitValue(3, 4, !GetBitValue(3, 4, false), false);
      SetBitValue(13, 1, !GetBitValue(13, 1, false), false);
      SetBitValue(1, 14, !GetBitValue(1, 14, false), false);
      SetBitValue(11, 10, !GetBitValue(11, 10, false), false);
      SetBitValue(6, 15, !GetBitValue(6, 15, false), false);
      if(numOfMaps >= 4) {
        SetBitValue(0, 6, !GetBitValue(0, 6, false), false);
      }
    }
    if(currMap == 4) {
      SetBitValue(7, 3, !GetBitValue(7, 3, false), false);
      SetBitValue(2, 10, !GetBitValue(2, 10, false), false);
      SetBitValue(13, 13, !GetBitValue(13, 13, false), false);
      SetBitValue(6, 15, !GetBitValue(6, 15, false), false);
      SetBitValue(15, 6, !GetBitValue(15, 6, false), false);
    }
    lastPortBlink = millis();
    matrixChanged = HIGH;
  }
}

void DisplayLevelMultiplier() { //// Change lcd to display multiplier of level
  lcd.setCursor(0, 1);
  if(currMap == 1) {
    if(playerPosX + currRow == 2 && playerPosY + currCol == 4) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[0]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 11 && playerPosY + currCol == 3) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[1]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 2 && playerPosY + currCol == 10) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[2]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 12 && playerPosY + currCol == 13) lcd.print(F("Shop            "));
    else if(numOfMaps > 1 && playerPosX + currRow == 15 && playerPosY + currCol == 6) lcd.print(F("To Map 2          "));
    else if(numOfMaps >= 4 && playerPosX + currRow == 6 && playerPosY + currCol == 0) lcd.print(F("To Map 4          "));
    else lcd.print("Choose island");
  }
  else if(currMap == 2) {
    if(playerPosX + currRow == 3 && playerPosY + currCol == 4) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[3]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 12 && playerPosY + currCol == 6) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[4]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 8 && playerPosY + currCol == 12) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[7]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 0 && playerPosY + currCol == 6) lcd.print(F("To Map 1          "));
    else if(numOfMaps >= 3 && playerPosX + currRow == 6 && playerPosY + currCol == 0) lcd.print(F("To Map 3          "));
    else lcd.print("Choose island");
  }
  else if(currMap == 3) {
    if(playerPosX + currRow == 3 && playerPosY + currCol == 4) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[5]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 13 && playerPosY + currCol == 1) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[10]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 1 && playerPosY + currCol == 14) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[9]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 11 && playerPosY + currCol == 10) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[8]);
      lcd.print("          ");
    }
    else if(numOfMaps > 1 && playerPosX + currRow == 6 && playerPosY + currCol == 15) lcd.print(F("To Map 2          "));
    else if(numOfMaps >= 4 && playerPosX + currRow == 0 && playerPosY + currCol == 6) lcd.print(F("To Map 4          "));
    else lcd.print("Choose island");
  }
  else if(currMap == 4) {
    if(playerPosX + currRow == 7 && playerPosY + currCol == 3) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[11]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 2 && playerPosY + currCol == 10) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[12]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 13 && playerPosY + currCol == 13) {
      lcd.print(F("Mult : "));
      lcd.print(mulLevel[6]);
      lcd.print("          ");
    }
    else if(playerPosX + currRow == 6 && playerPosY + currCol == 15) lcd.print(F("To Map 1          "));
    else if(playerPosX + currRow == 15 && playerPosY + currCol == 6) lcd.print(F("To Map 3          "));
    else lcd.print("Choose island");
  }
}

bool CheckLevel() { //// enter level if clicked
  if(currMap == 1) {
    if(playerPosX + currRow == 2 && playerPosY + currCol == 4) InitializeMap1();
    else if(playerPosX + currRow == 11 && playerPosY + currCol == 3) InitializeMap2();
    else if(playerPosX + currRow == 2 && playerPosY + currCol == 10) InitializeMap3();
    else if(playerPosX + currRow == 12 && playerPosY + currCol == 13) {
      EnterShop();
      return 0;
    }
    else if(numOfMaps > 1 && playerPosX + currRow == 15 && playerPosY + currCol == 6) {
      currMap = 2;
      StartGame();
      return 0;
    }
    else if(numOfMaps >= 4 && playerPosX + currRow == 6 && playerPosY + currCol == 0) {
      currMap = 4;
      StartGame();
      return 0;
    }
    else return 0;
  }
  else if(currMap == 2) {
    if(playerPosX + currRow == 3 && playerPosY + currCol == 4) InitializeMap4();
    else if(playerPosX + currRow == 12 && playerPosY + currCol == 6) InitializeMap5();
    else if(playerPosX + currRow == 8 && playerPosY + currCol == 12) InitializeMap8();
    else if(playerPosX + currRow == 0 && playerPosY + currCol == 6) {
      currMap = 1;
      StartGame();
      return 0;
    }
    else if(numOfMaps >= 3 && playerPosX + currRow == 6 && playerPosY + currCol == 0) {
      currMap = 3;
      StartGame();
      return 0;
    }
    else return 0;
  }
  else if(currMap == 3) {
    if(playerPosX + currRow == 3 && playerPosY + currCol == 4) InitializeMap6();
    else if(playerPosX + currRow == 13 && playerPosY + currCol == 1) InitializeMap11();
    else if(playerPosX + currRow == 1 && playerPosY + currCol == 14) InitializeMap10();
    else if(playerPosX + currRow == 11 && playerPosY + currCol == 10) InitializeMap9();
    else if(playerPosX + currRow == 6 && playerPosY + currCol == 15) {
      currMap = 2;
      StartGame();
      return 0;
    }
    else if(numOfMaps >= 4 && playerPosX + currRow == 0 && playerPosY + currCol == 6) {
      currMap = 4;
      StartGame();
      return 0;
    }
    else return 0;
  }
  else if(currMap == 4) {
    if(playerPosX + currRow == 7 && playerPosY + currCol == 3) InitializeMap12();
    else if(playerPosX + currRow == 2 && playerPosY + currCol == 10) InitializeMap13();
    else if(playerPosX + currRow == 13 && playerPosY + currCol == 13) InitializeMap7();
    else if(playerPosX + currRow == 6 && playerPosY + currCol == 15) {
      currMap = 1;
      StartGame();
      return 0;
    }
    else if(playerPosX + currRow == 15 && playerPosY + currCol == 6) {
      currMap = 3;
      StartGame();
      return 0;
    }
    else return 0;
  }
  return 1;
}

void InitDisplayGame() { //// Initialization of lcd display 
  lcd.clear();
  DisplayScore();
  DisplayCoins();
  lcd.setCursor(0, 1);
  lcd.print(F("Choose island"));
}

void DisplayScore() { /// routine for score on lcd
  lcd.setCursor(0, 0);
  lcd.write(7);
  lcd.setCursor(1, 0);
  lcd.print(score);
}

void DisplayCoins() { /// routine for coins on lcd
  lcd.setCursor(7, 0);
  lcd.write(6);
  lcd.print(numOfCoins);
}



//// ---------------------- Gameplay Functions : Initialize + Cleaning + Lcd Display + Exit cases for levels -------------------------



void StartLevel() { //// Start game routine
  playerDirectionX = 1;
  playerDirectionY = 0;
  playerLastDirectionX = 1;
  playerLastDirectionY = 0;
  state = 2;
  SetBitValue(playerPosX, playerPosY, 1);
  UpdateMatrix();
  DuringGameDisplay();
  EnemySpawn(numOfEnemies);
  song2 = LOW;
  currNote = -1;
  noTone(buzzerPin);
}

void EnemySpawn(byte numOfEnemies) { ///// Enemy Spawn routine - random positions
  byte ct = 0;
  while(ct != numOfEnemies) {
    int valueX = random(0, spawnAux) % virtualMatrixSize;
    int valueY = random(0, spawnAux) % virtualMatrixSize;
    if(GetBitValue(valueX, valueY, false));
    else if(valueX == playerPosX + currRow && valueY == playerPosY + currCol - 1);
    else if(valueX == playerPosX + currRow && valueY == playerPosY + currCol + 1);
    else if(valueY == playerPosY + currCol && valueX == playerPosX + currRow - 1);
    else if(valueY == playerPosY + currCol && valueX == playerPosX + currRow + 1);
    else if(valueX == playerPosX + currRow - 1 && valueY == playerPosY + currCol - 1);
    else if(valueX == playerPosX + currRow + 1 && valueY == playerPosY + currCol + 1);
    else if(valueY == playerPosY + currCol + 1 && valueX == playerPosX + currRow - 1);
    else if(valueY == playerPosY + currCol - 1 && valueX == playerPosX + currRow + 1);
    else {
      enemyList[ct].isAlive = true;
      enemyList[ct].posX = valueX;
      enemyList[ct].posY = valueY;
      SetBitValue(valueX, valueY, 1, false);
      enemyList[ct].viewDistance = viewDistanceForEnemies;
      ct++;
    }
  }
}

void DeleteAllEnemies() { ///// Sets all enemies as dead and deletes bullets
  for(byte iterator = 0; iterator < maxNumOfEnemies; iterator++) {
    enemyList[iterator].isAlive = false;
    enemyList[iterator].bullet.shot = false;
  }
}

void DuringGameDisplay() { ///// LCD update to display info in game
  lcd.clear();
  DisplayScore();
  DisplayCoins();
  DisplayLives();
  DisplayEnemies();
}

void DisplayLives() { //// Display lives left
  lcd.setCursor(0, 1);
  lcd.write(4);
  lcd.print(lives);
}

void DisplayEnemies() { //// Display number of enemies left
  lcd.setCursor(7, 1);
  lcd.write(5);
  lcd.print(numOfEnemies);
}

void ExitGameOnLose() { //// Handle lose - display messages and clean
  lcd.setCursor(0, 0);
  lcd.print("You Lost! Press ");
  lcd.setCursor(0, 1);
  if(score <= highscores[2]) lcd.print("    for menu!   ");
  ClearMatrix();
  DeleteAllEnemies();
  currRow = 0;
  currCol = 0;
  state = 3;
  Serial.println("ExitGameOnLose");
  if(score > highscores[2]) {
    lcd.print(" to enter name  ");
    state = 12;
  }
  else score = 0;
  lives = 5;
  numOfMaps = 1;
  shootIntervalEnemy = 1000;
  numOfCoins = 0;
  currMap = 1;
  
  maxNote = 16;
  currNote = 7;
  song1 = HIGH;
  if(soundState) Song1();

  playerBullet1.shot = false;
}

void EnterName() { /// routine for changing name
  state = 13;
  if (millis() - lastMovedJoystick > moveCheckInterval) {
    byte move = CheckMove();
    if(move == 0);
    else if(move == 1) {
      if(currPosName < 3) {currPosName++; EnterNameDisplay();}
    }
    else if(move == 2) {
      if(currPosName > 0) {currPosName--; EnterNameDisplay();}
    }
    else if(move == 3) {
      if(name[currPosName] == 'z') {
        name[currPosName] = 'a';
        EnterNameDisplay();
      }
      else {
        name[currPosName]++;
        EnterNameDisplay();
      }
    }
    else if(move == 4) {
      if(name[currPosName] == 'a'){
        name[currPosName] = 'z';
        EnterNameDisplay();
      }
      else {
        name[currPosName]--;
        EnterNameDisplay();
      }
    }
    lastMovedJoystick = millis();
  }
}

void EnterNameDisplay() { /// lcd display of name
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("      ");
  if(currPosName == 0) lcd.write(byte(0));
  else if(currPosName == 1) { lcd.write(" "); lcd.write(byte(0)); }
  else { lcd.write("  "); lcd.write(byte(0)); }
  lcd.setCursor(0, 1);
  lcd.print("      ");
  lcd.print(name);
  lcd.setCursor(9, 1);
  lcd.print("       ");
}

void UpdateHighscores() { /// routine for bringing highscore up to date
  if(score > highscores[0]) {
    highscores[2] = highscores[1];
    highscores[1] = highscores[0];
    highscores[0] = score;
    for(byte i = 0; i < 3; i++) {
      names[2][i] = names[1][i];
      names[1][i] = names[0][i];
      names[0][i] = name[i];
    }
  }
  else if(score > highscores[1]) {
    highscores[2] = highscores[1];
    highscores[1] = score;
    for(byte i = 0; i < 3; i++) {
      names[2][i] = names[1][i];
      names[1][i] = name[i];
    }
  }
  else if(score > highscores[2]) {
    highscores[2] = score;
    for(byte i = 0; i < 3; i++) {
      names[2][i] = name[i];
    }
  }
  else return;
  score = 0;
  HighscoresToEEPROM();
}

void ExitGameOnWin() { //// Handle win - display messages and clean
  numOfCoins += levelValue;
  lcd.setCursor(0, 1);
  lcd.print("You Won!       ");
  ClearMatrix();
  DeleteAllEnemies();
  currRow = 0;
  currCol = 0;
  state = 9;
  maxNote = 8;
  currNote = -1;
  song1 = HIGH;
  if(soundState) Song1();
  playerBullet1.shot = false;
}



//// -------------------------------------------- Shop functions ----------------------------------------------



void EnterShop() { //// Shop logic initialization
  state = 10;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Buy potion ");
  lcd.write(6);
  lcd.print(numOfCoins);
  shopState = 1;
  DisplayCurrentItem();
}

void DisplayCurrentItem() { //// current item of shop on lcd
  lcd.setCursor(0, 1);
  switch(shopState) {
    case 1 : 
      lcd.print("Life - 10 ");
      lcd.setCursor(10, 1);
      lcd.write(6);
      lcd.print("       ");
      lcd.setCursor(15, 1);
      lcd.write(byte(0));
      break;
    case 2 :
      lcd.write(5);
      lcd.setCursor(1, 1);
      lcd.print("Fatigue - 20");
      lcd.setCursor(13, 1);
      lcd.write(6);
      lcd.print("       ");
      lcd.setCursor(15, 1);
      lcd.write(byte(1));
      break;
    case 3 :
      if(numOfMaps < 4) {
        lcd.write("New map - 30");
        lcd.write(6);
        lcd.print("       ");
      }
      else {
        lcd.write("Out of Stock     ");
      }
      lcd.setCursor(15, 1);
      lcd.write(1);
      break;
    case 4:
      lcd.write("Exit            ");
      lcd.setCursor(15, 1);
      lcd.write(2);
  }
}

void MoveInShop() { //// joystick logic for inshop movement
  if (millis() - lastMovedJoystick > moveCheckInterval) {
    byte move = CheckMove();
    if(move == 4 && shopState > 1) {
      shopState--;
      DisplayCurrentItem();
    }
    if(move == 3 && shopState < 4) {
      shopState++;
      DisplayCurrentItem();
    }
    lastMovedJoystick = millis();
  }
}

void BuyLife() { //// Handle buy life
  if(numOfCoins >= 10) {
    numOfCoins -= 10;
    lives++;
    EnterShop();
  }
}

void BuyFatigue() { //// Handle buy fatigue for enemy
  if(numOfCoins >= 20) {
    numOfCoins -= 20;
    shootIntervalEnemy += 100;
    EnterShop();
  }
}

void BuyMap() {
  if(numOfCoins >= 30 && numOfMaps < 4) {
    numOfCoins -= 30;
    numOfMaps++;
    EnterShop();
  }
}



//// -------------------------------------------- Music Segment -----------------------------------------------



short GetNoteSong1() { //// return note of song1
  switch(currNote) {
    case 0: soundStart = millis(); return 1047;
    case 1: return 1245;
    case 2: return 1175;
    case 3: return 1568;
    case 4: return 1047;
    case 5: return 1245;
    case 6: return 1175;
    case 7: return 784;
    case 8: return 523;
    case 9: return 622;
    case 10: return 587;
    case 11: return 784;
    case 12: return 523;
    case 13: return 622;
    case 14: return 584;
    case 15: return 392;
  }
  return -1;
}

short GetNoteDurationsSong1() { //// return duration of note for song 1
  switch(currNote) {
    case 0: return 4;
    case 1: return 4;
    case 2: return 4;
    case 3: return 2;
    case 4: return 4;
    case 5: return 4;
    case 6: return 4;
    case 7: return 2;
    case 8: return 4;
    case 9: return 4;
    case 10: return 4;
    case 11: return 2;
    case 12: return 4;
    case 13: return 4;
    case 14: return 4;
    case 15: return 2;
  }
  return 4;
}

void Song1() { //// handle song 1 notes
  int noteDuration = musicNorm / GetNoteDurationsSong1();
    if((millis() - startNote) > noteDuration){
      int pauseBetweenNotes = noteDuration * pauseFactor;
      if((millis() - startNote) > (noteDuration + pauseBetweenNotes)){
        currNote += 1;
        startNote = millis();
        noteDuration = musicNorm / GetNoteDurationsSong1();
        if(soundState) tone(buzzerPin, GetNoteSong1(), noteDuration);
        if(currNote == maxNote){
          int x = millis() - soundStart;
          Serial.println(x);
          currNote = -1;
          song1 = LOW;
          noTone(buzzerPin);
          if(state == 0) song1 = LOW;
        }
      }
    }
}

short GetNoteSong2() { //// return note of song2
  if(currNote < 13) {
    switch(currNote) {
      case 0 : soundStart = millis(); return 147;
      case 1 : return 220;
      case 2 : return 349;
      case 3 : return 220;
      case 4 : return 147;
      case 5 : return 220;
      case 6 : return 349;
      case 7 : return 220;
      case 8 : return 147;
      case 9 : return 220;
      case 10 : return 349;
      case 11 : return 220;
      case 12 : return 147;
    }
  }
  else if(currNote < 26) {
    switch(currNote) {
      case 13 : return 220;
      case 14 : return 587;
      case 15 : return 554;
      case 16 : return 523;
      case 17 : return 523;
      case 18 : return 523;
      case 19 : return 494;
      case 20 : return 466;
      case 21 : return 440;
      case 22 : return 440;
      case 23 : return 415;
      case 24 : return 392;
      case 25 : return 349;
    }
  }
  else if(currNote < 39) {
    switch(currNote) {
      case 26 : return 392;
      case 27 : return 342;
      case 28 : return 294;
      case 29 : return 330;
      case 30 : return 349;
      case 31 : return 330;
      case 32 : return 294;
      case 33 : return 587;
      case 34 : return 554;
      case 35 : return 523;
      case 36 : return 523;
      case 37 : return 523;
      case 38 : return 494;
    }
  }
  else if(currNote < 52) {
    switch(currNote) {
      case 39 : return 466;
      case 40 : return 440;
      case 41 : return 440;
      case 42 : return 392;
      case 43 : return 349;
      case 44 : return 330;
      case 45 : return 349;
      case 46 : return 330;
      case 47 : return 294;
      case 48 : return 330;
      case 49 : return 349;
      case 50 : return 330;
      case 51 : return 294;
    }
  }
  return -1;
}

short GetNoteDurationsSong2() { //// return duration of note for song 2
  if(currNote == 0 || currNote == 4 || currNote == 8 || currNote == 12 || currNote == 16 ||
    currNote == 21 || currNote == 32 || currNote == 35 || currNote == 40 || currNote == 51
  ) return 2;
  return 4;
}

void Song2() { //// handle song 2 notes
  int noteDuration = musicNorm / GetNoteDurationsSong2();
    if((millis() - startNote) > noteDuration){
      int pauseBetweenNotes = noteDuration * pauseFactor;
      if((millis() - startNote) > (noteDuration + pauseBetweenNotes)){
        currNote += 1;
        startNote = millis();
        noteDuration = musicNorm / GetNoteDurationsSong2();
        if(soundState && inGameMusic) tone(buzzerPin, GetNoteSong2(), noteDuration);
        if(currNote == maxNoteSong2){
          int x = millis() - soundStart;
          Serial.println(x);
          currNote = -1;
          noTone(buzzerPin);
        }
      }
    }
}


//// ---------------------------------------------- Routines ------------------------------------------------------



bool CheckPos(byte row, byte col, byte isBullet, bool flag = true) { //// return if pos is occupied, handle bullet collision
  if(playerLastPosX + currRow * (!flag) == row && playerLastPosY + currCol * (!flag) == col) {
    if(isBullet == 1) { 
      lives--; 
      updateLcd = true;
      soundDuration = musicNorm / noteDuration;
      soundStart = millis();
      if(soundState) tone(buzzerPin, loseLifeSound, soundDuration);
      isSinging = true;
    }
    Serial.print("Hit player ");
    Serial.println(lives);
    return 1;
  }

  for(byte iterator = 0; iterator < maxNumOfEnemies; iterator++) {
    if(enemyList[iterator].isAlive) {
      if(enemyList[iterator].posX == row + currRow * (flag) && enemyList[iterator].posY == col + currCol * (flag)) {
        if(isBullet) {
          SetBitValue(enemyList[iterator].posX, enemyList[iterator].posY, 0, false);
          enemyList[iterator].isAlive = false;
          numOfEnemies--;
          soundDuration = musicNorm / noteDuration;
          soundStart = millis();
          if(soundState) tone(buzzerPin, killEnemySound, soundDuration);
          isSinging = true;
          score += levelValue;
          updateLcd = true;
          matrixChanged = HIGH;
        }
        return 1;
      }
    }
    if(enemyList[iterator].bullet.shot && enemyList[iterator].bullet.posX == row + currRow * (flag) && enemyList[iterator].bullet.posY == col + currCol * (flag)) {
      if(isBullet) {
        SetBitValue(enemyList[iterator].bullet.posX, enemyList[iterator].bullet.posY, 0, false);
        enemyList[iterator].bullet.shot = 0;
        matrixChanged = HIGH;
      }
      return 1;
    }
  }
  
  if(GetBitValue(row, col, flag) == 1) return 1;
  return 0;
}

void ClearMatrix() { //// clear matrix routine - put everything on 0
  for(short row = 0; row < virtualMatrixSize; row++) matrix[row] = 0;
  matrixChanged = HIGH;
}

void HighscoresToEEPROM() { //// Write values in EEPROM on demand
  for(byte i = 0; i < 3; i++) {
    EEPROM.update(3 + i * 2, highscores[i] >> 8);
    EEPROM.update(3 + i * 2 + 1, highscores[i] & 0xFF);
  }
  for(byte i = 0; i < 3; i++) {
    for(byte j = 0; j < 3; j++)
    EEPROM.update(9 + i * 3 + j, names[i][j]);
  }
}

