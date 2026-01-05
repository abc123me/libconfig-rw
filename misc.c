#include "main.h"

int type_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	config_setting_t *sett;
	cfg_type_t type;
	int ret;

	/* get the configuration setting */
	ret = common_init(cfg, (const char*) tstr, (const char*) key, &sett, &type);
	if(ret)
		return ret;

	/* print the type */
	printf("%s\n", type.str);
	return 0;
}

int dump_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	return ENOTSUP;
}
