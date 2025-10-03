// GANT GAUCHE - Arduino NANO avec HC-06
// Ce code gère uniquement les flex sensors

// Configuration des pins
const int flexPins[4] = {A0, A1, A2, A3};   
const int ledPins[4] = {8, 9, 10, 11};

// Variables de calibration
int seuils[4];
int minVals[4];
int maxVals[4];
bool calibrationDone = false;

// Identifiant du gant
const String HAND_ID = "MAIN_GAUCHE";

void setup() {
  Serial.begin(9600);
  
  // Configuration des LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  
  Serial.println("READY:" + HAND_ID);
  
  // Auto-calibration au démarrage
  delay(2000);
  autoCalibration();
}

void loop() {
  // Vérifier les commandes Bluetooth
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "C" || cmd == "CALIBRATE") {
      calibration();
    } else if (cmd == "R" || cmd == "RESET") {
      calibrationDone = false;
      Serial.println("RESET_OK:" + HAND_ID);
    } else if (cmd == "AUTO_CALIB") {
      autoCalibration();
    }
  }
  
  // Envoyer les données si calibré
  if (calibrationDone) {
    sendSensorData();
  }
  
  delay(50); // 20Hz
}

void autoCalibration() {
  Serial.println("CALIB_START:" + HAND_ID);
  
  int tempMin[4], tempMax[4];
  
  // Initialiser avec les premières valeurs
  for (int i = 0; i < 4; i++) {
    int val = analogRead(flexPins[i]);
    tempMin[i] = val;
    tempMax[i] = val;
  }
  
  // Échantillonner pendant 2 secondes
  for (int sample = 0; sample < 40; sample++) {
    for (int i = 0; i < 4; i++) {
      int val = analogRead(flexPins[i]);
      if (val < tempMin[i]) tempMin[i] = val;
      if (val > tempMax[i]) tempMax[i] = val;
    }
    delay(50);
  }
  
  // Configurer les seuils avec marge
  for (int i = 0; i < 4; i++) {
    int range = tempMax[i] - tempMin[i];
    minVals[i] = max(0, tempMin[i] - (range * 10 / 100));
    maxVals[i] = min(1023, tempMax[i] + (range * 10 / 100));
    seuils[i] = tempMin[i] + (range * 55 / 100); // 55% du range
  }
  
  calibrationDone = true;
  
  Serial.println("CALIB_DONE:" + HAND_ID);
  for (int i = 0; i < 4; i++) {
    Serial.print("FINGER" + String(i) + ":");
    Serial.print(minVals[i]);
    Serial.print(",");
    Serial.print(maxVals[i]);
    Serial.print(",");
    Serial.println(seuils[i]);
  }
}

void calibration() {
  Serial.println("CALIB_START:" + HAND_ID);
  Serial.println("EXTEND_FINGERS:" + HAND_ID);
  
  // Animation LED
  for (int countdown = 5; countdown > 0; countdown--) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(150);
      digitalWrite(ledPins[i], LOW);
    }
    Serial.println("COUNTDOWN:" + HAND_ID + ":" + String(countdown));
  }
  
  // Lecture valeurs minimales (doigts droits)
  for (int i = 0; i < 4; i++) {
    minVals[i] = analogRead(flexPins[i]);
    digitalWrite(ledPins[i], HIGH);
  }
  delay(1000);
  
  Serial.println("BEND_FINGERS:" + HAND_ID);
  
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  
  for (int countdown = 5; countdown > 0; countdown--) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(150);
      digitalWrite(ledPins[i], LOW);
    }
    Serial.println("COUNTDOWN:" + HAND_ID + ":" + String(countdown));
  }
  
  // Lecture valeurs maximales (doigts pliés)
  for (int i = 0; i < 4; i++) {
    maxVals[i] = analogRead(flexPins[i]);
    seuils[i] = minVals[i] + ((maxVals[i] - minVals[i]) * 55 / 100);
    digitalWrite(ledPins[i], HIGH);
  }
  
  calibrationDone = true;
  
  Serial.println("CALIB_DONE:" + HAND_ID);
  for (int i = 0; i < 4; i++) {
    Serial.print("FINGER" + String(i) + ":");
    Serial.print(minVals[i]);
    Serial.print(",");
    Serial.print(maxVals[i]);
    Serial.print(",");
    Serial.println(seuils[i]);
  }
  
  delay(2000);
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void sendSensorData() {
  String flexData = "";
  
  for (int i = 0; i < 4; i++) {
    int val = analogRead(flexPins[i]);
    bool isBent = (val > seuils[i]);
    
    digitalWrite(ledPins[i], isBent ? HIGH : LOW);
    
    flexData += String(val) + "," + (isBent ? "1" : "0");
    if (i < 3) flexData += "|";
  }
  
  Serial.println("DATA:" + HAND_ID + ":" + flexData);
}