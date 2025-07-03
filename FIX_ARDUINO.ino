#define NUM_SENSORS 3

const int TRIG_PINS[NUM_SENSORS] = {3, 5, 7};
const int ECHO_PINS[NUM_SENSORS] = {4, 6, 8};
const int LED_PIN = 9;

float totalWeightKg = 0.0;
unsigned long lastDetectionTime = 0;
unsigned long lastSend = 0;

const unsigned long detectionCooldown = 500; // 1 detik
const unsigned long sendInterval = 1000;
const int DISTANCE_THRESHOLD = 22;

bool isObjectStillPresent = false;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
    digitalWrite(TRIG_PINS[i], LOW);
  }
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  unsigned long currentTime = millis();
  bool detected = false;

  // Cek semua sensor
  for (int i = 0; i < NUM_SENSORS; i++) {
    int distances[3];

    for (int j = 0; j < 3; j++) {
      long duration = readUltrasonic(TRIG_PINS[i], ECHO_PINS[i]);
      distances[j] = duration * 0.034 / 2;
      delay(5); 
    }

    int medianDist = getMedian(distances);
    if (medianDist > 0 && medianDist < DISTANCE_THRESHOLD) {
      detected = true;
      break; 
    }
  }

  // Penambahan berat hanya jika:
  // - Ada deteksi
  // - Benda baru (belum dideteksi sebelumnya)
  // - Cooldown terpenuhi
  if (detected && !isObjectStillPresent && (currentTime - lastDetectionTime >= detectionCooldown)) {
    float estimatedWeight = 0.1;
    totalWeightKg += estimatedWeight;
    lastDetectionTime = currentTime;
    isObjectStillPresent = true;

    digitalWrite(LED_PIN, HIGH);
    Serial.println("Deteksi: 1 benda terdeteksi. Estimasi berat: 0.1 kg");
    digitalWrite(LED_PIN, LOW);
  }

  // Kirim total berat setiap interval
  if (currentTime - lastSend >= sendInterval) {
    Serial.print("Total Berat: ");
    Serial.print(totalWeightKg, 3);
    Serial.println(" kg");
    lastSend = currentTime;
    totalWeightKg = 0.0;
  }

  // Cek perintah serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "RESET") {
      totalWeightKg = 0.0;
      Serial.println("✅ Berat telah direset oleh ESP32.");
    }
  }
}

// Fungsi baca sensor
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH, 30000); 
}

// Fungsi median dari 3 nilai
int getMedian(int arr[3]) {
  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (arr[j] < arr[i]) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
  return arr[1]; // nilai median
}

