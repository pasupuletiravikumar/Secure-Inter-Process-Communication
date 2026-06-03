#include "../include/governance.h"
#include "../include/cryptography.h"
#include <iostream>
#include <algorithm>

std::string classification_to_string(DataClassification classification) {
    switch(classification) {
        case DataClassification::PUBLIC: return "Public";
        case DataClassification::INTERNAL: return "Internal";
        case DataClassification::CONFIDENTIAL: return "Confidential";
        case DataClassification::RESTRICTED: return "Restricted";
    }
    return "Unknown";
}

std::string role_to_string(UserRole role) {
    switch(role) {
        case UserRole::ADMIN: return "Admin";
        case UserRole::AUDITOR: return "Auditor";
        case UserRole::USER: return "User";
    }
    return "Unknown";
}

GovernanceRegistry::GovernanceRegistry() {
    // Populate default authorized systems / client process keys
    register_identity("Process_Admin", UserRole::ADMIN, "admin_rsa_pubkey_2048");
    register_identity("Process_Auditor", UserRole::AUDITOR, "auditor_rsa_pubkey_2048");
    register_identity("Process_User", UserRole::USER, "user_rsa_pubkey_2048");

    // Populate default consent records for data subjects
    set_consent("Subject_001", true);
    set_consent("Subject_002", false); // Denied consent
    set_consent("Subject_003", true);
}

void GovernanceRegistry::set_consent(const std::string &subject_id, bool granted) {
    consent_db[subject_id] = granted;
}

bool GovernanceRegistry::check_consent(const std::string &subject_id) {
    // Privacy-by-Design Default: If subject not found, default to false (opt-in consent)
    auto it = consent_db.find(subject_id);
    if (it != consent_db.end()) {
        return it->second;
    }
    return false; // Consent default is FALSE
}

void GovernanceRegistry::register_identity(const std::string &id, UserRole role, const std::string &pubkey) {
    rbac_roles[id] = role;
    rbac_keys[id] = pubkey;
}

UserRole GovernanceRegistry::get_role(const std::string &id) {
    auto it = rbac_roles.find(id);
    if (it != rbac_roles.end()) {
        return it->second;
    }
    return UserRole::USER; // Default to lowest privilege role
}

bool GovernanceRegistry::verify_role_access(const std::string &id, UserRole required_role) {
    UserRole current = get_role(id);
    if (required_role == UserRole::ADMIN) {
        return current == UserRole::ADMIN;
    }
    if (required_role == UserRole::AUDITOR) {
        return current == UserRole::ADMIN || current == UserRole::AUDITOR;
    }
    return true; // USER has access to basic operations
}

// Data Minimization Engine
std::string GovernanceRegistry::apply_data_minimization(const std::string &payload, DataClassification classification, UserRole role) {
    if (classification == DataClassification::PUBLIC) {
        return payload; // Public data requires no minimization
    }

    if (role == UserRole::ADMIN) {
        return payload; // Admins have full access
    }

    // Confidential Data
    if (classification == DataClassification::CONFIDENTIAL) {
        if (role == UserRole::AUDITOR) {
            return payload; // Auditors see confidential audit trails
        }
        // Users get minimized payload (PII masking)
        std::string result = payload;
        // Simple scan for emails (e.g. contain '@') and mask them
        size_t at_pos = result.find('@');
        if (at_pos != std::string::npos) {
            size_t start = result.rfind(' ', at_pos);
            if (start == std::string::npos) start = 0;
            else start += 1;
            size_t end = result.find(' ', at_pos);
            if (end == std::string::npos) end = result.size();
            
            // Mask email as [PII_MASKED]
            result.replace(start, end - start, "[PII_MASKED]");
        }
        return result;
    }

    // Restricted Data (Highest risk, e.g., SSN, Passwords, Financials)
    if (classification == DataClassification::RESTRICTED) {
        if (role == UserRole::AUDITOR) {
            // Pseudonymize payload using SHA-256 for integrity auditing without exposing raw values
            return "PSEUDONYMOUS_HASH:" + generate_sha256(payload);
        }
        // Users are completely blocked from seeing restricted content
        return "[REDACTED_RESTRICTED_ACCESS]";
    }

    return "[ACCESS_DENIED]";
}
