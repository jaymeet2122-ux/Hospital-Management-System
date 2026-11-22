
let editingPatientID = null;
let editingDoctorID = null;

// =====================================
// WEBSITE NAVIGATION
// =====================================

function openSection(sectionId) {

    document.querySelectorAll(".page-section").forEach(section => {
        section.hidden = true;
    });

    const selectedSection = document.getElementById(sectionId);

    if (selectedSection) {
        selectedSection.hidden = false;
    }

    document.querySelectorAll(".nav-link").forEach(button => {
        button.classList.remove("active");

        if (button.getAttribute("onclick")?.includes(sectionId)) {
            button.classList.add("active");
        }
    });

    const headings = {
        dashboard: "Hospital Dashboard",
        patients: "Patient Management",
        doctors: "Doctor Management",
        appointments: "Appointment Management",
        rooms: "Room Management",
        billing: "Billing Management"
    };

    const topbarHeading = document.querySelector(".topbar h2");

    if (topbarHeading) {
        topbarHeading.textContent =
            headings[sectionId] || "Hospital Dashboard";
    }

    if (sectionId === "patients") loadPatients();
    if (sectionId === "doctors") loadDoctors();
    if (sectionId === "appointments") loadAppointments();
    if (sectionId === "rooms") loadRooms();
    if (sectionId === "billing") loadBills();
}

// =====================================
// SHARED MESSAGE HELPER
// =====================================

function showMessage(elementID, message, success = true) {

    const element = document.getElementById(elementID);

    if (!element) return;

    element.textContent = message;
    element.style.color = success ? "#087E8B" : "#C0392B";
}

// =====================================
// PATIENT MANAGEMENT
// =====================================

function resetPatientForm() {

    document.getElementById("patientForm").reset();
    document.getElementById("patientID").readOnly = false;

    editingPatientID = null;

    document.getElementById("patientFormTitle").textContent =
        "Register New Patient";

    document.getElementById("patientSubmitButton").textContent =
        "Register Patient";

    document.getElementById("cancelPatientEdit").hidden = true;
}

document.getElementById("patientForm").addEventListener(
    "submit", async function(event) {

    event.preventDefault();

    const patient = {
        patientID: Number(document.getElementById("patientID").value),
        name: document.getElementById("patientName").value.trim(),
        age: Number(document.getElementById("patientAge").value),
        gender: document.getElementById("patientGender").value,
        diagnosis: document.getElementById("patientDiagnosis").value.trim(),
        phone: document.getElementById("patientPhone").value.trim()
    };

    if (!patient.patientID || !patient.name ||
        patient.age < 1 || patient.age > 120 ||
        !patient.gender || !patient.diagnosis || !patient.phone) {

        showMessage("patientMessage", "Please enter valid details.", false);
        return;
    }

    const isEditing = editingPatientID !== null;

    try {

        const response = await fetch("/api/patients", {
            method: isEditing ? "PUT" : "POST",
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify(patient)
        });

        const result = await response.json();

        if (!response.ok) throw new Error(result.error);

        showMessage("patientMessage", result.message);

        resetPatientForm();
        await loadPatients();

    } catch (error) {

        showMessage("patientMessage",
                    error.message || "Patient operation failed.", false);
    }
});

// =====================================
// LOAD PATIENTS
// =====================================

async function loadPatients() {

    const tableBody = document.getElementById("patientTableBody");

    try {

        const response = await fetch("/api/patients");
        const data = await response.json();

        if (!response.ok) throw new Error(data.error);

        renderPatients(data.patients);

    } catch (error) {

        if (!tableBody) return;

        tableBody.replaceChildren();

        const row = tableBody.insertRow();
        const cell = row.insertCell();

        cell.colSpan = 7;
        cell.textContent = error.message || "Unable to load patients.";
    }
}

function renderPatients(patients) {

    const tableBody = document.getElementById("patientTableBody");

    if (!tableBody) return;

    tableBody.replaceChildren();

    if (!patients || patients.length === 0) {

        const row = tableBody.insertRow();
        const cell = row.insertCell();

        cell.colSpan = 7;
        cell.textContent = "No patient records found.";

        return;
    }

    patients.forEach(patient => {

        const row = tableBody.insertRow();

        [
            patient.patientID,
            patient.name,
            patient.age,
            patient.gender,
            patient.diagnosis,
            patient.phone
        ].forEach(value => {
            row.insertCell().textContent = value ?? "";
        });

        const actionCell = row.insertCell();

        const editButton = document.createElement("button");
        editButton.textContent = "Edit";
        editButton.type = "button";
        editButton.addEventListener("click", () => editPatient(patient));

        const deleteButton = document.createElement("button");
        deleteButton.textContent = "Delete";
        deleteButton.type = "button";
        deleteButton.className = "delete-button";
        deleteButton.addEventListener("click",
            () => deletePatientRecord(patient.patientID));

        actionCell.append(editButton, deleteButton);
    });
}

// =====================================
// SEARCH PATIENT
// =====================================

async function searchPatientRecord() {

    const id = document.getElementById("searchPatientID").value;

    if (!id) {
        showMessage("patientMessage", "Enter a Patient ID.", false);
        return;
    }

    try {

        const response = await fetch(
            "/api/patients?id=" + encodeURIComponent(id)
        );

        const data = await response.json();

        if (!response.ok) throw new Error(data.error);

        renderPatients([data.patient]);
        showMessage("patientMessage", "Patient found.");

    } catch (error) {

        renderPatients([]);
        showMessage("patientMessage", error.message, false);
    }
}

function clearPatientSearch() {

    document.getElementById("searchPatientID").value = "";
    loadPatients();
}

// =====================================
// EDIT PATIENT
// =====================================

function editPatient(patient) {

    editingPatientID = patient.patientID;

    document.getElementById("patientID").value = patient.patientID;
    document.getElementById("patientID").readOnly = true;
    document.getElementById("patientName").value = patient.name;
    document.getElementById("patientAge").value = patient.age;
    document.getElementById("patientGender").value = patient.gender;
    document.getElementById("patientDiagnosis").value = patient.diagnosis;
    document.getElementById("patientPhone").value = patient.phone;

    document.getElementById("patientFormTitle").textContent =
        "Update Patient Details";

    document.getElementById("patientSubmitButton").textContent =
        "Save Changes";

    document.getElementById("cancelPatientEdit").hidden = false;

    document.getElementById("patientForm").scrollIntoView({
        behavior: "smooth"
    });
}

async function deletePatientRecord(id) {

    if (!confirm("Delete Patient ID " + id + "?")) return;

    try {

        const response = await fetch(
            "/api/patients?id=" + encodeURIComponent(id),
            {method: "DELETE"}
        );

        const result = await response.json();

        if (!response.ok) throw new Error(result.error);

        showMessage("patientMessage", result.message);
        await loadPatients();

    } catch (error) {

        showMessage("patientMessage", error.message, false);
    }
}

document.getElementById("cancelPatientEdit").addEventListener(
    "click", function() {

    resetPatientForm();
    showMessage("patientMessage", "Update cancelled.");
});

// =====================================
// DOCTOR MANAGEMENT
// =====================================

function resetDoctorForm() {

    document.getElementById("doctorForm").reset();
    document.getElementById("doctorID").readOnly = false;

    editingDoctorID = null;

    document.getElementById("doctorFormTitle").textContent =
        "Register New Doctor";

    document.getElementById("doctorSubmitButton").textContent =
        "Register Doctor";

    document.getElementById("cancelDoctorEdit").hidden = true;
}

document.getElementById("doctorForm").addEventListener(
    "submit", async function(event) {

    event.preventDefault();

    const doctor = {
        doctorID: Number(document.getElementById("doctorID").value),
        name: document.getElementById("doctorName").value.trim(),
        specialization: document.getElementById("doctorSpecialization").value.trim(),
        experience: Number(document.getElementById("doctorExperience").value),
        phone: document.getElementById("doctorPhone").value.trim()
    };

    if (!doctor.doctorID || !doctor.name ||
        !doctor.specialization ||
        doctor.experience < 0 || doctor.experience > 80 ||
        !doctor.phone) {

        showMessage("doctorMessage", "Please enter valid doctor details.", false);
        return;
    }

    const isEditing = editingDoctorID !== null;

    try {

        const response = await fetch("/api/doctors", {
            method: isEditing ? "PUT" : "POST",
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify(doctor)
        });

        const result = await response.json();

        if (!response.ok) throw new Error(result.error);

        showMessage("doctorMessage", result.message);

        resetDoctorForm();
        await loadDoctors();

    } catch (error) {

        showMessage("doctorMessage",
                    error.message || "Doctor operation failed.", false);
    }
});

// =====================================
// LOAD DOCTORS
// =====================================

async function loadDoctors() {

    const tableBody = document.getElementById("doctorTableBody");

    try {

        const response = await fetch("/api/doctors");
        const data = await response.json();

        if (!response.ok) throw new Error(data.error);

        renderDoctors(data.doctors);

    } catch (error) {

        if (!tableBody) return;

        tableBody.replaceChildren();

        const row = tableBody.insertRow();
        const cell = row.insertCell();

        cell.colSpan = 6;
        cell.textContent = error.message || "Unable to load doctors.";
    }
}

// =====================================
// DISPLAY DOCTOR RECORDS
// =====================================

function renderDoctors(doctors) {

    const tableBody = document.getElementById("doctorTableBody");

    if (!tableBody) return;

    tableBody.replaceChildren();

    if (!doctors || doctors.length === 0) {

        const row = tableBody.insertRow();
        const cell = row.insertCell();

        cell.colSpan = 6;
        cell.textContent = "No doctor records found.";

        return;
    }

    doctors.forEach(doctor => {

        const row = tableBody.insertRow();

        [
            doctor.doctorID,
            doctor.name,
            doctor.specialization,
            doctor.experience + " years",
            doctor.phone
        ].forEach(value => {
            row.insertCell().textContent = value ?? "";
        });

        const actionCell = row.insertCell();

        const editButton = document.createElement("button");
        editButton.textContent = "Edit";
        editButton.type = "button";
        editButton.addEventListener("click", () => editDoctor(doctor));

        const deleteButton = document.createElement("button");
        deleteButton.textContent = "Delete";
        deleteButton.type = "button";
        deleteButton.className = "delete-button";
        deleteButton.addEventListener("click",
            () => deleteDoctorRecord(doctor.doctorID));

        actionCell.append(editButton, deleteButton);
    });
}

// =====================================
// SEARCH DOCTOR
// =====================================

async function searchDoctorRecord() {

    const id = document.getElementById("searchDoctorID").value;

    if (!id) {
        showMessage("doctorMessage", "Enter a Doctor ID.", false);
        return;
    }

    try {

        const response = await fetch(
            "/api/doctors?id=" + encodeURIComponent(id)
        );

        const data = await response.json();

        if (!response.ok) throw new Error(data.error);

        renderDoctors([data.doctor]);
        showMessage("doctorMessage", "Doctor found.");

    } catch (error) {

        renderDoctors([]);
        showMessage("doctorMessage", error.message, false);
    }
}

function clearDoctorSearch() {

    document.getElementById("searchDoctorID").value = "";
    loadDoctors();
}

// =====================================
// EDIT DOCTOR
// =====================================

function editDoctor(doctor) {

    editingDoctorID = doctor.doctorID;

    document.getElementById("doctorID").value = doctor.doctorID;
    document.getElementById("doctorID").readOnly = true;
    document.getElementById("doctorName").value = doctor.name;
    document.getElementById("doctorSpecialization").value =
        doctor.specialization;
    document.getElementById("doctorExperience").value =
        doctor.experience;
    document.getElementById("doctorPhone").value = doctor.phone;

    document.getElementById("doctorFormTitle").textContent =
        "Update Doctor Details";

    document.getElementById("doctorSubmitButton").textContent =
        "Save Changes";

    document.getElementById("cancelDoctorEdit").hidden = false;

    document.getElementById("doctorForm").scrollIntoView({
        behavior: "smooth"
    });
}

// =====================================
// DELETE DOCTOR
// =====================================

async function deleteDoctorRecord(id) {

    if (!confirm("Delete Doctor ID " + id + "?")) return;

    try {

        const response = await fetch(
            "/api/doctors?id=" + encodeURIComponent(id),
            {method: "DELETE"}
        );

        const result = await response.json();

        if (!response.ok) throw new Error(result.error);

        showMessage("doctorMessage", result.message);
        await loadDoctors();

    } catch (error) {

        showMessage("doctorMessage", error.message, false);
    }
}

// =====================================
// CANCEL DOCTOR UPDATE
// =====================================

document.getElementById("cancelDoctorEdit").addEventListener(
    "click", function() {

    resetDoctorForm();
    showMessage("doctorMessage", "Update cancelled.");
});

// =====================================
// INITIAL SETUP
// =====================================

resetPatientForm();
resetDoctorForm();

// =====================================
// APPOINTMENT MANAGEMENT
// =====================================

let appointmentsData = [];
let editingAppointmentID = null;

// Load all appointments
async function loadAppointments() {

    const table = document.getElementById("appointmentTableBody");

    try {

        const response = await fetch("/api/appointments");
        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Could not load appointments");
        }

        appointmentsData = Array.isArray(data)
            ? data
            : (data.appointments || []);

        displayAppointments(appointmentsData);

    } catch (error) {

        console.error("Appointment loading error:", error);

        if (table) {
            table.innerHTML =
                '<tr><td colspan="8">Unable to load appointments.</td></tr>';
        }
    }
}

// Display appointment records
function displayAppointments(records) {

    const table = document.getElementById("appointmentTableBody");

    if (!table) return;

    if (!records || records.length === 0) {

        table.innerHTML =
            '<tr><td colspan="8">No appointments found.</td></tr>';

        return;
    }

    table.innerHTML = records.map(a => `
        <tr>
            <td>${a.appointmentID}</td>
            <td>${a.patientID}</td>
            <td>${a.doctorID}</td>
            <td>${a.date}</td>
            <td>${a.time}</td>
            <td>${a.reason}</td>
            <td>${a.status}</td>
            <td>
                <button type="button"
                    onclick="editAppointment(${a.appointmentID})">
                    Edit
                </button>

                <button type="button"
                    onclick="cancelAppointment(${a.appointmentID})">
                    Cancel
                </button>
            </td>
        </tr>
    `).join("");
}

// Book or update appointment
document.getElementById("appointmentForm")
    .addEventListener("submit", async function(event) {

    event.preventDefault();

    const appointment = {
        appointmentID: Number(
            document.getElementById("appointmentID").value
        ),
        patientID: Number(
            document.getElementById("appointmentPatientID").value
        ),
        doctorID: Number(
            document.getElementById("appointmentDoctorID").value
        ),
        date: document.getElementById("appointmentDate").value,
        time: document.getElementById("appointmentTime").value,
        reason: document.getElementById("appointmentReason").value
    };

    const message = document.getElementById("appointmentMessage");

    try {

        const isEditing = editingAppointmentID !== null;

        const response = await fetch("/api/appointments", {
            method: isEditing ? "PUT" : "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(appointment)
        });

        const result = await response.json();

        message.textContent = result.message || result.error || "";

        if (!response.ok) return;

        resetAppointmentForm();
        await loadAppointments();

    } catch (error) {

        console.error(error);
        message.textContent = "Server connection failed.";
    }
});

// Edit appointment
function editAppointment(id) {

    const appointment = appointmentsData.find(
        a => Number(a.appointmentID) === Number(id)
    );

    if (!appointment) return;

    if (appointment.status === "Cancelled") {
        alert("Cancelled appointments cannot be edited.");
        return;
    }

    editingAppointmentID = id;

    document.getElementById("appointmentID").value =
        appointment.appointmentID;

    document.getElementById("appointmentID").readOnly = true;

    document.getElementById("appointmentPatientID").value =
        appointment.patientID;

    document.getElementById("appointmentDoctorID").value =
        appointment.doctorID;

    document.getElementById("appointmentDate").value =
        appointment.date;

    document.getElementById("appointmentTime").value =
        appointment.time;

    document.getElementById("appointmentReason").value =
        appointment.reason;

    document.getElementById("appointmentFormTitle").textContent =
        "Update Appointment";

    document.getElementById("appointmentSubmitButton").textContent =
        "Update Appointment";

    document.getElementById("cancelAppointmentEdit").hidden = false;

    document.getElementById("appointmentMessage").textContent = "";

    document.getElementById("appointmentForm").scrollIntoView({
        behavior: "smooth"
    });
}

// Cancel appointment
async function cancelAppointment(id) {

    if (!confirm("Are you sure you want to cancel this appointment?")) {
        return;
    }

    try {

        const response = await fetch(
            `/api/appointments/cancel?id=${id}`,
            { method: "POST" }
        );

        const result = await response.json();

        alert(result.message || result.error);

        if (response.ok) {
            await loadAppointments();
        }

    } catch (error) {

        console.error(error);
        alert("Could not connect to the server.");
    }
}

// Search appointment by ID
function searchAppointment() {

    const id = document.getElementById("searchAppointmentID").value;

    if (!id) {
        alert("Please enter an Appointment ID.");
        return;
    }

    const result = appointmentsData.filter(
        a => Number(a.appointmentID) === Number(id)
    );

    displayAppointments(result);
}

// Reset appointment form
function resetAppointmentForm() {

    const form = document.getElementById("appointmentForm");

    if (!form) return;

    form.reset();

    editingAppointmentID = null;

    document.getElementById("appointmentID").readOnly = false;

    document.getElementById("appointmentFormTitle").textContent =
        "Book New Appointment";

    document.getElementById("appointmentSubmitButton").textContent =
        "Book Appointment";

    document.getElementById("cancelAppointmentEdit").hidden = true;

    document.getElementById("appointmentMessage").textContent = "";
}

// =====================================
// ROOM MANAGEMENT
// =====================================

let roomsData = [];
let editingRoomID = null;

// Load all rooms
async function loadRooms() {

    const table = document.getElementById("roomTableBody");

    try {

        const response = await fetch("/api/rooms");
        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Could not load rooms");
        }

        roomsData = Array.isArray(data)
            ? data
            : (data.rooms || []);

        displayRooms(roomsData);

    } catch (error) {

        console.error("Room loading error:", error);

        if (table) {
            table.innerHTML =
                '<tr><td colspan="7">Unable to load rooms.</td></tr>';
        }
    }
}

// Display room records
function displayRooms(records) {

    const table = document.getElementById("roomTableBody");

    if (!table) return;

    if (!records || records.length === 0) {

        table.innerHTML =
            '<tr><td colspan="7">No rooms found.</td></tr>';

        return;
    }

    table.innerHTML = records.map(r => `
        <tr>
            <td>${r.roomID}</td>
            <td>${r.roomNumber}</td>
            <td>${r.roomType}</td>
            <td>${r.chargesPerDay}</td>
            <td>${Number(r.isOccupied) ? "Occupied" : "Available"}</td>
            <td>${r.patientID}</td>
            <td>
                <button type="button"
                    onclick="editRoom(${r.roomID})">
                    Edit
                </button>

                ${Number(r.isOccupied) ?
                    `<button type="button"
                        onclick="releaseRoom(${r.roomID})">
                        Release
                    </button>` : ""}
            </td>
        </tr>
    `).join("");
}

// Add or update room
document.getElementById("roomForm")
    .addEventListener("submit", async function(event) {

    event.preventDefault();

    const room = {
        roomID: Number(document.getElementById("roomID").value),
        roomNumber: Number(document.getElementById("roomNumber").value),
        roomType: document.getElementById("roomType").value,
        chargesPerDay: Number(document.getElementById("roomCharges").value)
    };

    const message = document.getElementById("roomMessage");

    try {

        const isEditing = editingRoomID !== null;

        const response = await fetch("/api/rooms", {
            method: isEditing ? "PUT" : "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(room)
        });

        const result = await response.json();

        message.textContent = result.message || result.error || "";

        if (!response.ok) return;

        resetRoomForm();
        await loadRooms();

    } catch (error) {

        console.error(error);
        message.textContent = "Server connection failed.";
    }
});

// Edit room
function editRoom(id) {

    const room = roomsData.find(
        r => Number(r.roomID) === Number(id)
    );

    if (!room) return;

    editingRoomID = id;

    document.getElementById("roomID").value = room.roomID;
    document.getElementById("roomID").readOnly = true;

    document.getElementById("roomNumber").value = room.roomNumber;
    document.getElementById("roomType").value = room.roomType;
    document.getElementById("roomCharges").value = room.chargesPerDay;

    document.getElementById("roomFormTitle").textContent =
        "Update Room Details";

    document.getElementById("roomSubmitButton").textContent =
        "Update Room";

    document.getElementById("cancelRoomEdit").hidden = false;

    document.getElementById("roomMessage").textContent = "";

    document.getElementById("roomForm").scrollIntoView({
        behavior: "smooth"
    });
}

// Allocate room to patient
async function allocateRoom() {

    const roomID = document.getElementById("allocateRoomID").value;
    const patientID = document.getElementById("allocatePatientID").value;

    if (!roomID || !patientID) {
        alert("Please enter both Room ID and Patient ID.");
        return;
    }

    try {

        const response = await fetch(
            `/api/rooms/allocate?id=${roomID}`,
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    patientID: Number(patientID)
                })
            }
        );

        const result = await response.json();

        alert(result.message || result.error);

        if (response.ok) {

            document.getElementById("allocateRoomID").value = "";
            document.getElementById("allocatePatientID").value = "";

            await loadRooms();
        }

    } catch (error) {

        console.error(error);
        alert("Could not connect to the server.");
    }
}

// Release occupied room
async function releaseRoom(id) {

    if (!confirm("Are you sure you want to release this room?")) {
        return;
    }

    try {

        const response = await fetch(
            `/api/rooms/release?id=${id}`,
            { method: "POST" }
        );

        const result = await response.json();

        alert(result.message || result.error);

        if (response.ok) {
            await loadRooms();
        }

    } catch (error) {

        console.error(error);
        alert("Could not connect to the server.");
    }
}

// Search room by ID
function searchRoom() {

    const id = document.getElementById("searchRoomID").value;

    if (!id) {
        alert("Please enter a Room ID.");
        return;
    }

    const result = roomsData.filter(
        r => Number(r.roomID) === Number(id)
    );

    displayRooms(result);
}

// Reset room form
function resetRoomForm() {

    const form = document.getElementById("roomForm");

    if (!form) return;

    form.reset();

    editingRoomID = null;

    document.getElementById("roomID").readOnly = false;

    document.getElementById("roomFormTitle").textContent =
        "Add New Room";

    document.getElementById("roomSubmitButton").textContent =
        "Add Room";

    document.getElementById("cancelRoomEdit").hidden = true;

    document.getElementById("roomMessage").textContent = "";
}

// =====================================
// BILLING MANAGEMENT
// =====================================

let billsData = [];

// Load all bills
async function loadBills() {

    const table = document.getElementById("billTableBody");

    try {

        const response = await fetch("/api/bills");

        if (!response.ok) {
            throw new Error("Could not load bills");
        }

        const data = await response.json();

        billsData = Array.isArray(data)
            ? data
            : (data.bills || []);

        displayBills(billsData);

    } catch (error) {

        console.error(error);

        if (table) {
            table.innerHTML =
                '<tr><td colspan="9">Unable to load bills.</td></tr>';
        }
    }
}

// Display billing records
function displayBills(records) {

    const table = document.getElementById("billTableBody");

    if (!table) return;

    if (!records || records.length === 0) {

        table.innerHTML =
            '<tr><td colspan="9">No billing records found.</td></tr>';

        return;
    }

    table.innerHTML = records.map(b => `
        <tr>
            <td>${b.billID}</td>
            <td>${b.patientID}</td>
            <td>${b.roomNumber}</td>
            <td>${b.days}</td>
            <td>${b.chargesPerDay}</td>
            <td>${b.otherCharges}</td>
            <td>${b.totalAmount}</td>
            <td>${b.paymentStatus}</td>
            <td>
                <button type="button"
                    onclick="updateBillStatus(${b.billID}, 'Paid')">
                    Mark Paid
                </button>

                <button type="button"
                    onclick="updateBillStatus(${b.billID}, 'Unpaid')">
                    Mark Unpaid
                </button>
            </td>
        </tr>
    `).join("");
}

// Generate new bill
document.getElementById("billForm")
    .addEventListener("submit", async function(event) {

    event.preventDefault();

    const bill = {
        billID: Number(document.getElementById("billID").value),
        patientID: Number(document.getElementById("billPatientID").value),
        roomNumber: Number(document.getElementById("billRoomNumber").value),
        days: Number(document.getElementById("billDays").value),
        chargesPerDay: Number(
            document.getElementById("billChargesPerDay").value
        ),
        otherCharges: Number(
            document.getElementById("billOtherCharges").value
        )
    };

    const message = document.getElementById("billMessage");

    try {

        const response = await fetch("/api/bills", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(bill)
        });

        const result = await response.json();

        message.textContent = result.message || result.error || "";

        if (!response.ok) return;

        document.getElementById("billForm").reset();

        await loadBills();

    } catch (error) {

        console.error(error);
        message.textContent = "Server connection failed.";
    }
});

// Update payment status
async function updateBillStatus(id, status) {

    if (!confirm(`Change bill ${id} status to ${status}?`)) {
        return;
    }

    try {

        const response = await fetch("/api/bills/status", {
            method: "PUT",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                billID: Number(id),
                paymentStatus: status
            })
        });

        const result = await response.json();

        alert(result.message || result.error);

        if (response.ok) {
            await loadBills();
        }

    } catch (error) {

        console.error(error);
        alert("Could not connect to the server.");
    }
}

// Search bill by ID
function searchBill() {

    const id = document.getElementById("searchBillID").value;

    if (!id) {
        alert("Please enter a Bill ID.");
        return;
    }

    const result = billsData.filter(
        b => Number(b.billID) === Number(id)
    );

    displayBills(result);
}

// =====================================
// INITIAL DATA LOADING
// =====================================

loadAppointments();
loadRooms();
loadBills();
