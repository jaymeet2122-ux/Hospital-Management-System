
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "doctor.h"

static Doctor doctors[MAX_DOCTORS];
static int doctorCount = 0;

static void saveDoctors(void)
{
    FILE *file = fopen("doctors.dat", "wb");

    if (file == NULL)
    {
        printf("\nError saving doctor records!\n");
        return;
    }

    fwrite(&doctorCount, sizeof(int), 1, file);
    fwrite(doctors, sizeof(Doctor), doctorCount, file);

    fclose(file);
}

static void loadDoctors(void)
{
    FILE *file = fopen("doctors.dat", "rb");

    if (file == NULL)
    {
        doctorCount = 0;
        return;
    }

    int count;

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > MAX_DOCTORS)
    {
        doctorCount = 0;
        fclose(file);
        return;
    }

    if (fread(doctors, sizeof(Doctor), count, file) !=
        (size_t)count)
    {
        doctorCount = 0;
        fclose(file);
        return;
    }

    doctorCount = count;
    fclose(file);
}

static int readInt(const char *message)
{
    char buffer[100];
    char *end;
    long value;

    while (1)
    {
        printf("%s", message);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            return -1;

        value = strtol(buffer, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (end != buffer && *end == '\0' &&
            value >= 0 && value <= 1000000)
            return (int)value;

        printf("Invalid input! Please enter a valid number.\n");
    }
}

static void readText(const char *message, char *buffer, int size)
{
    printf("%s", message);

    if (fgets(buffer, size, stdin) == NULL)
    {
        buffer[0] = '\0';
        return;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
}

static int findDoctor(int id)
{
    for (int i = 0; i < doctorCount; i++)
    {
        if (doctors[i].doctorID == id)
            return i;
    }

    return -1;
}

static int validDoctor(const Doctor *d)
{
    if (d == NULL)
        return 0;

    if (d->doctorID <= 0 ||
        d->experience < 0 || d->experience > 80)
        return 0;

    if (memchr(d->name, '\0', sizeof(d->name)) == NULL ||
        memchr(d->specialization, '\0',
               sizeof(d->specialization)) == NULL ||
        memchr(d->phone, '\0', sizeof(d->phone)) == NULL)
        return 0;

    if (strlen(d->name) == 0 ||
        strlen(d->specialization) == 0 ||
        strlen(d->phone) == 0)
        return 0;

    return 1;
}

// =====================================
// EXISTING CONSOLE FUNCTIONS
// =====================================

void addDoctor(void)
{
    if (doctorCount >= MAX_DOCTORS)
    {
        printf("\nDoctor storage is full!\n");
        return;
    }

    Doctor d = {0};

    printf("\n========== ADD NEW DOCTOR ==========\n");

    d.doctorID = readInt("Enter Doctor ID: ");

    if (d.doctorID <= 0)
    {
        printf("Invalid Doctor ID!\n");
        return;
    }

    if (findDoctor(d.doctorID) != -1)
    {
        printf("Doctor ID already exists!\n");
        return;
    }

    readText("Enter Doctor Name: ", d.name, MAX_DOC_NAME);

    readText("Enter Specialization: ",
             d.specialization, MAX_SPECIALIZATION);

    d.experience = readInt("Enter Years of Experience: ");

    if (d.experience < 0 || d.experience > 80)
    {
        printf("Invalid experience value!\n");
        return;
    }

    readText("Enter Phone Number: ", d.phone, 15);

    if (!validDoctor(&d))
    {
        printf("Invalid doctor details!\n");
        return;
    }

    doctors[doctorCount++] = d;
    saveDoctors();

    printf("\nDoctor added successfully!\n");
}

void viewDoctors(void)
{
    if (doctorCount == 0)
    {
        printf("\nNo doctor records available.\n");
        return;
    }

    printf("\n========== DOCTOR RECORDS ==========\n");

    for (int i = 0; i < doctorCount; i++)
    {
        printf("\nDoctor ID       : %d", doctors[i].doctorID);
        printf("\nName            : %s", doctors[i].name);
        printf("\nSpecialization  : %s",
               doctors[i].specialization);
        printf("\nExperience      : %d years",
               doctors[i].experience);
        printf("\nPhone           : %s\n", doctors[i].phone);
    }
}

void searchDoctor(void)
{
    int id = readInt("\nEnter Doctor ID to search: ");
    int index = findDoctor(id);

    if (index == -1)
    {
        printf("\nDoctor not found!\n");
        return;
    }

    printf("\nDoctor Found!\n");
    printf("Name           : %s\n", doctors[index].name);
    printf("Specialization : %s\n",
           doctors[index].specialization);
    printf("Experience     : %d years\n",
           doctors[index].experience);
    printf("Phone          : %s\n", doctors[index].phone);
}

void updateDoctor(void)
{
    int id = readInt("\nEnter Doctor ID to update: ");
    int index = findDoctor(id);

    if (index == -1)
    {
        printf("\nDoctor not found!\n");
        return;
    }

    Doctor updated = doctors[index];

    readText("Enter New Name: ",
             updated.name, MAX_DOC_NAME);

    readText("Enter New Specialization: ",
             updated.specialization, MAX_SPECIALIZATION);

    updated.experience = readInt("Enter New Experience: ");

    if (updated.experience < 0 || updated.experience > 80)
    {
        printf("Invalid experience! Update not completed.\n");
        return;
    }

    readText("Enter New Phone Number: ",
             updated.phone, 15);

    if (!validDoctor(&updated))
    {
        printf("Invalid doctor details!\n");
        return;
    }

    doctors[index] = updated;
    saveDoctors();

    printf("\nDoctor details updated successfully!\n");
}

void deleteDoctor(void)
{
    int id = readInt("\nEnter Doctor ID to delete: ");
    int index = findDoctor(id);

    if (index == -1)
    {
        printf("\nDoctor not found!\n");
        return;
    }

    for (int i = index; i < doctorCount - 1; i++)
        doctors[i] = doctors[i + 1];

    doctorCount--;

    saveDoctors();

    printf("\nDoctor deleted successfully!\n");
}

void doctorMenu(void)
{
    loadDoctors();

    int choice;

    do
    {
        printf("\n====================================\n");
        printf("          DOCTOR MANAGEMENT\n");
        printf("====================================\n");

        printf("1. Add New Doctor\n");
        printf("2. View All Doctors\n");
        printf("3. Search Doctor\n");
        printf("4. Update Doctor Details\n");
        printf("5. Delete Doctor\n");
        printf("6. Back to Main Menu\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
            case 1: addDoctor(); break;
            case 2: viewDoctors(); break;
            case 3: searchDoctor(); break;
            case 4: updateDoctor(); break;
            case 5: deleteDoctor(); break;
            case 6:
                printf("\nReturning to Main Menu...\n");
                break;
            default:
                printf("\nInvalid choice!\n");
        }

    } while (choice != 6);
}

// =====================================
// WEBSITE INTEGRATION FUNCTIONS
// =====================================

void doctorWebInit(void)
{
    loadDoctors();
}

int doctorWebCreate(const Doctor *d)
{
    if (!validDoctor(d) ||
        doctorCount >= MAX_DOCTORS ||
        findDoctor(d->doctorID) != -1)
        return 0;

    doctors[doctorCount++] = *d;
    saveDoctors();

    return 1;
}

int doctorWebList(Doctor *output, int maxRecords)
{
    if (output == NULL || maxRecords <= 0)
        return 0;

    int count = doctorCount < maxRecords
                    ? doctorCount : maxRecords;

    for (int i = 0; i < count; i++)
        output[i] = doctors[i];

    return count;
}

int doctorWebFind(int id, Doctor *output)
{
    int index = findDoctor(id);

    if (index == -1 || output == NULL)
        return 0;

    *output = doctors[index];
    return 1;
}

int doctorWebUpdate(int id, const Doctor *d)
{
    int index = findDoctor(id);

    if (index == -1 || !validDoctor(d))
        return 0;

    Doctor updated = *d;
    updated.doctorID = id;

    doctors[index] = updated;
    saveDoctors();

    return 1;
}

int doctorWebDelete(int id)
{
    int index = findDoctor(id);

    if (index == -1)
        return 0;

    for (int i = index; i < doctorCount - 1; i++)
        doctors[i] = doctors[i + 1];

    doctorCount--;
    saveDoctors();

    return 1;
}
