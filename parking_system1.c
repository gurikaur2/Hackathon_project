#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PARKING_SPACES 50
#define HOURLY_RATE 100.0
#define DATA_FILE "parking_data.txt"

// Structure for parking details
typedef struct {
    int space_id;
    char vehicle_number[20];
    time_t entry_time;
    time_t exit_time;
    double parking_fee;
    int is_occupied;
} ParkingSpace;

ParkingSpace parking_lot[MAX_PARKING_SPACES];

// Function Prototypes
void initialize_parking_lot();
void load_from_file();
void save_to_file();
void add_vehicle(GtkWidget *button, gpointer data);
void update_exit(GtkWidget *button, gpointer data);
void display_records(GtkWidget *button, gpointer data);
void calculate_fee(ParkingSpace *space);
double calculate_duration(time_t entry, time_t exit);

// Initialize parking lot and load existing data
void initialize_parking_lot() {
    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        parking_lot[i].space_id = i + 1;
        parking_lot[i].is_occupied = 0;
        parking_lot[i].exit_time = 0;
    }
    load_from_file();  // Load data if available
}

// Load parking data from file
void load_from_file() {
    FILE *file = fopen(DATA_FILE, "r");
    if (file) {
        fread(parking_lot, sizeof(ParkingSpace), MAX_PARKING_SPACES, file);
        fclose(file);
    }
}

// Save parking data to file
void save_to_file() {
    FILE *file = fopen(DATA_FILE, "w");
    if (file) {
        fwrite(parking_lot, sizeof(ParkingSpace), MAX_PARKING_SPACES, file);
        fclose(file);
    }
}

// Function to handle "Add Vehicle"
void add_vehicle(GtkWidget *button, gpointer data) {
    if (!data) {
        g_print("Error: NULL data passed to callback.\n");
        return;
    }

    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *entry = widgets[0];
    GtkWidget *output_label = widgets[1];

    if (!GTK_IS_EDITABLE(entry) || !GTK_IS_LABEL(output_label)) {
        g_print("Error: Invalid widgets passed.\n");
        return;
    }

    const char *vehicle_number = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (strlen(vehicle_number) == 0) {
        gtk_label_set_text(GTK_LABEL(output_label), "Vehicle number cannot be empty.");
        return;
    }

    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        if (!parking_lot[i].is_occupied) {
            strncpy(parking_lot[i].vehicle_number, vehicle_number, sizeof(parking_lot[i].vehicle_number));
            parking_lot[i].entry_time = time(NULL);
            parking_lot[i].is_occupied = 1;

            char buffer[256];
            char entry_time_str[26];
            strftime(entry_time_str, sizeof(entry_time_str), "%Y-%m-%d %H:%M:%S", localtime(&parking_lot[i].entry_time));

            snprintf(buffer, sizeof(buffer), "Vehicle %s added to space %d at %s", 
                     vehicle_number, parking_lot[i].space_id, entry_time_str);
            gtk_label_set_text(GTK_LABEL(output_label), buffer);

            gtk_editable_set_text(GTK_EDITABLE(entry), ""); // Clear input field
            save_to_file();  // Save changes
            return;
        }
    }
    gtk_label_set_text(GTK_LABEL(output_label), "No parking spaces available.");
}

// Function to handle "Update Vehicle Exit"
void update_exit(GtkWidget *button, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *entry = widgets[0];
    GtkWidget *output_label = widgets[1];

    const char *vehicle_number = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (strlen(vehicle_number) == 0) {
        gtk_label_set_text(GTK_LABEL(output_label), "Vehicle number cannot be empty.");
        return;
    }

    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        if (parking_lot[i].is_occupied && strcmp(parking_lot[i].vehicle_number, vehicle_number) == 0) {
            parking_lot[i].exit_time = time(NULL);
            calculate_fee(&parking_lot[i]);
            parking_lot[i].is_occupied = 0;

            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Vehicle %s exited at %sParking Fee: ₹%.2f", 
                     vehicle_number, ctime(&parking_lot[i].exit_time), parking_lot[i].parking_fee);
            gtk_label_set_text(GTK_LABEL(output_label), buffer);

            gtk_editable_set_text(GTK_EDITABLE(entry), ""); // Clear input field
            save_to_file();  // Save changes
            return;
        }
    }
    gtk_label_set_text(GTK_LABEL(output_label), "Vehicle not found in the parking lot.");
}

// Display parking records
void display_records(GtkWidget *button, gpointer data) {
    GtkWidget *output_text_view = (GtkWidget *)data;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_text_view));
    gtk_text_buffer_set_text(buffer, "Space ID | Vehicle No. | Entry Time         | Exit Time          | Fee\n", -1);

    char record[256];
    for (int i = 0; i < MAX_PARKING_SPACES; i++) {
        if (parking_lot[i].is_occupied || parking_lot[i].exit_time != 0) {
            snprintf(record, sizeof(record), "%-8d | %-12s | %-18s | %-18s | ₹%.2f\n",
                     parking_lot[i].space_id, parking_lot[i].vehicle_number,
                     ctime(&parking_lot[i].entry_time),
                     parking_lot[i].exit_time ? ctime(&parking_lot[i].exit_time) : "Still Parked",
                     parking_lot[i].parking_fee);
            gtk_text_buffer_insert_at_cursor(buffer, record, -1);
        }
    }
}

// Calculate parking fee
void calculate_fee(ParkingSpace *space) {
    double duration = calculate_duration(space->entry_time, space->exit_time);
    space->parking_fee = duration * HOURLY_RATE;
}

// Calculate duration
double calculate_duration(time_t entry, time_t exit) {
    return difftime(exit, entry) / 3600.0; // Duration in hours
}

// GTK Application
static void activate(GtkApplication *app, gpointer user_data) {
    initialize_parking_lot();

    // Main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Parking Management System");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Widgets
    GtkWidget *add_entry = gtk_entry_new();
    GtkWidget *exit_entry = gtk_entry_new();
    GtkWidget *output_label = gtk_label_new("");
    GtkWidget *output_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_text_view), FALSE);

    GtkWidget *add_button = gtk_button_new_with_label("Add Vehicle");
    GtkWidget *exit_button = gtk_button_new_with_label("Update Exit");
    GtkWidget *display_button = gtk_button_new_with_label("Display Records");

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Vehicle Number:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), add_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), add_button, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Vehicle Exit:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), exit_entry, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), exit_button, 3, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), display_button, 0, 2, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), output_label, 0, 3, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), output_text_view, 0, 4, 4, 1);

    // Pass widgets safely
    GtkWidget **widgets_add = g_malloc(2 * sizeof(GtkWidget *));
    widgets_add[0] = add_entry;
    widgets_add[1] = output_label;

    GtkWidget **widgets_exit = g_malloc(2 * sizeof(GtkWidget *));
    widgets_exit[0] = exit_entry;
    widgets_exit[1] = output_label;

    // Connect signals
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_vehicle), widgets_add);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(update_exit), widgets_exit);
    g_signal_connect(display_button, "clicked", G_CALLBACK(display_records), output_text_view);

    // Free memory on shutdown
    g_signal_connect(app, "shutdown", G_CALLBACK(g_free), widgets_add);
    g_signal_connect(app, "shutdown", G_CALLBACK(g_free), widgets_exit);

    gtk_window_present(GTK_WINDOW(window));
}


int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.parking", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
