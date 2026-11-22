
#ifndef ROOM_H
#define ROOM_H

typedef struct {
    int roomID;
    int roomNumber;
    char roomType[30];
    float chargesPerDay;
    int isOccupied;
    int patientID;
} Room;

void addRoom();
void viewRooms();
void searchRoom();
void updateRoom();
void allocateRoom();
void releaseRoom();
void roomMenu();
#define MAX_ROOMS 100

void roomWebInit(void);
int roomWebCreate(const Room *r);
int roomWebList(Room *output, int maxRecords);
int roomWebFind(int id, Room *output);
int roomWebUpdate(int id, const Room *r);
int roomWebAllocate(int id, int patientID);
int roomWebRelease(int id);
#endif
