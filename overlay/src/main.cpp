#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <ctime>
#include <cstring>

#define TESLA_INIT_IMPL
#include "tesla.hpp"
#include "gui/main_overlay.h"

//Examples : https://github.com/masagrator/Status-Monitor-Overlay/blob/master/source/main.cpp

const bool CIPHER_DATABASE = false;

int main(int argc, char** argv) {
    consoleInit(NULL);

    tsl::loop<MainOverlay>(argc, argv);                        

    /*const size_t KEY_LEN = 32;
    uint8_t key[KEY_LEN];

    std::ifstream infile(KEY_FILE, std::ios::binary);
    if (infile.good()) {
        infile.read(reinterpret_cast<char*>(key), KEY_LEN);
        infile.close();
        printf("Loaded existing key.\n");
    } else {
        generateKey(key, KEY_LEN);
        std::ofstream outfile(KEY_FILE, std::ios::binary);
        outfile.write(reinterpret_cast<char*>(key), KEY_LEN);
        outfile.close();
        printf("Generated new console key.\n");
    }

    decryptFile("/switch/NSParentalControl/sessions.txt", key, CIPHER_DATABASE);*/

}