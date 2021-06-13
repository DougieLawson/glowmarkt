/* 
 Copyright © Dougie Lawson, 2021, All rights reserved 
*/

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <string.h>
#include <libconfig.h>

#define CONFIG_FILE "/home/pi/.glow.cfg"

const char* username;
const char* password;
const char* device;

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

int main(int argc, char **argv)
{
	readConfig();
	printf("User: %s Pwd: %s Dev: %s\n", username, password, device);
	exit(EXIT_SUCCESS);
}
