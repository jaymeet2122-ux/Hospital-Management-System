
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "patient.h"

#define MAX_PATIENTS 100

static Patient patients[MAX_PATIENTS];
static int patientCount = 0;


// Save patient records permanently
static void savePatients(void)
{
    FILE *file = fopen("patients.dat", "wb");

    if (file == NULL)
    {
        printf("\nError saving patient records!\n");
        return;
    }

    fwrite(&patientCount, sizeof(int), 1, file);
    fwrite(patients, sizeof(Patient), patientCount, file);

    fclose(file);
}

// Load patient records from file
static void loadPatients(void)
{
    FILE *file = fopen("patients.dat", "rb");

    if (file == NULL)
    {
        patientCount = 0;
        return;
    }

    int count;

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > MAX_PATIENTS)
    {
        patientCount = 0;
        fclose(file);
        return;
    }

    if (fread(patients, sizeof(Patient), count, file) != (size_t)count)
    {
        patientCount = 0;
        fclose(file);
        return;
    }

    patientCount = count;
    fclose(file);
}


// Read a line of text safely
static void readText(const char *message, char *buffer, int size)
{
    printf("%s", message);

    if (fgets(buffer, size, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

// Read an integer safely
static int readNumber(const char *message)
{
    char buffer[50];
    char *end;
    long value;

    while (1)
    {
        readText(message, buffer, sizeof(buffer));

        value = strtol(buffer, &end, 10);

        if (end != buffer && *end == '\0')
            return (int)value;

        printf("Invalid input! Please enter a number.\n");
    }
}

// Find patient using ID
static int findPatient(int id)
{
    for (int i = 0; i < patientCount; i++)
    {
        if (patients[i].patientID == id)
            return i;
    }

    return -1;
}

// Register a new patient
void registerPatient(void)
{
    if (patientCount >= MAX_PATIENTS)
    {
        printf("\nPatient storage is full!\n");
        return;
    }

    Patient p;

    printf("\n========== PATIENT REGISTRATION ==========\n");

    p.patientID = readNumber("Enter Patient ID: ");

    if (p.patientID <= 0 || findPatient(p.patientID) != -1)
    {
        printf("Invalid or duplicate Patient ID!\n");
        return;
    }

    readText("Enter Patient Name: ", p.name, MAX_NAME);

    if (strlen(p.name) == 0)
    {
        printf("Name cannot be empty!\n");
        return;
    }

    p.age = readNumber("Enter Age: ");

    if (p.age <= 0 || p.age > 120)
    {
        printf("Invalid age!\n");
        return;
    }

    readText("Enter Gender: ", p.gender, MAX_GENDER);
    readText("Enter Diagnosis: ", p.diagnosis, MAX_DIAGNOSIS);
    readText("Enter Phone Number: ", p.phone, MAX_PHONE);

    patients[patientCount] = p;
    patientCount++;
    
    savePatients();

    printf("\nPatient registered successfully!\n");
}

// Display all patients
void viewPatients(void)
{
    if (patientCount == 0)
    {
        printf("\nNo patient records available.\n");
        return;
    }

    printf("\n========== ALL PATIENT RECORDS ==========\n");

    for (int i = 0; i < patientCount; i++)
    {
        printf("\nPatient ID : %d\n", patients[i].patientID);
        printf("Name       : %s\n", patients[i].name);
        printf("Age        : %d\n", patients[i].age);
        printf("Gender     : %s\n", patients[i].gender);
        printf("Diagnosis  : %s\n", patients[i].diagnosis);
        printf("Phone      : %s\n", patients[i].phone);

        printf("-----------------------------------------\n");
    }
}

// Search patient
void searchPatient(void)
{
    int id = readNumber("\nEnter Patient ID to search: ");

    int index = findPatient(id);

    if (index == -1)
    {
        printf("\nPatient not found!\n");
        return;
    }

    printf("\nPatient Found!\n");
    printf("ID        : %d\n", patients[index].patientID);
    printf("Name      : %s\n", patients[index].name);
    printf("Age       : %d\n", patients[index].age);
    printf("Gender    : %s\n", patients[index].gender);
    printf("Diagnosis : %s\n", patients[index].diagnosis);
    printf("Phone     : %s\n", patients[index].phone);
}

// Update patient
void updatePatient(void)
{
    int id = readNumber("\nEnter Patient ID to update: ");

    int index = findPatient(id);

    if (index == -1)
    {
        printf("\nPatient not found!\n");
        return;
    }

    printf("\nEnter new patient details:\n");

    readText("New Name: ", patients[index].name, MAX_NAME);

    patients[index].age = readNumber("New Age: ");

    if (patients[index].age <= 0 ||
        patients[index].age > 120)
    {
        printf("Invalid age! Update cancelled.\n");
        return;
    }

    readText("New Gender: ", patients[index].gender, MAX_GENDER);
    readText("New Diagnosis: ", patients[index].diagnosis, MAX_DIAGNOSIS);
    readText("New Phone: ", patients[index].phone, MAX_PHONE);

    printf("\nPatient updated successfully!\n");
    savePatients();
}

// Delete patient
void deletePatient(void)
{
    int id = readNumber("\nEnter Patient ID to delete: ");

    int index = findPatient(id);

    if (index == -1)
    {
        printf("\nPatient not found!\n");
        return;
    }

    for (int i = index; i < patientCount - 1; i++)
    {
        patients[i] = patients[i + 1];
    }

    patientCount--;
    savePatients();

    printf("\nPatient deleted successfully!\n");
}

// Patient management menu
void patientMenu(void)
{
    loadPatients();
    int choice;

    do
    {
        printf("\n========================================\n");
        printf("          PATIENT MANAGEMENT\n");
        printf("========================================\n");

        printf("1. Register New Patient\n");
        printf("2. View All Patients\n");
        printf("3. Search Patient\n");
        printf("4. Update Patient Details\n");
        printf("5. Delete Patient\n");
        printf("6. Back to Main Menu\n");

        choice = readNumber("\nEnter your choice: ");

        switch (choice)
        {
            case 1:
                registerPatient();
                break;

            case 2:
                viewPatients();
                break;

            case 3:
                searchPatient();
                break;

            case 4:
                updatePatient();
                break;

            case 5:
                deletePatient();
                break;

            case 6:
                printf("\nReturning to Main Menu...\n");
                break;

            default:
                printf("\nInvalid choice! Try again.\n");
        }

    } while (choice != 6);
}

// ==========================================
// WEBSITE INTEGRATION FUNCTIONS
// ==========================================

void patientWebInit(void)
{
    loadPatients();
}

int patientWebCreate(const Patient *p)
{
    if (p == NULL || patientCount >= MAX_PATIENTS)
        return 0;

    if (p->patientID <= 0 ||
        findPatient(p->patientID) != -1 ||
        strlen(p->name) == 0 ||
        p->age <= 0 || p->age > 120)
        return 0;

    patients[patientCount] = *p;
    patientCount++;

    savePatients();
    return 1;
}

int patientWebList(Patient *output, int maxRecords)
{
    if (output == NULL || maxRecords <= 0)
        return 0;

    int count = patientCount < maxRecords
                    ? patientCount
                    : maxRecords;

    for (int i = 0; i < count; i++)
        output[i] = patients[i];

    return count;
}

int patientWebFind(int id, Patient *output)
{
    int index = findPatient(id);

    if (index == -1 || output == NULL)
        return 0;

    *output = patients[index];
    return 1;
}

int patientWebUpdate(int id, const Patient *p)
{
    int index = findPatient(id);

    if (index == -1 || p == NULL ||
        p->age <= 0 || p->age > 120 ||
        strlen(p->name) == 0)
        return 0;

    Patient updated = *p;
    updated.patientID = id;

    patients[index] = updated;

    savePatients();
    return 1;
}

int patientWebDelete(int id)
{
    int index = findPatient(id);

    if (index == -1)
        return 0;

    for (int i = index; i < patientCount - 1; i++)
        patients[i] = patients[i + 1];

    patientCount--;

    savePatients();
    return 1;
}
