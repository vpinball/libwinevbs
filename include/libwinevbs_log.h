#ifndef LIBWINEVBS_LOG_H
#define LIBWINEVBS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
   LIBWINEVBS_LOG_INFO,
   LIBWINEVBS_LOG_DEBUG,
   LIBWINEVBS_LOG_WARN,
   LIBWINEVBS_LOG_ERROR
} libwinevbs_log_level_t;

void external_log(libwinevbs_log_level_t level, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
