import processing.serial.*;

Serial sPort;
int numPixels = 32;
int squareSize = 32; // 32 pixels
int[] pixelGrid = new int[numPixels];
int cols = 8;
int rows = 4;
boolean buttonColorToggle = false;
void setup() {
  size(600,600);
  noStroke();
  
  printArray(Serial.list());
  
  String portName = Serial.list()[4];
  sPort = new Serial(this, portName, 9600);
  
}

void draw() {
  background(0);
  fill(127);
  for(int i = 0; i < numPixels; i ++) {
    if ( pixelGrid[i] == 0 ) {
      fill(127);
    } else if ( pixelGrid[i] == 1 ) {
      fill(255);
    }
    int x = (i % cols) * 32;
    int y = (i /cols) * 32;
    rect(x,y, 24, 24);
  }
  if(buttonColorToggle) {
    fill(170);
  } else {
    fill(200);
  }
  rect(32, 32*5 - 16, 96, 32);
  fill(64);
  text("send pixels",32+ 8, 32 * 5 );
}

void mouseClicked() {
  // find rect the mouse clicked on
  int col = mouseX / squareSize;
  int row = mouseY / squareSize;
  
  if( col < cols && row < rows) {
    
    int i = row * 8 + col;
    if (pixelGrid[i] == 0) {
      pixelGrid[i] = 1;
    } else {
      pixelGrid[i] = 0;
    }
  }
  if( mouseX > 32 && mouseX < 32+96 && mouseY > (32*5 - 16) && mouseY < (32*5 +16)) {
    buttonColorToggle = !buttonColorToggle;
  }

  
  
}
