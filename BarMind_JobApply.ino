#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi connection Data:
const char* ssid = "ssid";
const char* password =  "password";

// MQTT connection Data:
const char* mqttServer = "farmer.cloudmqtt.com";
const int mqttPort = 11538;
const char* mqttUser = "mqttuser";
const char* mqttPassword = "mqttpassword";

// Machine info:
String machineId = "sd-112485-6665";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
// pump initiation:
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(4, OUTPUT);
// buzzer initiation:
  pinMode(14, OUTPUT);

// Connecting to WiFi:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(600);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Wifi connected");

// MQTT initiation:
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) 
  {
    Serial.println("Connecting with MQTT Broker...");
    
    if (client.connect(machineId.c_str(), mqttUser, mqttPassword )) 
    {
      Serial.println("MQTT Connected!");  
    } 
    else 
    {
      Serial.print("failed with state: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  //Subscribing to topic:
  client.subscribe("barkeeper");
}

boolean reconnect() {
// Method used to reconnect with MQTT broker
  if (client.connect(machineId.c_str(), mqttUser, mqttPassword )) {
    client.subscribe("barkeeper");
  }
  return client.connected();
}

void checkConnection() {
// Method used to make sure we are connected with MQTT broker
  if (!client.connected()) 
  {
    reconnect();
  }
}

void soundSignal() {
  analogWrite(14, 19);
  delay(685);
  analogWrite(14, 0);
}

int arrayToInt(int* converted)
// Method for converting array to INT value for algorithm
{
    String temporaryString = "";
    for(int i = 0; i<4; i++) {
        if(converted[i] != 0)
            temporaryString += String(converted[i]);
    }
    return temporaryString.toInt();
}

void pourDrink(int pumpNumber, int mililiters)
// Method that gets information about what pump needs to be turned on and how much fluid we need to pour.
{
  // ~90ml/min = 1.49ml/s - from pump specyfication
  int waitFor = (mililiters / 1.49) * 1000;
  
  switch(pumpNumber)
  {
    case 1:
    {
      digitalWrite(16, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      break;
    } case 2:
    {
      digitalWrite(5, HIGH);
      delay(waitFor);
      digitalWrite(5, LOW);
      break;
    } case 3:
    {
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(0, LOW);
      break;
    } case 4:
    {
      digitalWrite(4, HIGH);
      delay(waitFor);
      digitalWrite(4, LOW);
      break;
    } case 12:
    {
      digitalWrite(16, HIGH);
      digitalWrite(5, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(5, LOW);
      break;
    }  case 13:
    {
      digitalWrite(16, HIGH);
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(0, LOW);
      break;
    } case 14:
    {
      digitalWrite(16, HIGH);
      digitalWrite(4, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(4, LOW);
      break;
    } case 23:
    {
      digitalWrite(5, HIGH);
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(5, LOW);
      digitalWrite(0, LOW);
      break;
    } case 24:
    {
      digitalWrite(5, HIGH);
      digitalWrite(4, HIGH);
      delay(waitFor);
      digitalWrite(5, LOW);
      digitalWrite(4, LOW);
      break;
    } case 34:
    {
      digitalWrite(0, HIGH);
      digitalWrite(4, HIGH);
      delay(waitFor);
      digitalWrite(0, LOW);
      digitalWrite(4, LOW);
      break;
    } case 123:
    {
      digitalWrite(16, HIGH);
      digitalWrite(5, HIGH);
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(5, LOW);
      digitalWrite(0, LOW);
      break;
    } case 124:
    {
      digitalWrite(16, HIGH);
      digitalWrite(5, HIGH);
      digitalWrite(4, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(5, LOW);
      digitalWrite(4, LOW);
      break;
    } case 134:
    {
      digitalWrite(16, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(4, LOW);
      digitalWrite(0, LOW);
      break;
    } case 234:
    {
      digitalWrite(5, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(5, LOW);
      digitalWrite(4, LOW);
      digitalWrite(0, LOW);
      break;
    } case 1234:
    {
      digitalWrite(16, HIGH);
      digitalWrite(5, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(0, HIGH);
      delay(waitFor);
      digitalWrite(16, LOW);
      digitalWrite(5, LOW);
      digitalWrite(4, LOW);
      digitalWrite(0, LOW);
      break;
    } default:
    {
      checkConnection();
      client.publish(machineId.c_str(), "Pump problem");
      soundSignal();
      soundSignal();
      break;
    }
  }
}

void getDrink(int* pumps, int* mls)
// Algorithm that specifies how much fluid needs to be poured on which pump
{
    int temp;

    //sorting our arrays for less switch cases. [2,3,1][10,30,15] -> [1,2,3][15,10,30]
    for(int i = 0; i<4; i++){      
      for(int j = i+1; j<4; j++){
        if(pumps[j] < pumps[i]){
          temp = pumps[i];
          pumps[i] = pumps[j];
          pumps[j] = temp;
              
          temp = mls[i];
          mls[i] = mls[j];
          mls[j] = temp;
        }
      }
    }

    int minimal = 1000;
    for(int i = 0; i < 4; i++){
      if(mls[i]<minimal && mls[i] != 0) {
          minimal = mls[i];
          //whichpump = i;
      }
    }
    
    if(minimal == 1000){
      checkConnection();
      client.publish(machineId.c_str(), "DONE");
      soundSignal();
      return;
    }

    pourDrink(arrayToInt(pumps), minimal);

    // Deleting poured values from array and deleting pump if there is no more fluid to pour.
  
    for(int i = 0; i<4; i++)
    {
        if(mls[i] != 0) {
            mls[i] -= minimal;
            if(mls[i] == 0) {
              pumps[i] = 0;
            }
        }
    }
    //recursion
    getDrink(pumps, mls);
}

void cleanPumps(){
  /* Method used to "clean" machine pumps. That was a first implementation to make sure no leftovers are accumulating in the pumps.
   * We can insert pump inlets to hot water and use it to flush pumps. (83 sec pours ~125ml from each pump. 
   */
  digitalWrite(16, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(0, HIGH);
  delay(83000);
  digitalWrite(16, LOW);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(0, LOW);
  checkConnection();
  client.publish(machineId.c_str(), "DONE");
  soundSignal();
}

//Overriding MQTT specified method.
void callback(char* topic, byte* payload, unsigned int length) {
  // Method gathers messages from MQTT broker in specified topic.
  char newMsg[100] = "";
  int i = 0;
  for (i = 0; i < length; i++) {
    newMsg[i] = payload[i];
  }
  // Checking if gathered message is a command to clean the machine.
  if (strcmp(newMsg, "CLEAN") == 0) 
  {
    cleanPumps();
    return;
  }
  /* Checking if gathered message is a command to check connection with servers. 
   * This is used for backend server to make sure we can communicate with the machine 
   */
  if (strcmp(newMsg, machineId.c_str()) == 0)
  {
    checkConnection();
    client.publish(machineId.c_str(), "TRUE");
    return;
  }
  // Gathering information about "dring" - which pump and how much mililiters
  char* readPump = strtok(newMsg, ";");
  if (readPump[0] == '1'||readPump[0] == '2'||readPump[0] == '3'||readPump[0] == '4')
  {
    int appender = 0; //used for incrementation
    int pumps[4] = {0,0,0,0};
    int mls[4] = {0,0,0,0};
    while(readPump != 0)
    {
      char* separator = strchr(readPump, ':');
      if (separator != 0)
      {
        
        *separator = 0;
        int pumpNumber = atoi(readPump);
        ++separator;
        int mililiters = atoi(separator);
  
        pumps[appender] = pumpNumber;
        mls[appender] = mililiters;
        ++appender;
      }
      readPump = strtok(0, ";");
    }
    getDrink(pumps, mls);
  }
  return;
}

void loop() {
  /* In loop method, we are making sure that machine is allways connected to MQTT broker so we can read messages.
   * Connection can be sometimes lost due to arduino idling. This makes sure that we can gather every message and respond to them as soon as possible.
   */
  if (!client.connected()) 
  {
    //Serial.println("MQTT disconected");
    if(reconnect())
    {
      Serial.println("MQTT reconnected");
    } else {
      Serial.println("Reconnect unsuccessful, please check Internet connection.");
    }
  } else {
    client.loop();
  }
}
