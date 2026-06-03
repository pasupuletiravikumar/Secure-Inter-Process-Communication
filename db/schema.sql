-- Consent Management Registry
CREATE TABLE IF NOT EXISTS consent_registry (
    subject_id TEXT PRIMARY KEY,
    consent_granted BOOLEAN NOT NULL,
    authorized_purposes TEXT, -- comma-separated (e.g., "analytics,marketing")
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Privacy-Enabled Audit Log
CREATE TABLE IF NOT EXISTS privacy_audit_log (
    log_id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    process_id INTEGER,
    operator_role TEXT CHECK(operator_role IN ('Admin', 'Auditor', 'User')),
    ipc_channel TEXT CHECK(ipc_channel IN ('SharedMemory', 'NamedPipe', 'MessageQueue', 'Socket')),
    data_classification TEXT CHECK(data_classification IN ('Public', 'Internal', 'Confidential', 'Restricted')),
    sha256_integrity TEXT,
    consent_verified BOOLEAN,
    action_status TEXT CHECK(action_status IN ('SUCCESS', 'DENIED_BY_RBAC', 'DENIED_BY_CONSENT', 'TAMPERED')),
    details TEXT
);

-- Data Subject Rights (DSR) Requests
CREATE TABLE IF NOT EXISTS dsr_requests (
    request_id INTEGER PRIMARY KEY AUTOINCREMENT,
    subject_id TEXT,
    request_type TEXT CHECK(request_type IN ('Access', 'Export', 'Correction', 'Deletion')),
    status TEXT CHECK(status IN ('Pending', 'Processing', 'Completed', 'Rejected')),
    request_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completion_date TIMESTAMP
);
