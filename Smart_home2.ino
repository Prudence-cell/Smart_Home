#include <WiFi.h>
#include <WebServer.h>      // ← AJOUT CRUCIAL
#include <ESP32Servo.h>

/* ===== BROCHES ===== */
#define WATER_PIN      33
#define SERVO_LINGE    14

/* ===== WIFI ===== */
//const char* ssid = "M022_83C4";
//const char* password = "20252025@A!aze";
const char* ssid = "Infinity";
const char* password = "alpha@beta2021";
unsigned long dernierAffichage = 0; 
const long intervalle = 3000;

/* ===== OBJETS ===== */
Servo linge;
WebServer server(80);  // ← AJOUT : Serveur Web sur le port 80



void setup() {
  /* Capteurs */
  pinMode(WATER_PIN, INPUT);

  /* Servos */
  linge.attach(SERVO_LINGE);
  linge.write(0);

  /* Communication série */

  Serial.begin(115200);
  delay(1500);
  Serial.println("\n--- INITIALISATION DU SYSTEME ---");
  Serial.println("=== Systeme Domotique ESP32 Demarre ===");

  /* WIFI */
  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ WiFi connecté");
  Serial.print("📡 IP ESP32 : ");
  Serial.println(WiFi.localIP());

  Serial.println("🌐 Serveur Web démarré !");
}

void loop() {
  server.handleClient();  // ← ÉCOUTE DES REQUÊTES (CRUCIAL)

  /* ===== LECTURE DES CAPTEURS ===== */
  int presence = digitalRead(PIR_PIN);
  int lumiere = analogRead(LDR_PIN);
  int eau = digitalRead(WATER_PIN);
  bool nuit = (lumiere > seuilNuit);

  /* ===== DEBUG SERIE ===== */
  Serial.print("Presence: ");
  Serial.print(presence);
  Serial.print(" | LDR: ");
  Serial.print(lumiere);
  Serial.print(" | Eau: ");
  Serial.println(eau);
  Serial.print(" | Nuit: ");
  Serial.println(nuit);

  /* ===== PROTECTION DU LINGE ===== */
  if (eau == HIGH) {
    linge.write(90);   // Pluie → linge à l'abri
  } else {
    linge.write(0);    // Pas de pluie → linge dehors
  }

  delay(1000);  // Petit délai pour stabilité
}                                                                                                                                                                                                                                            
