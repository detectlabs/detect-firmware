#include <cstdlib>

#include "micro_error_reporter_connector.h"
#include "micro_error_reporter.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>

static tflite::ErrorReporter *error_reporter = NULL;

void lazyMER() {
    if (error_reporter == NULL) {
        tflite::MicroErrorReporter micro_error_reporter;
        error_reporter = &micro_error_reporter;
    }
}

int MicroErrorReporter_Report() {
    lazyMER();

    int code = error_reporter->Report("Number: %d", 4);
    return code;
}

#ifdef __cplusplus
}
#endif