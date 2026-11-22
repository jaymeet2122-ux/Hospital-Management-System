
#include <stdio.h>
#include <string.h>
#include "billing.h"

#define MAX_BILLS 100

Bill bills[MAX_BILLS];
int billCount = 0;

// Save billing records permanently
static void saveBills(void)
{
    FILE *file = fopen("bills.dat", "wb");

    if (file == NULL)
    {
        printf("\nError saving billing records!\n");
        return;
    }

    fwrite(&billCount, sizeof(int), 1, file);
    fwrite(bills, sizeof(Bill), billCount, file);

    fclose(file);
}

// Load billing records from file
static void loadBills(void)
{
    FILE *file = fopen("bills.dat", "rb");

    if (file == NULL)
    {
        billCount = 0;
        return;
    }

    int count;

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > MAX_BILLS)
    {
        billCount = 0;
        fclose(file);
        return;
    }

    if (fread(bills, sizeof(Bill), count, file) != (size_t)count)
    {
        billCount = 0;
        fclose(file);
        return;
    }

    billCount = count;
    fclose(file);
}

void generateBill() {
    if (billCount >= MAX_BILLS) {
        printf("\nBill limit reached!\n");
        return;
    }

    Bill b;

    printf("\nEnter Bill ID: ");
    scanf("%d", &b.billID);

    for (int i = 0; i < billCount; i++) {
        if (bills[i].billID == b.billID) {
            printf("\nBill ID already exists!\n");
            return;
        }
    }

    printf("Enter Patient ID: ");
    scanf("%d", &b.patientID);

    printf("Enter Room Number: ");
    scanf("%d", &b.roomNumber);

    printf("Enter Number of Days: ");
    scanf("%d", &b.days);

    printf("Enter Charges Per Day: ");
    scanf("%f", &b.chargesPerDay);

    printf("Enter Other Charges: ");
    scanf("%f", &b.otherCharges);

    b.totalAmount =
        (b.days * b.chargesPerDay) + b.otherCharges;

    strcpy(b.paymentStatus, "Unpaid");

    bills[billCount++] = b;

    saveBills();

    printf("\nBill generated successfully!\n");
    printf("Total Amount: Rs. %.2f\n", b.totalAmount);
}

void viewBills() {
    if (billCount == 0) {
        printf("\nNo bills available!\n");
        return;
    }

    printf("\n===== ALL BILLS =====\n");

    for (int i = 0; i < billCount; i++) {
        printf("\nBill ID: %d", bills[i].billID);
        printf("\nPatient ID: %d", bills[i].patientID);
        printf("\nRoom Number: %d", bills[i].roomNumber);
        printf("\nDays: %d", bills[i].days);
        printf("\nCharges Per Day: Rs. %.2f",
               bills[i].chargesPerDay);
        printf("\nOther Charges: Rs. %.2f",
               bills[i].otherCharges);
        printf("\nTotal Amount: Rs. %.2f",
               bills[i].totalAmount);
        printf("\nPayment Status: %s",
               bills[i].paymentStatus);
        printf("\n-------------------------\n");
    }
}

void searchBill() {
    int id, found = 0;

    printf("\nEnter Bill ID to search: ");
    scanf("%d", &id);

    for (int i = 0; i < billCount; i++) {
        if (bills[i].billID == id) {
            printf("\nBill Found!");
            printf("\nBill ID: %d", bills[i].billID);
            printf("\nPatient ID: %d", bills[i].patientID);
            printf("\nTotal Amount: Rs. %.2f",
                   bills[i].totalAmount);
            printf("\nPayment Status: %s\n",
                   bills[i].paymentStatus);

            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nBill not found!\n");
    }
}

void updatePaymentStatus() {
    int id, choice, found = 0;

    printf("\nEnter Bill ID: ");
    scanf("%d", &id);

    for (int i = 0; i < billCount; i++) {
        if (bills[i].billID == id) {
            found = 1;

            printf("\n1. Paid");
            printf("\n2. Unpaid");
            printf("\nEnter new payment status: ");
            scanf("%d", &choice);

            if (choice == 1) {
                strcpy(bills[i].paymentStatus, "Paid");
                saveBills();
                printf("\nPayment marked as Paid!\n");
            } else if (choice == 2) {
                strcpy(bills[i].paymentStatus, "Unpaid");
                saveBills();
                printf("\nPayment marked as Unpaid!\n");
            } else {
                printf("\nInvalid choice!\n");
            }

            break;
        }
    }

    if (!found) {
        printf("\nBill not found!\n");
    }
}

void billingMenu() {
    loadBills();
    int choice;

    do {
        printf("\n===== BILLING MANAGEMENT =====");
        printf("\n1. Generate Bill");
        printf("\n2. View All Bills");
        printf("\n3. Search Bill");
        printf("\n4. Update Payment Status");
        printf("\n0. Back to Main Menu");
        printf("\nEnter your choice: ");

        scanf("%d", &choice);

        switch (choice) {
            case 1: generateBill(); break;
            case 2: viewBills(); break;
            case 3: searchBill(); break;
            case 4: updatePaymentStatus(); break;
            case 0: printf("\nReturning to Main Menu...\n"); break;
            default: printf("\nInvalid choice!\n");
        }

    } while (choice != 0);
}
// =====================================
// WEBSITE API FUNCTIONS
// =====================================

void billingWebInit(void)
{
    loadBills();
}

int billingWebCreate(const Bill *b)
{
    if (b == NULL || billCount >= MAX_BILLS)
        return 0;

    if (b->billID <= 0 ||
        b->patientID <= 0 ||
        b->roomNumber <= 0 ||
        b->days <= 0 ||
        b->chargesPerDay < 0 ||
        b->otherCharges < 0)
        return 0;

    for (int i = 0; i < billCount; i++)
    {
        if (bills[i].billID == b->billID)
            return 0;
    }

    bills[billCount] = *b;

    bills[billCount].totalAmount =
        (b->days * b->chargesPerDay) + b->otherCharges;

    strcpy(bills[billCount].paymentStatus, "Unpaid");

    billCount++;
    saveBills();

    return 1;
}

int billingWebList(Bill *output, int maxRecords)
{
    if (output == NULL || maxRecords <= 0)
        return 0;

    int count = billCount;

    if (count > maxRecords)
        count = maxRecords;

    for (int i = 0; i < count; i++)
        output[i] = bills[i];

    return count;
}

int billingWebFind(int id, Bill *output)
{
    if (output == NULL)
        return 0;

    for (int i = 0; i < billCount; i++)
    {
        if (bills[i].billID == id)
        {
            *output = bills[i];
            return 1;
        }
    }

    return 0;
}

int billingWebUpdateStatus(int id, const char *status)
{
    if (status == NULL)
        return 0;

    if (strcmp(status, "Paid") != 0 &&
        strcmp(status, "Unpaid") != 0)
        return 0;

    for (int i = 0; i < billCount; i++)
    {
        if (bills[i].billID == id)
        {
            strcpy(bills[i].paymentStatus, status);
            saveBills();

            return 1;
        }
    }

    return 0;
}