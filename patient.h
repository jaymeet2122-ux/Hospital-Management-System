
#ifndef PATIENT_H
#define PATIENT_H

#define MAX_NAME 50
#define MAX_GENDER 15
#define MAX_DIAGNOSIS 100
#define MAX_PHONE 16

typedef struct
{
    int patientID;
    char name[MAX_NAME];
    int age;
    char gender[MAX_GENDER];
    char diagnosis[MAX_DIAGNOSIS];
    char phone[MAX_PHONE];

} Patient;

void patientMenu(void);
void registerPatient(void);
void viewPatients(void);
void searchPatient(void);
void updatePatient(void);
void deletePatient(void);

// Functions for website integration
void patientWebInit(void);

int patientWebCreate(const Patient *p);
int patientWebList(Patient *output, int maxRecords);
int patientWebFind(int id, Patient *output);
int patientWebUpdate(int id, const Patient *p);
int patientWebDelete(int id);

#endif
