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
<h4>TODO: Electronic Speed Controller(s)</h4>
<ol>
  <li>TODO: processor? (STM32, RP2040, Atmega328p, ?)</li>
  <li>Integrated switching MOSFETs</li>
  <li>One data line from central computer per motor (8 bits - 1 sign bit direction; 7 bits (0-127) speed)</li>
  <li>Three output lines per motor (3-phase BLDC)</li>
  <li>3S capability (minimum); max amperage TBD</li>
</ol>

<h3>Main framework</h3>
<h4>TODO</h4>
