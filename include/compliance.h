#ifndef COMPLIANCE_H
#define COMPLIANCE_H

#include <string>
#include <vector>
#include "governance.h"

// Log struct for in-memory / SQLite logging
struct AuditEntry {
    std::string timestamp;
    std::string process_id;
    std::string role;
    std::string channel;
    std::string classification;
    std::string hash;
    bool consent_verified;
    std::string status; // SUCCESS, DENIED_BY_RBAC, DENIED_BY_CONSENT
    std::string details;
};

class ComplianceEngine {
private:
    std::vector<AuditEntry> audit_log;
    int retention_days; // Default 30 days

public:
    ComplianceEngine();

    // Audit Log Generation
    void log_audit_event(const AuditEntry &entry);
    void save_log_to_file();
    std::vector<AuditEntry> get_audit_trail();

    // Data Subject Rights (DSR) Actions
    std::string execute_dsr_access(const std::string &subject_id);
    std::string execute_dsr_export(const std::string &subject_id);
    bool execute_dsr_correction(const std::string &subject_id, const std::string &field, const std::string &new_val);
    bool execute_dsr_deletion(const std::string &subject_id);

    // Compliance Framework Verification
    bool verify_gdpr_compliance(std::string &report_output);
    bool verify_dpdpa_compliance(std::string &report_output);
    
    // Data Retention Policies
    void enforce_data_retention_policy();
};

#endif // COMPLIANCE_H
