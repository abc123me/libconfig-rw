#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <libconfig.h>

typedef int (*config_setting_func_t)(config_setting_t*, volatile void*);

struct cfg_type {
	int id;
	const char* str;
	const char* sstr;
	size_t size;
	config_setting_func_t setting_get;
	config_setting_func_t setting_set;
};
typedef struct cfg_type cfg_type_t;

#define _CFG_GET_FUNC(LTYPE) (config_setting_func_t) config_setting_get_##LTYPE##_safe
#define _CFG_SET_FUNC(LTYPE) (config_setting_func_t) _ptr_impl_config_setting_set_##LTYPE
#define _CFG_ENTRY(UTYPE, LTYPE, SHAND, CTYPE) { CONFIG_TYPE_##UTYPE, #LTYPE, #SHAND, sizeof(CTYPE), _CFG_GET_FUNC(LTYPE), _CFG_SET_FUNC(LTYPE) },

#ifdef _CFG_DEFINE_IMPLS
#define _CFG_SET_FUNC_IMPL(LTYPE, PTRTYPE) int _ptr_impl_config_setting_set_##LTYPE(config_setting_t *sett, volatile void *ptr) { return config_setting_set_##LTYPE(sett, *((PTRTYPE) ptr)); }
#else
#define _CFG_SET_FUNC_IMPL(LTYPE, PTRTYPE) int _ptr_impl_config_setting_set_##LTYPE(config_setting_t *sett, volatile void *ptr);
#endif

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
	{ 0, "auto", "a", 0, NULL, NULL },
	{ 0, NULL, NULL, 0, NULL, NULL },
};

int is_type_auto(const cfg_type_t*);
int common_init(const config_t*, const char*, const char*, config_setting_t**, cfg_type_t*);
int write_cfg(const config_t*, char*, char*, char*);
int read_cfg(const config_t*, char*, char*, char*);
int print_cfg_err(const config_t*, const char*);

#endif
