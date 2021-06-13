/* 
 Copyright © Dougie Lawson, 2021, All rights reserved 
*/

#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <mosquitto.h>
#include <libconfig.h>

#define CONFIG_FILE "/home/pi/.glow.cfg"

const char* username;
const char* password;
const char* device;

/*
 * Fields gathered from the ZigBee Smart Energy Standard document
 * 0702: Metering
 *   - 00: Reading Information Set
 *     - 00: CurrentSummationDelivered: meter reading
 *     - 01: CurrentSummationReceived
 *     - 02: CurrentMaxDemandDelivered
 *     - 07: ReadingSnapshotTime (UTC time)
 *     - 14: Supply Status (enum): 0x2 is on
 *   - 02: Meter Status
 *     - 00: Status (bit map): 10 means power quality event
 *   - 03: Formatting
 *     - 00: UnitofMeasure (enum): 00 means kWh, 01 means m3
 *     - 01: Multiplier
 *     - 02: Divisor
 *     - 03: SummationFormatting (bit map):
 *          2B means 3 digits after the decimal point, 2 digits before the decimal poin
 *          FB means 3 digits after the decimal point, 16 digits before the decimal point,
 *          no leading zeros
 *     - 04: DemandFormatting
 *     - 06: MeteringDeviceType: 00 means Electric Metering, 80 means Mirrored Gas Metering
 *     - 07: SiteID: MPAN encoded in UTF-8
 *     - 08: MeterSerialNumber (string)
 *     - 12: AlternativeUnitofMeasure (enum)
 *   - 04: Historical Consumption
 *     - 00: InstantaneousDemand (signed): current consumption
 *     - 01: CurrentDayConsumptionDelivered
 *     - 30: CurrentWeekConsumptionDelivered
 *     - 40: CurrentMonthConsumptionDelivered
 *   - 0C: Alternative Historical Consumption
 *     - 01: CurrentDayConsumptionDelivered
 *     - 30: CurrentWeekConsumptionDelivered
 *     - 40: CurrentMonthConsumptionDelivered
 *  0705: Prepayment
 *   - 00: Prepayment Information Set
 *     - 00: PaymentControlConfiguration (bit map)
 *     - 01: CreditRemaining (signed)
 *  0708: Device Management
 *   - 01: Supplier Control Attribute Set
 *     - 01: ProviderName (string)
 */

static int run = 1;

void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc) printf("Connect callback, rc=%d\n", rc);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	printf("Topic: %s\n Payload:", message->topic);	
	printf("%.*s\n", message->payloadlen, (char*) message->payload);
}

void readConfig()
{
	static const char *config_file = CONFIG_FILE;
	config_t cfg;
	config_setting_t *root, *glow;
	config_setting_t *g_user, *g_pwd, *g_device;

	config_init(&cfg);
	if(! config_read_file(&cfg, CONFIG_FILE))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
	}

	root = config_root_setting(&cfg);

	glow = config_setting_get_member(root, "Glow");

	g_user = config_setting_get_member(glow, "username");
	username = config_setting_get_string(g_user);
	g_pwd = config_setting_get_member(glow, "password");
	password = config_setting_get_string(g_pwd);
	g_device = config_setting_get_member(glow, "device");
	device = config_setting_get_string(g_device);

}

int main(int argc, char* argv[]) 
{
	uint8_t reconnect = true;
	struct mosquitto *mosq;
	int rc = 0;
	char topic[23];
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, 0);

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	readConfig();

	if(mosq)
	{
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

		mosquitto_username_pw_set(mosq, username, password);
		mosquitto_tls_set(mosq, NULL,  "/etc/ssl/certs",  NULL, NULL, NULL);
		mosquitto_tls_opts_set(mosq, 1, NULL, NULL);

		rc = mosquitto_connect(mosq, "glowmqtt.energyhive.com", 8883, 30);
		if (rc) printf("mosquitto connect, rc=%d\n", rc);

		sprintf(topic, "SMART/+/%s", device);
		mosquitto_subscribe(mosq, NULL, topic, 0);

		while(run)
		{
			rc = mosquitto_loop(mosq, -1, 1);
			if (run && rc)
			{
				printf("Conn error\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}
		}
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();
}
