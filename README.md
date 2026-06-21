<h1>Custom Flight Controller</h1>

<h3>Why did I make this?</h3>
<h4>Flight has always been something that has fascinated me and so far my only experience has been with assembling premade drone kits or glider kits. I want to be able to go out and fly something that I made essentially from scratch. I have this vision of myself standing on the roof, tossing the plane off, and watching it fly up into the sky, and then looking at the camera feed to see the world from up high.</h4>

<h3>Main components/features</h3>
<ol>
  <li>ESP32-S3-WROOM-1U - Central processor</li>
  <li>BMS management circuitry; computer is powered by 3S LiPo battery</li>
  <li>IMU (6-axis; accelerometer/gyroscope)</li>
  <li>Pressure sensor (for altitude)</li>
  <li>OV5640 Camera (external)</li>
  <li>Miscellaneous: external 2.4ghz antenna capability; dual USB-C for charging/data; external microSD support</li>
</ol>

<h3>Firmware brief description</h3>
<h4>Modular handling of all sensors with interrupts configured to save battery power</h4>
<h4>Verbose error/initialization logging to detect I2C issues</h4>
<h5>Configurable periodic logging of data to microSD card</h5>

<h3>Images</h3>

[Schematic](media/PCB Schematic.png)
