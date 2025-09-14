##Ceci c'est le schema de la première version Mais actuelement se shema est associer au MOU6050 qui es un acceleromètre
<img width="1153" height="740" alt="image" src="https://github.com/user-attachments/assets/275af607-9bbd-4f10-958d-a753a1f40d48" />
<img width="1438" height="775" alt="image" src="https://github.com/user-attachments/assets/6d278736-bac8-4118-8f23-bcb34149cdcd" />

nouvelle version plus a jour



#  SCHÉMA GANT LSF - 5 CAPTEURS FLEX + MPU6050 + 5 LEDs

##  CONNEXIONS ARDUINO UNO

###  **CAPTEURS FLEX (5 doigts)**
```
Capteur Flex POUCE      → A0 + Résistance 10kΩ → GND
Capteur Flex INDEX      → A1 + Résistance 10kΩ → GND  
Capteur Flex MAJEUR     → A2 + Résistance 10kΩ → GND
Capteur Flex ANNULAIRE  → A3 + Résistance 10kΩ → GND
Capteur Flex AURICULAIRE → A4 + Résistance 10kΩ → GND
```

### 💡 **LEDs INDICATEURS (5 LEDs)**
```
LED POUCE      → D8  + Résistance 220Ω → GND
LED INDEX      → D9  + Résistance 220Ω → GND
LED MAJEUR     → D10 + Résistance 220Ω → GND
LED ANNULAIRE  → D11 + Résistance 220Ω → GND
LED AURICULAIRE → D12 + Résistance 220Ω → GND
```

###  **MPU6050 (I2C)**
```
MPU6050 VCC → 3.3V  (⚠️ PAS 5V!)
MPU6050 GND → GND
MPU6050 SCL → A5
MPU6050 SDA → A4
```

---

## 🛠️ **SCHÉMA VISUEL**

```
    ARDUINO UNO
    ┌─────────────────┐
    │  3.3V    5V     │
    │   │       │     │
    │  GND     GND    │ 
    │   │       │     │
    │              A0 ├── FLEX POUCE + 10kΩ → GND
    │              A1 ├── FLEX INDEX + 10kΩ → GND  
    │              A2 ├── FLEX MAJEUR + 10kΩ → GND
    │              A3 ├── FLEX ANNULAIRE + 10kΩ → GND
    │              A4 ├── FLEX AURICULAIRE + 10kΩ → GND
    │              A5 ├── MPU6050 SCL
    │                 │
    │               D8 ├── LED POUCE + 220Ω → GND
    │               D9 ├── LED INDEX + 220Ω → GND
    │              D10 ├── LED MAJEUR + 220Ω → GND
    │              D11 ├── LED ANNULAIRE + 220Ω → GND
    │              D12 ├── LED AURICULAIRE + 220Ω → GND
    └─────────────────┘
           │
           └── MPU6050
               ├── VCC → 3.3V
               ├── GND → GND  
               ├── SCL → A5
               └── SDA → A4
```

---

##  **MATÉRIEL NÉCESSAIRE**

- **1x** Arduino UNO
- **5x** Capteurs Flex 2.2" (un par doigt)
- **1x** MPU6050 (gyroscope/accéléromètre)
- **5x** LEDs (couleur au choix)
- **5x** Résistances 10kΩ (pour capteurs flex)
- **5x** Résistances 220Ω (pour LEDs)
- **1x** Breadboard ou PCB
- Fils de connexion
- Gant en tissu

---

## ⚠️ **POINTS CRITIQUES**

1. **MPU6050 OBLIGATOIREMENT en 3.3V** (pas 5V!)
2. **Résistances 10kΩ obligatoires** pour chaque capteur flex
3. **Résistances 220Ω obligatoires** pour chaque LED
4. **A4/A5 réservés pour I2C** (MPU6050)
5. **Ne pas plier les capteurs flex à plus de 90°**

---

## 🔧 **INSTALLATION PHYSIQUE**

```
GANT:
├── POUCE: Capteur Flex + LED
├── INDEX: Capteur Flex + LED  
├── MAJEUR: Capteur Flex + LED
├── ANNULAIRE: Capteur Flex + LED
├── AURICULAIRE: Capteur Flex + LED
└── DOS DE LA MAIN: MPU6050 (bien fixé)

ARDUINO: Dans pochette au poignet
```

---

##  **CONFIGURATION CODE**

```cpp
// Capteurs Flex (5 doigts)
int flexPins[5] = {A0, A1, A2, A3, A4};  // P,I,M,An,Au

// LEDs (5 doigts) 
int ledPins[5] = {8, 9, 10, 11, 12};     // P,I,M,An,Au

// MPU6050 sur I2C (A4=SDA, A5=SCL)
```

**Format des gestes:** `PIMAA` (Pouce, Index, Majeur, Annulaire, Auriculaire)

**Exemple:** `00000` = Main ouverte, `11111` = Poing fermé
