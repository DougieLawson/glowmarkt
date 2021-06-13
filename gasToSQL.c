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
#include <mysql.h>

#define LOCALBROKER "192.168.3.14"
#define TOPIC "meterReading/out/gas"

json_object* json_object_parse(json_object* jObj);
json_object* print_json_value(json_object* jObj);
int lexer(const char *s);

static int run = 1;
char* current_key;
char* unk_string;
signed long unk_value;
int nesting;
char* keys[1];
char concat_key[15];
char* gasMPRN;
char* time_stamp;
double gasReading;
double gasDailyConsumption;
double gasWeeklyConsumption;
double gasMonthlyConsumption;
float elecConsumption;

MYSQL* con;
MYSQL_STMT *stmt1;
MYSQL_STMT *stmt2;
MYSQL_STMT *stmt3;

#define d_host "192.168.3.14"
#define d_user "mysqluser"
#define d_pwd "mypassword"
#define d_db "EV"

#define FALSE 0
#define TRUE !(FALSE)

enum {unk, unused,
       	gas_mprn,
       	gas, 
       	gas_daily, 
	gas_weekly, 
	gas_monthly,
       	elec,
       	ts, };

void exit_on_error(char* call, MYSQL *con)
{
	fprintf(stderr, "SQL error %s\n", call);
	if (con != NULL)
	{       
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
	}
	exit(-20);
}

void sqlInitialise()
{
	my_bool trunc = 0;
   	char* sql1;
	char* sql2;
	char* sql3;

	mysql_library_init(0, NULL, NULL);

	con = mysql_init(NULL);
	mysql_options(con, MYSQL_REPORT_DATA_TRUNCATION, &trunc);

	if( con == NULL )
	{
		fprintf(stderr, "Can't run Mariadb");
		exit_on_error("Really bad", NULL);
	}

	if (mysql_real_connect(con, d_host, d_user, d_pwd, d_db, 0, NULL, 0) == NULL)
	{
		exit_on_error("Connect", con);
	}
 
	//printf("mariadb-connection-id: %ld\n" , mysql_thread_id(con));

	
	sql1 = "INSERT IGNORE INTO gas_usage(reading, daily, weekly, monthly, datetime) VALUES(?, ?, ?, ?, ?)";
	sql2 = "SELECT current_reading from gas_reading";
	sql3 = "UPDATE gas_reading set current_reading = ?";

	stmt1 = mysql_stmt_init(con);
	stmt2 = mysql_stmt_init(con);
	stmt3 = mysql_stmt_init(con);

	if (!stmt1) 
	{
		exit_on_error("Stmt1 init", con);
	}
	if (!stmt2) 
	{
		exit_on_error("Stmt2 init", con);
	}
	if (!stmt3) 
	{
		exit_on_error("Stmt3 init", con);
	}

	if (mysql_stmt_prepare(stmt1, sql1, strlen(sql1)))
	{
		exit_on_error("Stmt1 prepare", con);
	}
	if (mysql_stmt_prepare(stmt2, sql2, strlen(sql2)))
	{
		exit_on_error("Stmt2 prepare", con);
	}
	if (mysql_stmt_prepare(stmt3, sql3, strlen(sql3)))
	{
		exit_on_error("Stmt3 prepare", con);
	}
}

void sqlTerminate()
{
   	if (mysql_stmt_close(stmt1))
   	{
        	exit_on_error("Stmt1 close", con);
   	} 
   	if (mysql_stmt_close(stmt2))
   	{
        	exit_on_error("Stmt2 close", con);
   	} 
   	if (mysql_stmt_close(stmt3))
   	{
        	exit_on_error("Stmt3 close", con);
   	} 

	mysql_close(con);
	mysql_library_end();
}

int sqlFetchUpdateandInsert()
{
	double current_reading;
	int updatedValue;
	MYSQL_BIND bind[5];
	unsigned long length[5];
	my_bool error[5];
	int rc;
	int y, mo, d, h, mi, s;
	MYSQL_TIME ts;
	char currentReading[30];
	char newReading[30];

	memset(bind, 0, sizeof(bind));

	rc = mysql_stmt_execute(stmt2);
	
	bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
	bind[0].buffer = (double *)&current_reading;
	bind[0].is_null = (my_bool)0;
	bind[0].length = &length[0];
	bind[0].error = &error[0];

	if (mysql_stmt_bind_result(stmt2, bind))
	{
		exit_on_error("Stmt2 bind result", con);
	}

	rc = mysql_stmt_fetch(stmt2);
	while(!rc)
	{
		printf("%.3f\n", current_reading );
		rc = mysql_stmt_fetch(stmt2);
	}

	sprintf(currentReading, "%.3f\0",current_reading);
	sprintf(newReading, "%.3f\0", gasReading);

	rc = strcmp(currentReading, newReading);
	if (rc) 
	{
	printf("cur: %s , new %s ", currentReading, newReading);
	printf("compare: %d\n", rc);
		printf("Not the same\n");
	
		bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[0].buffer = (double *)&gasReading;
		bind[0].is_null = (my_bool)0;
		bind[0].length = &length[0];
		bind[0].error = &error[0];

		if (mysql_stmt_bind_param(stmt3, bind))
		{
			exit_on_error("Stmt bind param", con);
		}
		rc = mysql_stmt_execute(stmt3);

		if (rc)
		{
			exit_on_error("Stmt execute", con);
		}
	
		bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[1].buffer = (double *)&gasDailyConsumption;
		bind[1].is_null = (my_bool)0;
		bind[1].length = &length[1];
		bind[1].error = &error[1];
	
		bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[2].buffer = (double *)&gasWeeklyConsumption;
		bind[2].is_null = (my_bool)0;
		bind[2].length = &length[2];
		bind[2].error = &error[2];
		
		bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[3].buffer = (double *)&gasMonthlyConsumption;
		bind[3].is_null = (my_bool)0;
		bind[3].length = &length[3];
		bind[3].error = &error[3];
  
		sscanf(time_stamp, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s);

		ts.year = y;
		ts.month = mo;
		ts.day = d;
		ts.hour = h;
		ts.minute = mi;
		ts.second = s;
		ts.second_part = 0;

		bind[4].buffer_type = MYSQL_TYPE_DATETIME;
		bind[4].buffer = &ts;
		bind[4].is_null = (my_bool)0;
		bind[4].error = &error[4];

		if (mysql_stmt_bind_param(stmt1, bind))
		{
			exit_on_error("Stmt1 bind param", con);
		}
		rc = mysql_stmt_execute(stmt1);

		if (rc)
		{
			exit_on_error("Stmt1 execute", con);
		}
	}

	mysql_commit(con);

}


void handle_signal(int s)
{
	run = 0;
	sqlTerminate();
}

int lexer(const char *s)
{
	static struct entry_s
	{
		const char *key;
		int token;
	
	}
	token_table[] = {
	       	{"MPRN", gas_mprn},
	       	{"Gas_reading", gas},
	       	{"Daily", gas_daily},
	       	{"Weekly", gas_weekly},
	       	{"Monthly", gas_monthly},
	       	{"Timestamp", ts},
	       	{"Meter", elec},
     	};
	struct entry_s *p = token_table;
	for(; p->key != NULL && strcmp(p->key, s) != 0; ++p);
	return p->token;
}

json_object* print_json_value(json_object* jObj)
{
	sprintf(concat_key, "%s", keys[0]);
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
		case elec:
			elecConsumption = json_object_get_double(jObj);
			break;
		case gas:
			gasReading = json_object_get_double(jObj);
			break;
		case gas_monthly:
			gasMonthlyConsumption = json_object_get_double(jObj);
			break;
		case gas_weekly:
			gasWeeklyConsumption = json_object_get_double(jObj);
			break;
		case gas_daily:
			gasDailyConsumption = json_object_get_double(jObj);
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
			printf("key(%s) ", keys[0]);
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

void local_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc) printf("Connect callback, rc=%d\n", rc);
}

void local_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	char gasReadingStr[30];
	nesting = 0;
	json_object* mqttJson = json_tokener_parse(message->payload);
	json_object_parse(mqttJson);
	json_object_put(mqttJson);
		//printf("G: %.3f GM: %.3f GW: %.3f GD: %.3f T: %s\n", gasReading, gasMonthlyConsumption, gasWeeklyConsumption, gasDailyConsumption, time_stamp);

	sprintf(gasReadingStr, "%.3f\0",gasReading);
	if (strcmp(gasReadingStr, "0.000")) {
		//printf("GasReading: %.3f\n",gasReading);
		sqlFetchUpdateandInsert();
	}
	gasReading = 0.0;
	gasDailyConsumption = 0.0;
	gasWeeklyConsumption = 0.0;
	gasMonthlyConsumption = 0.0;

}

int main(int argc, char* argv[]) 
{
	uint8_t reconnect = true;
	struct mosquitto *local_sub;
	int rc = 0;
	char topic[23];
	mosquitto_lib_init();
	sqlInitialise();
	local_sub = mosquitto_new(NULL, true, 0);

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	if(local_sub)
	{
		mosquitto_connect_callback_set(local_sub, local_connect);
		mosquitto_message_callback_set(local_sub, local_message);

		rc = mosquitto_connect(local_sub, LOCALBROKER, 1883, 30);
		if (rc) printf("mosquitto connect, rc=%d\n", rc);

		mosquitto_subscribe(local_sub, NULL, TOPIC, 0);

		while(run)
		{
			rc = mosquitto_loop(local_sub, -1, 1);
			if (run && rc)
			{
				printf("Conn error\n");
				sleep(10);
				mosquitto_reconnect(local_sub);
			}

		}
		mosquitto_destroy(local_sub);
	}
	mosquitto_lib_cleanup();
}
