#include "main.h"

int type_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	config_setting_t *sett;
	cfg_type_t type;
	int ret;

	/* get the configuration setting */
	ret = common_init(cfg, tstr, key, &sett, &type, INIT_FLAGS_READ);
	if(ret)
		return ret;

	/* print the type */
	printf("%s\n", type.str);
	return 0;
}

int delete_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	config_setting_t *sett;
	int ret;

	/* grab root configuration */
	sett = config_root_setting(cfg);
	if(sett == NULL)
		return print_cfg_err(cfg, "config_root_setting");

	/* delete the value */
	ret = config_setting_remove(sett, key);
	if(ret != CONFIG_TRUE)
		return print_cfg_err(cfg, "config_setting_remove");

	return 0;
}

int dump_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	return ENOTSUP;
}
