
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#define MAX_APPOINTMENTS 200

#define MAX_DATE 15
#define MAX_TIME 10
#define MAX_REASON 100

typedef struct
{
    int appointmentID;
    int patientID;
    int doctorID;

    char date[MAX_DATE];
    char time[MAX_TIME];
    char reason[MAX_REASON];
    char status[20];

} Appointment;

void appointmentMenu(void);
void addAppointment(void);
void viewAppointments(void);
void searchAppointment(void);
void updateAppointment(void);
void cancelAppointment(void);
void appointmentWebInit(void);
int appointmentWebCreate(const Appointment *a);
int appointmentWebList(Appointment *output, int maxRecords);
int appointmentWebFind(int id, Appointment *output);
int appointmentWebUpdate(int id, const Appointment *a);
int appointmentWebCancel(int id);
#endif
