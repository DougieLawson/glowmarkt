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
char* elecSerial;
char* gasSerial;
char* elecMPAN;
char* gasMPRN;
char* e_time_stamp;
char* time_stamp;
long unix_hex_time;
char* epoch_time;
char* CAD_guid;
char* pan_status;
char* smets_ver;
char* h_version;
char* zb_soft_ver;
int rssi;
float elecReading;
float elecDailyConsumption;
float elecWeeklyConsumption;
float elecMonthlyConsumption;
float elecConsumption;
float elecMultiplier;
float elecDivisor;
float gasReading;
float gasDailyConsumption;
float gasWeeklyConsumption;
float gasMonthlyConsumption;
float gasMultiplier;
float gasDivisor;
char* gasUnit;
char* elecUnit;
char* elecMeterType;
char* elecSupplyStatus;
char* gasMeterType;
char* gasSupplyStatus;
int linkQual;

enum {unk, elec_reading, elec_received, elec_delivered, elec_utc, elec_supply_status, elec_meter_status, elec_units, elec_multiplier, elec_divisor, elec_demand_formatting, elec_formatting, elec_meter_type, elec_mpan, elec_serial, elec_consumption, elec_daily_consumption, elec_weekly_consumption, elec_monthly_consumption, elec_prepayment, elec_credit, elec_provider, ets, gas_reading, gas_supply_status, gas_meter_status, gas_units, gas_multiplier, gas_divisor, gas_formatting, gas_meter_type, gas_mprn, gas_serial, gas_alternative_unit, gas_daily_consumption, gas_weekly_consumption, gas_monthly_consumption, gas_prepayment, gas_credit, gas_provider, gid, gmtime, hversion, panjoin, panlqi, pannPAN, panrssi, panstatus, smetsVer, time, ts, zbSoftVer, ihdscreen, ihdbutton, ihdserverScreenId, };

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
	       	{"elecMtr07020000", elec_reading},
	       	{"elecMtr07020001", elec_received},
	       	{"elecMtr07020002", elec_delivered},
	       	{"elecMtr07020007", elec_utc},
	       	{"elecMtr07020014", elec_supply_status},
	       	{"elecMtr07020200", elec_meter_status},
	       	{"elecMtr07020300", elec_units},
	       	{"elecMtr07020301", elec_multiplier},
	       	{"elecMtr07020302", elec_divisor},
	       	{"elecMtr07020303", elec_demand_formatting},
	       	{"elecMtr07020304", elec_formatting},
	       	{"elecMtr07020306", elec_meter_type},
	       	{"elecMtr07020307", elec_mpan},
	       	{"elecMtr07020308", elec_serial},
	       	{"elecMtr07020400", elec_consumption},
	       	{"elecMtr07020401", elec_daily_consumption},
	       	{"elecMtr07020430", elec_weekly_consumption},
	       	{"elecMtr07020440", elec_monthly_consumption},
	       	{"elecMtr07050000", elec_prepayment},
	       	{"elecMtr07050001", elec_credit},
	       	{"elecMtr07080101", elec_provider},
	       	{"ets", ets},
	       	{"gasMtr07020000", gas_reading},
	       	{"gasMtr07020014", gas_supply_status},
	       	{"gasMtr07020200", gas_meter_status},
	       	{"gasMtr07020300", gas_units},
	       	{"gasMtr07020301", gas_multiplier},
	       	{"gasMtr07020302", gas_divisor},
	       	{"gasMtr07020303", gas_formatting},
	       	{"gasMtr07020306", gas_meter_type},
	       	{"gasMtr07020307", gas_mprn},
	       	{"gasMtr07020308", gas_serial},
	       	{"gasMtr07020312", gas_alternative_unit},
	       	{"gasMtr07020C01", gas_daily_consumption},
	       	{"gasMtr07020C30", gas_weekly_consumption},
	       	{"gasMtr07020C40", gas_monthly_consumption},
	       	{"gasMtr07050000", gas_prepayment},
	       	{"gasMtr07050001", gas_credit},
	       	{"gasMtr07080101", gas_provider},
	       	{"gid", gid},
	       	{"gmtime", gmtime},
	       	{"hversion", hversion},
	       	{"panjoin", panjoin},
	       	{"panlqi", panlqi},
	       	{"pannPAN", pannPAN},
	       	{"panrssi", panrssi},
	       	{"panstatus", panstatus},
	       	{"smetsVer", smetsVer},
	       	{"time", time},
	       	{"ts", ts},
	       	{"zbSoftVer", zbSoftVer},
		{"ihdscreen", ihdscreen},
		{"ihdbutton" ,ihdbutton},
		{"ihdserverScreenId", ihdserverScreenId},
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
		case elec_reading:
			elecReading = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case gas_reading:
			gasReading = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case elec_monthly_consumption:
			elecMonthlyConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case elec_weekly_consumption:
			elecWeeklyConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case elec_daily_consumption:
			elecDailyConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case elec_consumption:
			elecConsumption = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case elec_multiplier:
			elecMultiplier = (float)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case elec_divisor:
			elecDivisor = (float)strtol(json_object_get_string(jObj), NULL, 16);
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
		case elec_meter_type:
			elecMeterType = "    ";
			if ((int)strtol(json_object_get_string(jObj), NULL, 16) == 0) elecMeterType = "ELEC";
			break;
		case gas_meter_type:
			gasMeterType = "   ";
			if ((int)strtol(json_object_get_string(jObj), NULL, 16) == 128) gasMeterType = "GAS";
			break;
		case elec_supply_status:
			elecSupplyStatus = "OFF";
			if ((int)strtol(json_object_get_string(jObj), NULL, 16) == 2) elecSupplyStatus = "ON";
			break;
		case gas_supply_status:
			gasSupplyStatus = "OFF";
			if ((int)strtol(json_object_get_string(jObj), NULL, 16) == 2) gasSupplyStatus = "ON";
			break;
		case gas_units:
			gasUnit = ((int)strtol(json_object_get_string(jObj), NULL, 16) ? "m^3" : "kWh");
			break;
		case elec_units:
			elecUnit = ((int)strtol(json_object_get_string(jObj), NULL, 16) ? "m^3" : "kWh");
			break;
		case panlqi:
			linkQual = (int)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case panrssi:
			rssi = (int)strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case zbSoftVer:
			zb_soft_ver = strdup(json_object_get_string(jObj));
			break;
		case hversion:
			h_version = strdup(json_object_get_string(jObj));
			break;
		case panstatus:
			pan_status = strdup(json_object_get_string(jObj));
			break;
		case smetsVer:
			smets_ver = strdup(json_object_get_string(jObj));
			break;
		case gmtime:
			epoch_time = strdup(json_object_get_string(jObj));
			break;
		case time:
			unix_hex_time = strtol(json_object_get_string(jObj), NULL, 16);
			break;
		case ts:
			time_stamp = strdup(json_object_get_string(jObj));
			break;
		case gid:
			CAD_guid = strdup(json_object_get_string(jObj));
			break;
		case ets:
			e_time_stamp = strdup(json_object_get_string(jObj));
			break;
		case elec_mpan:
			elecMPAN = strdup(json_object_get_string(jObj));
			break;
		case gas_mprn:
			gasMPRN = strdup(json_object_get_string(jObj));
			break;
		case gas_serial:
			gasSerial = strdup(json_object_get_string(jObj));
			break;
		case elec_serial:
			elecSerial = strdup(json_object_get_string(jObj));
			break;
		case elec_formatting:
		case elec_demand_formatting:
		case gas_formatting:
		case pannPAN:
		case panjoin:
		case elec_utc:
		case elec_received: 
		case elec_delivered:
		case elec_prepayment:
	       	case elec_credit:
	       	case elec_provider:
	       	case gas_prepayment:
	       	case gas_credit:
	       	case gas_provider:
		case elec_meter_status:
		case gas_meter_status:
		case gas_alternative_unit:
		case ihdscreen:
		case ihdbutton:
		case ihdserverScreenId: 
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

void connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc) printf("Connect callback, rc=%d\n", rc);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	json_object* mqttJson = json_tokener_parse(message->payload);
	json_object_parse(mqttJson);
	json_object_put(mqttJson);

	elecReading = elecReading * elecMultiplier / elecDivisor;
	elecDailyConsumption = elecDailyConsumption * elecMultiplier / elecDivisor;
	elecWeeklyConsumption = elecWeeklyConsumption * elecMultiplier / elecDivisor;
	elecMonthlyConsumption = elecMonthlyConsumption * elecMultiplier / elecDivisor;
	elecConsumption = elecConsumption * elecMultiplier / elecDivisor;

	gasReading = gasReading * gasMultiplier / gasDivisor;
	gasDailyConsumption = gasDailyConsumption * gasMultiplier / gasDivisor;
	gasWeeklyConsumption = gasWeeklyConsumption * gasMultiplier / gasDivisor;
	gasMonthlyConsumption = gasMonthlyConsumption * gasMultiplier / gasDivisor;

	printf("Electricity\n-----------\n");
	printf("%s MPAN %s\n",elecMeterType,  elecMPAN);
	//printf(" Elec serial: %s\n", elecSerial);
       	printf(" Elec unit %s\n", elecUnit);
	printf(" Meter reading %.3f\n", elecReading);
	printf(" Daily %.3f\n", elecDailyConsumption);
	printf(" Weekly %.3f\n", elecWeeklyConsumption);
	printf(" Monthly %.3f\n", elecMonthlyConsumption);
	printf(" Meter %.3f\n", elecConsumption);
	printf(" Electricity supply status %s\n", elecSupplyStatus);
      	printf("\n");	
	printf("Gas\n---\n");
	printf("%s MPRN %s\n", gasMeterType, gasMPRN);
	//printf(" Gas serial: %s\n", gasSerial);
	printf(" Gas unit %s\n", gasUnit);
	printf(" Gas reading %.3f\n", gasReading);
	printf(" Daily %.3f\n", gasDailyConsumption);
	printf(" Weekly %.3f\n", gasWeeklyConsumption);
	printf(" Monthly %.3f\n", gasMonthlyConsumption);
	printf(" Gas supply status %s\n", gasSupplyStatus);
      	printf("\n");	
	printf("Miscellaneous\n-------------\n");
	printf(" Timestamp %s\n", time_stamp);
	printf(" Unix epoch time %s\n", epoch_time);
	printf(" Unix epoch time (hex) %lu\n", unix_hex_time);
	printf(" Pan status %s\n", pan_status);
	printf(" Link quality %i\n", linkQual);
	printf(" RSSI %i\n", rssi);
	printf(" CAD guid %s\n", CAD_guid);
	printf(" e-timestamp %s\n", e_time_stamp);
	printf(" SMETS version %s\n", smets_ver);
	printf(" Zigbee software version %s\n", zb_soft_ver);
	printf(" Hardware version %s\n", h_version);

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

		//sprintf(topic, "SMART/HILD/%s", device);
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
