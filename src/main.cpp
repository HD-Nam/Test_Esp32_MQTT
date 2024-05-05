#include <WiFi.h>
#include <PubSubClient.h>

// Định nghĩa các chân cho các đèn LED
#define LED1 17
#define LED2 5

// Khởi tạo trạng thái ban đầu của đèn LED
String ledStatus1 = "ON";
String ledStatus2 = "ON";

// Cài đặt tên và mật khẩu của mạng WiFi
const char* ssid = "Tang 2";
const char* password = "tanghai201206";

// Cài đặt thông tin của MQTT broker
#define MQTT_SERVER "b1825bc0a3c4482eb745141382c19010.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USER "hoducnam"
#define MQTT_PASSWORD "22082002"

// Định nghĩa các chủ đề MQTT cho từng đèn LED
#define MQTT_LED1_TOPIC "MQTT_ESP32/LED1"
#define MQTT_LED2_TOPIC "MQTT_ESP32/LED2"

// Biến để lưu thời gian trước đó và khoảng thời gian giữa các lần gửi tin nhắn
unsigned long previousMillis = 0; 
const long interval = 5000;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Hàm cài đặt kết nối WiFi
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  // Chờ kết nối WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Hàm kết nối đến MQTT broker
void connect_to_broker() {
  // Lặp lại cho đến khi kết nối thành công
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    // Kết nối đến broker với ID ngẫu nhiên
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Đăng ký các chủ đề để nhận tin nhắn
      client.subscribe(MQTT_LED1_TOPIC);
      client.subscribe(MQTT_LED2_TOPIC);
    } else {
      // Hiển thị thông báo lỗi nếu kết nối không thành công
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

// Hàm callback được gọi khi nhận được tin nhắn từ MQTT broker
void callback(char* topic, byte *payload, unsigned int length) {
  char status[20];
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  // Sao chép nội dung của tin nhắn vào một chuỗi ký tự
  for(int i = 0; i<length; i++)
  {
    status[i] = payload[i];
  }
  Serial.println(status);
  // Xử lý tin nhắn theo từng chủ đề
  // Chủ đề 1
  if(String(topic) == MQTT_LED1_TOPIC)
  {
    if(String(status) == "OFF")
    {
      ledStatus1 = "OFF";
      digitalWrite(LED1, LOW);
      Serial.println("LED1 OFF");
    }
    else if(String(status) == "ON")
    {
      ledStatus1 = "ON";
      digitalWrite(LED1, HIGH);
      Serial.println("LED1 ON");
    }
  }
  // Chủ đề 2
  if(String(topic) == MQTT_LED2_TOPIC)
  {
    if(String(status) == "OFF")
    {
      ledStatus2 = "OFF";
      digitalWrite(LED2, LOW);
      Serial.println("LED2 OFF");
    }
    else if(String(status) == "ON")
    {
      ledStatus2 = "ON";
      digitalWrite(LED2, HIGH);
      Serial.println("LED2 ON");
    }
  } 
}

// Hàm cài đặt ban đầu
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);

  setup_wifi();     // Cài đặt kết nối WiFi
  client.setServer(MQTT_SERVER, MQTT_PORT );     // Cài đặt thông tin của MQTT broker
  client.setCallback(callback);     // Thiết lập hàm callback để xử lý tin nhắn từ broker
  
  connect_to_broker();      // Kết nối đến MQTT broker
  Serial.println("Start transfer");

  // Cài đặt chân IO cho các đèn LED
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}
 
// Hàm lặp chính
void loop() {
  // Duy trì kết nối với MQTT broker
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }

  // Kiểm tra thời gian và gửi tin nhắn để điều khiển đèn LED
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Chuyển đổi trạng thái của LED1
    if (ledStatus1 == "ON") {
      client.publish(MQTT_LED1_TOPIC, "OFF");
      ledStatus1 = "OFF";
    } else {
      client.publish(MQTT_LED1_TOPIC, "ON");
      ledStatus1 = "ON";
    }
    // Chuyển đổi trạng thái của LED2
    if (ledStatus2 == "ON") {
      client.publish(MQTT_LED2_TOPIC, "OFF");
      ledStatus2 = "OFF";
    } else {
      client.publish(MQTT_LED2_TOPIC, "ON");
      ledStatus2 = "ON";
    }
  }
}
