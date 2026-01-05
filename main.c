#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>

#include <libconfig.h>

#include "main.h"

#define _CFG_SET_FUNC_IMPL(LTYPE, PTRTYPE) int _ptr_impl_config_setting_set_##LTYPE(config_setting_t *sett, volatile void *ptr) { return config_setting_set_##LTYPE(sett, *((PTRTYPE) ptr)); }
#define _CFG_GET_FUNC(LTYPE) (config_lookup_func_t)      config_lookup_##LTYPE
#define _CFG_SET_FUNC(LTYPE) (config_setting_set_func_t) _ptr_impl_config_setting_set_##LTYPE
#define _CFG_ENTRY(UTYPE, LTYPE, SHAND, CTYPE) { CONFIG_TYPE_##UTYPE, #LTYPE, #SHAND, sizeof(CTYPE), _CFG_GET_FUNC(LTYPE), _CFG_SET_FUNC(LTYPE) },

_CFG_SET_FUNC_IMPL(int, int*)
_CFG_SET_FUNC_IMPL(int64, long long*)
_CFG_SET_FUNC_IMPL(float, double*)
_CFG_SET_FUNC_IMPL(bool, int*)
_CFG_SET_FUNC_IMPL(string, char**)

static const cfg_type_t types[] = {
	_CFG_ENTRY(INT,    int,    i, int)
	_CFG_ENTRY(INT64,  int64,  l, long long)
	_CFG_ENTRY(FLOAT,  float,  f, double)
	_CFG_ENTRY(BOOL,   bool,   b, int)
	_CFG_ENTRY(STRING, string, s, const char*)
//	{ 0, "auto", "a", 0, NULL, NULL }, TODO
	{ 0, NULL, NULL, 0, NULL, NULL },
};

int usage(char *prog, char *msg);
int get_type(char *tstr, cfg_type_t *type);
int print_cfg_err(const config_t *cfg, const char *func);
char* strip_dashes(char*);

int read_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	cfg_type_t type;
	volatile void *res;
	int ret;

	// TODO handle default values
	if(val)
		return ENOTSUP;

	// TODO support automatic type detection via config_setting_type
	ret = get_type(tstr, &type);
	if(ret)
		return ret;

	/* allocate the value */
	res = alloca(type.size);

	/* read the value */
	ret = type.lookup(cfg, key, res);

	if(ret == CONFIG_TRUE) {
		switch(type.id) {
			case CONFIG_TYPE_INT:    printf("%d\n", *((int*) res)); break;
			case CONFIG_TYPE_INT64:  printf("%lld\n", *((long long*) res)); break;
			case CONFIG_TYPE_FLOAT:  printf("%lf\n", *((double*) res)); break;
			case CONFIG_TYPE_BOOL:   printf("%s\n", (*((int*) res)) ? "true" : "false"); break;
			case CONFIG_TYPE_STRING: printf("%s\n", *((const char**) res)); break;
		}
		return 0;
	} else {
		return print_cfg_err(cfg, "config_lookup");
	}
}
int write_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	config_setting_t *sett;
	volatile void *res;
	cfg_type_t type;
	int ret;

	// TODO support automatic type detection via config_setting_type
	ret = get_type(tstr, &type);
	if(ret)
		return ret;

	/* get the conffiguration setting */
	sett = config_lookup(cfg, ((const char*) key));
	if(sett == NULL) {
		sett = config_root_setting(cfg);
		if(sett == NULL)
			return print_cfg_err(cfg, "config_root_setting");
		sett = config_setting_add(sett, (const char*) key, type.id);
		if(sett == NULL)
			return print_cfg_err(cfg, "config_setting_add");
	}

	/* parse the value */
	res = alloca(type.size);
	errno = 0;
	switch(type.id) {
		case CONFIG_TYPE_INT:    ret = sscanf(val, "%d\n", (int*) res); break;
		case CONFIG_TYPE_INT64:  ret = sscanf(val, "%lld\n", (long long*) res); break;
		case CONFIG_TYPE_FLOAT:  ret = sscanf(val, "%lf\n", (double*) res); break;
		case CONFIG_TYPE_STRING: ret = strlen(val); res = (volatile void*) &val; break;
		case CONFIG_TYPE_BOOL:
			if(strcasecmp(val, "true") == 0    || strcasecmp(val, "yes") == 0 || strcasecmp(val, "y") == 0 || strcasecmp(val, "t") == 0) {
				ret = strlen(val); *((int*) res) = 1;
			} else if(strcasecmp(val, "false") == 0 || strcasecmp(val, "no") == 0  || strcasecmp(val, "n") == 0 || strcasecmp(val, "f") == 0) {
				ret = strlen(val); *((int*) res) = 0;
			} else {
				ret = sscanf(val, "%d\n", (int*) res);
			}
			break;
	}

	/* ensure scanf actually worked */
	if(errno || ret == 0 || ret == EOF) {
		fprintf(stderr, "Error: sscanf failed to parse value!\n");
		if(errno) fprintf(stderr, "ERRNO %d - %s\n", errno, strerror(errno));
		else if(ret == EOF) fprintf(stderr, "Reached EOF before a conversion!\n");
		else fprintf(stderr, "Was unable to match any bytes!\n");
		return errno ? errno : -1;
	}

	/* store the value */
	ret = type.setting_set(sett, res);
	if(ret != CONFIG_TRUE)
		return print_cfg_err(cfg, "config_setting_set");

	return 0;
}
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
