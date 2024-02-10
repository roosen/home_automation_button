#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <PubSubClient.h>


#define CONFIG_BTN_TIMEOUT_MS 20000
#define TASMOTA_DEVICE ""


const char *ssid        = "";
const char *password    = "";
const char *mqtt_server = "";
const char *mqtt_user   = "";
const char *mqtt_pass   = "";

const char *tasmota_stat = "stat/" TASMOTA_DEVICE "/POWER";
const char *tasmota_cmnd = "cmnd/" TASMOTA_DEVICE "/POWER";


static WiFiClient espClient;
static PubSubClient client(espClient);
static unsigned long last_btn;


static void setupWifi(void)
{
	StickCP2.Display.fillScreen(BLACK);
	StickCP2.Display.setCursor(10, 10);
	StickCP2.Display.print("WIFI..");
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		StickCP2.Display.print(".");
	}
	StickCP2.Display.println("\nSuccess");
}

static void connectMQTT(void)
{
	StickCP2.Display.fillScreen(BLACK);
	StickCP2.Display.setCursor(10, 10);
	StickCP2.Display.println("MQTT..");

	while (!client.connected()) {
		StickCP2.Display.setCursor(10, 20);
		if (client.connect(TASMOTA_DEVICE, mqtt_user, mqtt_pass)) {
			StickCP2.Display.println("\nSuccess");
			client.subscribe(tasmota_stat);
			client.publish(tasmota_cmnd, "");
		} else {
			StickCP2.Display.print("failed, rc=");
			StickCP2.Display.println(client.state());
			StickCP2.Display.println("retry in 5");
			delay(5000);
		}
	}
}

static void callback(char* topic, byte* payload, unsigned int length)
{
	StickCP2.Display.setCursor(10, 30);
	if (payload[1] == 'n' || payload[1] == 'N')
		StickCP2.Display.fillScreen(GREEN);
	else
		StickCP2.Display.fillScreen(RED);
}

void setup(void)
{
	StickCP2.begin();
	StickCP2.Display.setRotation(0);
	setupWifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	last_btn = millis();
}

void loop(void)
{
	unsigned long now;

	client.loop();
	if (!client.connected())
		connectMQTT();

	now = millis();

	StickCP2.update();
	if (StickCP2.BtnA.wasPressed()) {
		client.publish(tasmota_cmnd, "toggle");
		last_btn = now;
	}

	if (now - last_btn > CONFIG_BTN_TIMEOUT_MS)
		StickCP2.Power.powerOff();
}

