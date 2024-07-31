// Configurações de rede
String ssid = "Simulator Wifi";
String password = "";
String host = "api.thingspeak.com";
String apiKey = "Y393Z5C2BF1CLU3B"; // Substitua pela sua chave de API do ThingSpeak
const int httpPort = 80;
String uri = "/update?api_key=" + apiKey + "&field1=";

// Pinos
const int ledPin = 9;
const int buttonPin = 2;

// Variáveis
bool ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 1000; // Intervalo de 1 segundo
unsigned long reactionTime;
bool gameActive = false;
unsigned long buttonPressTime = 0;
const long debounceDelay = 50; // Atraso para debounce

void setup() {
  // Inicializa a comunicação serial com o PC e o ESP8266
  Serial.begin(115200);

  // Configura os pinos
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Configura o ESP8266
  setupESP8266();
}

void loop() {
  unsigned long currentMillis = millis();

  if (gameActive) {
    if (ledState == HIGH && digitalRead(buttonPin) == LOW) {
      if ((millis() - buttonPressTime) > debounceDelay) {
        reactionTime = millis() - previousMillis;
        Serial.print("Tempo de Reação: ");
        Serial.print(reactionTime);
        Serial.println(" ms");
        digitalWrite(ledPin, LOW);
        ledState = LOW;
        gameActive = false;
        sendToThingSpeak(reactionTime); // Enviar para ThingSpeak
      }
    }

    if (currentMillis - previousMillis >= interval) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      previousMillis = currentMillis;

      if (ledState == HIGH) {
        Serial.println("LED ON! Pressione o botão!");
      }
    }
  } else {
    if (digitalRead(buttonPin) == LOW) {
      if ((millis() - buttonPressTime) > debounceDelay) {
        Serial.println("Jogo iniciado! Espere o LED acender...");
        delay(1000); // Pequeno atraso para evitar múltiplas leituras do botão
        previousMillis = millis();
        gameActive = true;
      }
    }
  }
}

void setupESP8266() {
  // Inicia a comunicação serial com o ESP8266
  Serial.println("AT");
  delay(1000);
  if (!findResponse("OK")) {
    Serial.println("Erro ao comunicar com o ESP8266");
    return;
  }

  // Conecta ao WiFi
  Serial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
  delay(5000);
  if (!findResponse("OK")) {
    Serial.println("Erro ao conectar ao WiFi");
    return;
  }
}

void sendToThingSpeak(unsigned long reactionTime) {
  // Abre conexão TCP
  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  delay(1000);
  if (!findResponse("OK")) {
    Serial.println("Erro ao conectar ao host");
    return;
  }

  // Cria o pacote HTTP
  String httpPacket = "GET " + uri + String(reactionTime) + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
  int length = httpPacket.length();

  // Envia o comprimento do pacote
  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(100);
  if (!findResponse(">")) {
    Serial.println("Erro ao iniciar envio de dados");
    return;
  }

  // Envia o pedido HTTP
  Serial.print(httpPacket);
  delay(1000);
  if (!findResponse("SEND OK")) {
    Serial.println("Erro ao enviar dados");
  } else {
    Serial.println("Dados enviados para o ThingSpeak");
  }

  // Fecha a conexão
  Serial.println("AT+CIPCLOSE");
  delay(1000);
  if (!findResponse("OK")) {
    Serial.println("Erro ao fechar a conexão");
  } else {
    Serial.println("Conexão fechada");
  }
}

bool findResponse(String response) {
  String data = "";
  while (Serial.available()) {
    char c = Serial.read();
    data += c;
  }
  return data.indexOf(response) != -1;
}