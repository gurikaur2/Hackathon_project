#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PARKING_SPACES 50
#define HOURLY_RATE 100.0

// Structure to store parking details
typedef struct {
    int space_id;
    char vehicle_number[20];
    time_t entry_time;
    time_t exit_time;
    double parking_fee;
    int is_occupied;
} ParkingSpace;

// Global parking records array
ParkingSpace parking_lot[MAX_PARKING_SPACES];

// Function prototypes
void initialize_parking_lot();
void display_menu();
void add_vehicle();
void update_exit();
void delete_vehicle();
void display_records();
void calculate_fee(ParkingSpace *space);
double calculate_duration(time_t entry, time_t exit);

int main() {
    int choice;

    initialize_parking_lot();

    // Menu-driven program
    do {
        display_menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume trailing newline

        switch (choice) {
            case 1:
                add_vehicle();
                break;
            case 2:
                update_exit();
                break;
            case 3:
                delete_vehicle();
                break;
            case 4:
                display_records();
                break;
            case 5:
                printf("Exiting program. Thank you!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 5);

    return 0;
}

// Initialize parking lot
void initialize_parking_lot() {
    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        parking_lot[i].space_id = i + 1;
        parking_lot[i].is_occupied = 0;
    }
}

// Display menu options
void display_menu() {
    printf("\n=== Parking Management System ===\n");
    printf("1. Add Vehicle (Entry)\n");
    printf("2. Update Vehicle (Exit)\n");
    printf("3. Delete Vehicle Record\n");
    printf("4. Display Parking Records\n");
    printf("5. Exit\n");
}

// Add vehicle entry
void add_vehicle() {
    int i;
    for (i = 0; i < MAX_PARKING_SPACES; i++) {
        if (!parking_lot[i].is_occupied) {
            printf("Enter vehicle number: ");
            fgets(parking_lot[i].vehicle_number, 20, stdin);
            parking_lot[i].vehicle_number[strcspn(parking_lot[i].vehicle_number, "\n")] = '\0'; // Remove newline
            parking_lot[i].entry_time = time(NULL);
            parking_lot[i].is_occupied = 1;
            printf("Vehicle added to parking space %d at %s", parking_lot[i].space_id, ctime(&parking_lot[i].entry_time));
            return;
        }
    }
    printf("Sorry, no parking spaces available.\n");
}

// Update vehicle exit
void update_exit() {
    char vehicle_number[20];
    int found = 0;

    printf("Enter vehicle number: ");
    fgets(vehicle_number, 20, stdin);
    vehicle_number[strcspn(vehicle_number, "\n")] = '\0';

    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        if (parking_lot[i].is_occupied && strcmp(parking_lot[i].vehicle_number, vehicle_number) == 0) {
            parking_lot[i].exit_time = time(NULL);
            calculate_fee(&parking_lot[i]);
            printf("Vehicle %s exited at %s", parking_lot[i].vehicle_number, ctime(&parking_lot[i].exit_time));
            printf("Parking Fee: ₹%.2f\n", parking_lot[i].parking_fee);
            parking_lot[i].is_occupied = 0;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Vehicle not found in the parking lot.\n");
    }
}

// Delete vehicle record
void delete_vehicle() {
    char vehicle_number[20];
    int found = 0;

    printf("Enter vehicle number to delete: ");
    fgets(vehicle_number, 20, stdin);
    vehicle_number[strcspn(vehicle_number, "\n")] = '\0';

    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        if (strcmp(parking_lot[i].vehicle_number, vehicle_number) == 0) {
            printf("Deleting record for vehicle %s from space %d.\n", vehicle_number, parking_lot[i].space_id);
            memset(&parking_lot[i], 0, sizeof(ParkingSpace)); // Reset structure
            parking_lot[i].space_id = i + 1;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Vehicle not found in the parking lot.\n");
    }
}

// Display parking records
void display_records() {
    printf("\n%-10s %-15s %-25s %-25s %-10s\n", "Space ID", "Vehicle No.", "Entry Time", "Exit Time", "Fee");
    printf("---------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        if (parking_lot[i].is_occupied || parking_lot[i].exit_time != 0) {
            printf("%-10d %-15s %-25s %-25s ₹%-10.2f\n", 
                parking_lot[i].space_id,
                parking_lot[i].vehicle_number,
                ctime(&parking_lot[i].entry_time),
                parking_lot[i].exit_time ? ctime(&parking_lot[i].exit_time) : "Still Parked",
                parking_lot[i].parking_fee
            );
        }
    }
}

// Calculate parking fee
void calculate_fee(ParkingSpace *space) {
    double duration = calculate_duration(space->entry_time, space->exit_time);
    space->parking_fee = duration * HOURLY_RATE;
}

// Calculate duration in hours
double calculate_duration(time_t entry, time_t exit) {
    return difftime(exit, entry) / 3600.0; // Duration in hours
}