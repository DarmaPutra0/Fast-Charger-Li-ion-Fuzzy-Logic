#include <Fuzzy.h>
#include "VEGA_MLX90614.h"
#include <PWM.h>
#define PWMSwitch 9
#define LED 14
#define VoltSensor A0

VEGA_MLX90614 mlx(2, 3);  // SDA , SCL

const long interval = 1000;
unsigned long previousMillis = 0;
volatile float tegangan = 0;
volatile float adc = 0;
volatile double output = 0;
double temperature = 0;

// Floats for resistor values in divider (in ohms)
const float R1 = 30000.0;
const float R2 = 7500.0;

int pwmvlaue = 0;

// Fuzzy Sets untuk Suhu dan Tegangan
Fuzzy *fuzzy = new Fuzzy();

FuzzySet *Dingin = new FuzzySet(26, 28.5, 31, 31.5);
FuzzySet *Sedang = new FuzzySet(32, 34, 36, 40);
FuzzySet *Panas = new FuzzySet(41, 43, 44, 45);
//FuzzySet *PanasSekali = new FuzzySet(42.2, 42.6, 45, 45.5);


FuzzySet *Low = new FuzzySet(9.4, 9.6, 9.8, 10.2);
FuzzySet *Mid = new FuzzySet(11.02, 11.20, 11.40, 12.05);
FuzzySet *High = new FuzzySet(12.10, 12.20, 12.30, 12.80);
//FuzzySet *HighSekali = new FuzzySet(12.40, 12.40, 12.40, 12.40);


FuzzySet *Lambat = new FuzzySet(180, 190, 190, 195);
FuzzySet *Tengah = new FuzzySet(200, 215, 220, 225);
FuzzySet *Cepat = new FuzzySet(230, 240, 245, 255);
FuzzySet *Stop = new FuzzySet(0, 0, 0, 0);


void setup() {
  pinMode(VoltSensor, INPUT);
  pinMode(PWMSwitch, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  delay(100);
  fuzzyruleInit();
  getSensor();
  delay(1000);
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    getSensor();

    fuzzy->setInput(1, temperature);
    fuzzy->setInput(2, tegangan);

    fuzzy->fuzzify();
    output = fuzzy->defuzzify(1);
    Serial.print("OUTPUT PWM :");
    Serial.println(output);
    Serial.println("----------------------------");


    analogWrite(PWMSwitch, output);
    previousMillis = currentMillis;
  }
  if (output == 0) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}




void getSensor() {
  adc = analogRead(A0);
  adc = (adc * 5.0) / 1024.0;
  tegangan = adc / (R2 / (R1 + R2));
  temperature = mlx.mlx90614ReadTargetTempC();
  Serial.print("tegangan :");
  Serial.println(tegangan);
  Serial.print("Temperature :");
  Serial.println(temperature);
  Serial.println("----------------------------");
}


void fuzzyruleInit() {
  // FuzzyInput
  FuzzyInput *Temperature = new FuzzyInput(1);
  Temperature->addFuzzySet(Dingin);
  Temperature->addFuzzySet(Sedang);
  Temperature->addFuzzySet(Panas);
  //Temperature->addFuzzySet(PanasSekali);
  fuzzy->addFuzzyInput(Temperature);

  FuzzyInput *Voltage = new FuzzyInput(2);
  Voltage->addFuzzySet(Low);
  Voltage->addFuzzySet(Mid);
  Voltage->addFuzzySet(High);
  //Voltage->addFuzzySet(HighSekali);
  fuzzy->addFuzzyInput(Voltage);

  FuzzyOutput *PWM = new FuzzyOutput(1);
  PWM->addFuzzySet(Stop);
  PWM->addFuzzySet(Lambat);
  PWM->addFuzzySet(Tengah);
  PWM->addFuzzySet(Cepat);
  fuzzy->addFuzzyOutput(PWM);

  FuzzyRuleAntecedent *D_L = new FuzzyRuleAntecedent();
  D_L->joinWithAND(Dingin, Low);
  FuzzyRuleConsequent *D_LO = new FuzzyRuleConsequent();
  D_LO->addOutput(Cepat);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, D_L, D_LO);
  fuzzy->addFuzzyRule(fuzzyRule1);

  FuzzyRuleAntecedent *D_M = new FuzzyRuleAntecedent();
  D_M->joinWithAND(Dingin, Mid);
  FuzzyRuleConsequent *D_MO = new FuzzyRuleConsequent();
  D_MO->addOutput(Tengah);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, D_M, D_MO);
  fuzzy->addFuzzyRule(fuzzyRule2);

  FuzzyRuleAntecedent *D_H = new FuzzyRuleAntecedent();
  D_H->joinWithAND(Dingin, High);
  FuzzyRuleConsequent *D_HO = new FuzzyRuleConsequent();
  D_HO->addOutput(Lambat);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, D_H, D_HO);
  fuzzy->addFuzzyRule(fuzzyRule3);

  //////////////////////////////////////////////////////////////

  FuzzyRuleAntecedent *S_L = new FuzzyRuleAntecedent();
  S_L->joinWithAND(Sedang, Low);
  FuzzyRuleConsequent *S_LO = new FuzzyRuleConsequent();
  S_LO->addOutput(Cepat);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, S_L, S_LO);
  fuzzy->addFuzzyRule(fuzzyRule4);

  FuzzyRuleAntecedent *S_M = new FuzzyRuleAntecedent();
  S_M->joinWithAND(Sedang, Mid);
  FuzzyRuleConsequent *S_MO = new FuzzyRuleConsequent();
  S_MO->addOutput(Tengah);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, S_M, S_MO);
  fuzzy->addFuzzyRule(fuzzyRule5);

  FuzzyRuleAntecedent *S_H = new FuzzyRuleAntecedent();
  S_H->joinWithAND(Sedang, High);
  FuzzyRuleConsequent *S_HO = new FuzzyRuleConsequent();
  S_HO->addOutput(Lambat);
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, S_H, S_HO);
  fuzzy->addFuzzyRule(fuzzyRule6);

  //////////////////////////////////////////////////////////////

  FuzzyRuleAntecedent *P_L = new FuzzyRuleAntecedent();
  P_L->joinWithAND(Panas, Low);
  FuzzyRuleConsequent *P_LO = new FuzzyRuleConsequent();
  S_LO->addOutput(Stop);
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, P_L, P_LO);
  fuzzy->addFuzzyRule(fuzzyRule7);

  FuzzyRuleAntecedent *P_M = new FuzzyRuleAntecedent();
  P_M->joinWithAND(Panas, Mid);
  FuzzyRuleConsequent *P_MO = new FuzzyRuleConsequent();
  P_MO->addOutput(Stop);
  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, P_M, P_MO);
  fuzzy->addFuzzyRule(fuzzyRule7);

  FuzzyRuleAntecedent *P_H = new FuzzyRuleAntecedent();
  P_H->joinWithAND(Panas, High);
  FuzzyRuleConsequent *P_HO = new FuzzyRuleConsequent();
  P_HO->addOutput(Stop);
  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, S_H, S_HO);
  fuzzy->addFuzzyRule(fuzzyRule9);

  //////////////////////////////////////////////////////////////

  // FuzzyRuleAntecedent *PS = new FuzzyRuleAntecedent();
  // PS->joinWithOR(PanasSekali, HighSekali);
  // FuzzyRuleConsequent *PSO = new FuzzyRuleConsequent();
  // PSO->addOutput(Stop);
  // FuzzyRule *fuzzyRule10 = new FuzzyRule(10, PS, PSO);
  // fuzzy->addFuzzyRule(fuzzyRule10);

  //////////////////////////////////////////////////////////////
}