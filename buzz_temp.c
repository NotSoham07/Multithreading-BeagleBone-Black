#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUZZER_PIN "66"  // Use the GPIO number, not the pin number

void setup_buzzer() {
    char export_path[50];
    snprintf(export_path, sizeof(export_path), "/sys/class/gpio/gpio%s", BUZZER_PIN);
    
    // Check if the pin is already exported
    if (access(export_path, F_OK) == -1) {
        // Export the pin
        int export_fd = open("/sys/class/gpio/export", O_WRONLY);
        if (export_fd < 0) {
            fprintf(stderr, "Failed to open export for buzzer GPIO pin\n");
            exit(EXIT_FAILURE);
        }
        if (write(export_fd, BUZZER_PIN, strlen(BUZZER_PIN)) != strlen(BUZZER_PIN)) {
            fprintf(stderr, "Failed to export buzzer GPIO pin\n");
            close(export_fd);
            exit(EXIT_FAILURE);
        }
        close(export_fd);
    }

    // Set the pin direction to output
    char direction_path[50];
    snprintf(direction_path, sizeof(direction_path), "/sys/class/gpio/gpio%s/direction", BUZZER_PIN);
    int direction_fd = open(direction_path, O_WRONLY);
    if (direction_fd < 0) {
        fprintf(stderr, "Failed to open direction for buzzer GPIO pin\n");
        exit(EXIT_FAILURE);
    }
    if (write(direction_fd, "out", 3) != 3) {
        fprintf(stderr, "Failed to set buzzer GPIO pin direction\n");
        close(direction_fd);
        exit(EXIT_FAILURE);
    }
    close(direction_fd);
}

void set_buzzer_state(int state) {
    char value_path[50];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%s/value", BUZZER_PIN);
    int value_fd = open(value_path, O_WRONLY);
    if (value_fd < 0) {
        perror("Failed to open value file");
        exit(EXIT_FAILURE);
    }
    if (write(value_fd, state ? "1" : "0", 1) < 0) {
        perror("Failed to set buzzer GPIO pin value");
        close(value_fd);
        exit(EXIT_FAILURE);
    }
    close(value_fd);
}

int main() {
    setup_buzzer();

    printf("Turning on the buzzer\n");
    set_buzzer_state(1);
    sleep(2);

    printf("Turning off the buzzer\n");
    set_buzzer_state(0);
    sleep(2);

    return 0;
}
