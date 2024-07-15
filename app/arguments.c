#include "arguments.h"

error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *args = state->input;

	switch (key) {
	case 'r':
		args->replicaof = arg;
		break;
	case 'p':
		args->port = atoi(arg);
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	case ARGP_KEY_END:
		if (state->arg_num != 0)
			argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}