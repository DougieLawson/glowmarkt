/* 
 Copyright © Dougie Lawson, 2021, All rights reserved 
*/

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <string.h>
#include <getopt.h>
#include <libconfig.h>
#include <sys/stat.h>

#define CONFIG_FILE "/home/pi/.glow.cfg"

#define FALSE 0
#define TRUE !(FALSE)

int main(int argc, char **argv)
{
	int c;
	int digit_optind = 0;
	static const char *output_file = CONFIG_FILE;
	_Bool u_OK, p_OK, d_OK;
	_Bool t_OK, q_OK, h_OK, s_OK;
	_Bool b_OK;
	u_OK = FALSE;
	p_OK = FALSE;
	d_OK = FALSE;
	b_OK = FALSE;
	t_OK = FALSE;
	q_OK = FALSE;
	h_OK = FALSE;
	s_OK = FALSE;
	mode_t mode;
	config_t cfg;
	config_setting_t *root, *glow, *mosquitto, *mariadb;
	config_setting_t *_user, *_pwd, *_device;
	config_setting_t *_sqluser, *_sqlpwd, *_sqldbase, *_sqlhost;
	config_setting_t *_broker;
	config_init(&cfg);
	root = config_root_setting(&cfg);
	glow = config_setting_add(root, "Glow", CONFIG_TYPE_GROUP);
	mariadb = config_setting_add(root, "MariaDB", CONFIG_TYPE_GROUP);
	mosquitto = config_setting_add(root, "Mosquitto", CONFIG_TYPE_GROUP);

	while (1)
	{
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"username", required_argument, 0,  0 },
			{"password", required_argument, 0,  0 },
			{"device", required_argument, 0,  0 },
			{"sqlhost", required_argument, 0,  0 },
			{"sqluser", required_argument, 0,  0 },
			{"sqlpwd", required_argument, 0,  0 },
			{"sqldbase", required_argument, 0,  0 },
			{"broker", required_argument, 0,  0 },
			{0,          0,                 0,  0 }
		};
		c = getopt_long(argc, argv, "u:p:d:h:s:q:t:b:", long_options, &option_index);
		if (c == -1) break;
		switch (c)
		{
			case 0:
				printf("option --%s", long_options[option_index].name);

				if (!strcmp(long_options[option_index].name, "username"))
				{
				       	u_OK = TRUE;
					_user = config_setting_add(glow, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_user, optarg);
				}
				else if (!strcmp(long_options[option_index].name, "password"))
				{
					p_OK = TRUE;
					_pwd = config_setting_add(glow, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_pwd, optarg);
				}
				else if (!strcmp(long_options[option_index].name, "device"))
				{       
					d_OK = TRUE;
					_device = config_setting_add(glow, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_device, optarg);
				}
				else if (!strcmp(long_options[option_index].name, "sqlhost"))
				{       
					h_OK = TRUE;
					_sqlhost = config_setting_add(mariadb, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_sqlhost, optarg);
				}

				else if (!strcmp(long_options[option_index].name, "sqluser"))
				{       
					s_OK = TRUE;
					_sqluser = config_setting_add(mariadb, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_sqluser, optarg);
				}

				else if (!strcmp(long_options[option_index].name, "sqldbase"))
				{       
					t_OK = TRUE;
					_sqldbase = config_setting_add(mariadb, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_sqldbase, optarg);
				}

				else if (!strcmp(long_options[option_index].name, "sqlpwd"))
				{       
					q_OK = TRUE;
					_sqlpwd = config_setting_add(mariadb, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_sqlpwd, optarg);
				}

				else if (!strcmp(long_options[option_index].name, "broker"))
				{       
					b_OK = TRUE;
					_broker = config_setting_add(mosquitto, long_options[option_index].name, CONFIG_TYPE_STRING);
					config_setting_set_string(_broker, optarg);
				}


				if (optarg) printf(" with value %s\n", optarg);
				break;
			case 'u':
				printf("option -u with value '%s'\n", optarg);
				_user = config_setting_add(glow, "username", CONFIG_TYPE_STRING);
				config_setting_set_string(_user, optarg);
				u_OK = TRUE;
				break;
			case 'p':
				printf("option -p with value '%s'\n", optarg);
				_pwd = config_setting_add(glow, "password", CONFIG_TYPE_STRING);
				config_setting_set_string(_pwd, optarg);
				p_OK = TRUE;
				break;
			case 'd':
				printf("option -d with value '%s'\n", optarg);
				_device = config_setting_add(glow, "device", CONFIG_TYPE_STRING);
				config_setting_set_string(_device, optarg);
				d_OK = TRUE;
				break;
			case 'h':
				printf("option -h with value '%s'\n", optarg);
				_sqlhost = config_setting_add(mariadb, "sqlhost", CONFIG_TYPE_STRING);
				config_setting_set_string(_sqlhost, optarg);
				h_OK = TRUE;
				break;
			case 's':
				printf("option -s with value '%s'\n", optarg);
				_sqluser = config_setting_add(mariadb, "sqluser", CONFIG_TYPE_STRING);
				config_setting_set_string(_sqluser, optarg);
				s_OK = TRUE;
				break;
			case 'q':
				printf("option -q with value '%s'\n", optarg);
				_sqlpwd = config_setting_add(mariadb, "sqlpwd", CONFIG_TYPE_STRING);
				config_setting_set_string(_sqlpwd, optarg);
				q_OK = TRUE;
				break;
			case 't':
				printf("option -t with value '%s'\n", optarg);
				_sqldbase = config_setting_add(mariadb, "sqldbase", CONFIG_TYPE_STRING);
				config_setting_set_string(_sqldbase, optarg);
				t_OK = TRUE;
				break;
			case 'b':
				printf("option -b with value '%s'\n", optarg);
				_broker = config_setting_add(mosquitto, "broker", CONFIG_TYPE_STRING);
				config_setting_set_string(_broker, optarg);
				b_OK = TRUE;
				break;
			break;
			case '?':
				break;
			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}
	if (optind < argc)
	{
		printf("non-option ARGV-elements: ");
		while (optind < argc)
		printf("%s ", argv[optind++]);
		printf("\n");
	}
	if (u_OK == FALSE || p_OK == FALSE || d_OK == FALSE || h_OK == FALSE ||
	    s_OK == FALSE || b_OK == FALSE || t_OK == FALSE || q_OK == FALSE)
	{
		fprintf(stderr, "Missing option(s): ");
		if (u_OK == FALSE) fprintf(stderr, "\nGlow userid. Use: -uXXX or --username XXX ");
		if (p_OK == FALSE) fprintf(stderr, "\nGlow password. Use:  -pXXX or --password XXX ");
		if (d_OK == FALSE) fprintf(stderr, "\nGlow device ID. Use: -dXXX or --device XXX ");
		if (b_OK == FALSE) fprintf(stderr, "\nMosquitto broker. Use: -bXX.XX.XX.XX or --broker XX.XX.XX.XX or --broker raspberrypi.local ");
		if (h_OK == FALSE) fprintf(stderr, "\nMariaDB hostname. Use: -hXX.XX.XX.XX or --sqlhost XX.XX.XX.XX or -sqlhost raspberrypi.local ");
		if (s_OK == FALSE) fprintf(stderr, "\nMariaDB username. Use: -sXXX or --sqluser XXX ");
		if (t_OK == FALSE) fprintf(stderr, "\nMariaDB DB name. Use: -tXXX or --sqldbase XXX ");
		if (q_OK == FALSE) fprintf(stderr, "\nMariaDB password. Use: -qXXX or --sqlpwd XXX ");
		fprintf(stderr, "\n");
		exit(20);
	}

	if(! config_write_file(&cfg, output_file))
	{
		fprintf(stderr, "Error while writing file.\n");
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}

	fprintf(stderr, "New configuration successfully written to: %s\n", output_file);
	config_destroy(&cfg);
	mode = 0600;
	chmod(output_file, mode);
	exit(EXIT_SUCCESS);
}
