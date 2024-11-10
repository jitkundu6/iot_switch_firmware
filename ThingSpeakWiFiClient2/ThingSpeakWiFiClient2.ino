/*
    Go to thingspeak.com and create an account if you don't have one already.
    After logging in, click on the "New Channel" button to create a new channel for your data. This is where your data will be stored and displayed.
    Fill in the Name, Description, and other fields for your channel as desired, then click the "Save Channel" button.
    Take note of the "Write API Key" located in the "API keys" tab, this is the key you will use to send data to your channel.
    Replace the channelID from tab "Channel Settings" and privateKey with "Read API Keys" from "API Keys" tab.
    Replace the host variable with the thingspeak server hostname "api.thingspeak.com"
    Upload the sketch to your ESP32 board and make sure that the board is connected to the internet. The ESP32 should now send data to your Thingspeak channel at the intervals specified by the loop function.
    Go to the channel view page on thingspeak and check the "Field1" for the new incoming data.
    You can use the data visualization and analysis tools provided by Thingspeak to display and process your data in various ways.
    Please note, that Thingspeak accepts only integer values.

    You can later check the values at https://thingspeak.com/channels/2005329
    Please note that this public channel can be accessed by anyone and it is possible that more people will write their values.
 */

/*  Write a Channel Feed
      GET https://api.thingspeak.com/update?api_key=25NW11L7I3VXTI7O&field1=0
    Read a Channel Feed
      GET https://api.thingspeak.com/channels/2725686/feeds.json?api_key=882ZHLEJ3HS50QNB&results=2
    Read a Channel Field
      GET https://api.thingspeak.com/channels/2725686/fields/1.json?api_key=882ZHLEJ3HS50QNB&results=2
    Read Channel Status Updates
      GET https://api.thingspeak.com/channels/2725686/status.json?api_key=882ZHLEJ3HS50QNB
      
    Read Last Channel Feeds
      GET https://api.thingspeak.com/channels/2725686/feeds/last.json?api_key=882ZHLEJ3HS50QNB
      {"created_at":"2024-11-03T17:03:41Z","entry_id":3,"field1":"11","field2":"10110010","field3":null,"field4":null,"field5":null,"field6":null,"field7":null,"field8":null}
  
    Read Last Value of a Channel Field
      GET https://api.thingspeak.com/channels/2725686/field/1/last.txt?api_key=882ZHLEJ3HS50QNB
      x10110010

 *  Read
 *   GET https://api.thingspeak.com/channels/2725686/feeds.json?api_key=882ZHLEJ3HS50QNB&results=1
 *  RESPONSE:
{
  "channel": {
    "id": 2725686,
    "name": "Optra",
    "description": "Optra Lab IoT switch",
    "latitude": "0.0",
    "longitude": "0.0",
    "field1": "f1",
    "field2": "f2",
    "field3": "f3",
    "field4": "f4",
    "field5": "f5",
    "field6": "f6",
    "field7": "f7",
    "field8": "f8",
    "created_at": "2024-11-03T16:44:03Z",
    "updated_at": "2024-11-03T16:44:03Z",
    "last_entry_id": 2
  },
  "feeds": [
    {
      "created_at": "2024-11-03T16:55:25Z",
      "entry_id": 2,
      "field1": "11",
      "field2": "22",
      "field3": null,
      "field4": null,
      "field5": null,
      "field6": null,
      "field7": null,
      "field8": null
    }
  ]
}
*/

#include <WiFi.h>

const char *ssid = "Smart Switch";          // Change this to your WiFi SSID
const char *password = "subhajit";          // Change this to your WiFi password

const char *host = "api.thingspeak.com";        // This should not be changed
const int httpPort = 80;                        // This should not be changed
const String channelID = "2725686";             // Change this to your channel ID
const String writeApiKey = "25NW11L7I3VXTI7O";  // Change this to your Write API key
const String readApiKey = "882ZHLEJ3HS50QNB";   // Change this to your Read API key

const int remoteFieldNumber = 1;    // Field number which will be read out
const int switchFieldNumber = 2;    // Field number where to write

const int serialBaudRate = 115200;

const int sw[8] = {15,2,4,5,18,19,21,22};

String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";
String remoteValue = "x00000000x";
String switchValue = "x00000000x";
String lastSwitchValue = "";

void connect_wifi(const char* ssid, const char* password) {
  // Connecting to a WiFi network
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool connect_client(NetworkClient *client) {
  if (!client->connect(host, httpPort)) {
    Serial.println("Network Client not available....");
    delay(1000);
    return false;
  }
  return true;
}

String readResponse(NetworkClient *client) {
  String response = "";
  unsigned long timeout = millis();
  while (client->available() == 0) {
    if (millis() - timeout > 15000) {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return response;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client->available()) {
    String line = client->readStringUntil('\r');
    // Serial.print(line);
    response += line;
  }
  Serial.println("Received Response: " + response);
  Serial.println("Response Length: " + String(response.length()));
  Serial.printf("\nClosing connection\n\n");
  return response;
}

void setup() {
  Serial.begin(serialBaudRate);

  // We start by connecting to a WiFi network
  connect_wifi(ssid, password);

  // initialize the I/O pins as outputs iterate over the pins:
  for (int thisPin = 0; thisPin < 8; thisPin++) {
    // initialize the output pins:
    pinMode(sw[thisPin], OUTPUT);
    // take the pin LOW to ensure that the LEDS are off:
    // digitalWrite(sw[thisPin], LOW);
  }
}

String get_field_value(String response, int fieldNumber) {
  int startIndex = response.indexOf("field" + String(fieldNumber)) + 9;
  response = response.substring(startIndex, response.length());
  int endIndex = response.indexOf("\"");
  String string_value = response.substring(0, endIndex);  // "x12345678x"
  Serial.println("Field " + String(fieldNumber) + " Value: <<" + string_value + ">>\n");
  return string_value;
}

void loop() {
  NetworkClient client;
  
  // WRITE --------------------------------------------------------------------------------------------
  if(lastSwitchValue != "") {
    Serial.println("Writing ------");
    if (!connect_client(&client)) {
      return;
    }
    // GET https://api.thingspeak.com/update.json?api_key=25NW11L7I3VXTI7O&field1=x01110011x
    //{"channel_id":2725686,"created_at":"2024-11-10T12:49:42Z","entry_id":183,"field1":"x01110011x","field2":null,"field3":null,"field4":null,"field5":null,"field6":null,"field7":null,"field8":null,"latitude":null,"longitude":null,"elevation":null,"status":null}
    String writeRequest = "GET /update.json?api_key=" + writeApiKey + "&field" + String(switchFieldNumber) + "=" + String(lastSwitchValue) + footer;
    Serial.println("Writing Request: " + writeRequest);
    client.print(writeRequest);
    String write_response = readResponse(&client);
    String write_message = get_field_value(write_response, switchFieldNumber);
    if(write_message == lastSwitchValue) {
      switchValue = lastSwitchValue;
      lastSwitchValue = "";
      delay(4000);
    }
    delay(2000); 
    return;
  }


  // READ --------------------------------------------------------------------------------------------
  Serial.println("Reading ------");
  if (!connect_client(&client)) {
    return;
  }
  // GET https://api.thingspeak.com/channels/2725686/field/1/last.json?api_key=882ZHLEJ3HS50QNB
  // {"created_at":"2024-11-04T14:32:08Z","entry_id":24,"field1":"x87654321x"}
  String readRequest = "GET /channels/" + channelID + "/field/" + String(remoteFieldNumber) + "/last.json?api_key=" + readApiKey + footer;
  Serial.println("Reading Request: " + readRequest);
  client.print(readRequest);
  String read_response = readResponse(&client);
  delay(100);
  

  // Control Logic -------------------------------------------------------------------------------------------------
  Serial.println("Controling ------");
  String message = get_field_value(read_response, remoteFieldNumber);
  String error = "";
  
  if(message.length() != 10)
    error = "Remote message length is not 10";
  else if(message[0] != 'x')
    error = "Starting character is not 'x'";
  else if(message[9] != 'x')
    error = "Ending character is not 'x'";
  else
    for (int i = 1; i <= 8; i++) {
      if(message[i] != '1' && message[i] != '0') {
        error = "Switch No. " + String(i) + " value is " + String(message[i]);
        break;
      }
    }

  if(error != "") {
    Serial.println("Remote Value is Invalid: " + error);
    delay(3000);
    return;
  }
 
  remoteValue = message;
  if(remoteValue == switchValue) {
    Serial.println("Value up to date, No changes required.");
    delay(5000);
    return;
  }
  
  lastSwitchValue = remoteValue;
  // led/switch control here
  for (int i = 0; i < 8; i++) {
    digitalWrite(sw[i], lastSwitchValue[i+1] - 48);
  }
  delay(2000);
}
