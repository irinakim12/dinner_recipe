#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

#define YOUR_API_KEY  "Enter API KEY";
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Arduino's MAC address
IPAddress serverIP; // ChatGPT server's IP address
int serverPort = 80; // ChatGPT server's port number

EthernetClient client;

void setup() {
  Ethernet.init(17);  // WIZnet W5100S-EVB-Pico W5500-EVB-Pico W6100-EVB-Pico
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
   Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
    // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Arduino Weather Recipe");

  // Connect to ChatGPT server
  if (client.connect("chat.openai.com", serverPort)) {
    Serial.println("Connected to ChatGPT server");
  } else {
    Serial.println("Connection failed");
  }
}

void loop() {
  // Get weather information from WeatherAPI
  String weatherData = getWeatherData();

  // Send weather information to ChatGPT
    // Connect to ChatGPT server
  if (client.connect("chat.openai.com", serverPort)) {
    Serial.println("Connected to ChatGPT server");
  } else {
    Serial.println("Connection failed");
  }
  sendToChatGPT(weatherData);

  // Receive and display recipe from ChatGPT
  String recipe = receiveFromChatGPT();
  Serial.println("Recipe: " + recipe);

  delay(5000); // Wait for 5 seconds before requesting new weather data
}

void sendToChatGPT(String data) {
  // Construct the HTTP POST request to send data to ChatGPT
  String request = "POST /api/recommendation HTTP/1.1\r\n";
  request += "Host: chat.openai.com\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(data.length()) + "\r\n";
  request += "\r\n";
  request += data;

  // Send the request to ChatGPT
  client.print(request);
  delay(1000); // Wait for ChatGPT's response
}

String receiveFromChatGPT() {
  String response;

  // Read the response from ChatGPT
  while (client.available()) {
    char c = client.read();
    response += c;
  }

  // Extract the recipe from the response (assuming JSON format)
  StaticJsonDocument<512> jsonDoc;
  deserializeJson(jsonDoc, response);
  String recipe = jsonDoc["recipe"];

  return recipe;
}

String getWeatherData() {
  // Make an HTTP GET request to WeatherAPI
  String url = "/v1/current.json?key=YOUR_API_KEY&q=/*Enter your address*/";
  // Connect toWeather server
  if (client.connect("api.weatherapi.com", serverPort)) {
    Serial.println("Connected to Weather server");
  } else {
    Serial.println("Connection failed( Weather)");
  }
  
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: api.weatherapi.com");
  client.println("Connection: close");
  client.println();

  // Wait for the response
  while (!client.available()) {
    delay(1);
  }

  // Read the response from WeatherAPI
  String response;
  while (client.available()) {
    char c = client.read();
    response += c;
  }

  // Extract the required weather information from the response
  DynamicJsonDocument jsonDoc(1024);
  deserializeJson(jsonDoc, response);
  float temperature = jsonDoc["current"]["temp_c"];
  int humidity = jsonDoc["current"]["humidity"];

  // Format the weather data
  String weatherData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);

  return weatherData;
}
