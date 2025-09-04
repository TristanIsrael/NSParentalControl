#include <string>
#include <fstream>
#include <vector>
#include "aes.h"

const char* KEY_FILE = "save:/console_key.bin";

void generateKey(uint8_t* key, size_t len) {
    for (size_t i = 0; i < len; i++)
        key[i] = rand() % 256;
}

/*void encryptFile(const std::string &filename, uint8_t key[32], bool activate_cipher) {
    if (!activate_cipher) return;

    std::ifstream in(filename, std::ios::binary);
    if (!in) return;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    AES_ECB_encrypt(&ctx, data.data());

    std::ofstream out(filename, std::ios::binary);
    out.write((char*)data.data(), data.size());
    out.close();
}

void decryptFile(const std::string &filename, uint8_t key[32], bool activate_cipher) {
    if (!activate_cipher) return;

    std::ifstream in(filename, std::ios::binary);
    if (!in) return;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    AES_ECB_decrypt(&ctx, data.data());

    std::ofstream out(filename, std::ios::binary);
    out.write((char*)data.data(), data.size());
    out.close();
}*/