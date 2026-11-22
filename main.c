
#include <stdio.h>
#include "patient.h"
#include "doctor.h"
#include "appointment.h"
#include "room.h"
#include "billing.h"

int main()
{
    int choice;

    do
    {
        printf("\n====================================\n");
        printf("      HOSPITAL MANAGEMENT SYSTEM\n");
        printf("====================================\n");

        printf("1. Patient Management\n");
        printf("2. Doctor Management\n");
        printf("3. Appointment Management\n");
        printf("4. Room Management\n");
        printf("5. Billing System\n");
        printf("6. Exit\n");

        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                patientMenu();
                break;

            case 2:
                doctorMenu();
                break;

            case 3:
                appointmentMenu();
                break;

            case 4:
                roomMenu();
                break;

            case 5:
                billingMenu();
                break;

            case 6:
                printf("Thank you for using our system!\n");
                break;

            default:
                printf("Invalid choice! Try again.\n");
        }

    } while(choice != 6);

    return 0;
}
