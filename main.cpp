#include <iostream>
#include <string>
#include <algorithm>
#include "include/ipc_channels.h"
#include "include/cryptography.h"
#include "include/governance.h"
#include "include/compliance.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

void enable_terminal_colors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif
}

void print_hexdump(const string &data) {
    cout << "HEX Payload: ";
    for (unsigned char c : data) {
        printf("%02X ", c);
    }
    printf("\n");
}

int main() {
    enable_terminal_colors();
    
    // Core systems initialization
    GovernanceRegistry gov_registry;
    ComplianceEngine comp_engine;

    cout << "\033[36m=================================================================\033[0m" << endl;
    cout << "\033[36m   SECURE INTER-PROCESS COMMUNICATION & PRIVACY GOVERNANCE HUB   \033[0m" << endl;
    cout << "\033[36m=================================================================\033[0m" << endl;
    
    // Credentials & Authentication
    string username, password;
    cout << "\033[33mUsername: \033[0m";
    cin >> username;
    cout << "\033[33mPassword: \033[0m";
    cin >> password;
    cin.ignore();

    UserRole role = gov_registry.get_role(username);
    string signature = rsa_sign_payload(username, username + "_sig_private_key_of_" + username);

    // Authenticate credentials
    if (!rsa_authenticate_client(username, signature, username + "_rsa_pubkey_2048") || 
        ((username == "Process_Admin" && password != "admin@nexus") &&
         (username == "Process_Auditor" && password != "auditor@nexus") &&
         (username == "Process_User" && password != "user@001"))) {
        
        AuditEntry entry = {
            "2026-06-03 14:25:32", "0", "Unknown", "None", "None", 
            "0000000000000000", false, "DENIED_BY_RBAC", 
            "Authentication failed for identity: " + username
        };
        comp_engine.log_audit_event(entry);
        
        cout << "\033[31mRSA Identity Authentication Failed! Exiting...\033[0m" << endl;
        return 1;
    }

    AuditEntry success_auth = {
        "2026-06-03 14:25:32", "1240", role_to_string(role), "None", "None",
        "0000000000000000", true, "SUCCESS", 
        "Identity verified via RSA-2048. Session initialized."
    };
    comp_engine.log_audit_event(success_auth);

    cout << "\033[32mRSA Certificate Verified. Role: " << role_to_string(role) << "\033[0m\n" << endl;

    int choice = 0;
    while (true) {
        cout << "\033[35m-------------------------------------------------\033[0m" << endl;
        cout << "\033[35m   COMMAND CONSOLE MENU\033[0m" << endl;
        cout << "\033[35m-------------------------------------------------\033[0m" << endl;
        cout << "1. Run IPC Simulation (Send Classified Data)" << endl;
        cout << "2. Run IPC Simulation (Receive Data)" << endl;
        cout << "3. View Privacy Audit Trail (Auditors & Admins)" << endl;
        cout << "4. Generate GDPR / DPDPA Compliance Reports" << endl;
        cout << "5. Data Subject Rights (DSR) Portal" << endl;
        cout << "6. Manage Consent Registry (Admin only)" << endl;
        cout << "7. Enforce Data Retention Policies" << endl;
        cout << "8. Exit Console" << endl;
        cout << "\033[33mSelect Option [1-8]: \033[0m";
        cin >> choice;
        cin.ignore();

        if (choice == 8) break;

        switch (choice) {
            case 1: { // SEND DATA
                int channel_sel = 0;
                cout << "\n\033[36mSelect IPC Channel:\033[0m" << endl;
                cout << "1. Shared Memory\n2. Named Pipe\n3. Message Queue\n4. Sockets" << endl;
                cout << "\033[33mChoice [1-4]: \033[0m";
                cin >> channel_sel;
                cin.ignore();

                int class_sel = 0;
                cout << "\n\033[36mSelect Data Classification Level:\033[0m" << endl;
                cout << "1. Public\n2. Internal\n3. Confidential\n4. Restricted" << endl;
                cout << "\033[33mChoice [1-4]: \033[0m";
                cin >> class_sel;
                cin.ignore();

                DataClassification classification = DataClassification::PUBLIC;
                if (class_sel == 2) classification = DataClassification::INTERNAL;
                if (class_sel == 3) classification = DataClassification::CONFIDENTIAL;
                if (class_sel == 4) classification = DataClassification::RESTRICTED;

                string subject_id;
                cout << "\033[33mEnter Data Subject ID (e.g., Subject_001): \033[0m";
                getline(cin, subject_id);

                string payload;
                cout << "\033[33mEnter message payload: \033[0m";
                getline(cin, payload);

                // --- PRIVACY GOVERNANCE CHECK ---
                bool consent = gov_registry.check_consent(subject_id);
                string ch_name = (channel_sel == 1 ? "SharedMemory" : (channel_sel == 2 ? "NamedPipe" : (channel_sel == 3 ? "MessageQueue" : "Socket")));

                // Consent Verification before transfer
                if (classification != DataClassification::PUBLIC && !consent) {
                    AuditEntry fail_entry = {
                        "2026-06-03 14:25:32", "1240", role_to_string(role), ch_name, 
                        classification_to_string(classification), "0000000000000000", false, 
                        "DENIED_BY_CONSENT", "Blocked transmission of Subject ID: " + subject_id + " (Consent Revoked)"
                    };
                    comp_engine.log_audit_event(fail_entry);
                    cout << "\033[31mError: Transfer blocked. Data subject has revoked consent.\033[0m" << endl;
                    break;
                }

                // RBAC and Data Minimization Filter
                string minimized_payload = gov_registry.apply_data_minimization(payload, classification, role);
                if (minimized_payload == "[REDACTED_RESTRICTED_ACCESS]") {
                    AuditEntry fail_entry = {
                        "2026-06-03 14:25:32", "1240", role_to_string(role), ch_name, 
                        classification_to_string(classification), "0000000000000000", consent, 
                        "DENIED_BY_RBAC", "Blocked process reading restricted payload for Subject ID: " + subject_id
                    };
                    comp_engine.log_audit_event(fail_entry);
                    cout << "\033[31mError: Access Denied. Your role is unauthorized to transmit Restricted data.\033[0m" << endl;
                    break;
                }

                bool minimized_applied = (minimized_payload != payload);

                // Cryptography: SHA-256 Hashing for Integrity
                string payload_hash = generate_sha256(minimized_payload);

                // Cryptography: AES-256-CBC Encryption
                string aes_key = "DemandbaseKey2026@";
                string aes_iv = "DemandbaseIV2026@";
                string encrypted_payload = aes_encrypt(minimized_payload, aes_key, aes_iv);

                // Initialize channel and write
                bool write_ok = false;
                if (channel_sel == 1) {
                    if (init_shared_memory()) write_ok = write_shared_memory(encrypted_payload);
                } else if (channel_sel == 2) {
                    if (init_pipe_channel()) write_ok = write_pipe_channel(encrypted_payload);
                } else if (channel_sel == 3) {
                    if (init_message_queue()) write_ok = write_message_queue(encrypted_payload);
                } else {
                    if (init_socket_channel()) write_ok = write_socket_channel(encrypted_payload);
                }

                if (write_ok) {
                    AuditEntry success_entry = {
                        "2026-06-03 14:25:32", "1240", role_to_string(role), ch_name, 
                        classification_to_string(classification), payload_hash, consent, 
                        "SUCCESS", "Subject ID " + subject_id + " payload written to channel" + (minimized_applied ? " (Data Minimization Applied)" : "")
                    };
                    comp_engine.log_audit_event(success_entry);
                    
                    cout << "\n\033[32mTransmission Succeeded:\033[0m" << endl;
                    cout << "AES Ciphertext: " << encrypted_payload << endl;
                    print_hexdump(encrypted_payload);
                    cout << "SHA-256 Signature: " << payload_hash << endl;
                } else {
                    cout << "\033[31mError: Channel initialization failed or socket write timed out.\033[0m" << endl;
                }
                break;
            }

            case 2: { // RECEIVE DATA
                int channel_sel = 0;
                cout << "\n\033[36mSelect IPC Channel to Read:\033[0m" << endl;
                cout << "1. Shared Memory\n2. Named Pipe\n3. Message Queue\n4. Sockets" << endl;
                cout << "\033[33mChoice [1-4]: \033[0m";
                cin >> channel_sel;
                cin.ignore();

                string encrypted_data = "";
                if (channel_sel == 1) {
                    encrypted_data = read_shared_memory();
                    close_shared_memory();
                } else if (channel_sel == 2) {
                    encrypted_data = read_pipe_channel();
                    close_pipe_channel();
                } else if (channel_sel == 3) {
                    encrypted_data = read_message_queue();
                    close_message_queue();
                } else {
                    encrypted_data = read_socket_channel();
                    close_socket_channel();
                }

                if (!encrypted_data.empty()) {
                    string aes_key = "DemandbaseKey2026@";
                    string aes_iv = "DemandbaseIV2026@";
                    string decrypted_data = aes_decrypt(encrypted_data, aes_key, aes_iv);

                    string payload_hash = generate_sha256(decrypted_data);
                    
                    cout << "\n\033[32mData Decrypted from Channel:\033[0m" << endl;
                    cout << "Raw Ciphertext: " << encrypted_data << endl;
                    cout << "Decrypted String: \033[36m" << decrypted_data << "\033[0m" << endl;
                    cout << "Payload SHA-256: " << payload_hash << endl;
                    
                    AuditEntry read_entry = {
                        "2026-06-03 14:25:32", "2480", role_to_string(role), 
                        (channel_sel == 1 ? "SharedMemory" : (channel_sel == 2 ? "NamedPipe" : (channel_sel == 3 ? "MessageQueue" : "Socket"))),
                        "None", payload_hash, true, "SUCCESS", "Decrypted read session payload successful"
                    };
                    comp_engine.log_audit_event(read_entry);
                } else {
                    cout << "\033[31mNo incoming payloads detected or connection closed.\033[0m" << endl;
                }
                break;
            }

            case 3: { // VIEW AUDIT LOGS
                if (!gov_registry.verify_role_access(username, UserRole::AUDITOR)) {
                    cout << "\033[31mError: Access Denied. Auditors and Admins only.\033[0m" << endl;
                    break;
                }

                cout << "\n\033[36m--- Active Privacy Audit Trail (ipc_log.txt / DB) ---\033[0m" << endl;
                vector<AuditEntry> logs = comp_engine.get_audit_trail();
                for (const auto &log : logs) {
                    cout << "[" << log.timestamp << "] Role: " << log.role 
                         << " | Channel: " << log.channel 
                         << " | Status: " << log.status 
                         << " | Info: " << log.details << endl;
                }
                break;
            }

            case 4: { // GENERATE REPORTS
                if (!gov_registry.verify_role_access(username, UserRole::AUDITOR)) {
                    cout << "\033[31mError: Access Denied. Auditors and Admins only.\033[0m" << endl;
                    break;
                }

                string gdpr_rep, dpdpa_rep;
                comp_engine.verify_gdpr_compliance(gdpr_rep);
                comp_engine.verify_dpdpa_compliance(dpdpa_rep);

                cout << "\n" << gdpr_rep << endl;
                cout << dpdpa_rep << endl;
                break;
            }

            case 5: { // DATA SUBJECT RIGHTS (DSR)
                int dsr_sel = 0;
                cout << "\n\033[36mData Subject Rights (DSR) Actions:\033[0m" << endl;
                cout << "1. Request Access (DSAR)\n2. Request Portability Export\n3. Request Correction\n4. Request Erasure (Right to be Forgotten)" << endl;
                cout << "\033[33mChoice [1-4]: \033[0m";
                cin >> dsr_sel;
                cin.ignore();

                string subject_id;
                cout << "\033[33mEnter Data Subject ID: \033[0m";
                getline(cin, subject_id);

                if (dsr_sel == 1) {
                    cout << "\n" << comp_engine.execute_dsr_access(subject_id) << endl;
                    AuditEntry dsr_log = {
                        "2026-06-03 14:25:32", "1240", role_to_string(role), "None", "None", 
                        "0000000000000000", true, "SUCCESS", "Executed DSR Access Request for Subject: " + subject_id
                    };
                    comp_engine.log_audit_event(dsr_log);
                } 
                else if (dsr_sel == 2) {
                    cout << "\n--- Exported Data Pack (JSON) ---\n" << comp_engine.execute_dsr_export(subject_id) << "\n" << endl;
                    AuditEntry dsr_log = {
                        "2026-06-03 14:25:32", "1240", role_to_string(role), "None", "None", 
                        "0000000000000000", true, "SUCCESS", "Executed DSR Export Request for Subject: " + subject_id
                    };
                    comp_engine.log_audit_event(dsr_log);
                } 
                else if (dsr_sel == 3) {
                    string field, val;
                    cout << "\033[33mEnter field to correct (e.g. email): \033[0m";
                    getline(cin, field);
                    cout << "\033[33mEnter corrected value: \033[0m";
                    getline(cin, val);
                    
                    if (comp_engine.execute_dsr_correction(subject_id, field, val)) {
                        cout << "\033[32mData updated successfully.\033[0m" << endl;
                    } else {
                        cout << "\033[31mNo records found containing target fields.\033[0m" << endl;
                    }
                } 
                else if (dsr_sel == 4) {
                    if (comp_engine.execute_dsr_deletion(subject_id)) {
                        gov_registry.set_consent(subject_id, false); // Erase consent registry settings
                        cout << "\033[32mSuccessfully erased all logs referencing subject " << subject_id << " (Right to be Forgotten enforced).\033[0m" << endl;
                        
                        AuditEntry dsr_log = {
                            "2026-06-03 14:25:32", "1240", role_to_string(role), "None", "None", 
                            "0000000000000000", false, "SUCCESS", "Enforced Erasure (Right to be Forgotten) for Subject: " + subject_id
                        };
                        comp_engine.log_audit_event(dsr_log);
                    } else {
                        cout << "\033[31mNo log occurrences found for subject.\033[0m" << endl;
                    }
                }
                break;
            }

            case 6: { // MANAGE CONSENT REGISTRY
                if (role != UserRole::ADMIN) {
                    cout << "\033[31mError: Access Denied. Admin privilege required.\033[0m" << endl;
                    break;
                }

                string subject_id;
                int consent_val = 0;
                cout << "\033[33mEnter Data Subject ID: \033[0m";
                getline(cin, subject_id);
                cout << "\033[33mSet Consent: [1 = Grant / 0 = Revoke]: \033[0m";
                cin >> consent_val;
                cin.ignore();

                gov_registry.set_consent(subject_id, consent_val == 1);
                
                AuditEntry consent_log = {
                    "2026-06-03 14:25:32", "1240", role_to_string(role), "None", "None", 
                    "0000000000000000", consent_val == 1, "SUCCESS", "Consent setting modified for Subject: " + subject_id
                };
                comp_engine.log_audit_event(consent_log);
                cout << "\033[32mConsent Registry updated.\033[0m" << endl;
                break;
            }

            case 7: { // ENFORCE RETENTION
                comp_engine.enforce_data_retention_policy();
                
                AuditEntry dsr_log = {
                    "2026-06-03 14:25:32", "1240", role_to_string(role), "None", "None", 
                    "0000000000000000", true, "SUCCESS", "Executed automated Data Retention clean sweep."
                };
                comp_engine.log_audit_event(dsr_log);
                cout << "\033[32mData Retention Sweep successfully completed. Outdated audit records truncated.\033[0m" << endl;
                break;
            }

            default:
                cout << "\033[31mInvalid selection.\033[0m" << endl;
                break;
        }
    }

    cout << "\033[36mSession ended. Secure shutdown initiated.\033[0m" << endl;
    return 0;
}
