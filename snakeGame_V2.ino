 /**
 * Arduino Snake game
 * 
 * The game is created for Arduino Mega 2560
 * 
 * Control the snakes using two buttons, eat food and try
 * not to hit your body = end of game
 * 
 * There is also a speaker and a SD card connected to the 
 * Arduino, to provide sound effects
 * 
 * Created in May 2021
 * by Michal Benes
 * 
 **/


// Include the 4 digit display library to display the score
#include <TM1637Display.h>


// The directions of the Snake (0, 1, 2, 3):
enum {UP, RIGHT, DOWN, LEFT};


// Change the speed of the game (lower number = faster)
#define SPEED 40


// Right and left buttons used to control the snake
#define BUTTONL 33
#define BUTTONR 32


// Defining the pins for the 8x8 LED display:
#define ROW_1 2
#define ROW_2 3
#define ROW_3 4
#define ROW_4 5
#define ROW_5 6
#define ROW_6 7
#define ROW_7 8
#define ROW_8 9

#define COL_1 10
#define COL_2 11
#define COL_3 12
#define COL_4 13
#define COL_5 A0
#define COL_6 A1
#define COL_7 A2
#define COL_8 A3


// Defining the pins for the 4 digit display:
#define CLK 34
#define DIO 35




// -----------------------------------------------------------------------------------------------



// Create display object of type TM1637Display:
TM1637Display display = TM1637Display(CLK, DIO);


// Pins where the 8x8 LED matrix display is connected:
const byte rows[] = {
    ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8
};
const byte col[] = {
  COL_1,COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8
};



// -----------------------------------------------------------------------------------------------



void setup() {

  // Open Serial port:
  Serial.begin(9600);


  // Setup the buttons:
  pinMode(BUTTONL, INPUT_PULLUP);
  pinMode(BUTTONR, INPUT_PULLUP);


  // Setting all the 8x8 display pins to output
  for (byte i = 2; i <= 13; i++) pinMode(i, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);


  // Clear the 4 digit display
  display.clear();


  // Set the brightness of the 4 digit display
  display.setBrightness(7);

}



// -----------------------------------------------------------------------------------------------



/**
 * Global variables:
 **/


// Useful reset variables for 8x8 display:
// 1 = led is turned off, 0 = led shines
byte NONE[] = {B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111};
byte ALL[] = {B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000};
byte CPY[] = {B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111};


// Pause/Start
bool on = 0;


// The direction of the Snake is store there:
unsigned int dir = 0;


// Helps to make sure that the button is pressed just once
bool LAlreadyPressed = false;
bool RAlreadyPressed = false;


// Helps to control the game's speed (change the game speed in the #define section ^)
int timeCount = 0;


// Array to keep all the snake's x and y coordinations
int posx[64];
int posy[64];


// Starting food position:
int foodx = 5;
int foody = 5;


// Score at the beginning
unsigned int score = 1;


// Iniciating the button states:
bool LButtonState = LOW;
bool RButtonState = LOW;



// -----------------------------------------------------------------------------------------------



void loop() {

  // Read the buttons:
  readButtons();
  

  // If anything is pressed at the really beginning, the game starts
  turnOn();
 

  // TODO: if both buttons are pressed at one time = pause the game


  // Changing the direction of the Snake if the buttons are pressed (seperately)
  changeDirection();
 
  // Move the snake 'internally', if the time is equal to SPEED
  moveSnake();
  
  
  if (on == 0) timeCount = 0;

  // Move the snake 'externally' (change will be seenable only once in SPEED times)
  drawScreen(CPY);


  delay(5);
  timeCount++; 

}



// -----------------------------------------------------------------------------------------------



/**
 * Functions
 **/

// Function used to 'paint' on the screen
void  drawScreen(byte buffer2[])
{ 
  // Turn on each row in series
  for (byte i = 0; i < 8; i++)        // count next row
    {
      digitalWrite(rows[i], HIGH);    //initiate whole row
      for (byte a = 0; a < 8; a++)    // count next row
      {
        // if You set (~buffer2[i] >> a) then you will have positive
        digitalWrite(col[a], (buffer2[i] >> a) & 0x01); // initiate whole column
        
        delayMicroseconds(100);       // uncoment deley for diferent speed of display
        //delayMicroseconds(1000);
        //delay(10);
        //delay(100);
        
        digitalWrite(col[a], 1);      // reset whole column
      }
      digitalWrite(rows[i], LOW);     // reset whole row
      // otherwise last row will intersect with next row
  }
}


// Reading the buttons at the beggining of the loop
void readButtons() {
  LButtonState = digitalRead(BUTTONL);
  RButtonState = digitalRead(BUTTONR);
}


// Changing the Snake's direction in case of a button press
void changeDirection() {
  if (on && !LAlreadyPressed && LButtonState == LOW && RButtonState != LOW) {
    if (dir == 3) dir = 0;
    else dir++;
    Serial.println("Direction increased");
    LAlreadyPressed = true;
  }

  if (on && !RAlreadyPressed && RButtonState == LOW && LButtonState != LOW) {
    if (dir == 0) dir = 3;
    else dir--;
    Serial.println("Direction decreased");
    RAlreadyPressed = true;
  }
}


// Function starts the game, if anything is pressed
void turnOn() {
  if (!on && (LButtonState == LOW || RButtonState == LOW)) {
  	Serial.println("The game started");
    on = true;
    CPY[posy[0]] &= ~(1 << posx[0]);
    CPY[foody] &= ~(1 << foodx);

    // Show score
    display.showNumberDec(score);
  }
}


// Function that 'moves' the snake if the game is on
void moveSnake() {
  if (on && timeCount == SPEED) {

    if (foodx == 8) { // If the food was eaten
      foodx = random(0,8);
      foody = random(0,8);
    }
    CPY[foody] &= ~(1 << foodx);
    
    CPY[posy[score]] |= 1 << posx[score];

    // Moving the snake's head in specific direction:
    moveHead();

    // "Moving the body" :
    moveBody();

    // If the snake eats the food:
    snakeEats();
    
    CPY[posy[0]] &= ~(1 << posx[0]);

    // If the snake hits itself = GAMEOVER:
    isHit();

    // Reseting the timeCount
    timeCount = 0;

    // The direction can be changed again only by pressing the button again
    if (LButtonState != LOW) LAlreadyPressed = false;
    if (RButtonState != LOW) RAlreadyPressed = false;
  }
}


// Move the head in specific direction
void moveHead() {
  switch (dir) {
    case UP: {
      posy[0] += 1;
      if (posy[0] == 8) posy[0] = 0;
      break;
    }

    case RIGHT: {
      posx[0]-= 1;
      if (posx[0] == -1) posx[0] = 7;
      break;
    }

    case DOWN: {
      posy[0] -= 1;
      if (posy[0] == -1) posy[0] = 7;
      break;
    }

    case LEFT: {
      posx[0] += 1;
      if (posx[0] == 8) posx[0]= 0;
      break;
    }

    default: break;
  }
}


// Move the body of the snake
void moveBody() {
  for (int a = 0; a < score; a++) {
    posx[score - a] = posx[score - 1 - a];
    posy[score - a] = posy[score - 1 - a];
  }
}


// If the snake eats food:
void snakeEats() {
  if (posx[0] == foodx && posy[0] == foody) {
      foodx = 8;
      score += 1;
      printScore();
    }
}


// Checks if the head hit the body
void isHit() {
  for (int a = 2; a <= score; a++) {
    if (posx[0] == posx[a] && posy[0] == posy[a]) {
      on = 0;
      score = 1;
      dir = 0;
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++){
          CPY[i] |= 1 << j;
        }
      }
    }
  }
}


// Prints the score the the digit display + Serial output
void printScore() {
  Serial.println(score); // Prints score
  display.showNumberDec(score);
}
