// ***** GANT INTELLIGENT LSF - CAPTEURS FLEX + IMU *****
// Arduino UNO + 4 LEDs + 4 Capteurs Flex + MPU6050
// Détection avancée avec position de la main ET flexion des doigts

#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

// ===== CONFIGURATION MPU6050 =====
MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;
float angleX, angleY, angleZ;
float gyroX, gyroY, gyroZ;
float dt = 0.01;
float alpha = 0.96;
unsigned long lastIMUTime = 0;

// ===== CONFIGURATION CAPTEURS FLEX =====
int flexPins[4] = {A0, A1, A2, A3};  // Index, Majeur, Annulaire, Auriculaire
int flexVals[4];
int ledPins[4] = {8, 9, 10, 11};
int seuils[4] = {500, 500, 500, 500};
int minVals[4] = {1023, 1023, 1023, 1023};
int maxVals[4] = {0, 0, 0, 0};

// ===== VARIABLES DE GESTION =====
String dernierGeste = "";
unsigned long dernierTemps = 0;
bool calibrationTerminee = false;

// Structure pour définir un geste complet
struct Geste {
  String nom;
  int doigts[4];        // Configuration des doigts (0=tendu, 1=plié)
  float yawMin, yawMax; // Plage d'angle Yaw (rotation Z)
  float pitchMin, pitchMax; // Plage d'angle Pitch (rotation Y)  
  float rollMin, rollMax;   // Plage d'angle Roll (rotation X)
  bool checkOrientation;    // Vérifier l'orientation ou pas
};

// ===== BASE DE DONNÉES DES GESTES LSF =====
Geste gestesLSF[] = {
  // LETTRES DE BASE
  {"Lettre A", {0,0,0,0}, -30,30, -30,30, -30,30, false},
  {"Lettre E", {1,1,1,1}, -30,30, -30,30, -30,30, false},
  
  // LETTRES AVEC ORIENTATION SPÉCIFIQUE
  {"Lettre B (main verticale)", {0,1,1,1}, -20,20, -15,15, 70,110, true},
  {"Lettre C (main courbe)", {0,0,0,0}, 45,135, -30,30, -45,45, true},
  {"Lettre D (index pointé)", {0,1,1,1}, -15,15, -10,40, -20,20, true},
  {"Lettre F (index + pouce)", {0,1,1,1}, -20,20, -20,20, -30,30, true},
  {"Lettre G (index horizontal)", {0,1,1,1}, 80,100, -10,10, -20,20, true},
  {"Lettre H (2 doigts horizontaux)", {0,0,1,1}, 80,100, -10,10, -20,20, true},
  {"Lettre I (auriculaire levé)", {1,1,1,0}, -20,20, -20,20, 70,110, true},
  {"Lettre L (angle droit)", {0,1,1,0}, -20,20, -20,20, -30,30, true},
  {"Lettre V (victoire)", {0,0,1,1}, -20,20, -20,20, 70,110, true},
  
  // SALUTATIONS ET POLITESSE
  {"BONJOUR (main ouverte vers l'avant)", {0,0,0,0}, -15,15, 20,60, -20,20, true},
  {"BONSOIR (main vers le bas)", {0,0,0,0}, -15,15, -60,-20, -20,20, true},
  {"AU REVOIR (main qui bouge)", {0,0,0,0}, -30,30, 10,50, -30,30, true},
  {"MERCI (main vers soi)", {1,1,1,1}, -20,20, -40,0, -20,20, true},
  {"S'IL VOUS PLAÎT", {0,1,1,0}, -15,15, 30,70, -30,30, true},
  
  // RÉPONSES COURANTES
  {"OUI (hochement)", {0,0,0,0}, -20,20, 40,80, -20,20, true},
  {"NON (balancement)", {0,0,0,0}, -80,-40, -20,20, -20,20, true},
  {"PEUT-ÊTRE", {0,1,0,1}, -30,30, -10,30, -30,30, true},
  
  // ÉMOTIONS ET EXPRESSIONS
  {"CONTENT (sourire avec main)", {0,0,0,0}, -15,15, -10,30, 45,75, true},
  {"TRISTE (main vers le bas)", {1,1,1,1}, -15,15, -50,-10, -30,30, true},
  {"COLÈRE (poing serré)", {1,1,1,1}, 60,120, -20,20, -30,30, true},
  
  // QUESTIONS ET COMMUNICATION
  {"COMMENT ÇA VA ?", {0,0,1,0}, 70,110, -10,30, -20,20, true},
  {"QUI ?", {0,1,1,1}, -15,15, 30,70, -20,20, true},
  {"QUOI ?", {0,0,0,0}, 30,60, 20,60, -30,30, true},
  {"OÙ ?", {0,1,1,1}, 45,90, 10,50, -30,30, true},
  {"QUAND ?", {0,1,0,1}, -30,30, 20,60, 60,90, true},
  
  // ACTIONS COURANTES
  {"MANGER", {1,0,0,1}, -20,20, -30,10, -45,15, true},
  {"BOIRE", {1,1,1,0}, -15,15, -20,40, 30,70, true},
  {"DORMIR", {0,0,0,0}, 70,110, -40,0, 45,90, true},
  
  // GESTES DE POINTAGE ET DIRECTION
  {"LÀ-BAS (pointage)", {0,1,1,1}, 45,135, -10,30, -20,20, true},
  {"ICI (vers soi)", {0,1,1,1}, -45,45, -30,10, -20,20, true},
  {"VENIR", {0,0,0,0}, -20,20, -20,20, -90,-45, true},
  {"PARTIR", {0,0,0,0}, -20,20, -20,20, 45,90, true},
};

const int nombreGestes = sizeof(gestesLSF) / sizeof(gestesLSF[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("=== GANT LSF INTELLIGENT v2.0 ===");
  Serial.println("Capteurs Flex + IMU MPU6050");
  Serial.println("==================================");
  
  // ===== INITIALISATION IMU =====
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(400000);
  #endif
  
  Serial.print("Initialisation MPU6050... ");
  mpu.initialize();
  
  if (mpu.testConnection()) {
    Serial.println("OK!");
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  } else {
    Serial.println("ÉCHEC!");
    Serial.println("Vérifiez les connexions I2C du MPU6050");
    while(1);
  }
  
  // ===== INITIALISATION LEDs =====
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);
    delay(100);
    digitalWrite(ledPins[i], LOW);
  }
  
  Serial.println("LEDs initialisées ✓");
  
  // ===== CALIBRATION =====
  Serial.println("\n🎯 PHASE DE CALIBRATION (5 secondes)");
  Serial.println("Bougez vos doigts et votre main dans tous les sens...");
  
  calibrerCapteurs();
  
  Serial.println("✅ Calibration terminée!");
  Serial.println("\n📊 Format de sortie:");
  Serial.println("Angles(Y,P,R) | Pattern | Geste détecté");
  Serial.println("===========================================");
  
  calibrationTerminee = true;
  lastIMUTime = millis();
}

void calibrerCapteurs() {
  unsigned long startTime = millis();
  int compteur = 0;
  
  while (millis() - startTime < 5000) {
    // Affichage du progress
    if (compteur % 100 == 0) {
      int progress = ((millis() - startTime) * 100) / 5000;
      Serial.print("Calibration: ");
      Serial.print(progress);
      Serial.println("%");
    }
    
    // Calibrer les capteurs flex
    for (int i = 0; i < 4; i++) {
      int val = analogRead(flexPins[i]);
      if (val < minVals[i]) minVals[i] = val;
      if (val > maxVals[i]) maxVals[i] = val;
    }
    
    compteur++;
    delay(10);
  }
  
  // Calculer les seuils
  for (int i = 0; i < 4; i++) {
    seuils[i] = (minVals[i] + maxVals[i]) / 2;
  }
  
  // Afficher les résultats de calibration
  Serial.println("\n📈 Résultats calibration capteurs:");
  String doigtNoms[] = {"Index", "Majeur", "Annulaire", "Auriculaire"};
  for (int i = 0; i < 4; i++) {
    Serial.print(doigtNoms[i]);
    Serial.print(": Min=");
    Serial.print(minVals[i]);
    Serial.print(", Max=");
    Serial.print(maxVals[i]);
    Serial.print(", Seuil=");
    Serial.println(seuils[i]);
  }
}

void loop() {
  if (!calibrationTerminee) return;
  
  // ===== LECTURE IMU (100Hz) =====
  unsigned long currentTime = millis();
  if (currentTime - lastIMUTime >= 10) {
    dt = (currentTime - lastIMUTime) / 1000.0;
    lastIMUTime = currentTime;
    
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    // Convertir gyroscope
    gyroX = gx / 131.0;
    gyroY = gy / 131.0;
    gyroZ = gz / 131.0;
    
    // Calculer angles accéléromètre
    float accelAngleX = atan2(ay, az) * 180 / PI;
    float denominator = sqrt(ay*ay + az*az);
    float accelAngleY = (denominator > 0.001) ? atan2(-ax, denominator) * 180 / PI : 0;
    
    // Filtre complémentaire
    angleX = alpha * (angleX + gyroX * dt) + (1 - alpha) * accelAngleX;
    angleY = alpha * (angleY + gyroY * dt) + (1 - alpha) * accelAngleY;
    angleZ += gyroZ * dt;
    
    // Normaliser angleZ
    while (angleZ > 180) angleZ -= 360;
    while (angleZ < -180) angleZ += 360;
  }
  
  // ===== LECTURE CAPTEURS FLEX =====
  int etat[4];
  for (int i = 0; i < 4; i++) {
    flexVals[i] = analogRead(flexPins[i]);
    etat[i] = (flexVals[i] > seuils[i]) ? 1 : 0;
    digitalWrite(ledPins[i], etat[i] ? HIGH : LOW);
  }
  
  // ===== DÉTECTION DE GESTE AVANCÉE =====
  String gesteDetecte = detecterGesteAvance(etat, angleZ, angleY, angleX);
  
  // ===== AFFICHAGE =====
  if (gesteDetecte != "" && gesteDetecte != dernierGeste && 
      (millis() - dernierTemps > 1500)) {
    
    // Créer pattern binaire pour debug
    String pattern = "";
    for (int i = 0; i < 4; i++) {
      pattern += String(etat[i]);
    }
    
    // Affichage formaté
    Serial.print("Angles(");
    Serial.print((int)angleZ); Serial.print(",");
    Serial.print((int)angleY); Serial.print(",");
    Serial.print((int)angleX); Serial.print(") | ");
    Serial.print(pattern);
    Serial.print(" | 🤟 ");
    Serial.println(gesteDetecte);
    
    dernierGeste = gesteDetecte;
    dernierTemps = millis();
    
    // Feedback LED
    clignoterConfirmation();
  }
  
  delay(50); // 20Hz pour l'affichage
}

String detecterGesteAvance(int etat[], float yaw, float pitch, float roll) {
  // Parcourir tous les gestes définis
  for (int i = 0; i < nombreGestes; i++) {
    Geste g = gestesLSF[i];
    
    // 1. Vérifier la configuration des doigts
    bool doigtsOK = true;
    for (int j = 0; j < 4; j++) {
      if (etat[j] != g.doigts[j]) {
        doigtsOK = false;
        break;
      }
    }
    
    if (!doigtsOK) continue;
    
    // 2. Vérifier l'orientation si nécessaire
    if (g.checkOrientation) {
      if (yaw < g.yawMin || yaw > g.yawMax) continue;
      if (pitch < g.pitchMin || pitch > g.pitchMax) continue;
      if (roll < g.rollMin || roll > g.rollMax) continue;
    }
    
    // Geste trouvé !
    return g.nom;
  }
  
  return ""; // Aucun geste reconnu
}

void clignoterConfirmation() {
  // Clignotement rapide de confirmation
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      digitalWrite(ledPins[j], HIGH);
    }
    delay(100);
    for (int j = 0; j < 4; j++) {
      digitalWrite(ledPins[j], LOW);
    }
    delay(100);
  }
}

// Fonction d'aide pour le debug
void afficherStatut() {
  Serial.println("\n=== STATUT CAPTEURS ===");
  Serial.print("IMU Angles - Yaw: ");
  Serial.print(angleZ);
  Serial.print("°, Pitch: ");
  Serial.print(angleY);
  Serial.print("°, Roll: ");
  Serial.print(angleX);
  Serial.println("°");
  
  Serial.print("Capteurs Flex: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(flexVals[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("=====================");
}