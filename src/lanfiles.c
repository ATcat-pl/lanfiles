#include "lanfiles.h"
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>

static int parse_options(int key, char *arg, struct argp_state *state) {
	switch(key) {
		case 'i':
			//ip = arg
			break;
		case 'u':
			//encypt = no
			action = action & ~ACTION_ENCRYPT;
			break;
		case 'f':
			//filePath = arg
			filepath = arg;
			break;
		case 's':
			//action send
			action = action | ACTION_SEND;
			break;
		case 'r':
			//action recive
			action = action | ACTION_RECIVE;
			break;
	}
	return 0;
}

static void decode_options(int argc, char **argv) {
	//set default options
	action = 0;
	action = action | ACTION_ENCRYPT; //encrytion is on by default
	//set up argp
	struct argp_option options[] = {
		{"send", 's', 0, 0, "Send files specified by file option"},
		{"tx", 0, 0, OPTION_ALIAS, ""},
		{"recive", 'r', 0, 0, "Recive files specified by ip"},
		{"rx", 0, 0, OPTION_ALIAS, ""},
		{"ip", 'i', "IP", 0, "Sets IP address for transmission"},
		{"file", 'f', "PATH", 0, "File ar folder to transfer"},
		{"unencrypted", 'u', 0, 0, "Disables transport encryption"},
		{0 }
	};
	struct argp argp = { options, parse_options };
	argp_parse(&argp, argc, argv, 0, 0, 0);
}

int main(int argc, char **argv) {
	decode_options(argc, argv);
	if(ACTION_SEND & action){
		printf("Sending file\nFilePath = %s\n", filepath);
	} else if(ACTION_RECIVE & action) {
		printf("Reciving file\n");
	} else {
		printf("ERROR: Action not specified, or multiple actions specified.\n");
		return -1;
	}
	return 0;
}
