// ============================================================================
// DATA & STATE REGISTRIES
// ============================================================================
let currentRole = "Admin";
let activeTech = "shm";
let totalCommunications = 0;
let privacyIncidents = 0;
let failedAuthAttempts = 0;

// Data Subjects Consent DB
let subjectsRegistry = [
    { id: "Subject_001", name: "John Doe", consent: true, purposes: "B2B Analytics, Outreach Targeting" },
    { id: "Subject_002", name: "Alice Smith", consent: false, purposes: "None" },
    { id: "Subject_003", name: "Bob Johnson", consent: true, purposes: "Lead Scoring" }
];

// Audit Trails (Simulation Database)
let systemAuditTrail = [
    { timestamp: "14:20:11", processId: "1024", role: "Admin", channel: "SharedMemory", classification: "Internal", hash: "8A4C62B0E19A", consentVerified: true, status: "SUCCESS", details: "Process mapping configuration table loaded." },
    { timestamp: "14:22:45", processId: "1480", role: "User", channel: "Socket", classification: "Public", hash: "9D4A1B2E3F4C", consentVerified: true, status: "SUCCESS", details: "Public contact record shared for marketing validation." }
];

// SHA-256 JS Emulator
function getSHA256(str) {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
        hash = (hash << 5) - hash + str.charCodeAt(i);
        hash |= 0; 
    }
    return Math.abs(hash).toString(16).toUpperCase().padStart(8, '0') + "A9F4C2D81B5E87C2";
}

// AES-256 JS Cipher Emulator
function getAESCipher(plaintext, key) {
    if (!key) return plaintext;
    let cipher = "";
    for (let i = 0; i < plaintext.length; i++) {
        let charCode = plaintext.charCodeAt(i);
        let keyChar = key.charCodeAt(i % key.length);
        let encCode = (charCode + keyChar + i) % 256;
        cipher += String.fromCharCode(encCode);
    }
    // Hex encode ciphertext payload
    return btoa(cipher).substring(0, 32) + "...[AES_256_CBC]";
}

// ============================================================================
// INITIALIZATION
// ============================================================================
document.addEventListener("DOMContentLoaded", () => {
    // Nav Pane Switching
    const navButtons = document.querySelectorAll(".nav-btn");
    navButtons.forEach(btn => {
        btn.addEventListener("click", () => {
            navButtons.forEach(b => b.classList.remove("active"));
            btn.classList.add("active");
            
            const targetPane = btn.getAttribute("data-pane");
            document.querySelectorAll(".pane-content").forEach(pane => {
                pane.classList.remove("active");
            });
            document.getElementById(`pane-${targetPane}`).classList.add("active");
        });
    });

    // Login Submission
    const loginForm = document.getElementById("login-form");
    loginForm.addEventListener("submit", (e) => {
        e.preventDefault();
        const username = document.getElementById("username").value;
        const pass = document.getElementById("password").value;
        
        let role = "";
        if (username === "Process_Admin" && pass === "admin@nexus") role = "Admin";
        else if (username === "Process_Auditor" && pass === "auditor@nexus") role = "Auditor";
        else if (username === "Process_User" && pass === "user@001") role = "User";

        if (role) {
            currentRole = role;
            document.getElementById("auth-username").innerText = username;
            changeRoleContext(role);
            
            document.getElementById("login-overlay").classList.add("hidden");
            document.getElementById("app-container").classList.remove("hidden");
            
            // Log Successful Auth
            addConsoleLine("success", `RSA identity certificate verified for '${username}'. Session token generated under role [${role}].`);
            
            // Populates dropdowns
            updateRegistries();
        } else {
            failedAuthAttempts++;
            document.getElementById("metric-failed-auth").innerText = failedAuthAttempts;
            document.getElementById("login-error").classList.remove("hidden");
            addConsoleLine("error", `RSA Authentication failure count increased. Failed attempt count: ${failedAuthAttempts}`);
        }
    });

    // Logout
    document.getElementById("logout-btn").addEventListener("click", () => {
        document.getElementById("app-container").classList.add("hidden");
        document.getElementById("login-overlay").classList.remove("hidden");
        document.getElementById("password").value = "";
        document.getElementById("login-error").classList.add("hidden");
    });

    // Generate RAM Grid
    buildShmGrid();
    updateRegistries();
});

// ============================================================================
// REGISTRY & DROPDOWN BUILDERS
// ============================================================================
function updateRegistries() {
    // Render Consent Registry Table
    const tbody = document.getElementById("consent-table-body");
    tbody.innerHTML = "";
    subjectsRegistry.forEach(sub => {
        const tr = document.createElement("tr");
        tr.innerHTML = `
            <td><strong>${sub.id}</strong></td>
            <td>${sub.name}</td>
            <td>
                <span class="badge ${sub.consent ? 'badge-emerald' : 'badge-pink'}">
                    ${sub.consent ? 'CONSENT GRANTED' : 'CONSENT REVOKED'}
                </span>
            </td>
            <td><code>${sub.purposes}</code></td>
            <td>
                <button class="btn btn-secondary btn-sm" onclick="toggleConsentState('${sub.id}')">
                    <i class="fa-solid fa-rotate"></i> Toggle
                </button>
            </td>
        `;
        tbody.appendChild(tr);
    });

    // Populate drop downs
    const simSelect = document.getElementById("sim-subject");
    const dsrSelect = document.getElementById("dsr-subject-select");
    simSelect.innerHTML = "";
    dsrSelect.innerHTML = "";
    
    subjectsRegistry.forEach(sub => {
        simSelect.innerHTML += `<option value="${sub.id}">${sub.id} (${sub.name})</option>`;
        dsrSelect.innerHTML += `<option value="${sub.id}">${sub.id} (${sub.name})</option>`;
    });

    // Update Dashboard Metrics
    document.getElementById("metric-comms").innerText = totalCommunications;
    document.getElementById("metric-incidents").innerText = privacyIncidents;
    
    const approvedCount = subjectsRegistry.filter(s => s.consent).length;
    const consentRate = Math.round((approvedCount / subjectsRegistry.length) * 100);
    document.getElementById("metric-consent-rate").innerText = `${consentRate}%`;
}

function toggleConsentState(subjectId) {
    if (currentRole !== "Admin") {
        alert("Access Denied: Only Admins can modify the Consent Registry.");
        return;
    }
    const subject = subjectsRegistry.find(s => s.id === subjectId);
    if (subject) {
        subject.consent = !subject.consent;
        if (!subject.consent) {
            subject.purposes = "None";
        } else {
            subject.purposes = "B2B Analytics, Outreach Targeting";
        }
        addConsoleLine("warning", `Consent Registry modified: Subject '${subjectId}' toggled consent to [${subject.consent}].`);
        updateRegistries();
    }
}

function addNewSubject() {
    if (currentRole !== "Admin") {
        alert("Access Denied: Only Admins can add data subjects.");
        return;
    }
    const name = prompt("Enter new subject name:");
    if (!name) return;
    const newId = `Subject_00${subjectsRegistry.length + 1}`;
    subjectsRegistry.push({
        id: newId,
        name: name,
        consent: true,
        purposes: "Analytics, Marketing"
    });
    addConsoleLine("success", `Registered new data subject: '${newId}' (${name}) with opt-in defaults.`);
    updateRegistries();
}

// ============================================================================
// RBAC ROLE SWITCHER
// ============================================================================
function changeRoleContext(role) {
    currentRole = role;
    
    // Update role badges & pill
    const pill = document.getElementById("role-pill");
    pill.innerText = role;
    pill.className = `role-badge badge-${role.toLowerCase()}`;
    
    // Update visual state buttons
    document.querySelectorAll(".role-select-btn").forEach(btn => btn.classList.remove("active"));
    document.getElementById(`btn-role-${role.toLowerCase()}`).classList.add("active");

    // Enforce data minimization settings visual
    addConsoleLine("system", `System privileges adjusted. Active role: ${role}. Enforcing least privilege boundary.`);
}

// ============================================================================
// SHM GRID BUILDER
// ============================================================================
function buildShmGrid() {
    const shmGrid = document.getElementById("stage-shm-grid");
    shmGrid.innerHTML = "";
    for (let i = 0; i < 48; i++) {
        shmGrid.innerHTML += `<div class="shm-cell" id="shm-cell-${i}">00</div>`;
    }
}

// ============================================================================
// SIMULATION PIPELINE
// ============================================================================
function selectTech(tech) {
    activeTech = tech;
    document.querySelectorAll(".tech-card").forEach(c => c.classList.remove("active"));
    document.getElementById(`tech-${tech}`).classList.add("active");

    // Toggle panels
    if (tech === "shm") {
        document.getElementById("stage-visualizer-shm").classList.remove("hidden");
        document.getElementById("stage-visualizer-conduit").classList.add("hidden");
    } else {
        document.getElementById("stage-visualizer-shm").classList.add("hidden");
        document.getElementById("stage-visualizer-conduit").classList.remove("hidden");
        
        const conduitTitle = document.getElementById("conduit-channel-title");
        if (tech === "pipe") conduitTitle.innerHTML = `<i class="fa-solid fa-arrow-right-arrow-left"></i> Active Pipeline: Named Pipe (\\\\.\\pipe\\SecureIPC_NamedPipe)`;
        if (tech === "mq") conduitTitle.innerHTML = `<i class="fa-solid fa-list-ol"></i> Active Pipeline: System V Message Queue`;
        if (tech === "socket") conduitTitle.innerHTML = `<i class="fa-solid fa-ethernet"></i> Active Pipeline: Socket (TCP loopback port 9090)`;
    }
}

function updateClassificationHint() {
    const selection = document.getElementById("sim-classification").value;
    const hint = document.getElementById("class-hint");
    if (selection === "Public") hint.innerText = "Public data requires no privacy controls.";
    if (selection === "Internal") hint.innerText = "Internal data is restricted to internal processes; contains no sensitive attributes.";
    if (selection === "Confidential") hint.innerText = "Confidential data. Names/Emails will mask automatically for standard Users.";
    if (selection === "Restricted") hint.innerText = "Restricted data. Completely blocked for Users; Auditor receives pseudonymized hash.";
}

function toggleEavesdropVisual() {
    const isChecked = document.getElementById("chk-eavesdrop").checked;
    const advNode = document.getElementById("sim-node-adversary");
    if (isChecked) {
        advNode.classList.remove("hidden");
    } else {
        advNode.classList.add("hidden");
    }
}

function triggerSimulation() {
    const classification = document.getElementById("sim-classification").value;
    const subjectId = document.getElementById("sim-subject").value;
    const payload = document.getElementById("sim-payload").value;
    const key = document.getElementById("sim-aes-key").value;
    const verifyConsent = document.getElementById("chk-verify-consent").checked;
    const eavesdrop = document.getElementById("chk-eavesdrop").checked;

    const subject = subjectsRegistry.find(s => s.id === subjectId);
    
    // --- 1. PRIVACY BY DESIGN: CONSENT CHECK ---
    if (verifyConsent && classification !== "Public" && (!subject || !subject.consent)) {
        privacyIncidents++;
        updateRegistries();
        addConsoleLine("error", `[PRIVACY VETO] Blocked IPC write. Data Subject '${subjectId}' has revoked processing consent!`);
        animateBlocked("bubble-sender", "Consent Denied");
        
        logSimulationEvent(subjectId, "DENIED_BY_CONSENT", "Blocked transmission of Subject ID: " + subjectId + " (Consent Revoked)");
        return;
    }

    // --- 2. PRIVACY BY DESIGN: RBAC & DATA MINIMIZATION FILTER ---
    let finalPayload = payload;
    let minimizationNotice = "";
    
    if (classification === "Confidential") {
        if (currentRole === "User") {
            // Masking emails or target sensitive terms
            finalPayload = payload.replace(/[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}/g, "[PII_MASKED]");
            minimizationNotice = " (Data Minimization applied: E-mails masked)";
            addConsoleLine("warning", `[DATA MINIMIZATION] Masked email fields in Confidential payload before serialization.`);
        }
    } else if (classification === "Restricted") {
        if (currentRole === "User") {
            privacyIncidents++;
            updateRegistries();
            addConsoleLine("error", `[RBAC DENIAL] Role 'User' lacks permission to serialize 'Restricted' data. Transfer terminated.`);
            animateBlocked("bubble-sender", "Access Denied (RBAC)");
            
            logSimulationEvent(subjectId, "DENIED_BY_RBAC", "Blocked process reading restricted payload for Subject ID: " + subjectId);
            return;
        } else if (currentRole === "Auditor") {
            // Pseudonymize payload
            finalPayload = "PSEUDONYMOUS_HASH:" + getSHA256(payload).substring(0, 16);
            minimizationNotice = " (Data Pseudonymization applied for Auditor)";
            addConsoleLine("warning", `[DATA PSEUDONYMIZATION] Pseudonymized restricted payload using SHA-256 for Auditor role.`);
        }
    }

    // --- 3. CRYPTOGRAPHY AND SERIALIZATION ---
    // Hashing
    const hash = getSHA256(finalPayload);
    // Encryption
    const ciphertext = getAESCipher(finalPayload, key);

    addConsoleLine("success", `[ENCRYPTION] Data classified [${classification}]. Signed via RSA, hashed SHA-256, encrypted AES-256-CBC.`);
    
    totalCommunications++;
    document.getElementById("metric-comms").innerText = totalCommunications;

    // Log the successful transfer in database
    logSimulationEvent(subjectId, "SUCCESS", `Subject ID ${subjectId} payload written to channel${minimizationNotice}`);

    // Trigger UI animation based on technology
    document.getElementById("sim-state-badge").innerText = "WRITING...";
    document.getElementById("sim-state-badge").className = "badge badge-cyan";

    if (activeTech === "shm") {
        // Shared Memory Animation
        document.getElementById("shm-lock-badge").innerText = "MUTEX LOCKED";
        document.getElementById("shm-lock-badge").className = "badge badge-pink";
        
        let counter = 0;
        const interval = setInterval(() => {
            const idx = Math.floor(Math.random() * 48);
            const cell = document.getElementById(`shm-cell-${idx}`);
            cell.classList.add("active-write");
            cell.innerText = Math.floor(Math.random() * 256).toString(16).toUpperCase().padStart(2, '0');
            counter++;
            if (counter > 15) {
                clearInterval(interval);
                
                // Show bubble
                showBubble("bubble-sender", "WRITE: Encrypted payload serialized.");
                
                setTimeout(() => {
                    document.getElementById("sim-state-badge").innerText = "READING...";
                    document.getElementById("sim-state-badge").className = "badge badge-emerald";
                    
                    // Receiver reads
                    document.querySelectorAll(".shm-cell").forEach(c => c.classList.remove("active-write"));
                    for (let j = 0; j < 10; j++) {
                        const rIdx = Math.floor(Math.random() * 48);
                        document.getElementById(`shm-cell-${rIdx}`).classList.add("active-read");
                    }
                    
                    showBubble("bubble-receiver", `DECRYPTED: "${finalPayload}"`);
                    
                    // Reset grid locks
                    setTimeout(() => {
                        document.getElementById("shm-lock-badge").innerText = "MUTEX UNLOCKED";
                        document.getElementById("shm-lock-badge").className = "badge badge-emerald";
                        document.getElementById("sim-state-badge").innerText = "IDLE";
                        document.getElementById("sim-state-badge").className = "badge";
                        document.querySelectorAll(".shm-cell").forEach(c => {
                            c.classList.remove("active-read");
                        });
                    }, 1200);
                }, 1000);
            }
        }, 50);

    } else {
        // Conduit Animation (Named Pipe, MQ, Sockets)
        const packet = document.getElementById("conduit-packet");
        packet.style.left = "45px";
        packet.style.opacity = "1";
        
        document.getElementById("conduit-packet-text").innerText = "CIPHERTEXT";
        showBubble("bubble-sender", "SENDING PACKET...");

        // Packet motion
        let position = 45;
        const anim = setInterval(() => {
            position += 8;
            packet.style.left = position + "px";
            
            // Check eavesdropper interception mid-way
            if (eavesdrop && position > 250 && position < 270) {
                showBubble("bubble-adversary", eavesdropSniff(ciphertext, finalPayload, key));
            }

            if (position > 520) {
                clearInterval(anim);
                packet.style.opacity = "0";
                showBubble("bubble-receiver", `DECRYPTED: "${finalPayload}"`);
                document.getElementById("sim-state-badge").innerText = "IDLE";
                document.getElementById("sim-state-badge").className = "badge";
            }
        }, 30);
    }
}

function eavesdropSniff(ciphertext, plaintext, key) {
    // If AES is correct, adversary only receives high entropy scrambled ciphertext
    return `Sniffed: "${ciphertext}"`;
}

function showBubble(id, text) {
    const bubble = document.getElementById(id);
    bubble.innerText = text;
    bubble.style.opacity = "1";
    bubble.style.transform = "scale(1)";
    setTimeout(() => {
        bubble.style.opacity = "0";
        bubble.style.transform = "scale(0.8)";
    }, 2800);
}

function animateBlocked(id, text) {
    const bubble = document.getElementById(id);
    bubble.innerText = text;
    bubble.style.opacity = "1";
    bubble.style.transform = "scale(1)";
    bubble.style.backgroundColor = "var(--pink-dim)";
    bubble.style.borderColor = "var(--pink)";
    bubble.style.color = "var(--pink)";
    
    setTimeout(() => {
        bubble.style.opacity = "0";
        bubble.style.transform = "scale(0.8)";
        bubble.style.backgroundColor = "rgba(0, 240, 255, 0.2)";
        bubble.style.borderColor = "var(--cyan)";
        bubble.style.color = "var(--text-primary)";
    }, 2000);
}

// Logging helper
function logSimulationEvent(subject, status, details) {
    const date = new Date();
    const timeStr = `${date.getHours().toString().padStart(2, '0')}:${date.getMinutes().toString().padStart(2, '0')}:${date.getSeconds().toString().padStart(2, '0')}`;
    const classification = document.getElementById("sim-classification").value;
    
    const newLog = {
        timestamp: timeStr,
        processId: "1240",
        role: currentRole,
        channel: activeTech.toUpperCase(),
        classification: classification,
        hash: getSHA256(details).substring(0, 12),
        consentVerified: subjectsRegistry.find(s => s.id === subject)?.consent || false,
        status: status,
        details: `Subject: ${subject} | ${details}`
    };

    systemAuditTrail.push(newLog);

    // Write to UI console log
    let type = status === "SUCCESS" ? "success" : "error";
    addConsoleLine(type, `[IPC WRITE] Channel: ${activeTech.toUpperCase()} | Class: ${classification} | Status: ${status} | Info: ${details}`);
}

function addConsoleLine(type, text) {
    const consoleBox = document.getElementById("sim-console");
    const date = new Date();
    const timeStr = `${date.getHours().toString().padStart(2, '0')}:${date.getMinutes().toString().padStart(2, '0')}:${date.getSeconds().toString().padStart(2, '0')}`;
    
    consoleBox.innerHTML += `<div class="console-line ${type}"><span class="c-time">[${timeStr}]</span> ${text}</div>`;
    consoleBox.scrollTop = consoleBox.scrollHeight;
}

function clearSimLogs() {
    document.getElementById("sim-console").innerHTML = "";
    addConsoleLine("system", "Trace log cleared by console operator.");
}

// ============================================================================
// DATA SUBJECT RIGHTS (DSR) ACTIONS
// ============================================================================
document.getElementById("dsr-type-select").addEventListener("change", (e) => {
    const type = e.target.value;
    const correctionBox = document.getElementById("dsr-correction-fields");
    if (type === "Correction") {
        correctionBox.classList.remove("hidden");
    } else {
        correctionBox.classList.add("hidden");
    }
});

function submitDsrRequest() {
    const subjectId = document.getElementById("dsr-subject-select").value;
    const type = document.getElementById("dsr-type-select").value;
    const execBox = document.getElementById("dsr-exec-box");
    const subject = subjectsRegistry.find(s => s.id === subjectId);

    if (!subject) return;

    execBox.innerHTML = "";
    addDsrLine("info", `Initiating Data Subject Right Request: [Right of ${type}] for subject ID: ${subjectId}...`);

    setTimeout(() => {
        if (type === "Access") {
            // Find occurrences in audit trail
            const matches = systemAuditTrail.filter(l => l.details.includes(subjectId));
            addDsrLine("success", `[GDPR Article 15 DSAR] Located ${matches.length} transaction instances in system log registry:`);
            matches.forEach(m => {
                addDsrLine("text", `  - [${m.timestamp}] Channel: ${m.channel} | Status: ${m.status} | Details: ${m.details}`);
            });
        } 
        
        else if (type === "Export") {
            // Generate portablity data pack
            const matches = systemAuditTrail.filter(l => l.details.includes(subjectId));
            const exportData = {
                subjectId: subjectId,
                name: subject.name,
                consentRecord: subject.consent,
                history: matches
            };
            addDsrLine("success", `[GDPR Article 20 Portability] Structured data pack successfully exported (JSON format):`);
            addDsrLine("code", JSON.stringify(exportData, null, 2));
            
            // Trigger simulated download
            const blob = new Blob([JSON.stringify(exportData, null, 2)], {type : 'application/json'});
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `DSR_Export_${subjectId}.json`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
        } 
        
        else if (type === "Correction") {
            const val = document.getElementById("dsr-corrected-text").value;
            if (!val) {
                addDsrLine("error", "Error: Correction value cannot be blank.");
                return;
            }
            const oldName = subject.name;
            subject.name = val;
            addDsrLine("success", `[GDPR Article 16 Rectification] Corrected identity name fields for ${subjectId}: '${oldName}' updated to '${val}'.`);
            updateRegistries();
        } 
        
        else if (type === "Erasure") {
            // Delete subject records
            const beforeCount = systemAuditTrail.length;
            systemAuditTrail = systemAuditTrail.filter(l => !l.details.includes(subjectId));
            const afterCount = systemAuditTrail.length;
            
            subject.consent = false;
            subject.purposes = "None";
            
            addDsrLine("success", `[GDPR Article 17 Right to Erasure / Forgotten]:`);
            addDsrLine("success", `  - Purged ${beforeCount - afterCount} transaction log traces matching Subject ID: ${subjectId}.`);
            addDsrLine("success", `  - Set registry consent to FALSE & disabled target pipeline hooks.`);
            updateRegistries();
        }
    }, 800);
}

function addDsrLine(type, text) {
    const box = document.getElementById("dsr-exec-box");
    if (type === "code") {
        box.innerHTML += `<pre style="color:var(--cyan); font-size:0.75rem; background:rgba(0,0,0,0.4); padding:10px; border-radius:5px; margin-top:8px;">${text}</pre>`;
    } else if (type === "error") {
        box.innerHTML += `<div class="dsr-terminal-line text-pink"><i class="fa-solid fa-triangle-exclamation"></i> ${text}</div>`;
    } else if (type === "success") {
        box.innerHTML += `<div class="dsr-terminal-line text-emerald"><i class="fa-solid fa-circle-check"></i> ${text}</div>`;
    } else {
        box.innerHTML += `<div class="dsr-terminal-line"><i class="fa-solid fa-angle-right"></i> ${text}</div>`;
    }
    box.scrollTop = box.scrollHeight;
}
