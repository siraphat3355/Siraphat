#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

EthernetClient ethClient;
PubSubClient client(ethClient);

byte mac[] = {  0xD1, 0xED, 0xAB, 0xFE, 0xFE, 0xE8 };
IPAddress    ip(158, 108, 5, 102);
IPAddress    server(158, 108, 5, 6);
const char*  mqtt_server = "158.108.5.6:1883";
const char*  Board = "Board_Tool";

String     mData;
int A = 0, B = 0, C = 0, D = 0;
#define INA1 2
#define INB1 3

#define INA2 4
#define INB2 5

#define INA3 8
#define INB3 9

#define INA4 10
#define INB4 11

#define PWMz 13

void setup()
{
  Serial.begin(57600);
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  Init_Output();

  delay(1000);

}

void loop()
{
  CheckMQTT();

  analogWrite(PWMz, 255);
  Control_Tool1();
  Control_Tool2();
  Control_Tool3();
  Control_Tool4();


  delay(10);
}

void callback(char* topic, byte* payload, unsigned int length)
{
  mData = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    mData += (char)payload[i];
  }
  A = mqttQueryString("A_", mData).toInt();
  B = mqttQueryString("B_", mData).toInt();
  C = mqttQueryString("C_", mData).toInt();
  D = mqttQueryString("D_", mData).toInt();
}
String mqttQueryString(String qstr1, String mstr1)
{
  int mstart, mstop;
  String mresult = " ";

  qstr1 = qstr1;
  mstart = mstr1.indexOf(qstr1);
  if (mstart == -1)
  {
    mresult = "";
    return mresult;
  }
  mstop = mstr1.indexOf(":", mstart);
  if (mstop == -1)
  {
    mresult = mstr1.substring(mstart + qstr1.length(), " " );
  }
  else
  {
    mresult = mstr1.substring(mstart + qstr1.length(), mstop);

  }
  return mresult;
}
void Init_Output ()
{
  pinMode(INA1, OUTPUT);
  pinMode(INB1, OUTPUT);

  pinMode(INA2, OUTPUT);
  pinMode(INB2, OUTPUT);

  pinMode(INA3, OUTPUT);
  pinMode(INB3, OUTPUT);

  pinMode(INA4, OUTPUT);
  pinMode(INB4, OUTPUT);

  pinMode(PWMz, OUTPUT);

}
void Control_Tool1()
{
  switch (A)
  {
    case 1:
      digitalWrite(INA1, 0);
      digitalWrite(INB1, 1);
      break;
    case 2:
      digitalWrite(INA1, 1);
      digitalWrite(INB1, 0);
      break;
    default:
      digitalWrite(INA1, 0);
      digitalWrite(INB1, 0);
      break;
  }
}
void Control_Tool2()
{
  switch (B)
  {
    case 1:
      digitalWrite(INA2, 0);
      digitalWrite(INB2, 1);
      break;
    case 2:
      digitalWrite(INA2, 1);
      digitalWrite(INB2, 0);
      break;
    default:
      digitalWrite(INA2, 0);
      digitalWrite(INB2, 0);
      break;
  }
}
void Control_Tool3()
{
  switch (C)
  {
    case 1:
      digitalWrite(INA3, 0);
      digitalWrite(INB3, 1);
      break;
    case 2:
      digitalWrite(INA3, 1);
      digitalWrite(INB3, 0);
      break;
    default:
      digitalWrite(INA3, 0);
      digitalWrite(INB3, 0);
      break;
  }
}
void Control_Tool4()
{
  switch (D)
  {
    case 1:
      digitalWrite(INA4, 0);
      digitalWrite(INB4, 1);
      break;
    case 2:
      digitalWrite(INA4, 1);
      digitalWrite(INB4, 0);
      break;
    default:
      digitalWrite(INA4, 0);
      digitalWrite(INB4, 0);
      break;
  }
}
void CheckMQTT()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
void reconnect()
{
  while (!client.connected())
  {
    if (client.connect("Board_Tool"))
    {
      Serial.println("connected");
      client.subscribe("Test3");
      client.publish("Test1", "Board_Tool is connection");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      //Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
