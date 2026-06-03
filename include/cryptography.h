#ifndef CRYPTOGRAPHY_H
#define CRYPTOGRAPHY_H

#include <string>

// Symmetric AES-256-CBC Simulation (Scrambles payload using AES state block shuffling)
std::string aes_encrypt(const std::string &plaintext, const std::string &key, const std::string &iv);
std::string aes_decrypt(const std::string &ciphertext, const std::string &key, const std::string &iv);

// Asymmetric RSA-2048 Simulation (Keys authentication)
bool rsa_authenticate_client(const std::string &client_id, const std::string &signature, const std::string &public_key);
std::string rsa_sign_payload(const std::string &payload, const std::string &private_key);

// SHA-256 Integrity Verification (FIPS 180-4 standard compliant SHA-256 helper)
std::string generate_sha256(const std::string &data);

#endif // CRYPTOGRAPHY_H
