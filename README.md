<h1>Custom Flight Controller</h1>

<h3>Main electronics components</h3>
<h4>Central computer</h4>
<ol>
  <li>ESP32-S3-WROOM-1U - Central SoC Processor</li>
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
