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

/* ===== ÉTATS (pour l'application) ===== */
bool laundryState = false;



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
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n✅ Linge connecté. IP : " + WiFi.localIP().toString());

  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"rain\":" + String(digitalRead(WATER_PIN) == HIGH ? "true" : "false") + ",";
    json += "\"laundry\":" + String(laundryState ? "true" : "false");
    json += "}";
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
  });

  server.on("/laundry/protect", HTTP_POST, []() { 
    laundryState = true; 
    linge.write(90); 
    server.sendHeader("Access-Control-Allow-Origin", "*"); 
    server.send(200); 
  });

  server.on("/laundry/expose", HTTP_POST, []() {
    if (digitalRead(WATER_PIN) == HIGH) { // SÉCURITÉ PLUIE
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(403, "text/plain", "Safety Block: Raining!"); 
    } else {
      laundryState = false;
      linge.write(0);
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200);
    }
  });

  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204);
    } else { server.send(404); }
  });

  server.begin();
  Serial.println("🌐 Serveur Web démarré !");
}

void loop() {
  server.handleClient();  // ← ÉCOUTE DES REQUÊTES (CRUCIAL)

  /* ===== LECTURE DES CAPTEURS ===== */
  int eau = digitalRead(WATER_PIN);

  /* ===== DEBUG SERIE ===== */
  Serial.print(" | Eau: ");
  Serial.println(eau);

  /* ===== PROTECTION DU LINGE ===== */
  /*
  if (eau == HIGH) {
    linge.write(90);   // Pluie → linge à l'abri
  } else {
    linge.write(0);    // Pas de pluie → linge dehors
  }

  delay(1000);  // Petit délai pour stabilité
  */



  // Sécurité automatique prioritaire
  if (eau == HIGH && !laundryState) {
    laundryState = true;
    linge.write(90);
    Serial.println("🌧️ Pluie ! Linge mis à l'abri.");
  }
  
  delay(1000);
}                                                                                                                                                                                                                                            