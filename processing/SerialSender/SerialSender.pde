import processing.serial.*;

Serial sPort;
int cols = 8;
int rows = 4;
int numPixels = cols * rows;
int squareSize = 32; // 32 pixels
int[] pixelGrid = new int[numPixels];

boolean buttonColorToggle = false;
void setup() {
  size(600,600);
  noStroke();
  
  // show serial devices
  printArray(Serial.list());
  
  //connect to one
  String portName = Serial.list()[3];
  print("\n\nConnecting to serial on: ");
  println(portName);
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

void mouseReleased() {
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
  // check if button was clicked then send serial
  if( mouseX > 32 && mouseX < 32+96 && mouseY > (32*5 - 16) && mouseY < (32*5 +16)) {
    buttonColorToggle = !buttonColorToggle;
    
    for(int i = 0; i < numPixels; i ++) {
      if(pixelGrid[i] == 0) {
        sPort.write("1");
      } else {
        sPort.write("2");
      }
    }
  }

  
  
}
