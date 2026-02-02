#include <WiFi.h>
#include <WebServer.h>      // ← AJOUT CRUCIAL
#include <ESP32Servo.h>

/* ===== BROCHES ===== */
#define PIR_PIN        27
#define LDR_PIN        34
#define LED_PIN        26
#define SERVO_PORTE    25

/* ===== WIFI ===== */
const char* ssid = "State Man 😎";
const char* password = "prudence";
//const char* ssid = "Infinity";
//const char* password = "alpha@beta2021";
unsigned long dernierAffichage = 0; 
const long intervalle = 3000;

/* ===== OBJETS ===== */
Servo porte;
WebServer server(80);  // ← AJOUT : Serveur Web sur le port 80

/* ===== PARAMÈTRES ===== */
int seuilNuit = 1200;

/* ===== ÉTATS (pour l'application) ===== */
bool lightState = false;
bool doorState = false;

void setup() {
  /* Capteurs */
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  /* Servos */
  porte.attach(SERVO_PORTE);
  porte.write(0);

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

  /* ===== ROUTES API (NOUVEAU) ===== */
  
  // Route /status : renvoie l'état complet
  server.on("/status", HTTP_GET, []() {
    int presence = digitalRead(PIR_PIN);
    int lumiere = analogRead(LDR_PIN);
    int eau = digitalRead(WATER_PIN);
    
    String json = "{";
    json += "\"presence\":" + String(presence == HIGH ? "true" : "false") + ",";
    json += "\"rain\":" + String(eau == HIGH ? "true" : "false") + ",";
    json += "\"light\":" + String(lightState ? "true" : "false") + ",";
    json += "\"door\":" + String(doorState ? "true" : "false") + ",";
    json += "\"humidity\":" + String(lumiere) + ",";  // LDR comme "niveau"
    json += "\"pressure\":1013";
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
  });

  // Route /light/on : allumer la LED
  server.on("/light/on", HTTP_POST, []() {
    lightState = true;
    digitalWrite(LED_PIN, HIGH);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Light ON");
  });

  // Route /light/off : éteindre la LED
  server.on("/light/off", HTTP_POST, []() {
    lightState = false;
    digitalWrite(LED_PIN, LOW);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Light OFF");
  });

  // Gestion CORS (obligatoire pour le navigateur)
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204);
    } else {
      server.send(404, "text/plain", "Not Found");
    }
  });

  server.begin();  // ← DÉMARRAGE DU SERVEUR
  Serial.println("🌐 Serveur Web démarré !");
}

void loop() {
  server.handleClient();  // ← ÉCOUTE DES REQUÊTES (CRUCIAL)

  /* ===== LECTURE DES CAPTEURS ===== */
  int presence = digitalRead(PIR_PIN);
  int lumiere = analogRead(LDR_PIN);
  bool nuit = (lumiere < seuilNuit);

  /* ===== DEBUG SERIE ===== */
  Serial.print("Presence: ");
  Serial.print(presence);
  Serial.print(" | LDR: ");
  Serial.print(lumiere);
  Serial.print(" | Nuit: ");
  Serial.println(nuit);

  /* ===== PORTE AUTOMATIQUE ===== */
  if (presence == HIGH) {
    porte.write(90);
    doorState = true;
  } else {
    porte.write(0);
    doorState = false;
  }

  /* ===== ECLAIRAGE AUTO (si pas de contrôle manuel) ===== */
  if (presence == HIGH && nuit && !lightState) {
    digitalWrite(LED_PIN, HIGH);
  } else if (!lightState) {
    digitalWrite(LED_PIN, LOW);
  }

  delay(1000);  // Petit délai pour stabilité
}
