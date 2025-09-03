#define __NO_SWITCH_H
#include "structs.h"

int main(int argc, char** argv) {
    ParentalControlState state = getParentalControlState();
    
    state.active = true;
    printf("Active = %s\n", state.active ? "true" : "false");
    setParentalControlState(state);

    state = getParentalControlState();
    printf("Active = %s\n", state.active ? "true" : "false");
}