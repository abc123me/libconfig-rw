#ifndef _MAIN_H
#define _MAIN_H

#include <libconfig.h>

typedef int (*config_lookup_func_t)(const config_t*, const char*, volatile void*);
typedef int (*config_setting_set_func_t)(config_setting_t*, volatile void*);

struct cfg_type {
	int id;
	const char* str;
	const char* sstr;
	size_t size;
	config_lookup_func_t lookup;
	config_setting_set_func_t setting_set;
};
typedef struct cfg_type cfg_type_t;

#endif
