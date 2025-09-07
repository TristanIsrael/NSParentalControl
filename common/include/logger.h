#include <switch.h>
#include <stdio.h>

void logToFile(const char* msg) {
    fsdevMountSdmc();

    FILE* f = fopen("sdmc:/atmosphere/logs/nsparentalcontrol.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }

    fsdevUnmountDevice("sdmc");
}