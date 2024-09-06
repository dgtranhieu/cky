#ifndef SIMPLOG_H
#define SIMPLOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Define logging levels
#define LOG_FATAL    -2    // A fatal error has occured: program will exit immediately
#define LOG_ERROR    -1    // An error has occured: program may not exit
#define LOG_INFO     0     // Nessessary information regarding program operation
#define LOG_WARN     1     // Any circumstance that may not affect normal operation
#define LOG_DEBUG    2     // Standard debug messages
#define LOG_VERBOSE  3     // All debug messages

// Public functions
typedef struct {
    void ( *const writeLog )( int loglvl, const char* str, ... );
    void ( *const writeStackTrace )( void );
    void ( *const setLogDebugLevel )( int level );
    void ( *const setLogFile )( const char* file );
    void ( *const setLogSilentMode )( bool silent );
    void ( *const setLineWrap )( bool wrap );
    void ( *const flushLog )( void );
    void ( *const loadConfig )( const char* config );
} _Logger;
extern _Logger const Logger;

#ifdef __cplusplus
}
#endif

#endif