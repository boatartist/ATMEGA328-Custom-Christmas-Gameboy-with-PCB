#include <Adafruit_GFX.h>      // Include Adafruit Core graphics library
#include <Adafruit_ST7789.h>   // Include Adafruit Hardware-specific library for ST7789
#include <SPI.h>               // Include Arduino SPI library

// Define ST7789 display pin connection
#define TFT_CS     10   
#define TFT_RST     8   // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC       9

// Initialize the ST7789 display library with previously defined connections
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
int inputs[6] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

void setup() {
  // put your setup code here, to run once:
  //buttons
  pinMode(2, INPUT_PULLUP); //up
  pinMode(4, INPUT_PULLUP); //down
  pinMode(3, INPUT_PULLUP); //right
  pinMode(5, INPUT_PULLUP); //left
  pinMode(6, INPUT_PULLUP); //start
  pinMode(7, INPUT_PULLUP); //back

  // Initialize the ST7789 TFT display
  tft.init(240, 320);   // Init ST7789 240x240

  // if the screen is flipped, remove this command
  tft.setRotation(2);
}

void loop() {
  // put your main code here, to run repeatedly:]
  menu();
  snake();
}

void write_text(char *text, uint16_t color, bool wrap, int size, int x, int y) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextWrap(wrap);
  tft.setTextSize(size);
  tft.print(text);
}

void getInputs(int (& inputarray) [6]) {
  //start, back, up, down, left, right
  inputarray[0] = digitalRead(6);
  inputarray[1] = digitalRead(7);
  inputarray[2] = digitalRead(2);
  inputarray[3] = digitalRead(4);
  inputarray[4] = digitalRead(5);
  inputarray[5] = digitalRead(3);
}

void menu() {
  tft.fillScreen(ST77XX_WHITE);
  write_text("Merry", ST77XX_GREEN, true, 4, 10, 10);
  write_text("Christmas!", ST77XX_RED, false, 4, 10, 50);
  write_text("Press start to play snake", ST77XX_BLACK, true, 2, 10, 100);
  bool start = false;
  while (start == false) {
    getInputs(inputs);
    if (inputs[0] == LOW) {
      start = true;
    }
  }
}

void snake() {
  int size = 8; //change to 12 or equivalent for larger gameplay
  int x = size*8;
  int y = size*10 + 20;
  bool running = true;
  bool exit_now = false;
  bool changed = false;
  int x_movement = size;
  int y_movement = 0;
  unsigned long prev_millis = millis();
  int interval = 60;
  unsigned long current_millis;
  #define MAX_TAIL 100
  int tail_x[MAX_TAIL];
  int tail_y[MAX_TAIL];
  int tail_length = 1;
  tail_x[0] = x-size;
  tail_y[0] = y;
  int apple[2] = {random(240/size)*size, random(300/size) * size + 20};

  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(x, y, size, size, ST77XX_WHITE);
  tft.drawRect(apple[0], apple[1], size, size, ST77XX_RED);
  write_text("Score: ", ST77XX_WHITE, false, 2, 0, 0);
  write_text("0", ST77XX_WHITE, false, 2, 80, 0);

  while (running == true) {
    getInputs(inputs);
    //back
    if (inputs[1] == LOW) {
      running = false;
      exit_now = true; 
    }

    //up
    if (inputs[2] == LOW) {
      y_movement = -size;
      x_movement = 0;
    }

    //down
    if (inputs[3] == LOW) {
      y_movement = size;
      x_movement = 0;
    }

    //right
    if (inputs[5] == LOW) {
      y_movement = 0;
      x_movement = size;
    }

    //left
    if (inputs[4] == LOW) {
      y_movement = 0;
      x_movement = -size;
    }

    current_millis = millis();

    if (current_millis - prev_millis >= interval) {
      prev_millis = current_millis;

      //wall collision
      if (x < 0 || x >= 240 || y < 20 || y >= 320) {
        running = false;
      }

      //self-collision
      for (int i=0; i<tail_length; i++) {
        if (tail_x[i] == x && tail_y[i] == y) {
          running = false;
          break;
        }
      }

      //eats apple
      if (x == apple[0] && y == apple[1]) {
        if (tail_length < MAX_TAIL) {
          tail_length++;
        }

        apple[0] = random(240 / size) * size;
        apple[1] = random(300 / size) * size + 20;

        tft.fillRect(0, 0, 240, 20, ST77XX_BLACK);
        write_text("Score: ", ST77XX_WHITE, false, 2, 0, 0);

        char b[5];
        itoa(tail_length - 1, b, 10);
        write_text(b, ST77XX_WHITE, false, 2, 80, 0);
      }

      // erase last tail segment
      tft.drawRect(tail_x[tail_length - 1], tail_y[tail_length - 1], size, size, ST77XX_BLACK);
      for (int i = tail_length - 1; i > 0; i--) {
        tail_x[i] = tail_x[i - 1];
        tail_y[i] = tail_y[i - 1];
      }

      tail_x[0] = x;
      tail_y[0] = y;
      x = x + x_movement;
      y = y + y_movement;
      //tft.fillScreen(ST77XX_BLACK);
      tft.drawRect(x, y, size, size, ST77XX_WHITE); //draw new head
      tft.drawRect(apple[0], apple[1], size, size, ST77XX_RED); //draw apple
      }
    
  }
  if (exit_now == false) {
    tft.fillScreen(ST77XX_RED);
    write_text("Game Over", ST77XX_BLUE, false, 3, 0, 0);
    write_text("Score: ", ST77XX_WHITE, false, 2, 10, 100);
    char b[3];
    String str;
    str=String(tail_length-1);
    str.toCharArray(b,3);
    write_text(b, ST77XX_WHITE, false, 2, 80, 100);

    bool done = false;
    while (done == false) {
      getInputs(inputs);
      int s = 6;
      for (int i=0; i<6; i++){
        s -= inputs[i]; //Low(pressed) = 0, high(default) = 1
      }
      if (s > 0) {
        done = true;
      }
    }
  }
}
