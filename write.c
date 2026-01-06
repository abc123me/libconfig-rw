#include "main.h"

int write_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	config_setting_t *sett;
	cfg_type_t type;
	volatile void *res;
	int ret;

	/* get the configuration setting */
	ret = common_init(cfg, (const char*) tstr, (const char*) key, &sett, &type, INIT_FLAGS_WRITE);
	if(ret)
		return ret;

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
