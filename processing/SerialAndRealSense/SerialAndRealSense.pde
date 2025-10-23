import ch.bildspur.realsense.*;
import processing.serial.*;

RealSenseCamera camera = new RealSenseCamera(this);
int camHeight = 640;
int camWidth = 480;


void doCameraUpdates()
{
  background(22);
  // read frames
  camera.readFrames();
  // read depth buffer
  short[][] data = camera.getDepthData();
  noFill();
  strokeWeight(1.0);
  // use it to display circles
  for (int y = 0; y < camHeight; y += 20) {
    for (int x = 160; x < camWidth; x += 20) {
      //if(y < height && width < width) {
      // get intensity
      int intensity = data[y][x];
      // map intensity (values between 0-65536)
      float d = map(intensity, 0, 3000, 20, 0);
      float c = map(intensity, 0, 3000, 255, 0);
      d = constrain(d, 0, 16);
      c = constrain(c, 0, 255);
      if (d > 9) {
      stroke(c, 255, 255);
      circle(x, y, d);
      }
     // }
    }
  }
}



Serial sPort;
int rows = 8;
int cols = 24;
int numPixels = cols * rows;
int squareSize = 32; // 32 pixels
int[] pixelGrid = new int[numPixels];
int[][] pixelGrid2D = new int[rows][cols]; // y,x

boolean buttonColorToggle = false;

int appState = 0; // 0 draw pixels mode, 1 test pattern loop
String testLoopText = "test loop OFF";
int testLoopIndex = 0;

// This can turn on or off debugging info from microcontroller over serial
boolean printSerialFromMicro = true;
void setup() {
  pixelDensity(2);
  size(900,600);

  camera.enableDepthStream(camWidth, camHeight);
  camera.start();
  
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
  rect(32 + (32+64+32)*2, 32*(rows+1) - 16, 96, 32); // test pattern button
  fill(64);
  text("send pixels",32+ 8, 32 * (rows+1) );
  text(testLoopText, (32+64+32) + 32+ 8, 32 * (rows+1) );
  text("run camera", (32+64+32)*2 + 32+ 8, 32 * (rows+1) );
  if (appState == 0) { // draw mode
    // serial is sent when button press is detected
  } else if(appState == 1) { // test pattern

    // flip a pixel
    if(pixelGrid[testLoopIndex] == 0) {
      pixelGrid[testLoopIndex] = 1;
    } else {
      pixelGrid[testLoopIndex] = 0;
    }
    sendPixelsOverSerial();
    // send all pixels over serial
    // for(int i = 0; i < numPixels; i ++) {
    //   if(i%16 == 0) {
    //     sPort.write(char(floor(i/16)));
    //   }
    //   if(pixelGrid[i] == 0) {
    //     sPort.write("1");
    //   } else {
    //     sPort.write("2");
    //   }
    // }
    // sPort.write(char(255));
    // for(int i = 0; i < 16; i ++ ) {
    //   sPort.write(char(0));
    // }


    if(testLoopIndex >= numPixels-1) {
      testLoopIndex = 0;
    } else {
      testLoopIndex++;
    }

  } else if (appState == 2) {
    // run camera
    doCameraUpdates();
    sendPixelsOverSerial();

  }
}

void sendPixelsOverSerial() {
    
  // send one section of 8x8 at a time
  int numOfSections = numPixels / 64;
  int colsPerBoard = 8;
  int sectionOffset = 0; // the start of pixel 0 for each section
  for(int sectionIndex = 0; sectionIndex < numOfSections; sectionIndex++) {
    int boardOffset = 0;
    // send serial header : each section has 4 boards
    for (int boardNum = 0; boardNum < 4; boardNum++) {

      // 4 boards per section
      sPort.write(char(sectionIndex*4 + boardNum));
      // for 2 rows of pixels per board
      for (int currentRow = 0; currentRow < 2; currentRow ++) {
        // read 8 pixels (colsPerBoard)
        for(int i = 0; i < colsPerBoard; i ++) {
          int currentPixel = sectionOffset + boardOffset + currentRow * cols + i; // (cols = 24);
          if(pixelGrid[currentPixel] == 0) {
            sPort.write("1");
          } else {
            sPort.write("2");
          }
        }
      }
      boardOffset += cols*2; // to get next board in the section
    }
    sectionOffset += colsPerBoard;
    if(sectionIndex == 2 || sectionIndex == 5) {
      sectionOffset += 64*3 - colsPerBoard*3;
    }
  }

  // old method 
  // for(int i = 0; i < numPixels; i ++) {
  //   if(i%16 == 0) {
  //     sPort.write(char(floor(i/16)));
  //   }
  //   if(pixelGrid[i] == 0) {
  //     sPort.write("1");
  //   } else {
  //     sPort.write("2");
  //   }
  // }
  sPort.write(char(255));
  for(int i = 0; i < 16; i ++ ) {
    sPort.write(char(0));
  }

}

void mouseReleased() {
  // find rect the mouse clicked on
  int col = mouseX / squareSize;
  int row = mouseY / squareSize;
  
  if( col < cols && row < rows) {
    
    int i = row * cols + col;
    if (pixelGrid[i] == 0) {
      pixelGrid[i] = 1;
    } else {
      pixelGrid[i] = 0;
    }
  }
  // check if send serial button was clicked then send serial
  if( mouseX > 32 && mouseX < 32+96 && mouseY > (32*(rows+1) - 16) && mouseY < (32*(rows+1) +16)) {
    buttonColorToggle = !buttonColorToggle;
    sendPixelsOverSerial();
  }
  // decode the first 8 pixels
  // jump position to next 8 pixels 
  
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