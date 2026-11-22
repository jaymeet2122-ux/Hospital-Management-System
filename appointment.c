
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "appointment.h"

static Appointment appointments[MAX_APPOINTMENTS];
static int appointmentCount = 0;

// Save appointment records permanently
static void saveAppointments(void)
{
    FILE *file = fopen("appointments.dat", "wb");

    if (file == NULL)
    {
        printf("\nError saving appointment records!\n");
        return;
    }

    fwrite(&appointmentCount, sizeof(int), 1, file);
    fwrite(appointments, sizeof(Appointment), appointmentCount, file);

    fclose(file);
}

// Load appointment records from file
static void loadAppointments(void)
{
    FILE *file = fopen("appointments.dat", "rb");

    if (file == NULL)
    {
        appointmentCount = 0;
        return;
    }

    int count;

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > MAX_APPOINTMENTS)
    {
        appointmentCount = 0;
        fclose(file);
        return;
    }

    if (fread(appointments, sizeof(Appointment), count, file) != (size_t)count)
    {
        appointmentCount = 0;
        fclose(file);
        return;
    }

    appointmentCount = count;
    fclose(file);
}

// Safely read an integer
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

        if (end != buffer && *end == '\0'
            && value > 0 && value <= 1000000)
            return (int)value;

        printf("Invalid input! Enter a positive number.\n");
    }
}

// Safely read text
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

// Find appointment using ID
static int findAppointment(int id)
{
    for (int i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].appointmentID == id)
            return i;
    }

    return -1;
}

// Add appointment
void addAppointment(void)
{
    if (appointmentCount >= MAX_APPOINTMENTS)
    {
        printf("\nAppointment storage is full!\n");
        return;
    }

    Appointment a;

    printf("\n========== SCHEDULE APPOINTMENT ==========\n");

    a.appointmentID = readInt("Enter Appointment ID: ");

    if (a.appointmentID <= 0)
        return;

    if (findAppointment(a.appointmentID) != -1)
    {
        printf("Appointment ID already exists!\n");
        return;
    }

    a.patientID = readInt("Enter Patient ID: ");

    if (a.patientID <= 0)
        return;

    a.doctorID = readInt("Enter Doctor ID: ");

    if (a.doctorID <= 0)
        return;

    readText("Enter Date (DD-MM-YYYY): ",
             a.date, MAX_DATE);

    readText("Enter Time (HH:MM): ",
             a.time, MAX_TIME);

    readText("Enter Appointment Reason: ",
             a.reason, MAX_REASON);

    strcpy(a.status, "Scheduled");

    appointments[appointmentCount] = a;
    appointmentCount++;

    saveAppointments();

    printf("\nAppointment scheduled successfully!\n");
}

// View all appointments
void viewAppointments(void)
{
    if (appointmentCount == 0)
    {
        printf("\nNo appointments available.\n");
        return;
    }

    printf("\n========== ALL APPOINTMENTS ==========\n");

    for (int i = 0; i < appointmentCount; i++)
    {
        printf("\nAppointment ID : %d",
               appointments[i].appointmentID);

        printf("\nPatient ID     : %d",
               appointments[i].patientID);

        printf("\nDoctor ID      : %d",
               appointments[i].doctorID);

        printf("\nDate           : %s",
               appointments[i].date);

        printf("\nTime           : %s",
               appointments[i].time);

        printf("\nReason         : %s",
               appointments[i].reason);

        printf("\nStatus         : %s\n",
               appointments[i].status);
    }
}

// Search appointment
void searchAppointment(void)
{
    int id = readInt("\nEnter Appointment ID: ");

    int index = findAppointment(id);

    if (index == -1)
    {
        printf("\nAppointment not found!\n");
        return;
    }

    printf("\nAppointment Found!\n");

    printf("Patient ID : %d\n",
           appointments[index].patientID);

    printf("Doctor ID  : %d\n",
           appointments[index].doctorID);

    printf("Date       : %s\n",
           appointments[index].date);

    printf("Time       : %s\n",
           appointments[index].time);

    printf("Reason     : %s\n",
           appointments[index].reason);

    printf("Status     : %s\n",
           appointments[index].status);
}

// Update appointment
void updateAppointment(void)
{
    int id = readInt("\nEnter Appointment ID to update: ");

    int index = findAppointment(id);

    if (index == -1)
    {
        printf("\nAppointment not found!\n");
        return;
    }

    if (strcmp(appointments[index].status, "Cancelled") == 0)
    {
        printf("\nCannot update a cancelled appointment!\n");
        return;
    }

    appointments[index].patientID =
        readInt("Enter New Patient ID: ");

    appointments[index].doctorID =
        readInt("Enter New Doctor ID: ");

    readText("Enter New Date (DD-MM-YYYY): ",
             appointments[index].date, MAX_DATE);

    readText("Enter New Time (HH:MM): ",
             appointments[index].time, MAX_TIME);

    readText("Enter New Reason: ",
             appointments[index].reason, MAX_REASON);

    saveAppointments();

    printf("\nAppointment updated successfully!\n");
}

// Cancel appointment
void cancelAppointment(void)
{
    int id = readInt("\nEnter Appointment ID to cancel: ");

    int index = findAppointment(id);

    if (index == -1)
    {
        printf("\nAppointment not found!\n");
        return;
    }

    if (strcmp(appointments[index].status, "Cancelled") == 0)
    {
        printf("\nAppointment is already cancelled!\n");
        return;
    }

    strcpy(appointments[index].status, "Cancelled");

    saveAppointments();

    printf("\nAppointment cancelled successfully!\n");
}

// Appointment menu
void appointmentMenu(void)
{
    loadAppointments();
    int choice;

    do
    {
        printf("\n====================================\n");
        printf("       APPOINTMENT MANAGEMENT\n");
        printf("====================================\n");

        printf("1. Schedule New Appointment\n");
        printf("2. View All Appointments\n");
        printf("3. Search Appointment\n");
        printf("4. Update Appointment\n");
        printf("5. Cancel Appointment\n");
        printf("6. Back to Main Menu\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
            case 1:
                addAppointment();
                break;

            case 2:
                viewAppointments();
                break;

            case 3:
                searchAppointment();
                break;

            case 4:
                updateAppointment();
                break;

            case 5:
                cancelAppointment();
                break;

            case 6:
                printf("\nReturning to Main Menu...\n");
                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while (choice != 6);
}
// =====================================
// WEBSITE API FUNCTIONS
// =====================================

void appointmentWebInit(void)
{
    loadAppointments();
}

int appointmentWebCreate(const Appointment *a)
{
    if (a == NULL || appointmentCount >= MAX_APPOINTMENTS)
        return 0;

    if (a->appointmentID <= 0 ||
        a->patientID <= 0 ||
        a->doctorID <= 0 ||
        strlen(a->date) == 0 ||
        strlen(a->time) == 0 ||
        strlen(a->reason) == 0)
        return 0;

    for (int i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].appointmentID == a->appointmentID)
            return 0;
    }

    appointments[appointmentCount] = *a;
    strcpy(appointments[appointmentCount].status, "Scheduled");

    appointmentCount++;
    saveAppointments();

    return 1;
}

int appointmentWebList(Appointment *output, int maxRecords)
{
    if (output == NULL || maxRecords <= 0)
        return 0;

    int count = appointmentCount;

    if (count > maxRecords)
        count = maxRecords;

    for (int i = 0; i < count; i++)
        output[i] = appointments[i];

    return count;
}

int appointmentWebFind(int id, Appointment *output)
{
    if (output == NULL)
        return 0;

    for (int i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].appointmentID == id)
        {
            *output = appointments[i];
            return 1;
        }
    }

    return 0;
}

int appointmentWebUpdate(int id, const Appointment *a)
{
    if (a == NULL)
        return 0;

    for (int i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].appointmentID == id)
        {
            if (strcmp(appointments[i].status, "Cancelled") == 0)
                return 0;

            if (a->patientID <= 0 ||
                a->doctorID <= 0 ||
                strlen(a->date) == 0 ||
                strlen(a->time) == 0 ||
                strlen(a->reason) == 0)
                return 0;

            appointments[i].patientID = a->patientID;
            appointments[i].doctorID = a->doctorID;
            strcpy(appointments[i].date, a->date);
            strcpy(appointments[i].time, a->time);
            strcpy(appointments[i].reason, a->reason);

            saveAppointments();
            return 1;
        }
    }

    return 0;
}

int appointmentWebCancel(int id)
{
    for (int i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].appointmentID == id)
        {
            if (strcmp(appointments[i].status, "Cancelled") == 0)
                return 0;

            strcpy(appointments[i].status, "Cancelled");
            saveAppointments();

            return 1;
        }
    }

    return 0;
}