#include <getopt.h>
#include <unistd.h>

#include <libconfig.h>

#define _CFG_DEFINE_IMPLS
#include "main.h"

int usage(char *prog, char *msg);
char* strip_dashes(char*);

int dump_cfg(config_t *cfg) {
	// TODO
	return ENOTSUP;
}

int main(int argc, char** argv) {
	int ret, cfg_ret, cfg_write;
	config_t cfg;
	char *mode, *fname;
	char *type, *key, *val;

	/* check for -h/--help */
	for(int i = 0; i < argc; i++)
		if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
			return usage(argv[0], NULL);

	/* handle the arguments */
	fname = argc > 1 ? argv[1] : NULL;
	mode  = argc > 2 ? argv[2] : NULL;
	if(fname && mode) {
		/* remove any leading dashes from mode */
		mode = strip_dashes(mode);

		/* check if mode is help */
		if(strcmp(mode, "help") == 0 || strcmp(mode, "h") == 0)
			return usage(argv[0], NULL);

		/* make sure filename is accessible */
		if(access(fname, F_OK) == 0) {
			cfg_ret = cfg_write = ret = 0;

			/* read the configuration file */
			config_init(&cfg);
			cfg_ret = config_read_file(&cfg, fname);
			if(cfg_ret != CONFIG_TRUE) {
				ret = print_cfg_err(&cfg, "config_read_file");
				goto exit_with_ret;
			}

			/* grab excess arguments */
			type = argc > 3 ? argv[3] : NULL;
			key  = argc > 4 ? argv[4] : NULL;
			val  = argc > 5 ? argv[5] : NULL;

			/* remove any leading dashes from type */
			type = strip_dashes(type);

			/* check for dump mode first, its the easiest */
			if(strcmp(mode, "dump") == 0 || strcmp(mode, "d") == 0) {
				ret = dump_cfg(&cfg);
			} else if(strcmp(mode, "read") == 0 || strcmp(mode, "r") == 0) {
				if(!type) { ret = usage(argv[0], "Type wasn't given for write command!");  if(ret) goto exit_with_ret; }
				if(!key)  { ret = usage(argv[0], "Key wasn't given for write command!");   if(ret) goto exit_with_ret; }
				if(ret)
					goto exit_with_ret;
				ret = read_cfg(&cfg, type, key, val);
			} else if(strcmp(mode, "write") == 0 || strcmp(mode, "w") == 0) {
				if(!type) { ret = usage(argv[0], "Type wasn't given for write command!");  if(ret) goto exit_with_ret; }
				if(!key)  { ret = usage(argv[0], "Key wasn't given for write command!");   if(ret) goto exit_with_ret; }
				if(!val)  { ret = usage(argv[0], "Value wasn't given for write command!"); if(ret) goto exit_with_ret; }

				ret = write_cfg(&cfg, type, key, val);
				if(ret == 0)
					cfg_write = 1;
			} else {
				fprintf(stderr, "Error: Invalid / Nonexistant mode (%s) used!\n", mode);
				ret = EINVAL;
			}

			if(ret)
				goto exit_with_ret;

			/* update the config file if needed */
			if(cfg_write) {
				cfg_ret = config_write_file(&cfg, fname);
				if(cfg_ret != CONFIG_TRUE) {
					ret = print_cfg_err(&cfg, "config_write_file");
					goto exit_with_ret;
				}
			}

			/* Exit with return code */
			goto exit_with_ret;
		} else {
			fprintf(stderr, "Error: File \"%s\" does not exist!\n", fname);
			return EINVAL;
		}
	} else {
		return usage(argv[0], "Not enough arguments!");
	}

exit_with_ret:
	if(ret == ENOTSUP)
		fprintf(stderr, "Error: A feature used is not yet supported!\n");
	config_destroy(&cfg);
	return ret;
}

int get_type(char *tstr, cfg_type_t *type) {
	if(!tstr) {
		fprintf(stderr, "Error: Type is nullptr, cannot continue!\n");
		return -1;
	}

	for(int i = 0; types[i].str; i++) {
		if(strcmp(types[i].str, tstr) == 0 || (types[i].sstr && strcmp(types[i].sstr, tstr) == 0)) {
			if(types[i].size <= 0 || types[i].setting_set == NULL || types[i].lookup == NULL) {
				fprintf(stderr, "Error: Type has an invalid parameter, cannot continue!\n");
				return -1;
			}
			memcpy(type, &types[i], sizeof(cfg_type_t));
			return 0;
		}
	}

	fprintf(stderr, "Error: Invalid / Unknown type \"%s\" given!\n", tstr);
	return EINVAL;
}
int get_setting(const config_t *cfg, const char *key, config_setting_t **sett, int type_id) {
	config_setting_t *tmp;
	/* get the configuration setting */
	tmp = config_lookup(cfg, ((const char*) key));
	if(tmp == NULL) {
		/* create it if it doesn't exist */
		tmp = config_root_setting(cfg);
		if(tmp == NULL)
			return print_cfg_err(cfg, "config_root_setting");
		tmp = config_setting_add(tmp, (const char*) key, type_id);
		if(tmp == NULL)
			return print_cfg_err(cfg, "config_setting_add");
	}
	*sett = tmp;
	return 0;
}
int usage(char *prog, char *msg) {
	const int len = 256;
	char tstr[len];
	int pos = 0;
	char *pstr;

	if(msg)
		fprintf(stderr, "Error: %s\n\n", msg);

	for(int i = 0; types[i].str; i++) {
		int alen = len - pos - 2;
		if(alen > 0 && pos < len - 1) {
			if(types[i].sstr)
				pos += snprintf(&tstr[pos], alen, "%s/%s, ", types[i].str, types[i].sstr);
			else
				pos += snprintf(&tstr[pos], alen, "%s, ", types[i].str);
		}
	}
	if(pos)
		tstr[pos - 2] = '\0';

	pstr = prog ? prog : "./main";
	fprintf(msg ? stderr : stdout,
		"Usage: %s <filename> <mode> [...]\n"
		"Modes:\n"
//		"  d/dump  - dumps all configuration values\n"
		"  r/read  - reads configuratiuon values\n"
		"            expects: <type> <key> [default]\n"
		"  w/write - sets configuration value\n"
		"            expects: <type> <key> <val>\n"
		"Accepted types:\n  %s\n"
		"Examples:\n"
		"  Write string \"test\" to \"example.cfg\":\n"
		"    input:   %s example.cfg w string test \"hello world\"\n"
		"    returns: 0\n"
		"  Read string \"test\" from \"example.cfg\":\n"
		"    input:   %s example.cfg r string test\n"
		"    outputs: hello world\n"
		"    returns: 0\n",
		pstr, tstr, pstr, pstr);
	return msg ? EINVAL : 0;
}
int print_cfg_err(const config_t *cfg, const char *func) {
	fprintf(stderr, "Error: %s failed with below error on line %d!\n", func, config_error_line(cfg));
	switch(config_error_type(cfg)) {
		case CONFIG_ERR_PARSE:
			fprintf(stderr, "Parsing error in %s: %s\n", config_error_file(cfg), config_error_text(cfg));
			return -4;
		case CONFIG_ERR_FILE_IO:
			fprintf(stderr, "File IO error: %s, %s\n", config_error_text(cfg), errno ? strerror(errno) : "ERRNO not set");
			return -3;
		case CONFIG_ERR_NONE:
			fprintf(stderr, "Return was not CONFIG_TRUE yet CONFIG_ERR_NONE is set?!\n");
			return -2;
		default:
			fprintf(stderr, "Unknown error?!\n");
			return -1;
	}
}
char* strip_dashes(char* str) {
	while(str && *str == '-')
		str++;
	return str;
}
