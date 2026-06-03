#ifndef GOVERNANCE_H
#define GOVERNANCE_H

#include <string>
#include <vector>
#include <map>

// Data Classification
enum class DataClassification {
    PUBLIC,
    INTERNAL,
    CONFIDENTIAL,
    RESTRICTED
};

// Role-Based Access Control (RBAC)
enum class UserRole {
    ADMIN,
    AUDITOR,
    USER
};

// Convert enums to string representation
std::string classification_to_string(DataClassification classification);
std::string role_to_string(UserRole role);

// Governance Systems
class GovernanceRegistry {
private:
    std::map<std::string, bool> consent_db; // subject_id -> consent_granted
    std::map<std::string, std::string> rbac_keys; // process/user -> public_key
    std::map<std::string, UserRole> rbac_roles; // process/user -> UserRole

public:
    GovernanceRegistry();
    
    // Consent Management
    void set_consent(const std::string &subject_id, bool granted);
    bool check_consent(const std::string &subject_id);
    
    // RBAC Administration
    void register_identity(const std::string &id, UserRole role, const std::string &pubkey);
    bool verify_role_access(const std::string &id, UserRole required_role);
    UserRole get_role(const std::string &id);

    // Data Minimization Filter
    std::string apply_data_minimization(const std::string &payload, DataClassification classification, UserRole role);
};

#endif // GOVERNANCE_H
