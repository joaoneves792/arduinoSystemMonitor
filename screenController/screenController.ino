#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 4, 8, 9, 10, 11);

int hddPin = A5;

char core0temp[] = "00";
char core1temp[] = "00";
char core2temp[] = "00";
char core3temp[] = "00";

char gpuTemp[] = "00";

bool hdd = false;
bool gpu = false;
bool cpu = false;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(100); // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(hddPin, INPUT_PULLUP); //We need to activate the internal pull up because hdd leds are ground triggered
                                 //Therefore we connect this pin to the HDD- front panel connector pin

}

void updateGpuData(char* data){
  strncpy(gpuTemp, ((const char*)data+8), 2);
  if(data[10] == '0'){
    gpu = true;
  }else{
    gpu = false;
  }  
}


void updateCpuData(char* data){
    strncpy(core0temp, ((const char*)data+0), 2);
    strncpy(core1temp, ((const char*)data+2), 2);
    strncpy(core2temp, ((const char*)data+4), 2);
    strncpy(core3temp, ((const char*)data+6), 2);

    if(data[11] == '0')
      cpu = true;
    else
      cpu = false;
}

void updateHddStatus(){
  static short highCounter = 0;
  static short readCounter = 0; 
  //We pool the disk activity line every 100ms
  //if we get at least one positive for every 1000ms 
  //then turn the indicator on turn
  
  if(!digitalRead(hddPin)){
    highCounter++;
  }
  
  readCounter++;
  
  if(readCounter >= 10){
    if(highCounter > 0){
      hdd = true;
    }else{
      hdd = false;
    }
    highCounter = 0;
    readCounter = 0;
  }
}

void refreshDisplay(){
  static char lines[2][16] = {"               ", "               "};
  char newLines[2][16];
  char gpu_active[] = "   ";
  char cpu_active[] = "   ";
  char hdd_active[] = "   ";

  if(gpu)
    strncpy(gpu_active, "GPU", 3);

  if(cpu)
    strncpy(cpu_active, "CPU", 3);

  if(hdd)
    strncpy(hdd_active, "HDD", 3);



  snprintf(newLines[0], 16, "%s %s %s %sC", hdd_active, gpu_active, cpu_active, gpuTemp);
  snprintf(newLines[1], 16, "%sC %sC %sC %sC", core0temp, core1temp, core2temp, core3temp);

  for(int row=0; row<2; row++){
    for( int col = 0; col<16; col++){
      if(newLines[row][col]!=lines[row][col]){
        lcd.home();
        lcd.setCursor(col, row);
        lcd.write(newLines[row][col]);
        lines[row][col] = newLines[row][col];
      }
    }
  }
  
}

void loop() {
  static bool booting = true;
  char data[32];
  int i;

  i = 0;
  if (Serial.available()){
    delay(100);
    while (Serial.available() > 0 && i < 30) {
      data[i++] = Serial.read();
    }
  }

  updateHddStatus();

  if (i > 0) {
    if(booting){
      booting = false;
      //We only setup the lcd here because Serial communications cause the arduino to reboot
      //However if the screen has already been initialized it wont reboot causing it to receive
      //commands out of order wich produce garbage on the screen!
      //Since there is no way of resetting the screen by code we have to initialize the screen 
      //only after serial communication is established
      lcd.begin(16, 2);
      lcd.clear();
      lcd.home();
    }
    
    data[i] = '\0';
    updateCpuData(data);
    updateGpuData(data);
    refreshDisplay(); //We only refresh the screen when we get data
    
  }
  delay(100); //This is the interval between pooling the hdd activity line
}
