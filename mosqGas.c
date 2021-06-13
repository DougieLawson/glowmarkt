/* 
 Copyright © Dougie Lawson, 2021, All rights reserved 
*/

#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "json-c/json.h"

#include <mosquitto.h>
#include <libconfig.h>

#define CONFIG_FILE "/home/pi/.glow.cfg"

const char* username;
const char* password;
const char* device;

json_object* json_object_parse(json_object* jObj);
json_object* print_json_value(json_object* jObj);
int lexer(const char *s);

static int run = 1;
char* current_key;
char* unk_string;
signed long unk_value;
int nesting;
char* keys[4];
char concat_key[15];
char* gasMPRN;
char* time_stamp;
float gasReading;
float gasDailyConsumption;
float gasWeeklyConsumption;
float gasMonthlyConsumption;
float gasMultiplier;
float gasDivisor;
char localMQTTmessage[140];

enum {unk, unused, gas_mprn, gas_reading, gas_multiplier, gas_divisor, gas_daily_consumption, gas_weekly_consumption, gas_monthly_consumption, ts, }; 
void handle_signal(int s)
{
	run = 0;
}


int lexer(const char *s)
{
	static struct entry_s
	{
		const char *key;
		int token;
	
	}
	token_table[] = {
	       	{"elecMtr07020000", unused},
	       	{"elecMtr07020001", unused},
	       	{"elecMtr07020002", unused},
	       	{"elecMtr07020007", unused},
	       	{"elecMtr07020014", unused},
	       	{"elecMtr07020200", unused},
	       	{"elecMtr07020300", unused},
	       	{"elecMtr07020301", unused},
	       	{"elecMtr07020302", unused},
	       	{"elecMtr07020303", unused},
	       	{"elecMtr07020304", unused},
	       	{"elecMtr07020306", unused},
	       	{"elecMtr07020307", unused},
	       	{"elecMtr07020308", unused},
	       	{"elecMtr07020400", unused},
	       	{"elecMtr07020401", unused},
	       	{"elecMtr07020430", unused},
	       	{"elecMtr07020440", unused},
	       	{"elecMtr07050000", unused},
	       	{"elecMtr07050001", unused},
	       	{"elecMtr07080101", unused},
	       	{"ets", unused},
	       	{"gasMtr07020000", gas_reading},
	       	{"gasMtr07020014", unused},
	       	{"gasMtr07020200", unused},
	       	{"gasMtr07020300", unused},
	       	{"gasMtr07020301", gas_multiplier},
	       	{"gasMtr07020302", gas_divisor},
	       	{"gasMtr07020303", unused},
	       	{"gasMtr07020306", unused},
	       	{"gasMtr07020307", gas_mprn},
	       	{"gasMtr07020308", unused},
	       	{"gasMtr07020312", unused},
	       	{"gasMtr07020C01", gas_daily_consumption},
	       	{"gasMtr07020C30", gas_weekly_consumption},
	       	{"gasMtr07020C40", gas_monthly_consumption},
	       	{"gasMtr07050000", unused},
	       	{"gasMtr07050001", unused},
	       	{"gasMtr07080101", unused},
	       	{"gid", unused},
	       	{"gmtime", unused},
	       	{"hversion", unused},
	       	{"panjoin", unused},
	       	{"panlqi", unused},
	       	{"pannPAN", unused},
	       	{"panrssi", unused},
	       	{"panstatus", unused},
	       	{"smetsVer", unused},
	       	{"time", unused},
	       	{"ts", ts},
	       	{"zbSoftVer", unused},
		{"ihdscreen", unused},
		{"ihdbutton" ,unused},
		{"ihdserverScreenId", unused},
     	};
	struct entry_s *p = token_table;
	for(; p->key != NULL && strcmp(p->key, s) != 0; ++p);
	return p->token;
}

json_object* print_json_value(json_object* jObj)
{
	sprintf(concat_key, "%s%s%s%s", keys[0], keys[1], keys[2], keys[3]);
	json_object* print_result;
	enum json_type type;
	type = json_object_get_type(jObj);
	switch (type)
	{
		case json_type_object:
		case json_type_array:
			print_result = json_object_parse(jObj);
			break;
	}
	switch(lexer(concat_key)) {
		case gas_reading:
			gasReading = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case gas_monthly_consumption:
			gasMonthlyConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case gas_weekly_consumption:
			gasWeeklyConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case gas_daily_consumption:
			gasDailyConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case gas_multiplier:
			gasMultiplier = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case gas_divisor:
			gasDivisor = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case ts:
			time_stamp = strdup(json_object_get_string(jObj));
			break;
		case gas_mprn:
			gasMPRN = strdup(json_object_get_string(jObj));
			break;
		case unused: 
			break;
		case unk:
		default:
			unk_string = strdup(json_object_get_string(jObj));
			unk_value = strtol(json_object_get_string(jObj), NULL, 16);
			printf("key(%s%s%s%s) ", keys[0], keys[1], keys[2], keys[3]);
			printf("String: %s  ", unk_string);
			printf("Integer: %li\n", unk_value);
	}
	return print_result;
}

json_object* json_object_parse(json_object* jObj)
{
	enum json_type type;
	json_object* parse_result;
	json_object* newObj;
	json_object_object_foreach(jObj, key, val)
	{
		current_key = strdup(key);
		keys[nesting] = strdup(key);
		type = json_object_get_type(val);
		switch (type)
		{
			case json_type_boolean: 
			case json_type_double: 
			case json_type_int: 
			case json_type_string:
				print_json_value(val);
				break; 
			case json_type_object:
				nesting++;
				newObj = json_tokener_parse(json_object_to_json_string(val));
				parse_result = json_object_parse(newObj);
				keys[nesting] = strdup("\0");
				nesting--;
				break;
			case json_type_array:
				break;

		}
	}

	if (parse_result == NULL) return parse_result;
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

void glow_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc) printf("Connect callback, rc=%d\n", rc);
}

void glow_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{

	struct mosquitto *mosq_pub;
	int rc;
	const char* broker = "192.168.3.14";
	const char* topic = "meterReading/out/gas";
	nesting = 0;
	json_object* mqttJson = json_tokener_parse(message->payload);
	json_object_parse(mqttJson);
	json_object_put(mqttJson);

	gasReading = gasReading * gasMultiplier / gasDivisor;
	gasDailyConsumption = gasDailyConsumption * gasMultiplier / gasDivisor;
	gasWeeklyConsumption = gasWeeklyConsumption * gasMultiplier / gasDivisor;
	gasMonthlyConsumption = gasMonthlyConsumption * gasMultiplier / gasDivisor;

	sprintf(localMQTTmessage,
			"{\"MPRN\":\"%s\",\"Gas_reading\": %.3f,\"Daily\": %.3f,\"Weekly\": %.3f,\"Monthly\": %.3f,\"Timestamp\": \"%s\"}",
		      	gasMPRN, gasReading, gasDailyConsumption, gasWeeklyConsumption, gasMonthlyConsumption, time_stamp);
	
	
	mosq_pub = mosquitto_new(NULL, true, NULL);
	mosquitto_connect(mosq_pub, broker, 1883, 0);
	mosquitto_publish(mosq_pub, NULL, topic, strlen(localMQTTmessage), localMQTTmessage, 0, 0);
	mosquitto_disconnect(mosq_pub);
	mosquitto_destroy(mosq_pub);

}

int main(int argc, char* argv[]) 
{
	uint8_t reconnect = true;
	struct mosquitto *mosq_sub;
	int rc = 0;
	char topic[23];
	mosquitto_lib_init();
	mosq_sub = mosquitto_new(NULL, true, 0);

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	readConfig();

	if(mosq_sub)
	{
		mosquitto_connect_callback_set(mosq_sub, glow_connect);
		mosquitto_message_callback_set(mosq_sub, glow_message);

		mosquitto_username_pw_set(mosq_sub, username, password);
		mosquitto_tls_set(mosq_sub, NULL,  "/etc/ssl/certs",  NULL, NULL, NULL);
		mosquitto_tls_opts_set(mosq_sub, 1, NULL, NULL);

		rc = mosquitto_connect(mosq_sub, "glowmqtt.energyhive.com", 8883, 30);
		if (rc) printf("mosquitto connect, rc=%d\n", rc);

		//sprintf(topic, "SMART/HILD/%s", device);
		sprintf(topic, "SMART/+/%s", device);
		mosquitto_subscribe(mosq_sub, NULL, topic, 0);

		while(run)
		{
			rc = mosquitto_loop(mosq_sub, -1, 1);
			if (run && rc)
			{
				printf("Conn error\n");
				sleep(10);
				mosquitto_reconnect(mosq_sub);
			}
		}
		mosquitto_destroy(mosq_sub);
	}
	mosquitto_lib_cleanup();
}
