#ifndef CSETTINGS_H_GUARD
#define CSETTINGS_H_GUARD

struct info_container;

void read_arguments_file(const char *filename, struct info_container *info);

void read_settings_file(const char *filename, struct info_container *info);

#endif