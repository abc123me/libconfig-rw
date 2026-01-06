#include "main.h"

int read_cfg(const config_t *cfg, char *tstr, char *key, char *val) {
	config_setting_t *sett;
	cfg_type_t type;
	volatile void *res;
	int ret;

	// TODO handle default values
	if(val)
		return ENOTSUP;

	/* get the configuration setting */
	ret = common_init(cfg, (const char*) tstr, (const char*) key, &sett, &type, INIT_FLAGS_READ);
	if(ret)
		return ret;

	/* allocate the value */
	res = alloca(type.size);

	/* read the value */
	ret = type.setting_get(sett, res);

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
