
#ifndef DOCTOR_H
#define DOCTOR_H

#define MAX_DOCTORS 100
#define MAX_DOC_NAME 50
#define MAX_SPECIALIZATION 50

typedef struct
{
    int doctorID;
    char name[MAX_DOC_NAME];
    char specialization[MAX_SPECIALIZATION];
    int experience;
    char phone[15];

} Doctor;

void doctorMenu(void);
void addDoctor(void);
void viewDoctors(void);
void searchDoctor(void);
void updateDoctor(void);
void deleteDoctor(void);

// Website integration functions
void doctorWebInit(void);

int doctorWebCreate(const Doctor *d);
int doctorWebList(Doctor *output, int maxRecords);
int doctorWebFind(int id, Doctor *output);
int doctorWebUpdate(int id, const Doctor *d);
int doctorWebDelete(int id);

#endif
