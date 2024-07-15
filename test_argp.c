/*
 * Demo of parsing program options with Argp
 * Build: gcc test-argp.c -o test-argp
 * Run: ./test-argp
 */

#include <argp.h>
#include <stdlib.h>

/* Program documentation. */
static char doc[] =
"Demo for argp";

/* A description of the arguments we accept. */
static char args_doc[] = "TABLE FILE";

/* The options we understand. */
static struct argp_option options[] = {
	{"host",   'h', "HOSTNAME", 0,  "master host(default: \"localhost\")" },
	{"port",	 'p', "PORT",			0,  "master port(default: 5432)" },
	{"dbname", 'd', "DBNAME",	  0,  "database name(default: \"postgres\")" },
	{"format", 'f', "FORMAT",		0,  "format string(default: \"\")" },
	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
	char *args[2];                /* table name and input file*/
	char *host, *dbname, *format;
	int port;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
		 know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key)
	{
		case 'h':
			arguments->host = arg;
			break;
		case 'p':
			arguments->port = atoi(arg);
			break;
		case 'd':
			arguments->dbname = arg;
			break;
		case 'f':
			arguments->format = arg;
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num >= 2)
				/* Too many arguments. */
				argp_usage (state);
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if (state->arg_num < 2)
				/* Not enough arguments. */
				argp_usage (state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int
main (int argc, char **argv)
{
	struct arguments arguments;

	/* Default values. */
	arguments.host = "localhost";
	arguments.port = 5432;
	arguments.dbname = "postgres";

	/* Parse our arguments; every option seen by parse_opt will
		 be reflected in arguments. */
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	printf ("TABLE = %s\nFILE = %s\nHOST = %s\n"
			"PORT = %d\nDATABASE = %s\nFORMAT = %s\n",
			arguments.args[0], arguments.args[1],
			arguments.host,
			arguments.port,
			arguments.dbname,
			arguments.format);
	exit (0);
}