#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "mongoose.h"
#include "../patient.h"
#include "../doctor.h"
#include "../appointment.h"
#include "../room.h"
#include "../billing.h"

#define MAX_RECORDS 100
#define JSON_BUFFER_SIZE 65536

// =====================================
// JSON OUTPUT HELPERS
// =====================================

static int addText(char *buffer, size_t capacity,
                   size_t *used, const char *format, ...)
{
    if (*used >= capacity) return 0;

    va_list args;
    va_start(args, format);

    int n = vsnprintf(buffer + *used,
                      capacity - *used, format, args);

    va_end(args);

    if (n < 0 || (size_t)n >= capacity - *used)
        return 0;

    *used += (size_t)n;
    return 1;
}

static int addJsonString(char *buffer, size_t capacity,
                         size_t *used, const char *value)
{
    if (!addText(buffer, capacity, used, "\""))
        return 0;

    for (const unsigned char *p =
             (const unsigned char *)value; *p; p++)
    {
        switch (*p)
        {
            case '"':
                if (!addText(buffer, capacity, used, "\\\""))
                    return 0;
                break;

            case '\\':
                if (!addText(buffer, capacity, used, "\\\\"))
                    return 0;
                break;

            case '\n':
                if (!addText(buffer, capacity, used, "\\n"))
                    return 0;
                break;

            case '\r':
                if (!addText(buffer, capacity, used, "\\r"))
                    return 0;
                break;

            case '\t':
                if (!addText(buffer, capacity, used, "\\t"))
                    return 0;
                break;

            default:
                if (*p < 32)
                {
                    if (!addText(buffer, capacity, used,
                                 "\\u%04x", *p))
                        return 0;
                }
                else
                {
                    if (*used + 1 >= capacity)
                        return 0;

                    buffer[(*used)++] = (char)*p;
                    buffer[*used] = '\0';
                }
        }
    }

    return addText(buffer, capacity, used, "\"");
}

// =====================================
// PATIENT JSON
// =====================================

static int addPatient(char *buffer, size_t capacity,
                      size_t *used, const Patient *p)
{
    return
        addText(buffer, capacity, used,
                "{\"patientID\":%d,\"name\":", p->patientID) &&
        addJsonString(buffer, capacity, used, p->name) &&
        addText(buffer, capacity, used, ",\"age\":%d,\"gender\":",
                p->age) &&
        addJsonString(buffer, capacity, used, p->gender) &&
        addText(buffer, capacity, used, ",\"diagnosis\":") &&
        addJsonString(buffer, capacity, used, p->diagnosis) &&
        addText(buffer, capacity, used, ",\"phone\":") &&
        addJsonString(buffer, capacity, used, p->phone) &&
        addText(buffer, capacity, used, "}");
}

static void getPatients(struct mg_connection *c)
{
    Patient records[MAX_RECORDS];
    int count = patientWebList(records, MAX_RECORDS);

    char response[JSON_BUFFER_SIZE] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used,
                 "{\"patients\":["))
        goto error;

    for (int i = 0; i < count; i++)
    {
        if (i > 0 &&
            !addText(response, sizeof(response), &used, ","))
            goto error;

        if (!addPatient(response, sizeof(response),
                        &used, &records[i]))
            goto error;
    }

    if (!addText(response, sizeof(response), &used, "]}"))
        goto error;

    mg_http_reply(c, 200,
                  "Content-Type: application/json\r\n"
                  "Cache-Control: no-store\r\n",
                  "%s", response);
    return;

error:
    mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                  "{\"error\":\"Could not prepare patient records\"}");
}

static void searchPatientAPI(struct mg_connection *c,
                             struct mg_http_message *hm)
{
    char idBuffer[30];

    if (mg_http_get_var(&hm->query, "id",
                        idBuffer, sizeof(idBuffer)) <= 0)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Patient ID required\"}");
        return;
    }

    char *end;
    long id = strtol(idBuffer, &end, 10);

    if (*idBuffer == '\0' || *end != '\0' ||
        id <= 0 || id > 2147483647)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Invalid patient ID\"}");
        return;
    }

    Patient p;

    if (!patientWebFind((int)id, &p))
    {
        mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                      "{\"error\":\"Patient not found\"}");
        return;
    }

    char response[2048] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used,
                 "{\"patient\":") ||
        !addPatient(response, sizeof(response), &used, &p) ||
        !addText(response, sizeof(response), &used, "}"))
    {
        mg_http_reply(c, 500, "", "Server error");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "%s", response);
}

// =====================================
// DOCTOR JSON
// =====================================

static int appendDoctorJSON(char *buffer, size_t capacity,
                     size_t *used, const Doctor *d)
{
    return
        addText(buffer, capacity, used,
                "{\"doctorID\":%d,\"name\":", d->doctorID) &&
        addJsonString(buffer, capacity, used, d->name) &&
        addText(buffer, capacity, used, ",\"specialization\":") &&
        addJsonString(buffer, capacity, used, d->specialization) &&
        addText(buffer, capacity, used,
                ",\"experience\":%d,\"phone\":", d->experience) &&
        addJsonString(buffer, capacity, used, d->phone) &&
        addText(buffer, capacity, used, "}");
}

static void getDoctors(struct mg_connection *c)
{
    Doctor records[MAX_RECORDS];
    int count = doctorWebList(records, MAX_RECORDS);

    char response[JSON_BUFFER_SIZE] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used,
                 "{\"doctors\":["))
        goto error;

    for (int i = 0; i < count; i++)
    {
        if (i > 0 &&
            !addText(response, sizeof(response), &used, ","))
            goto error;

        if (!appendDoctorJSON(response, sizeof(response),
                              &used, &records[i]))
            goto error;
    }

    if (!addText(response, sizeof(response), &used, "]}"))
        goto error;

    mg_http_reply(c, 200,
                  "Content-Type: application/json\r\n"
                  "Cache-Control: no-store\r\n",
                  "%s", response);
    return;

error:
    mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                  "{\"error\":\"Could not prepare doctor records\"}");
}

static void searchDoctorAPI(struct mg_connection *c,
                            struct mg_http_message *hm)
{
    char idBuffer[30];

    if (mg_http_get_var(&hm->query, "id",
                        idBuffer, sizeof(idBuffer)) <= 0)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Doctor ID required\"}");
        return;
    }

    char *end;
    long id = strtol(idBuffer, &end, 10);

    if (*idBuffer == '\0' || *end != '\0' ||
        id <= 0 || id > 2147483647)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Invalid doctor ID\"}");
        return;
    }

    Doctor d;

    if (!doctorWebFind((int)id, &d))
    {
        mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                      "{\"error\":\"Doctor not found\"}");
        return;
    }

    char response[2048] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used,
                 "{\"doctor\":") ||
        !appendDoctorJSON(response, sizeof(response), &used, &d) ||
        !addText(response, sizeof(response), &used, "}"))
    {
        mg_http_reply(c, 500, "", "Server error");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "%s", response);
}

// =====================================
// JSON INPUT HELPERS
// =====================================

static int readJsonString(struct mg_str body,
                          const char *path,
                          char *destination,
                          size_t capacity)
{
    char *value = mg_json_get_str(body, path);

    if (value == NULL)
        return 0;

    size_t length = strlen(value);

    if (length == 0 || length >= capacity)
    {
        mg_free(value);
        return 0;
    }

    memcpy(destination, value, length + 1);
    mg_free(value);

    return 1;
}

static int readPatientJSON(struct mg_str body, Patient *p)
{
    memset(p, 0, sizeof(*p));

    long id = mg_json_get_long(body, "$.patientID", -1);
    long age = mg_json_get_long(body, "$.age", -1);

    if (id <= 0 || id > 2147483647 ||
        age <= 0 || age > 120)
        return 0;

    p->patientID = (int)id;
    p->age = (int)age;

    return
        readJsonString(body, "$.name",
                       p->name, sizeof(p->name)) &&
        readJsonString(body, "$.gender",
                       p->gender, sizeof(p->gender)) &&
        readJsonString(body, "$.diagnosis",
                       p->diagnosis, sizeof(p->diagnosis)) &&
        readJsonString(body, "$.phone",
                       p->phone, sizeof(p->phone));
}

static int readDoctorJSON(struct mg_str body, Doctor *d)
{
    memset(d, 0, sizeof(*d));

    long id = mg_json_get_long(body, "$.doctorID", -1);
    long experience = mg_json_get_long(body, "$.experience", -1);

    if (id <= 0 || id > 2147483647 ||
        experience < 0 || experience > 80)
        return 0;

    d->doctorID = (int)id;
    d->experience = (int)experience;

    return
        readJsonString(body, "$.name",
                       d->name, sizeof(d->name)) &&
        readJsonString(body, "$.specialization",
                       d->specialization,
                       sizeof(d->specialization)) &&
        readJsonString(body, "$.phone",
                       d->phone, sizeof(d->phone));
}

// =====================================
// PATIENT WRITE OPERATIONS
// =====================================

static void createPatientAPI(struct mg_connection *c,
                             struct mg_http_message *hm)
{
    Patient p;

    if (!readPatientJSON(hm->body, &p))
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Invalid patient details\"}");
        return;
    }

    if (!patientWebCreate(&p))
    {
        mg_http_reply(c, 409, "Content-Type: application/json\r\n",
                      "{\"error\":\"Registration failed. Check ID and storage.\"}");
        return;
    }

    mg_http_reply(c, 201, "Content-Type: application/json\r\n",
                  "{\"message\":\"Patient registered successfully\"}");
}

static void updatePatientAPI(struct mg_connection *c,
                             struct mg_http_message *hm)
{
    Patient p;

    if (!readPatientJSON(hm->body, &p))
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Invalid patient details\"}");
        return;
    }

    if (!patientWebUpdate(p.patientID, &p))
    {
        mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                      "{\"error\":\"Patient not found or update failed\"}");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{\"message\":\"Patient updated successfully\"}");
}

static void deletePatientAPI(struct mg_connection *c,
                             struct mg_http_message *hm)
{
    char idBuffer[30];

    if (mg_http_get_var(&hm->query, "id",
                        idBuffer, sizeof(idBuffer)) <= 0)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Patient ID required\"}");
        return;
    }

    char *end;
    long id = strtol(idBuffer, &end, 10);

    if (*idBuffer == '\0' || *end != '\0' ||
        id <= 0 || id > 2147483647 ||
        !patientWebDelete((int)id))
    {
        mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                      "{\"error\":\"Patient not found\"}");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{\"message\":\"Patient deleted successfully\"}");
}

// =====================================
// DOCTOR WRITE OPERATIONS
// =====================================

static void createDoctorAPI(struct mg_connection *c,
                            struct mg_http_message *hm)
{
    Doctor d;

    if (!readDoctorJSON(hm->body, &d))
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Invalid doctor details\"}");
        return;
    }

    if (!doctorWebCreate(&d))
    {
        mg_http_reply(c, 409, "Content-Type: application/json\r\n",
                      "{\"error\":\"Registration failed. Check ID and storage.\"}");
        return;
    }

    mg_http_reply(c, 201, "Content-Type: application/json\r\n",
                  "{\"message\":\"Doctor registered successfully\"}");
}

static void updateDoctorAPI(struct mg_connection *c,
                            struct mg_http_message *hm)
{
    Doctor d;

    if (!readDoctorJSON(hm->body, &d))
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Invalid doctor details\"}");
        return;
    }

    if (!doctorWebUpdate(d.doctorID, &d))
    {
        mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                      "{\"error\":\"Doctor not found or update failed\"}");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{\"message\":\"Doctor updated successfully\"}");
}

static void deleteDoctorAPI(struct mg_connection *c,
                            struct mg_http_message *hm)
{
    char idBuffer[30];

    if (mg_http_get_var(&hm->query, "id",
                        idBuffer, sizeof(idBuffer)) <= 0)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"Doctor ID required\"}");
        return;
    }

    char *end;
    long id = strtol(idBuffer, &end, 10);

    if (*idBuffer == '\0' || *end != '\0' ||
        id <= 0 || id > 2147483647 ||
        !doctorWebDelete((int)id))
    {
        mg_http_reply(c, 404, "Content-Type: application/json\r\n",
                      "{\"error\":\"Doctor not found\"}");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{\"message\":\"Doctor deleted successfully\"}");
}

// =====================================
// APPOINTMENT JSON
// =====================================

static int addAppointmentJSON(char *b, size_t cap,
                              size_t *used, const Appointment *a)
{
    return
        addText(b, cap, used,
                "{\"appointmentID\":%d,\"patientID\":%d,\"doctorID\":%d,\"date\":",
                a->appointmentID, a->patientID, a->doctorID) &&
        addJsonString(b, cap, used, a->date) &&
        addText(b, cap, used, ",\"time\":") &&
        addJsonString(b, cap, used, a->time) &&
        addText(b, cap, used, ",\"reason\":") &&
        addJsonString(b, cap, used, a->reason) &&
        addText(b, cap, used, ",\"status\":") &&
        addJsonString(b, cap, used, a->status) &&
        addText(b, cap, used, "}");
}

static int readAppointmentJSON(struct mg_str body, Appointment *a)
{
    memset(a, 0, sizeof(*a));

    long id = mg_json_get_long(body, "$.appointmentID", -1);
    long patient = mg_json_get_long(body, "$.patientID", -1);
    long doctor = mg_json_get_long(body, "$.doctorID", -1);

    if (id <= 0 || patient <= 0 || doctor <= 0)
        return 0;

    a->appointmentID = (int)id;
    a->patientID = (int)patient;
    a->doctorID = (int)doctor;

    return
        readJsonString(body, "$.date", a->date, sizeof(a->date)) &&
        readJsonString(body, "$.time", a->time, sizeof(a->time)) &&
        readJsonString(body, "$.reason", a->reason, sizeof(a->reason));
}

static void getAppointments(struct mg_connection *c)
{
    Appointment records[MAX_RECORDS];
    int count = appointmentWebList(records, MAX_RECORDS);

    char response[JSON_BUFFER_SIZE] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used,
                 "{\"appointments\":["))
        goto error;

    for (int i = 0; i < count; i++)
    {
        if (i && !addText(response, sizeof(response), &used, ","))
            goto error;

        if (!addAppointmentJSON(response, sizeof(response),
                                &used, &records[i]))
            goto error;
    }

    if (!addText(response, sizeof(response), &used, "]}"))
        goto error;

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "%s", response);
    return;

error:
    mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                  "{\"error\":\"Could not prepare appointments\"}");
}

// =====================================
// ROOM JSON
// =====================================

static int addRoomJSON(char *b, size_t cap,
                       size_t *used, const Room *r)
{
    return
        addText(b, cap, used,
                "{\"roomID\":%d,\"roomNumber\":%d,\"roomType\":",
                r->roomID, r->roomNumber) &&
        addJsonString(b, cap, used, r->roomType) &&
        addText(b, cap, used,
                ",\"chargesPerDay\":%.2f,\"isOccupied\":%d,\"patientID\":%d}",
                r->chargesPerDay, r->isOccupied, r->patientID);
}

static int readRoomJSON(struct mg_str body, Room *r)
{
    memset(r, 0, sizeof(*r));

    long id = mg_json_get_long(body, "$.roomID", -1);
    long number = mg_json_get_long(body, "$.roomNumber", -1);
    double charges = 0;

if (!mg_json_get_num(body, "$.chargesPerDay", &charges))
    return 0;

    if (id <= 0 || number <= 0 || charges < 0)
        return 0;

    r->roomID = (int)id;
    r->roomNumber = (int)number;
    r->chargesPerDay = (float)charges;

    return readJsonString(body, "$.roomType",
                          r->roomType, sizeof(r->roomType));
}

static void getRooms(struct mg_connection *c)
{
    Room records[MAX_RECORDS];
    int count = roomWebList(records, MAX_RECORDS);

    char response[JSON_BUFFER_SIZE] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used, "{\"rooms\":["))
        goto error;

    for (int i = 0; i < count; i++)
    {
        if (i && !addText(response, sizeof(response), &used, ","))
            goto error;

        if (!addRoomJSON(response, sizeof(response), &used, &records[i]))
            goto error;
    }

    if (!addText(response, sizeof(response), &used, "]}"))
        goto error;

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "%s", response);
    return;

error:
    mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                  "{\"error\":\"Could not prepare rooms\"}");
}

// =====================================
// BILLING JSON
// =====================================

static int addBillJSON(char *b, size_t cap,
                       size_t *used, const Bill *bill)
{
    return
        addText(b, cap, used,
                "{\"billID\":%d,\"patientID\":%d,\"roomNumber\":%d,\"days\":%d,\"chargesPerDay\":%.2f,\"otherCharges\":%.2f,\"totalAmount\":%.2f,\"paymentStatus\":",
                bill->billID, bill->patientID, bill->roomNumber,
                bill->days, bill->chargesPerDay,
                bill->otherCharges, bill->totalAmount) &&
        addJsonString(b, cap, used, bill->paymentStatus) &&
        addText(b, cap, used, "}");
}

static int readBillJSON(struct mg_str body, Bill *bill)
{
    memset(bill, 0, sizeof(*bill));

    long id = mg_json_get_long(body, "$.billID", -1);
    long patient = mg_json_get_long(body, "$.patientID", -1);
    long room = mg_json_get_long(body, "$.roomNumber", -1);
    long days = mg_json_get_long(body, "$.days", -1);

    double charges = 0;
double other = 0;

if (!mg_json_get_num(body, "$.chargesPerDay", &charges))
    return 0;

if (!mg_json_get_num(body, "$.otherCharges", &other))
    return 0;

    if (id <= 0 || patient <= 0 || room <= 0 ||
        days <= 0 || charges < 0 || other < 0)
        return 0;

    bill->billID = (int)id;
    bill->patientID = (int)patient;
    bill->roomNumber = (int)room;
    bill->days = (int)days;
    bill->chargesPerDay = (float)charges;
    bill->otherCharges = (float)other;

    bill->totalAmount =
        (bill->days * bill->chargesPerDay) + bill->otherCharges;

    return 1;
}

static void getBills(struct mg_connection *c)
{
    Bill records[MAX_RECORDS];
    int count = billingWebList(records, MAX_RECORDS);

    char response[JSON_BUFFER_SIZE] = "";
    size_t used = 0;

    if (!addText(response, sizeof(response), &used, "{\"bills\":["))
        goto error;

    for (int i = 0; i < count; i++)
    {
        if (i && !addText(response, sizeof(response), &used, ","))
            goto error;

        if (!addBillJSON(response, sizeof(response), &used, &records[i]))
            goto error;
    }

    if (!addText(response, sizeof(response), &used, "]}"))
        goto error;

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "%s", response);
    return;

error:
    mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                  "{\"error\":\"Could not prepare bills\"}");
}

// =====================================
// APPOINTMENT API OPERATIONS
// =====================================

static void createAppointmentAPI(struct mg_connection *c,
                                 struct mg_http_message *msg)
{
    Appointment a;

    if (!readAppointmentJSON(msg->body, &a)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Invalid appointment data\"}");
        return;
    }

    if (appointmentWebCreate(&a)) {
        mg_http_reply(c, 201, "Content-Type: application/json\r\n",
                      "{\"message\":\"Appointment created successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not create appointment\"}");
    }
}

static void updateAppointmentAPI(struct mg_connection *c,
                                 struct mg_http_message *msg)
{
    Appointment a;

    if (!readAppointmentJSON(msg->body, &a)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Invalid appointment data\"}");
        return;
    }

    if (appointmentWebUpdate(a.appointmentID, &a)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{\"message\":\"Appointment updated successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not update appointment\"}");
    }
}


// =====================================
// ROOM API OPERATIONS
// =====================================

static void createRoomAPI(struct mg_connection *c,
                          struct mg_http_message *msg)
{
    Room r;

    if (!readRoomJSON(msg->body, &r)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Invalid room data\"}");
        return;
    }

    if (roomWebCreate(&r)) {
        mg_http_reply(c, 201, "Content-Type: application/json\r\n",
                      "{\"message\":\"Room created successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not create room\"}");
    }
}

static void allocateRoomAPI(struct mg_connection *c,
                            struct mg_http_message *msg)
{
    int roomID = 0;
    int patientID = (int) mg_json_get_long(msg->body, "$.patientID", 0);

    char idText[30];
    mg_http_get_var(&msg->query, "id", idText, sizeof(idText));
    roomID = atoi(idText);

    if (roomWebAllocate(roomID, patientID)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{\"message\":\"Room allocated successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not allocate room\"}");
    }
}

static void releaseRoomAPI(struct mg_connection *c,
                           struct mg_http_message *msg)
{
    char idText[30] = "0";
    mg_http_get_var(&msg->query, "id", idText, sizeof(idText));

    int roomID = atoi(idText);

    if (roomWebRelease(roomID)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{\"message\":\"Room released successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not release room\"}");
    }
}

static void updateRoomAPI(struct mg_connection *c,
                          struct mg_http_message *msg)
{
    Room r;

    if (!readRoomJSON(msg->body, &r)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Invalid room data\"}");
        return;
    }

    if (roomWebUpdate(r.roomID, &r)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{\"message\":\"Room updated successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not update room\"}");
    }
}


// =====================================
// BILLING API OPERATIONS
// =====================================

static void createBillAPI(struct mg_connection *c,
                          struct mg_http_message *msg)
{
    Bill b;

    if (!readBillJSON(msg->body, &b)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Invalid bill data\"}");
        return;
    }

    if (billingWebCreate(&b)) {
        mg_http_reply(c, 201, "Content-Type: application/json\r\n",
                      "{\"message\":\"Bill generated successfully\"}");
    } else {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not generate bill\"}");
    }
}

static void updateBillStatusAPI(struct mg_connection *c,
                               struct mg_http_message *msg)
{
    int billID = (int) mg_json_get_long(msg->body, "$.billID", 0);

    char *paymentStatus = mg_json_get_str(
        msg->body,
        "$.paymentStatus"
    );

    if (paymentStatus == NULL)
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Payment status is missing\"}");
        return;
    }

    if (billingWebUpdateStatus(billID, paymentStatus))
    {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{\"message\":\"Payment status updated\"}");
    }
    else
    {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"message\":\"Could not update payment status\"}");
    }

    mg_free(paymentStatus);
}


// =====================================
// HTTP ROUTING
// =====================================

static void eventHandler(struct mg_connection *c,
                         int ev, void *ev_data)
{
    if (ev != MG_EV_HTTP_MSG)
        return;

    struct mg_http_message *hm =
        (struct mg_http_message *) ev_data;

    // =================================
    // PATIENT API
    // =================================

    if (mg_match(hm->uri, mg_str("/api/patients"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("GET")) == 0)
        {
            char idBuffer[30] = "";

            int hasID = mg_http_get_var(
                &hm->query, "id",
                idBuffer, sizeof(idBuffer)
            );

            if (hasID > 0)
                searchPatientAPI(c, hm);
            else
                getPatients(c);
        }
        else if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            createPatientAPI(c, hm);
        else if (mg_strcmp(hm->method, mg_str("PUT")) == 0)
            updatePatientAPI(c, hm);
        else if (mg_strcmp(hm->method, mg_str("DELETE")) == 0)
            deletePatientAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // DOCTOR API
    // =================================

    if (mg_match(hm->uri, mg_str("/api/doctors"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("GET")) == 0)
        {
            char idBuffer[30] = "";

            int hasID = mg_http_get_var(
                &hm->query, "id",
                idBuffer, sizeof(idBuffer)
            );

            if (hasID > 0)
                searchDoctorAPI(c, hm);
            else
                getDoctors(c);
        }
        else if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            createDoctorAPI(c, hm);
        else if (mg_strcmp(hm->method, mg_str("PUT")) == 0)
            updateDoctorAPI(c, hm);
        else if (mg_strcmp(hm->method, mg_str("DELETE")) == 0)
            deleteDoctorAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // APPOINTMENT CANCEL API
    // =================================

    if (mg_match(hm->uri,
                 mg_str("/api/appointments/cancel"), NULL))
    {
        char idBuffer[30] = "0";

        mg_http_get_var(
            &hm->query, "id",
            idBuffer, sizeof(idBuffer)
        );

        int appointmentID = atoi(idBuffer);

        if (mg_strcmp(hm->method, mg_str("POST")) == 0)
        {
            if (appointmentWebCancel(appointmentID))
                mg_http_reply(c, 200,
                    "Content-Type: application/json\r\n",
                    "{\"message\":\"Appointment cancelled\"}");
            else
                mg_http_reply(c, 400,
                    "Content-Type: application/json\r\n",
                    "{\"message\":\"Could not cancel appointment\"}");
        }
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // APPOINTMENT API
    // =================================

    if (mg_match(hm->uri,
                 mg_str("/api/appointments"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("GET")) == 0)
            getAppointments(c);
        else if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            createAppointmentAPI(c, hm);
        else if (mg_strcmp(hm->method, mg_str("PUT")) == 0)
            updateAppointmentAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // ROOM ALLOCATION API
    // =================================

    if (mg_match(hm->uri,
                 mg_str("/api/rooms/allocate"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            allocateRoomAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // ROOM RELEASE API
    // =================================

    if (mg_match(hm->uri,
                 mg_str("/api/rooms/release"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            releaseRoomAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // ROOM API
    // =================================

    if (mg_match(hm->uri, mg_str("/api/rooms"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("GET")) == 0)
            getRooms(c);
        else if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            createRoomAPI(c, hm);
        else if (mg_strcmp(hm->method, mg_str("PUT")) == 0)
            updateRoomAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // BILL PAYMENT STATUS API
    // =================================

    if (mg_match(hm->uri,
                 mg_str("/api/bills/status"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("PUT")) == 0)
            updateBillStatusAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // BILLING API
    // =================================

    if (mg_match(hm->uri, mg_str("/api/bills"), NULL))
    {
        if (mg_strcmp(hm->method, mg_str("GET")) == 0)
            getBills(c);
        else if (mg_strcmp(hm->method, mg_str("POST")) == 0)
            createBillAPI(c, hm);
        else
            mg_http_reply(c, 405, "", "Method not allowed");

        return;
    }

    // =================================
    // FRONTEND WEBSITE
    // =================================

    struct mg_http_serve_opts options = {
        .root_dir = "frontend"
    };

    mg_http_serve_dir(c, hm, &options);
}

// =====================================
// MAIN
// =====================================


int main(void)
{
    struct mg_mgr manager;
    char serverURL[100];

    const char *port = getenv("PORT");

    if (port == NULL || port[0] == '\0')
        port = "8082";

    snprintf(serverURL, sizeof(serverURL),
             "http://0.0.0.0:%s", port);

    mg_mgr_init(&manager);

    patientWebInit();
    doctorWebInit();
    appointmentWebInit();
    roomWebInit();
    billingWebInit();

    if (mg_http_listen(&manager,
                       serverURL,
                       eventHandler, NULL) == NULL)
    {
        printf("Could not start the web server!\n");
        mg_mgr_free(&manager);
        return 1;
    }

    printf("MedCare server running at %s\n", serverURL);

    for (;;)
        mg_mgr_poll(&manager, 1000);

    mg_mgr_free(&manager);
    return 0;
}
