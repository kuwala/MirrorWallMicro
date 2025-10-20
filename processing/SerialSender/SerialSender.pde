import processing.serial.*;

Serial sPort;
int rows = 8;
int cols = 8;
int numPixels = cols * rows;
int squareSize = 32; // 32 pixels
int[] pixelGrid = new int[numPixels];

boolean buttonColorToggle = false;

int appState = 0; // 0 draw pixels mode, 1 test pattern loop
String testLoopText = "test loop OFF";
int testLoopIndex = 0;

boolean printSerialFromMicro = false;
void setup() {
  pixelDensity(1);
  size(600,600);
  noStroke();
  
  // show serial devices
  printArray(Serial.list());
  
  //connect to one
  String portName = Serial.list()[3];
  print("\n\nConnecting to serial on: ");
  println(portName);
  sPort = new Serial(this, portName, 115200);
  
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
  rect(32, 32*(rows+1) - 16, 96, 32); // send serial button
  rect(32 + (32+64+32), 32*(rows+1) - 16, 96, 32); // test pattern button
  fill(64);
  text("send pixels",32+ 8, 32 * (rows+1) );
  text(testLoopText, (32+64+32) + 32+ 8, 32 * (rows+1) );
  if (appState == 0) { // draw mode
  } else if(appState == 1) { // test pattern

    // flip a pixel
    if(pixelGrid[testLoopIndex] == 0) {
      pixelGrid[testLoopIndex] = 1;
    } else {
      pixelGrid[testLoopIndex] = 0;
    }
    // send all pixels over serial
    for(int i = 0; i < numPixels; i ++) {
      if(i%16 == 0) {
        sPort.write(char(floor(i/16)));
      }
      if(pixelGrid[i] == 0) {
        sPort.write("1");
      } else {
        sPort.write("2");
      }
    }
    sPort.write(char(255));
    for(int i = 0; i < 16; i ++ ) {
      sPort.write(char(0));
    }


    if(testLoopIndex >= numPixels-1) {
      testLoopIndex = 0;
    } else {
      testLoopIndex++;
    }

  }
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
  if( mouseX > 32 && mouseX < 32+96 && mouseY > (32*(rows+1) - 16) && mouseY < (32*(rows+1) +16)) {
    buttonColorToggle = !buttonColorToggle;
    
    for(int i = 0; i < numPixels; i ++) {
      if(i%16 == 0) {
        sPort.write(char(floor(i/16)));
      }
      if(pixelGrid[i] == 0) {
        sPort.write("1");
      } else {
        sPort.write("2");
      }
    }
    sPort.write(char(255));
    for(int i = 0; i < 16; i ++ ) {
      sPort.write(char(0));
    }
  }
  // check if test loop is pressed
  if( mouseX > 32+(32+96) && mouseX < 32+96 + (32 +96) && mouseY > (32*(rows+1) - 16) && mouseY < (32*(rows+1) +16)) {
    if(appState != 1) {
      // reset grid
      // set test loop index = 0
      // update state
      for(int i = 0; i < numPixels; i ++) {
        pixelGrid[i] = 0;
      }
      testLoopIndex = 0;
      appState = 1;
    } else {
      appState = 0;
    }
  }
  
}


void serialEvent(Serial sPort) {
  if(printSerialFromMicro) {
    int inByte = sPort.read();
    print((char)inByte);
  }
}
