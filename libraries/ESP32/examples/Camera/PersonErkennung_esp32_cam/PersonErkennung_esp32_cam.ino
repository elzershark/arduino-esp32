/*

ENGLISH
!!!If the settings are not correct here, the cam will not work !!!!
Boards URL = https://www.elzershark.com/iobroker/package_esp32_index.json

Board = ESP32 Wrover Module
Uploadspeed = 921600
80Mhz
QIO
Partitions Scheme = Face Recocnitions 2621440 bytes
Debug = Verbose
!!!If the settings are not correct here, the cam will not work !!!!
################

Create web server under Linux (where ioBroker is).
Create folder upload.
create php file.
Give out rights.
---------------
sudo apt-get update
sudo apt-get install apache2
sudo apt-get install php
sudo mkdir /var/www/html/upload
sudo nano /var/www/html/upload.php

Copy and paste it with a right mouse click.
Save with CTRL + O and exit with CTRL + X

<?php
$received = file_get_contents('php://input');
$fileToWrite = "upload/picture.jpg";
file_put_contents($fileToWrite, $received);  
 ?>

Continue with:
sudo chmod 755 /var/www/html/upload.php
sudo chown -R www-data /var/www/html/upload.php
sudo chown -R www-data /var/www/html/upload
---------------

Finished!!

Explanation:

Create php file. Name = upload.php
php file is directly on the web server. The web server has a subfolder called upload.
Don't forget the rights so that something can be saved in the folder

Example of the content of the php file:

<?php
$received = file_get_contents('php://input');
$fileToWrite = "upload/picture.jpg"; // The file picture.jpg should be saved in the folder upload.
file_put_contents($fileToWrite, $received);  
 ?>
 
Camera functions:
1. Supply the camera with power.
2. LED lights up.
3. Cam connects to WiFi and MQTT
4. Cam goes into offline / inactive mode. (Face recognition is activated).
5. LED goes out.
6. If the LED stays on, something is wrong. Check in the Arduino serial monitor.
GPIO2 = connect relay. When a name is recognized, the relay switches and the LED flashes briefly. (Door opener function)
GPIO12 = push button connection with GND (-). A photo is created when a button is pressed. (Bell function)
A photo is created when the name is recognized.
Photos can easily be automatically displayed in Telegram.

MQTT function:
Data points:
Erkannt = A face was recognized. "Unbekannt" is written / updated.
Name = A face with a name was recognized. Name is displayed.
ip = The IP of the cam.
ring = If true, the bell rang (GPIO12 -> GND) and / or a photo is created when "true" is entered.
(In Blockly, set "if object" to "was changed". Then with "if": value of object is = false ... then do something. Do not use "true" in the block.)
wifi = strength of the signal. The smaller the number, then better. "-5 is better than -50" (~ -50 to -40)
info = Which ESP32 Cam is online.

Photo function:
Web server is required.
php is required.
The camera triggers a php script on the web server. The script saves an image in a subfolder.
#########################################
#########################################
#########################################

DEUTSCH!!!

!!!!Stimmen hier Einstellungen nicht, funktioniert die Cam nicht!!!!

Boards URL = https://www.elzershark.com/iobroker/package_esp32_index.json
Board = ESP32 Wrover Module
Uploadspeed = 921600
80Mhz
QIO
Partitions Scheme = Face Recocnitions 2621440 bytes
Debug = Verbose
!!!!Stimmen hier Einstellungen nicht, funktioniert die Cam nicht!!!!


Webserver erstellen unter Linux(wo ioBroker ist).
Ordner upload erstellen.
php datei erstellen.
Rechte vergeben.
---------------
sudo apt-get update
sudo apt-get install apache2
sudo apt-get install php
sudo mkdir /var/www/html/upload
sudo nano /var/www/html/upload.php

Das Folgende kopieren und mit rechtem Mausklick einfügen:

<?php
$received = file_get_contents('php://input');
$fileToWrite = "upload/picture.jpg";
file_put_contents($fileToWrite, $received);  
 ?>

Mit STRG + O u. Enter und mit STRG + X u. Enter verlassen

Weiter mit:
sudo chmod 755 /var/www/html/upload.php
sudo chown -R www-data /var/www/html/upload.php
sudo chown -R www-data /var/www/html/upload
---------------

Fertig!!

Erklärung:

Erstellen der php Datei. Name = upload.php
php Datei ist direkt auf dem Webserver. Der Webserver hat ein Unterordner, Namens upload.
Die Rechte nicht vergessen, damit in dem Ordner was gespeichert werden kann.

Beispiel vom Inhalt der php Datei:

<?php
$received = file_get_contents('php://input');
$fileToWrite = "upload/picture.jpg"; // Es soll die Datei picture.jpg in den Ordner upload gespeichert werden
file_put_contents($fileToWrite, $received);  
 ?>

Kamera Funktionen:
1. Kamera mit Strom versorgen.
2. LED leuchtet.
3. Cam verbindet sich mit W-Lan und MQTT
4. Cam geht in den offline/inaktive Modus. (Gesichtserkennung ist aktiviert).
5. LED geht aus.
6. Wenn die LED anbleibt, stimmt was nicht. Im Serieller Monitor von Arduino überprüfen.
GPIO2 = Relais anschließen. Bei Namens Erkennung schaltet das Relais und die LED Blinkt kurz. (Türöffnerfunktion)
GPIO12 = Taster Anschließen mit GND (-). Bei Tastendruck wird ein Foto erstellt. (Klingelfuntion)
Bei Namenserkennung wird ein Foto erstellt.
Fotos können leicht automatisch im Telegramm angezeigt werden.

MQTT Funktion:
Datenpunkte:
Erkannt = Ein Gesicht wurde erkannt. "Unbekannt" wird geschrieben/aktualisiert.
Name = Ein Gesicht mit Name wurde erkannt. Name wird angezeigt.
ip = Die IP von der Cam.
ring = Bei true wurde geklingelt(GPIO12 -> GND) und/oder ein Foto wird erstellt bei eingabe von "true".
(In Blockly, "falls Objekt" auf "wurde geändert" setzen. Dann mit, "falls": Wert von Objekt ist = false... dann mache etwas. Nicht "true" im Block nehmen.)
wifi = Stärke des Signales. Je kleiner die Zahl um so besser "-5 ist besser als -50" (~-50bis-40)
info = Welcher ESP32 Cam online ist.

Foto Funktion:
Webserver wird benötigt.
php wird benötigt.
Kamera löst ein php Script auf dem Webserver aus. Das Script speichert ein Bild in einem Unterorder.
*/  

#include <ArduinoWebsockets.h>
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "camera_index.h"
#include "Arduino.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include <PubSubClient.h>
unsigned long check_wifi = 30000;
// String chipid; // MAC-ID from SSP32 Cam
String wifist;
String inTopic;
String wifiip;
String msgTopic;
String ipconnect;
String nerkannt;
String foto;
String relaise;
//
//
//             !!!ALIGN DATA / DATEN ANGLEICHEN !!!
const char* ssid       = "WlanName"; //  your network SSID (name) / Wlan- Name
const char* password   = "WlanPassword"; // your network password / Wlan Passwort
const char* mqttServer = "ip.from.mqtt.server"; // your MQTT Server /  MQTT Server IP
const int mqttPort = 1883; // your MQTT Port / MQTT PORT (z.B.1883)
const char* mqttUser = "username"; // your MQTT Username / MQTT Benutzername
const char* mqttPassword = "userpassword"; // your MQTT password / MQTT Passwort
String espName = "ESP_NAME"; // How the ESP should be called e.g. ESP_front_door / Wie der ESP heissen soll z.B. ESP_Haustür
const char *post_url = "http://192.168.68.177/upload.php"; // where the php file is to download the image / Wo die php Datei ist zum runterladen des Bildes
#define relay_pin 2 // GPIO2 connect a relay there. - GPIO2 dort ein Relais anklemmen.
#define relay_pin1 12 // GPIO12 connect with GND for Picture. - GPIO12 mit GND erstellt Foto.
long interval = 5000;           // open lock for ... milliseconds /öffnet das Relais für ... Millisekunden
String noname = "Unbekannt"; // In the data point "Gesicht erkannt", "Unbekannt" is written on recognition / Im Datenpunkt "Gesicht erkannt" wird "Unbekannt" geschrieben bei Erkennung
// The paths can be changed under "void setup()". / Unter "void setup()" können die Pfade geändert werden.
//
//
    
String rssii;
unsigned long door_opened_millis = 0;
unsigned long led_millis = 0;
HTTPClient http; 

WiFiClient espClient;
void callback(char * topic, byte * payload, unsigned int length);
PubSubClient mqttClient(mqttServer, mqttPort, callback, espClient);

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 50
int freq = 5000;
int ledCHannel = 1;
int res = 8;
const int ledPin = 4;
int brightness = 0;
// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

using namespace websockets;
WebsocketsServer socket_server;

camera_fb_t * fb = NULL;
String gesicht;
long current_millis;
long last_detected_millis = 0;
bool websockets_active = false;
const char* get_url = "http://postman-echo.com/get?foo="; // Location to send data

bool face_recognised = false;

void app_facenet_main();
void app_httpserver_init();

typedef struct
{
  uint8_t *image;
  box_array_t *net_boxes;
  dl_matrix3d_t *face_id;
} http_img_process_result;

static inline mtmn_config_t app_mtmn_config()
{
  mtmn_config_t mtmn_config = {0};
  mtmn_config.type = FAST;
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;
  return mtmn_config;
}
mtmn_config_t mtmn_config = app_mtmn_config();

face_id_name_list st_face_list;
static dl_matrix3du_t *aligned_face = NULL;

httpd_handle_t camera_httpd = NULL;

typedef enum
{
  START_STREAM,
  START_DETECT,
  SHOW_FACES,
  START_RECOGNITION,
  START_ENROLL,
  ENROLL_COMPLETE,
  DELETE_ALL,
} en_fsm_state;
en_fsm_state g_state;

typedef struct
{
  char enroll_name[ENROLL_NAME_LEN];
} httpd_resp_value;

httpd_resp_value st_name;

void setup() {
  
  //  MQTT Pfade:
      nerkannt = espName;
      nerkannt += "/Erkannt";
      msgTopic = espName;
      msgTopic += "/Name";
      wifist = espName;
      wifist += "/WiFi";
      wifiip = espName;
      wifiip += "/IP";
      inTopic = espName;
      inTopic += "/Ring";
      nerkannt = espName;
      nerkannt += "/Erkannt";
      foto = espName;
      foto += "/Foto";
      relaise = espName;
      relaise += "/Relay";      
   //
       
  ledcSetup(ledCHannel, freq, res);
  ledcAttachPin(ledPin, ledCHannel);
  brightness = 2;
  ledcWrite(ledCHannel, brightness); 

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  digitalWrite(relay_pin, LOW);
  digitalWrite(relay_pin1, HIGH);
  pinMode(relay_pin, OUTPUT);
  pinMode(relay_pin1, INPUT_PULLUP);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  while (WiFi.status() != WL_CONNECTED) {
    brightness = 2;
  ledcWrite(ledCHannel, brightness); 
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    delay(3000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println();
  app_httpserver_init();
  app_facenet_main();
  socket_server.listen(82);
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
    long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
  rssii = rssi;
 ipconnect = "http://";
 ipconnect += WiFi.localIP().toString().c_str();
 ipconnect += ":82";
      mqttClient.connect(espName.c_str(), mqttUser, mqttPassword); //MQTT Server ESP32 Name
      mqttClient.publish(nerkannt.c_str(), ""); //MQTT Server ESP32 Noname
      mqttClient.publish(msgTopic.c_str(), ""); //MQTT Server ESP32 Face Name
      mqttClient.publish(wifist.c_str(), rssii.c_str()); //MQTT Server ESP32 wifi strength
      mqttClient.publish(wifiip.c_str(), WiFi.localIP().toString().c_str()); //MQTT Server ESP32 IP Number
      mqttClient.publish(inTopic.c_str(), "false"); 
      mqttClient.subscribe(inTopic.c_str());
      mqttClient.publish(foto.c_str(), "false"); 
      mqttClient.subscribe(foto.c_str());   
      mqttClient.publish(relaise.c_str(), "false"); 
      mqttClient.subscribe(relaise.c_str());    
      reconnect(); //inaktive Modus
      
      brightness = 0;
      ledcWrite(ledCHannel, brightness);
}

//auto aktive setzen bei Seite öffnen
static esp_err_t index_handler(httpd_req_t *req) {
  websockets_active = true;
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  int gesicht = 0;
  return httpd_resp_send(req, (const char *)index_ov2640_html_gz, index_ov2640_html_gz_len);
}

httpd_uri_t index_uri = {
  .uri       = "/",
  .method    = HTTP_GET,
  .handler   = index_handler,
  .user_ctx  = NULL
};

void app_httpserver_init ()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  if (httpd_start(&camera_httpd, &config) == ESP_OK)
    Serial.println("httpd_start");
  {
    httpd_register_uri_handler(camera_httpd, &index_uri);
  }
}

//CLIENT
esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
  switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
      ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
      printf("%.*s", evt->data_len, (char*)evt->data);
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      if (!esp_http_client_is_chunked_response(evt->client)) {
        printf("%.*s", evt->data_len, (char*)evt->data);
      }

      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}
static esp_err_t send_get_request(const char* name)
{
  esp_err_t res = ESP_OK;
  Serial.println("sending..");
  esp_http_client_config_t client_config = {0};

  char full_url[50];// space for URL and name
  strcpy(full_url, get_url);
  strcat(full_url, name);

  client_config.url = full_url;
  client_config.event_handler = _http_event_handle;

  esp_http_client_handle_t http_client = esp_http_client_init(&client_config);
  esp_err_t err = esp_http_client_perform(http_client);

  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Status = %d, content_length = %d",
             esp_http_client_get_status_code(http_client),
             esp_http_client_get_content_length(http_client));
  }
  esp_http_client_cleanup(http_client);
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  return encodedString;

}

unsigned char h2int(char c)
{
  if (c >= '0' && c <= '9') {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

void app_facenet_main()
{
  face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
  read_face_id_from_flash_with_name(&st_face_list);
}

static inline int do_enrollment(face_id_name_list *face_list, dl_matrix3d_t *new_id)
{
  ESP_LOGD(TAG, "START ENROLLING");
  int left_sample_face = enroll_face_id_to_flash_with_name(face_list, new_id, st_name.enroll_name);
  ESP_LOGD(TAG, "Face ID %s Enrollment: Sample %d",
           st_name.enroll_name,
           ENROLL_CONFIRM_TIMES - left_sample_face);
  return left_sample_face;
}

static esp_err_t send_face_list(WebsocketsClient &client)
{
  client.send("delete_faces"); // tell browser to delete all faces
  face_id_node *head = st_face_list.head;
  char add_face[64];
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
  {
    sprintf(add_face, "listface:%s", head->id_name);
    client.send(add_face); //send face to browser
    head = head->next;
  }
}

static esp_err_t delete_all_faces(WebsocketsClient &client)
{
  delete_face_all_in_flash_with_name(&st_face_list);
  client.send("delete_faces");
}

void handle_message(WebsocketsClient &client, WebsocketsMessage msg)
{
  if (msg.data() == "stream") {
    g_state = START_STREAM;
    client.send("STREAMING");
  }
  if (msg.data() == "detect") {
    g_state = START_DETECT;
    client.send("DETECTING");
  }
  if (msg.data().substring(0, 8) == "capture:") {
    g_state = START_ENROLL;
    char person[FACE_ID_SAVE_NUMBER * ENROLL_NAME_LEN] = {0,};
    msg.data().substring(8).toCharArray(person, sizeof(person));
    memcpy(st_name.enroll_name, person, strlen(person) + 1);
    client.send("CAPTURING");
  }
  if (msg.data() == "recognise") {
    g_state = START_RECOGNITION;
    client.send("RECOGNISING");
  }
  if (msg.data() == "disconnect") {
    websockets_active = false;
  }
  if (msg.data().substring(0, 7) == "remove:") {
    char person[ENROLL_NAME_LEN * FACE_ID_SAVE_NUMBER];
    msg.data().substring(7).toCharArray(person, sizeof(person));
    delete_face_id_in_flash_with_name(&st_face_list, person);
    send_face_list(client); // reset faces in the browser
  }
  if (msg.data() == "delete_all") {
    delete_all_faces(client);
  }
}

//Relay open
void open_door() {

    if (digitalRead(relay_pin) == LOW) {
      mqttClient.publish(relaise.c_str(), "true");
    digitalWrite(relay_pin, HIGH); //open (energise) relay so door unlocks
    door_opened_millis = millis(); // time relay closed and door opened
       
  }    
            if (brightness == 0) {
            brightness = 2;
            ledcWrite(ledCHannel, brightness);
            led_millis = millis(); // led
            long rssi = WiFi.RSSI();
            rssii = rssi;
      
// MAC ID from ESP32####
//chipid = "ESP32_";
//chipid += String((uint32_t)ESP.getEfuseMac(), HEX);
//Serial.printf("Chip id: %s\n", chipid.c_str());
//    espName = chipid;
     
      take_send_photo();        
            if (!mqttClient.connected()) {
       mqttClient.connect(espName.c_str(), mqttUser, mqttPassword); //MQTT Server ESP32 Name
  } 
      mqttClient.loop();
      mqttClient.publish(msgTopic.c_str(), gesicht.c_str()); //MQTT Server ESP32 Face Name
      mqttClient.publish(wifist.c_str(), rssii.c_str()); //MQTT Server ESP32 wifi strength
      mqttClient.publish(wifiip.c_str(), WiFi.localIP().toString().c_str()); //MQTT Server ESP32 IP Number
      
       }
}

//Aktive Modus
void interface_active(WebsocketsClient &client) {
     mqttpubsub();
    client.onMessage(handle_message);
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);
  http_img_process_result out_res = {0};
  out_res.image = image_matrix->item;

  send_face_list(client);
  client.send("STREAMING");

  while (client.available() && websockets_active) {
    client.poll();
      if ((WiFi.status() != WL_CONNECTED) && (millis() > check_wifi)) {
        brightness = 2;
  ledcWrite(ledCHannel, brightness); 
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    check_wifi = millis() + 30000;
    reconnect(); //inaktive Modus
  }
        if (!mqttClient.connected()) {
       mqttClient.connect(espName.c_str(), mqttUser, mqttPassword); //MQTT Server ESP32 Name
     mqttpubsub();
  } 
      mqttClient.loop();
      
 if (digitalRead(relay_pin) == HIGH) {
    if (millis() - interval > door_opened_millis) { // current time - face recognised time > 5 secs
      digitalWrite(relay_pin, LOW); //close relay
            mqttClient.publish(relaise.c_str(), "false"); 
      mqttClient.subscribe(relaise.c_str());  
    }
 }
  if (brightness == 2) {

    if (millis() - 500 > led_millis) { // current time - face recognised time > 5 secs
     brightness = 0; 
      ledcWrite(ledCHannel, brightness);
    }
  }
  
    ring(); //Klingel
    
    fb = esp_camera_fb_get();

    if (g_state == START_DETECT || g_state == START_ENROLL || g_state == START_RECOGNITION)
    {

      out_res.net_boxes = NULL;
      out_res.face_id = NULL;

      fmt2rgb888(fb->buf, fb->len, fb->format, out_res.image);

      out_res.net_boxes = face_detect(image_matrix, &mtmn_config);

      if (out_res.net_boxes)
      {
        if (align_face(out_res.net_boxes, image_matrix, aligned_face) == ESP_OK)
        {
          
          out_res.face_id = get_face_id(aligned_face);
          last_detected_millis = millis();
          if (g_state == START_DETECT) {
            client.send("GESICHT ERKANNT");
          }

          if (g_state == START_ENROLL)
          {
            int left_sample_face = do_enrollment(&st_face_list, out_res.face_id);
            char enrolling_message[64];
            sprintf(enrolling_message, "STICHPROBENNUMMER %d FOR %s", ENROLL_CONFIRM_TIMES - left_sample_face, st_name.enroll_name);
            client.send(enrolling_message);
            if (left_sample_face == 0)
            {
              ESP_LOGI(TAG, "Enrolled Face ID: %s", st_face_list.tail->id_name);
              g_state = START_STREAM;
              char captured_message[64];
              sprintf(captured_message, "GESICHT ERFASST VON %s", st_face_list.tail->id_name);
              client.send(captured_message);
              send_face_list(client);
                if (brightness == 0) {
            brightness = 2;
            ledcWrite(ledCHannel, brightness);
            led_millis = millis(); // led
  }
            }
          }
          if (g_state == START_RECOGNITION  && (st_face_list.count > 0))
          {
            face_id_node *f = recognize_face_with_name(&st_face_list, out_res.face_id);
            if (f)
            {
              char recognised_message[64];
              gesicht = String(f->id_name);
              sprintf(recognised_message, "TÜR GEÖFFNET FÜR %s", f->id_name);
              open_door();
              client.send(recognised_message);
            }
            else
            {
              mqttClient.publish(nerkannt.c_str(), noname.c_str()); //MQTT Server ESP32 Noname
              client.send("GESICHT NICHT ERKANNT");
            }
          }
          dl_matrix3d_free(out_res.face_id);
        }
      }
      else
      {
        if (g_state != START_DETECT) {
          client.send("KEIN GESICHT ERKANNT");
        }
      }

      if (g_state == START_DETECT && millis() - last_detected_millis > 500) { // Detecting but no face detected
        client.send("ERKENNUNG");
      }
    }
    client.sendBinary((const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    fb = NULL;
  }
}

//Inaktive Modus
void interface_inactive(WebsocketsClient &client) {
     mqttpubsub();
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);
  http_img_process_result out_res = {0};
  out_res.image = image_matrix->item;
  while (!websockets_active) {
        
      if ((WiFi.status() != WL_CONNECTED) && (millis() > check_wifi)) {
    Serial.println("Reconnecting to WiFi...");
                brightness = 2;
            ledcWrite(ledCHannel, brightness);
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    check_wifi = millis() + 30000;
    reconnect(); //inaktive Modus
  }
        if (!mqttClient.connected()) {
       mqttClient.connect(espName.c_str(), mqttUser, mqttPassword); //MQTT Server ESP32 Name
       mqttpubsub();
  } 
      mqttClient.loop();
        if (brightness == 2) {

    if (millis() - 500 > led_millis) { // current time - face recognised time > 5 secs
     brightness = 0; 
      ledcWrite(ledCHannel, brightness);
    }
  }
 if (digitalRead(relay_pin) == HIGH) {
    if (millis() - interval > door_opened_millis) { // current time - face recognised time > 5 secs
      digitalWrite(relay_pin, LOW); //close relay
            mqttClient.publish(relaise.c_str(), "false"); 
      mqttClient.subscribe(relaise.c_str());  
    }
 }
     ring(); //Klingel

    fb = esp_camera_fb_get();

    out_res.net_boxes = NULL;
    out_res.face_id = NULL;

    fmt2rgb888(fb->buf, fb->len, fb->format, out_res.image);

    out_res.net_boxes = face_detect(image_matrix, &mtmn_config);

    if (out_res.net_boxes)
    {
      if (align_face(out_res.net_boxes, image_matrix, aligned_face) == ESP_OK)
      {

        out_res.face_id = get_face_id(aligned_face);
        if (st_face_list.count) // recorded faces exist
        {
          face_id_node *f = recognize_face_with_name(&st_face_list, out_res.face_id);
          if (f) // face recognised
          {
            gesicht = String(f->id_name);
            open_door();
          //  send_get_request(f->id_name);
          }
                  else
        {
              mqttClient.publish(nerkannt.c_str(), noname.c_str()); //MQTT Server ESP32 Noname
          }
        }
        dl_matrix3d_free(out_res.face_id);
        free(out_res.net_boxes);
      }
    }
    esp_camera_fb_return(fb);
    fb = NULL;
  }
}

void loop() {

  auto client = socket_server.accept();
  Serial.println("loopin");

  if (client.available() && websockets_active) {
    interface_active(client);
    Serial.println("end active");
  }

  Serial.println("inactive selected");
  interface_inactive(client);
 }

//MQTT subscribe
void callback(char * topic, byte * payload, unsigned int length) {
 
  // let's transform a subject (topic) and value (payload) to a line
  payload[length] = '\0';
  String strTopic = String(topic);
  String strPayload = String((char * ) payload);
    if (strTopic == inTopic) {
    if (strPayload == "on" || strPayload == "1" || strPayload == "true"){
      take_send_photo();
      mqttClient.publish(inTopic.c_str(), "false"); 
      mqttClient.subscribe(inTopic.c_str());
         }
      }
        else
       if (strTopic == foto) {
    if (strPayload == "on" || strPayload == "1" || strPayload == "true"){
      take_send_photo();
      mqttClient.publish(foto.c_str(), "false"); 
      mqttClient.subscribe(foto.c_str());
         }
      }
      else
     if (strTopic == relaise) {
    if (strPayload == "on" || strPayload == "1" || strPayload == "true"){
          digitalWrite(relay_pin, HIGH); //open (energise) relay so door unlocks
    door_opened_millis = millis(); // time relay closed and door opened
         }
      }
}

//Inaktive Modus
void reconnect() {
  
            websockets_active = false;
            http.begin(ipconnect.c_str());
   int httpCode = http.POST("Message from ESP8266");   //Send the request
   String payload = http.getString();                  //Get the response payload
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
 //  http.end();  //Close connection
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
      Serial.println("HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      Serial.println("HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      Serial.println("HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      Serial.println();
      Serial.printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
      break;
    case HTTP_EVENT_ON_DATA:
      Serial.println();
      Serial.printf("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      if (!esp_http_client_is_chunked_response(evt->client)) {
        // Write out data
        // printf("%.*s", evt->data_len, (char*)evt->data);
      }
      break;
    case HTTP_EVENT_ON_FINISH:
      Serial.println("");
      Serial.println("HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      Serial.println("HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}

//Foto senden
static esp_err_t take_send_photo()
{
  Serial.println("Taking picture...");
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return ESP_FAIL;
  }

  esp_http_client_handle_t http_client;
  
  esp_http_client_config_t config_client = {0};
  config_client.url = post_url;
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;

  http_client = esp_http_client_init(&config_client);

  esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);

  esp_http_client_set_header(http_client, "Content-Type", "image/jpg");

  esp_err_t err = esp_http_client_perform(http_client);
  if (err == ESP_OK) {
    Serial.print("esp_http_client_get_status_code: ");
    Serial.println(esp_http_client_get_status_code(http_client));
  }

  esp_http_client_cleanup(http_client);

  esp_camera_fb_return(fb);
}

//Klingel
void ring() {
    if (digitalRead(relay_pin1) == LOW) {
 if (!mqttClient.connected()) {
       mqttClient.connect(espName.c_str(), mqttUser, mqttPassword); //MQTT Server ESP32 Name
  } 
      mqttClient.loop();
      mqttClient.publish(inTopic.c_str(), "true"); 
      mqttClient.subscribe(inTopic.c_str());
      delay(2000); 
      }
}
void mqttpubsub() {
      mqttClient.subscribe(inTopic.c_str());
      mqttClient.subscribe(foto.c_str()); 
      mqttClient.subscribe(relaise.c_str());
}
