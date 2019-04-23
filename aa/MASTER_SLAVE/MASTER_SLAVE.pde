import controlP5.*;
import processing.serial.*;

Serial port1, port2;
int[] PortNum = new int [2];
float[] Target = new float [2];
float[] Param_K = new float [2];
float[] Param_C = new float [2];
int[] s_to_m_theta = new int [2];
int[] m_to_s_theta = new int [2];

ControlP5 cp5;
PFont font;

// sync
boolean m_get_response = false;
boolean s_get_response = false;

int m_to_s_angle = 0;
int s_to_m_angle = 0;



static final int FPS = 60, INTERVAL = 1; //5000 = 5 seconds
long cnt = 0;

boolean printTemp = false;

float theta_m = 0, omega_m = 0, theta_mb = 0;
float theta_s = 0, omega_s = 0, theta_sb = 0;
int time_m, time_s;
int prv_time_m, prv_time_s;

void setup() 
{
  size(500, 400);
  printArray(Serial.list());

  font = createFont("Consolas", 15);

  cp5 = new ControlP5(this);
  cp5.addButton("MASTER_SERIAL")
    .setPosition(50, 90)
    .setSize(150, 50)
    .setFont(font);

  cp5.addButton("MASTER_BIAS")
    .setPosition(50, 160)
    .setSize(150, 50)
    .setFont(font);

  cp5.addButton("BILATERAL_CONTROL")
    .setPosition(50, 230)
    .setSize(150, 50)
    .setFont(font);

  cp5.addButton("SLAVE_SERIAL")
    .setPosition(300, 90)
    .setSize(150, 50)
    .setFont(font);

  cp5.addButton("SLAVE_BIAS")
    .setPosition(300, 160)
    .setSize(150, 50)
    .setFont(font);

  loadSerialPort(); 
  //thread("sync_talker");

  //"\\\\.\\COM10"
}


void MASTER_SERIAL()
{
  loadParam();

  port1 = new Serial(this, "COM"+PortNum[0], 115200);
  delay(2000);
  println("Master serial is connected");

  port1.write('K');
  port1.write((int)(Param_K[0] * 100.0));
  port1.write('\n');
  delay(10);

  port1.write('C');
  port1.write((int)(Param_C[0] * 1000.0));
  port1.write('\n');
  delay(10);
}

void MASTER_BIAS()
{
  String value = "B\n";
  port1.write(value);
}

void MASTER_CONTROL()
{
  // to master
  s_to_m_angle = (int)((theta_s * PI / 180 + PI/2) * 10000.0);
  port1.write('T');
  port1.write(s_to_m_angle / 256);
  port1.write(s_to_m_angle % 256);
  port1.write('\n');
        
      
}

void SLAVE_SERIAL()
{
  loadParam();

  port2 = new Serial(this, "COM"+PortNum[1], 115200);
  delay(2000);
  println("Slave serial is connected");

  port2.write('K');
  port2.write((int)(Param_K[1] * 100.0));
  port2.write('\n');
  delay(10);

  port2.write('C');
  port2.write((int)(Param_C[1] * 1000.0));
  port2.write('\n');
  delay(10);


}

void SLAVE_BIAS()
{
  String value = "B\n";
  port2.write(value);
}


void SLAVE_CONTROL()
{
  // to slave
    
  m_to_s_angle = (int)((theta_m * PI / 180 + PI/2) * 10000.0);
  port2.write('T');
  port2.write(m_to_s_angle / 256);
  port2.write(m_to_s_angle % 256);
  port2.write('\n');

}

void BILATERAL_CONTROL()
{
  print("bilateral start");
  thread("sync_talker");
}

void draw()
{
  background(150, 150, 150);

  fill(0, 0, 0);
  textFont(font, 21);
  text("[Bilateral Control for MAE 683 KAIST TCL]", 19, 30);

  textFont(font, 18);
  text("[MASTER]" + " COM:" + PortNum[0], 50, 80);
  text("[SLAVE]" + " COM:" + PortNum[1], 300, 80);

  textFont(font, 15);

  // Master information
  text("POS: " + round(theta_m * 100)/100.0, 50, 315); 
  text("POS_t: " + round(theta_s * 100)/100.0, 50, 330); 

  // Slave information
  text("POS: " + round(theta_s * 100)/100.0, 300, 315);
  text("POS_t: " + round(theta_m * 100)/100.0, 300, 330); 

  println(theta_m, theta_s);
  
}

String serialData_1;
String serialData_2;
void serialEvent(Serial thisPort) {

  if (thisPort==port1)
  {
    // data from serial 1
    serialData_1 = port1.readStringUntil('\n');
    if (serialData_1 != null && serialData_1.length() > 1)
    {
      //print(serialData_1);
      if (serialData_1.charAt(0) == 'P')
      {
        theta_m = float(serialData_1.substring(1, serialData_1.length() - 1)) * 180 / PI; //degree
        //prv_time_m = time_m;
        //time_m = millis();

      }
    }
  }

  if (thisPort==port2)
  {
    // data from serial 2
    serialData_2 = port2.readStringUntil('\n');
    if (serialData_2 != null && serialData_2.length() > 1)
    {
      //println(serialData_2);
      if (serialData_2.charAt(0) == 'P')
      {
        theta_s = float(serialData_2.substring(1, serialData_2.length() - 1)) * 180 / PI;
        //prv_time_s = time_s;
        //time_s = millis();

      }
    }
  }
}


void sync_talker() 
{
  while(true){
    MASTER_CONTROL();
    SLAVE_CONTROL();
    
    delay(50);
  }
}

void loadSerialPort()
{
  String[] lines = loadStrings("port.txt");

  for (int i = 0; i < lines.length; i++) 
  {
    int portValue = int(lines[i]);

    {
      PortNum[i] = portValue;
    }
  }
}


void loadParam()
{
  String[] lines_K = loadStrings("param_K.txt");
  String[] lines_C = loadStrings("param_B.txt");

  for (int i = 0; i < lines_K.length; i++) 
  {
    float value = float(lines_K[i]);
    Param_K[i] = value;
  }

  for (int i = 0; i < lines_C.length; i++) 
  {
    float value = float(lines_C[i]);
    Param_C[i] = value;
  }
}
