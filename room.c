
#include <stdio.h>
#include <string.h>
#include "room.h"

#define MAX_ROOMS 100

Room rooms[MAX_ROOMS];
int roomCount = 0;

// Save room records permanently
static void saveRooms(void)
{
    FILE *file = fopen("rooms.dat", "wb");

    if (file == NULL)
    {
        printf("\nError saving room records!\n");
        return;
    }

    fwrite(&roomCount, sizeof(int), 1, file);
    fwrite(rooms, sizeof(Room), roomCount, file);

    fclose(file);
}

// Load room records from file
static void loadRooms(void)
{
    FILE *file = fopen("rooms.dat", "rb");

    if (file == NULL)
    {
        roomCount = 0;
        return;
    }

    int count;

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > MAX_ROOMS)
    {
        roomCount = 0;
        fclose(file);
        return;
    }

    if (fread(rooms, sizeof(Room), count, file) != (size_t)count)
    {
        roomCount = 0;
        fclose(file);
        return;
    }

    roomCount = count;
    fclose(file);
}

void addRoom() {
    if (roomCount >= MAX_ROOMS) {
        printf("\nRoom limit reached!\n");
        return;
    }

    Room r;

    printf("\nEnter Room ID: ");
    scanf("%d", &r.roomID);

    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].roomID == r.roomID) {
            printf("\nRoom ID already exists!\n");
            return;
        }
    }

    printf("Enter Room Number: ");
    scanf("%d", &r.roomNumber);

    printf("Enter Room Type (General/Private/ICU): ");
    scanf(" %29[^\n]", r.roomType);

    printf("Enter Charges Per Day: ");
    scanf("%f", &r.chargesPerDay);

    r.isOccupied = 0;
    r.patientID = 0;

    rooms[roomCount++] = r;

    saveRooms();

    printf("\nRoom added successfully!\n");
}

void viewRooms() {
    if (roomCount == 0) {
        printf("\nNo rooms available!\n");
        return;
    }

    printf("\n--- ROOM LIST ---\n");

    for (int i = 0; i < roomCount; i++) {
        printf("\nRoom ID: %d", rooms[i].roomID);
        printf("\nRoom Number: %d", rooms[i].roomNumber);
        printf("\nRoom Type: %s", rooms[i].roomType);
        printf("\nCharges Per Day: %.2f", rooms[i].chargesPerDay);
        printf("\nStatus: %s",
               rooms[i].isOccupied ? "Occupied" : "Available");

        if (rooms[i].isOccupied) {
            printf("\nPatient ID: %d", rooms[i].patientID);
        }

        printf("\n----------------------\n");
    }
}

void searchRoom() {
    int id, found = 0;

    printf("\nEnter Room ID to search: ");
    scanf("%d", &id);

    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].roomID == id) {
            printf("\nRoom Found!");
            printf("\nRoom Number: %d", rooms[i].roomNumber);
            printf("\nRoom Type: %s", rooms[i].roomType);
            printf("\nCharges Per Day: %.2f", rooms[i].chargesPerDay);
            printf("\nStatus: %s",
                   rooms[i].isOccupied ? "Occupied" : "Available");
            if (rooms[i].isOccupied) {
                printf("\nPatient ID: %d", rooms[i].patientID);
            }
            printf("\n");
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nRoom not found!\n");
    }
}

void updateRoom() {
    int id, found = 0;

    printf("\nEnter Room ID to update: ");
    scanf("%d", &id);

    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].roomID == id) {
            printf("Enter New Room Number: ");
            scanf("%d", &rooms[i].roomNumber);

            printf("Enter New Room Type: ");
            scanf(" %29[^\n]", rooms[i].roomType);

            printf("Enter New Charges Per Day: ");
            scanf("%f", &rooms[i].chargesPerDay);
            
            saveRooms();

            printf("\nRoom updated successfully!\n");
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nRoom not found!\n");
    }
}

void allocateRoom() {
    int id, patientID, found = 0;

    printf("\nEnter Room ID to allocate: ");
    scanf("%d", &id);

    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].roomID == id) {
            found = 1;

            if (rooms[i].isOccupied) {
                printf("\nRoom is already occupied!\n");
                return;
            }

            printf("Enter Patient ID: ");
            scanf("%d", &patientID);

            rooms[i].isOccupied = 1;
            rooms[i].patientID = patientID;

            saveRooms();

            printf("\nRoom allocated successfully!\n");
            return;
        }
    }

    if (!found) {
        printf("\nRoom not found!\n");
    }
}

void releaseRoom() {
    int id, found = 0;

    printf("\nEnter Room ID to release: ");
    scanf("%d", &id);

    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].roomID == id) {
            found = 1;

            if (!rooms[i].isOccupied) {
                printf("\nRoom is already available!\n");
                return;
            }

            rooms[i].isOccupied = 0;
            rooms[i].patientID = 0;

            saveRooms();

            printf("\nRoom released successfully!\n");
            return;
        }
    }

    if (!found) {
        printf("\nRoom not found!\n");
    }
}

void roomMenu() {
    loadRooms();
    int choice;

    do {
        printf("\n===== ROOM MANAGEMENT =====");
        printf("\n1. Add Room");
        printf("\n2. View Rooms");
        printf("\n3. Search Room");
        printf("\n4. Update Room");
        printf("\n5. Allocate Room");
        printf("\n6. Release Room");
        printf("\n0. Back to Main Menu");
        printf("\nEnter your choice: ");

        scanf("%d", &choice);

        switch (choice) {
            case 1: addRoom(); break;
            case 2: viewRooms(); break;
            case 3: searchRoom(); break;
            case 4: updateRoom(); break;
            case 5: allocateRoom(); break;
            case 6: releaseRoom(); break;
            case 0: printf("\nReturning to Main Menu...\n"); break;
            default: printf("\nInvalid choice!\n");
        }

    } while (choice != 0);
}
// =====================================
// WEBSITE API FUNCTIONS
// =====================================

void roomWebInit(void)
{
    loadRooms();
}

int roomWebCreate(const Room *r)
{
    if (r == NULL || roomCount >= MAX_ROOMS)
        return 0;

    if (r->roomID <= 0 ||
        r->roomNumber <= 0 ||
        strlen(r->roomType) == 0 ||
        r->chargesPerDay < 0)
        return 0;

    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].roomID == r->roomID ||
            rooms[i].roomNumber == r->roomNumber)
            return 0;
    }

    rooms[roomCount] = *r;
    rooms[roomCount].isOccupied = 0;
    rooms[roomCount].patientID = 0;

    roomCount++;
    saveRooms();

    return 1;
}

int roomWebList(Room *output, int maxRecords)
{
    if (output == NULL || maxRecords <= 0)
        return 0;

    int count = roomCount;

    if (count > maxRecords)
        count = maxRecords;

    for (int i = 0; i < count; i++)
        output[i] = rooms[i];

    return count;
}

int roomWebFind(int id, Room *output)
{
    if (output == NULL)
        return 0;

    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].roomID == id)
        {
            *output = rooms[i];
            return 1;
        }
    }

    return 0;
}

int roomWebUpdate(int id, const Room *r)
{
    if (r == NULL)
        return 0;

    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].roomID == id)
        {
            if (r->roomNumber <= 0 ||
                strlen(r->roomType) == 0 ||
                r->chargesPerDay < 0)
                return 0;

            for (int j = 0; j < roomCount; j++)
            {
                if (j != i &&
                    rooms[j].roomNumber == r->roomNumber)
                    return 0;
            }

            rooms[i].roomNumber = r->roomNumber;
            strcpy(rooms[i].roomType, r->roomType);
            rooms[i].chargesPerDay = r->chargesPerDay;

            saveRooms();
            return 1;
        }
    }

    return 0;
}

int roomWebAllocate(int id, int patientID)
{
    if (patientID <= 0)
        return 0;

    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].roomID == id)
        {
            if (rooms[i].isOccupied)
                return 0;

            rooms[i].isOccupied = 1;
            rooms[i].patientID = patientID;

            saveRooms();
            return 1;
        }
    }

    return 0;
}

int roomWebRelease(int id)
{
    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].roomID == id)
        {
            if (!rooms[i].isOccupied)
                return 0;

            rooms[i].isOccupied = 0;
            rooms[i].patientID = 0;

            saveRooms();
            return 1;
        }
    }

    return 0;
}