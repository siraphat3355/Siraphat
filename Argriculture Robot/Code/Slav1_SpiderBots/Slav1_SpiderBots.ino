#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

EthernetClient ethClient;
PubSubClient client(ethClient);

byte mac[] = {  0xDE, 0xED, 0xAA, 0xFE, 0xFE, 0xEF };
IPAddress    ip(158, 108, 5, 1);
IPAddress    server(158, 108, 5, 6);
const char*  mqtt_server = "158.108.5.6:1883";
const char*  Board = "Board1";

#define PWMz      6
#define IN1       4
#define IN2       5
#define T       0.1      //Collect Data  100 ms // 0.01 //sec

int          Mn_max       =  30;
int          Mn_min       = -Mn_max;
const int slaveSelectEnc1 = 53;
double             Timerz = 59200;

float deltaTime ;
double Velocity ;
long  currentPos;
long   deltaPos ;
float oldTime    = 0;
long oldPos      = 0;
long setpoint    = 0;

double Cn, Mn, Mn1, Mn2, En, En1, En2;
double        Kp = 0;
double        Ki = 0;
double        Kd = 0;
double   counter = 0;
double        K0 = 0;
double        K1 = 0;
double        K2 = 0;
double        Rn = 0;
double        pulse ;
int        mMode = 0;

String     mData;
char mCharOutput[50];
double FactorPulseperCm = 117.72 ;
double XX = 560, YY = 560, ZZ = 400, A = 8.5;
double x = 280, y = 280, z = 90;

// @ XX=560,YY=560,ZZ=400
// @Home X=280,Y=280,Z=90   L1=493 cm,L2=493 cm,L3=493 cm,L4=493 cm;

int L1_Home = 493;
int L2_Home = 493;
int L3_Home = 493;
int L4_Home = 493;
long L1 =  sqrt(sq(x - A) + sq(YY - y - A) + sq(ZZ - z));
long L2 =  sqrt(sq(XX - x - A) + sq(YY - y - A) + sq(ZZ - z));
long L3 =  sqrt(sq(x - A) + sq(y - A) + sq(ZZ - z));
long L4 =  sqrt(sq(XX - x - A) + sq(y - A) + sq(ZZ - z));



////////////////////////////////---Init---//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Init_PWM()
{
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(PWMz, OUTPUT);
}

void Init_PID_Control()
{
  Mn_max       = 30;
  Mn_min       = -Mn_max;
  En2          = 0;
  En1          = 0;
  Mn           = 0;
  Mn1          = 0;
  Mn2          = 0;
  counter      = 0;
}


void Init_VeloPID()
{
  Serial.println(pulse);
  if (pulse > 0) //push
  {
    Rn =  pulse * 0.001;
    Kp =  abs(pulse * 0.0001);
    Ki =  abs(pulse * 0.00005);
  }
  else    //pull
  {
    Rn = pulse * 0.001;
    Kp = abs(pulse * 0.00025);
    Ki = abs(pulse * 0.00012);
    Kd =  0.005;
  }
}
void Init_PosPID()
{
  Rn = setpoint;

  Kp = 0.1;
  Ki = 0.003;
  Kd = 0.0001;
}


void Init_PID_Gain()
{
  K0 = Kp + (Ki * T) + (Kd / T);
  K1 = (-Kp) - (2 * Kd / T);
  K2 = (Kd / T);
}
ISR(TIMER1_OVF_vect)         // interrupt service routine Timer 01
{
  TCNT3 = 0;
  CheckMQTT();
  //String PPos ="Setpoint1_" + String(X);
  String PPos = " M " + String(mMode) + " En " + String(currentPos) + " Set " + String(setpoint) + " Mn " + String(Mn);
  PPos.toCharArray(mCharOutput, 50);
  client.publish("L1", mCharOutput);
  //deltaTime = CalculateDeltaTime();

  
}
ISR(TIMER3_OVF_vect)         // interrupt service routine Timer 03
{

  PID_Control_Run();

  //Serial.println(Velocity);
  /*
    Serial.print("Rn: " + String(Rn));
    Serial.print("   Cn: " + String(Cn));
    Serial.print("   En: " + String(En));
    Serial.print("   Mn: " + String(Mn));
    Serial.print("   Mn_max: " + String(Mn_max));
    Serial.print("   Mn_min: " + String(Mn_min));

    if (mMode == 0)
      Serial.println("    Velocity Mode");
    else
      Serial.println("    Position Mode");

    Serial.print("Setpoint: " + String(setpoint));
    Serial.println("  Current Position: " + String(currentPos));
    Serial.println("------------------------------------");
  */
  TCNT3 = Timerz;
}

void PID_Control_Run()
{
  deltaTime = CalculateDeltaTime();
  deltaPos =  CalculateDeltaPos();
  Velocity = ((deltaPos / deltaTime) * 1000 * 60) / 4096.0;


  if ( abs(abs(currentPos) - abs(setpoint)) >= 1000 && mMode == 0 )
  {
    mMode = 0;
    Init_VeloPID();
    Cn = Velocity;
  }
  else
  {
    mMode = 1;
    Init_PosPID();
    Cn = currentPos;
  }

  Init_PID_Gain();
  En = Rn - Cn;
  Mn = Mn1 + (K0 * En) + (K1 * En1) + (K2 * En2) ;

  if ( abs(En) <= 100 && mMode == 1 )
  {
    Mn = 0;
  }
  PWM();

  // Update Parameters-----------------
  En2 = En1;
  En1 = En;
  Mn2 = Mn1;
  Mn1 = Mn;
  counter ++;
}

float CalculateDeltaTime()
{
  float currentTime = millis();
  float deltaTime = currentTime - oldTime;
  oldTime = currentTime;
  return deltaTime;
}

long CalculateDeltaPos()
{
  currentPos = ((-1) * (readEncoder() * 1024) / 3600.0) + (L1_Home * FactorPulseperCm);
  deltaPos = currentPos - oldPos;
  oldPos = currentPos;
  return deltaPos;
}

void PWM()
{
  double MnTest;


  // Limit SPEED -----------------
  MnTest = Mn;

  if (Mn > Mn_max)
  {
    if (mMode == 0)
      Mn_max = Mn;
    else
    {
      MnTest = Mn_max;
    }
  }
  else if (Mn < Mn_min && counter >= 10)
  {
    if (mMode == 0)
      Mn_min = Mn;
    else
    {
      MnTest = Mn_min;
    }
  }

  Serial.println("Mn_Test: " + String(MnTest) );
  // ------------------------------
  analogWrite(PWMz, abs(MnTest));

  if (Mn > 0)
    dictertion_MotorM1(2);

  else if (Mn < 0)
    dictertion_MotorM1(1);

  else if (Mn == 0)
    dictertion_MotorM1(0);
}

void dictertion_MotorM1(int Dir)
{
  switch (Dir)
  {
    case 0  :
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      break;

    case 1 :
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      break;

    case 2 :
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      break;

  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(57600);
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  InitEncoders();       Serial.println("Encoders Initialized...");
  clearEncoderCount();  Serial.println("Encoders Cleared...");
  delay(1000);

  Init_PWM();
  Init_PID_Control();
  Init_TIMER1_interuppt();
  Init_TIMER3_interuppt();
  mData = "Setpoint1_58035:Setpoint2_58035:Setpoint3_58035:Setpoint4_58035";
}

void loop()
{
  delay(100);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InitEncoders()
{
  // Set slave selects as outputs
  pinMode(slaveSelectEnc1, OUTPUT);
  // Raise select pins
  // Communication begins when you drop the individual select signsl
  delay(1);
  digitalWrite(slaveSelectEnc1, HIGH);
  SPI.begin();
  digitalWrite(slaveSelectEnc1, LOW);
  delay(1);
  SPI.transfer(0x88);                       // Write to MDR0
  SPI.transfer(0x03);
  delay(1);
  digitalWrite(slaveSelectEnc1, HIGH);      // Terminate SPI conversation
  delay(1);
}

long readEncoder()
{
  unsigned int count_1, count_2, count_3, count_4;
  long count_value;

  digitalWrite(slaveSelectEnc1, LOW);     // Begin SPI conversation
  SPI.transfer(0x60);                     // Request count
  count_1 = SPI.transfer(0x00);           // Read highest order byte
  count_2 = SPI.transfer(0x00);
  count_3 = SPI.transfer(0x00);
  count_4 = SPI.transfer(0x00);           // Read lowest order byte
  digitalWrite(slaveSelectEnc1, HIGH);    // Terminate SPI conversation

  count_value = (count_1 << 8)     + count_2;
  count_value = (count_value << 8) + count_3;
  count_value = (count_value << 8) + count_4;
  delayMicroseconds(100);
  return count_value;
}

void clearEncoderCount()
{
  // Set encoder1's data register to 0
  digitalWrite(slaveSelectEnc1, LOW);     // Begin SPI conversation
  // Write to DTR
  SPI.transfer(0x98);
  // Load data
  SPI.transfer(0x00);  // Highest order byte
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);  // lowest order byte
  digitalWrite(slaveSelectEnc1, HIGH);    // Terminate SPI conversation

  delayMicroseconds(100);  // provides some breathing room between SPI conversations

  // Set encoder1's current data register to center
  digitalWrite(slaveSelectEnc1, LOW);     // Begin SPI conversation
  SPI.transfer(0xE0);
  digitalWrite(slaveSelectEnc1, HIGH);    // Terminate SPI conversation
}

void callback(char* topic, byte* payload, unsigned int length)
{
  mData = "";
  /*Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");*/
  for (int i = 0; i < length; i++)
  {
    // Serial.print((char)payload[i]);
    mData += (char)payload[i];
  }
  Init_PID_Control() ;
  setpoint = mqttQueryString("Setpoint1_", mData).toInt();
  pulse = setpoint - currentPos;
  mMode = 0;
  //Serial.println();

}

void reconnect()
{
  // Loop until we're reconnected
  //while (!client.connected())
  {
    //Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Board1"))
    {
      //Serial.println("connected");
      // Once connected, publish an announcement..
      client.subscribe("Test2");
      client.publish("Test1", "Board1 is connection");
    }
    else
    {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(10);
    }
  }
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
void Init_TIMER1_interuppt() {

  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;               // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}
void Init_TIMER3_interuppt()
{
  noInterrupts();           // disable all interrupts
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = Timerz;            // preload timer
  TCCR3B |= (1 << CS32);     // 256 prescaler
  TIMSK3 |= (1 << TOIE3);    // enable timer overflow interrupt
  interrupts();              // enable all interrupts
}
void CheckMQTT()
{
  if (!client.connected())
  {
    reconnect();

  }
  client.loop();
}
