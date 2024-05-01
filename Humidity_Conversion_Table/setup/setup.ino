#include <EEPROM.h>

byte hct[720];
  
void setup(void) {
  Serial.begin(115200);
  byte hct[]  = {
    98,86,73,60,47,34,22,11,0,0,0,0,0,0,0,0,
    98,87,74,61,49,37,25,14,0,0,0,0,0,0,0,0,
    98,87,75,63,51,39,28,17,0,0,0,0,0,0,0,0,
    98,88,76,64,53,42,31,21,10,0,0,0,0,0,0,0,
    99,88,76,65,54,44,33,23,14,0,0,0,0,0,0,0,
    99,88,77,66,56,46,36,26,17,0,0,0,0,0,0,0,
    99,89,78,68,57,48,38,29,20,11,0,0,0,0,0,0,
    99,89,79,69,59,49,40,31,22,14,0,0,0,0,0,0,
    99,89,79,70,60,51,42,33,25,17,0,0,0,0,0,0,
    99,90,80,70,61,52,44,35,27,19,12,0,0,0,0,0,
    99,90,81,71,62,54,45,37,29,22,15,0,0,0,0,0,
    99,90,81,72,63,55,47,39,32,24,17,10,0,0,0,0,
    99,91,82,73,64,56,48,41,33,26,19,13,0,0,0,0,
    99,91,82,74,65,57,50,42,35,28,22,15,0,0,0,0,
    99,91,83,74,66,59,51,44,37,30,24,18,11,0,0,0,
    99,91,83,75,67,60,52,45,39,32,26,20,14,0,0,0,
    99,92,83,75,68,61,54,47,40,34,28,22,16,10,0,0,
    99,92,84,76,69,62,55,48,42,35,29,24,18,13,0,0,
    99,92,84,77,69,62,56,49,43,37,31,25,20,15,10,0,
    99,92,84,77,70,63,57,50,44,38,33,27,22,17,12,0,
    99,92,85,78,71,64,58,51,45,40,34,29,24,19,14,0,
    99,92,85,78,71,65,58,52,47,41,36,30,25,20,16,11,
    99,93,85,78,72,65,59,53,48,42,37,32,27,22,17,13,
    100,93,86,79,72,66,60,54,49,43,38,33,28,24,19,15,
    100,93,86,79,73,67,61,55,50,44,39,34,30,25,21,16,
    100,93,86,80,73,67,62,56,51,45,40,36,31,27,22,18,
    100,93,86,80,74,68,62,57,51,46,41,37,32,28,24,20,
    100,93,87,80,74,68,63,57,52,47,43,38,33,29,25,21,
    100,93,87,81,75,69,63,58,53,48,43,39,35,30,26,22,
    100,93,87,81,75,69,64,59,54,49,44,40,36,32,28,24,
    100,94,87,81,76,70,65,60,55,50,45,41,37,33,29,25,
    100,94,87,82,76,70,65,60,55,51,46,42,38,34,30,26,
    100,94,88,82,76,71,66,61,56,51,47,43,39,35,31,27,
    100,94,88,82,77,71,66,61,57,52,48,44,39,36,32,28,
    100,94,88,82,77,72,67,62,57,53,48,44,40,37,33,29,
    100,94,88,82,77,72,67,62,58,53,49,45,41,37,34,30,
    100,94,88,83,78,72,68,63,58,54,50,46,42,38,35,31,
    100,94,88,83,78,73,68,63,59,55,50,46,43,39,35,32,
    100,94,89,83,78,73,68,64,59,55,51,47,43,40,36,33,
    100,94,89,83,78,73,69,64,60,56,52,48,44,41,37,34,
    100,94,89,84,79,74,69,65,60,56,52,48,45,41,38,35,
    100,94,89,84,79,74,69,65,61,57,53,49,45,42,39,35,
    100,94,89,84,79,74,70,65,61,57,53,50,46,43,39,36,
    100,95,89,84,79,75,70,66,61,58,54,50,47,43,40,37,
    100,95,89,84,80,75,71,66,62,58,54,51,47,44,41,37};
  int x;
  for(x=0;x<720;x++) {
    EEPROM.write(0x100+x,hct[x]);
  }
  Serial.println("Write done");
}

void loop(void) {
}

