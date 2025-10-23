import ch.bildspur.realsense.*;

RealSenseCamera camera = new RealSenseCamera(this);

void setup()
{
  size(640, 480);

  pixelDensity(2);

  camera.enableDepthStream(640, 480);
  camera.start();
}

void draw()
{
  background(22);

  // read frames
  camera.readFrames();

  // read depth buffer
  short[][] data = camera.getDepthData();

  noFill();
  strokeWeight(1.0);

  // use it to display circles
  for (int y = 0; y < height; y += 20) {
    for (int x = 160; x < width; x += 20) {
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