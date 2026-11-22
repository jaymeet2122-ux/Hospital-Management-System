
#ifndef BILLING_H
#define BILLING_H

typedef struct {
    int billID;
    int patientID;
    int roomNumber;
    int days;
    float chargesPerDay;
    float otherCharges;
    float totalAmount;
    char paymentStatus[20];
} Bill;

void generateBill();
void viewBills();
void searchBill();
void updatePaymentStatus();
void billingMenu();
#define MAX_BILLS 100

void billingWebInit(void);
int billingWebCreate(const Bill *b);
int billingWebList(Bill *output, int maxRecords);
int billingWebFind(int id, Bill *output);
int billingWebUpdateStatus(int id, const char *status);
#endif
