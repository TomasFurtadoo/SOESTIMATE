#ifndef CLOG_H_GUARD
#define CLOG_H_GUARD

struct info_container;
struct MeasurementInfo;

void log_operation(char *operation, struct info_container *info);

void log_result(int server_id, struct MeasurementInfo *info, struct info_container *container);

void clear_log(struct info_container *container);

double read_j(int m_id, struct info_container *info);

#endif