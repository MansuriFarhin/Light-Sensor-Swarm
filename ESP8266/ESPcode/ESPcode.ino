#include <ESP8266WiFi.h>
#include <WiFiUdp.h>    //Library for UDP packets

char ssid[] = "farhin3495"; 
char pass[] = "Khadijah2021";

const unsigned int localPort = 2910; 
WiFiUDP udp;  
IPAddress broadcastIP(255, 255, 255, 255); 

// Sensor variables
int lightReading;         
int ledPin2 = D7;         
bool isMaster = false;    
unsigned long lastPacketTime = 0; 
const unsigned long silenceThreshold = 100; 
unsigned long masterDeterminationStart; 

// LED Bar Graph Pins 
const int barGraphPins[] = {D0, D1, D2, D3}; 
const int BarGraphLEDs = sizeof(barGraphPins) / sizeof(barGraphPins[0]); // Number of LEDs in the bar graph

void setup() {
  Serial.begin(9600); 
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(ledPin2, OUTPUT);     

  // Initialize the LED bar graph pins
  for (int i = 0; i < BarGraphLEDs; i++) {
    pinMode(barGraphPins[i], OUTPUT);  // Set each bar graph pin as an output
    digitalWrite(barGraphPins[i], LOW); // Turn off all LEDs initially
  }

  // Connect to WiFi
  Serial.println("Attempting to connect to WiFi network...");
  WiFi.begin(ssid, pass);  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  
    Serial.print(".");  
  }
  Serial.println("\nWiFi connection successful!"); 
  Serial.print("Device IP address: ");
  Serial.println(WiFi.localIP()); 

  // Start the UDP service
  udp.begin(localPort); 
  Serial.print("UDP service started on port ");
  Serial.println(localPort); 

  masterDeterminationStart = millis(); 
}

void loop() {
  // Read the light sensor value (range 0 to 1023)
  lightReading = analogRead(A0); 
  Serial.print("Current light sensor value: ");
  Serial.println(lightReading);  

  // Adjust the brightness of the external LED based on the light reading
  setLEDBrightness(lightReading);

  // Update the LED bar graph to visually represent the light sensor reading
  controlLEDBarGraph(lightReading);

  // Check for incoming UDP packets
  handleIncomingPackets();

  // If no packet is received within the silence threshold, broadcast the current reading
  if (millis() - lastPacketTime > silenceThreshold) {
    isMaster = true; // Default to master if no higher reading received
    sendLightReading(lightReading); // Send the current light reading to the network
  }

  // Check if the device is master or slave
  if (millis() - lastPacketTime < silenceThreshold) {
    if (isMaster) {
      digitalWrite(LED_BUILTIN, LOW); // Turn on the onboard LED for Master
      Serial.println("This device is the Master."); 
    } else {
      digitalWrite(LED_BUILTIN, HIGH); // Turn off the onboard LED for non-Master
      Serial.println("This device is a Slave."); 
    }
  }

  delay(100); // Prevent overwhelming the network with too many packets
}

void sendLightReading(int reading) {
  byte packet[4];  
  packet[0] = 0x01; 
  packet[1] = (reading >> 8) & 0xFF; 
  packet[2] = reading & 0xFF;        
  packet[3] = 0xFF;                 

  // Begin UDP transmission to the broadcast IP on the specified port
  udp.beginPacket(broadcastIP, localPort);
  udp.write(packet, sizeof(packet));  
  udp.endPacket();  

  lastPacketTime = millis(); 
  Serial.println("Broadcasting light reading to network."); 
}

void handleIncomingPackets() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    byte buffer[4]; 
    udp.read(buffer, packetSize); 

    // Check if the packet type is a light reading (0x01)
    if (buffer[0] == 0x01) {
      int receivedReading = (buffer[1] << 8) | buffer[2]; 
      Serial.print("Received a light reading of: ");
      Serial.println(receivedReading);  

      // If the received reading is greater than the current reading, this device is no longer the master
      if (receivedReading > lightReading) {
        isMaster = false;  
      }
    }

    lastPacketTime = millis(); // Update the timestamp for the last packet received
  }
}

void setLEDBrightness(int reading) {
  // Map the light reading (0-1023) to a PWM brightness level (0-255)
  int brightness = map(reading, 0, 1023, 0, 255);
  analogWrite(ledPin2, brightness);  // Set the brightness of the external LED
  Serial.print("Setting external LED brightness to: ");
  Serial.println(brightness);  // Print the new brightness level
}

void controlLEDBarGraph(int reading) {
  // Map the light reading (0-1023) to the number of LEDs to light up (0 to BarGraphLEDs)
  int numLitLEDs = map(reading, 0, 50, 0, BarGraphLEDs);

  // Print how many LEDs will be activated based on the light reading
  Serial.print("Activating ");
  Serial.print(numLitLEDs);
  Serial.println(" LEDs on the bar graph.");

  // Loop through each LED in the bar graph and turn on or off based on the number to be lit
  for (int i = 0; i < BarGraphLEDs; i++) {
    if (i < numLitLEDs) {
      digitalWrite(barGraphPins[i], HIGH);  // Turn on LED at the current pin
      Serial.print("LED at pin ");
      Serial.print(barGraphPins[i]);
      Serial.println(" is now ON.");
    } else {
      digitalWrite(barGraphPins[i], LOW);   // Turn off LED at the current pin
      Serial.print("LED at pin ");
      Serial.print(barGraphPins[i]);
      Serial.println(" is now OFF.");
    }
  }
}
