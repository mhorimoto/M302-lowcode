void setup(void) {
  outconv(25.0);
  outconv(25.3);
  outconv(25.4);
  outconv(25.6);
  outconv(25.8);
  outconv(25.9);
}

void outconv(float a) {
  int   ia,na;
  char  txt[80],vtxt[6];

  Serial.begin(115200);
  
  ia = (int)a;
  na = (int)(a+0.9);

  dtostrf(a,0,1,vtxt);
  sprintf(txt,"A=%s  (int)=%d --- Next %d",vtxt,ia,na);
  Serial.println(txt);
  Serial.end();
}

void loop(void) {}
