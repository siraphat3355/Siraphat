#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

EthernetClient ethClient;
PubSubClient client(ethClient);

byte mac[] = {  0x1E, 0xD1, 0x11, 0x11, 0x11, 0x11 };
IPAddress    ip(158, 108, 5, 15);
IPAddress    server(158, 108, 5, 6);
const char*  mqtt_server = "158.108.5.6:1883";
const char*  Board = "Board_Master";

String     mData,mData1;
char mCharOutput[70];
String     mSent, mX;
int A = 0, B = 0, C = 0, D = 0;

void setup()
{
  Serial.begin(57600);
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  Init_TIMER1_interuppt();
  //Init_Input();
  delay(1000);
}

void loop()
{
  Step1();
  Step2();
  Step3();
  Step4();
  /*
    Control_Tool1();
    Control_Tool2();
    Control_Tool3();
    Control_Tool4();

    mSent = "A_" + String(A) + ":B_" + String(B) + ":C_" + String(C) + ":D_" + String(D);

      if (mSent != String(mX))
      {
      mSent.toCharArray(mCharOutput, 50);
      client.publish("Test3", mCharOutput);
      }

      mX = String(mSent);

      //x = readEncoder();
      //Serial.println(String(x));
      //Serial.println("V0 = "+String(analogRead(A0)));
      //Serial.println("V1 = "+String(analogRead(A1)));
      //Serial.println("V2 = "+String(analogRead(A2)));
      //Serial.println("----------------------------");
  */
  delay(200);
}
ISR(TIMER1_OVF_vect)         // interrupt service routine Timer 01
{
  CheckMQTT();
  TCNT3 = 0;
}
void Init_Input ()
{
  pinMode(38, INPUT);
  pinMode(39, INPUT);
  pinMode(40, INPUT);
  pinMode(41, INPUT);
  pinMode(42, INPUT);
  pinMode(43, INPUT);
  pinMode(44, INPUT);
  pinMode(45, INPUT);
}
void Step1()
{
  mData1 = "Setpoint1_56447:Setpoint2_68493:Setpoint3_41004:Setpoint4_56447";
  Data_Setpoint();
  delay(25000);

  mData1 = "A_1:B_1:C_0:D_0";
  Data_Tool();
  delay(2000);
  Data_StopTool();
}
void Step2()
{
  mData1 = "Setpoint1_68493:Setpoint2_56447:Setpoint3_56447:Setpoint4_41004";
  Data_Setpoint();
  delay(25000);

  mData1 = "A_0:B_2:C_2:D_0";
  Data_Tool();
  delay(2000);
  Data_StopTool();
}
void Step3()
{
  mData1 = "Setpoint1_56447:Setpoint2_41004:Setpoint3_68493:Setpoint4_56447";
  Data_Setpoint();
  delay(25000);

  mData1 = "A_0:B_0:C_1:D_1";
  Data_Tool();
  delay(2000);
  Data_StopTool();
}
void Step4()
{
  mData1 = "Setpoint1_41004:Setpoint2_56447:Setpoint3_56447:Setpoint4_68493";
  Data_Setpoint();
  delay(25000);

  mData1 = "A_2:B_0:C_0:D_2";
  Data_Tool();
  delay(2000);
  Data_StopTool();
}
void Data_Setpoint ()
{
  mData1.toCharArray(mCharOutput, 70);
  client.publish("Test2", mCharOutput);
}

void Data_Tool()
{
  mData1.toCharArray(mCharOutput, 70);
  client.publish("Test3", mCharOutput);
}

void Data_StopTool()
{
  mData1 = "A_0:B_0:C_0:D_0";
  Data_Tool();
}
//////////////////////////////////////////////////////////////////////////////////////
void Init_TIMER1_interuppt()
{
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;               // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
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
  Serial.println();
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
    if (client.connect("Board_Master"))
    {
      Serial.println("connected");
      //client.subscribe("Test3");
      client.publish("Test1", "Board_Master is connection");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      //Wait 5 seconds before retrying
      delay(1000);
    }
  }
}
void Control_Tool1()
{
  if (digitalRead(38) == LOW)
  {
    A = 1;
  }
  else if (digitalRead(39) == LOW)
  {
    A = 2;
  }
  else
  {
    A = 0;
  }
}
void Control_Tool2()
{
  if (digitalRead(40) == LOW)
  {
    B = 1;
  }
  else if (digitalRead(41) == LOW)
  {
    B = 2;
  }
  else
  {
    B = 0;
  }
}
void Control_Tool3()
{
  if (digitalRead(42) == LOW)
  {
    C = 1;
  }
  else if (digitalRead(43) == LOW)
  {
    C = 2;
  }
  else
  {
    C = 0;
  }
}
void Control_Tool4()
{
  if (digitalRead(44) == LOW)
  {
    D = 1;
  }
  else if (digitalRead(45) == LOW)
  {
    D = 2;
  }
  else
  {
    D = 0;
  }
}
