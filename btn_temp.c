#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUTTON1_PIN "67"  // Replace with the actual GPIO pin number for Button1
#define BUTTON2_PIN "68"  // Replace with the actual GPIO pin number for Button2

void setup_button(const char *pin) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s", pin);

    // Check if the pin is already exported
    if (access(path, F_OK) == -1) {
        // Export the pin
        int export_fd = open("/sys/class/gpio/export", O_WRONLY);
        if (export_fd < 0) {
            fprintf(stderr, "Failed to open export for button GPIO pin\n");
            exit(EXIT_FAILURE);
        }
        if (write(export_fd, pin, strlen(pin)) != strlen(pin)) {
            fprintf(stderr, "Failed to export button GPIO pin\n");
            close(export_fd);
            exit(EXIT_FAILURE);
        }
        close(export_fd);
        
        // Enable internal pull-up resistor
        char value_path[50];
        snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%s/active_low", pin);
        int value_fd = open(value_path, O_WRONLY);
        if (value_fd < 0) {
            fprintf(stderr, "Failed to open active_low for button GPIO pin\n");
            exit(EXIT_FAILURE);
        }
        if (write(value_fd, "1", 1) != 1) {
            fprintf(stderr, "Failed to enable pull-up resistor for button GPIO pin\n");
            close(value_fd);
            exit(EXIT_FAILURE);
        }
        close(value_fd);
    }

    // Set the pin direction to input
    char direction_path[50];
    snprintf(direction_path, sizeof(direction_path), "/sys/class/gpio/gpio%s/direction", pin);
    int direction_fd = open(direction_path, O_WRONLY);
    if (direction_fd < 0) {
        fprintf(stderr, "Failed to open direction for button GPIO pin\n");
        exit(EXIT_FAILURE);
    }
    if (write(direction_fd, "in", 2) != 2) {
        fprintf(stderr, "Failed to set button GPIO pin direction\n");
        close(direction_fd);
        exit(EXIT_FAILURE);
    }
    close(direction_fd);
}

int read_button_state(const char *pin) {
    char value_path[50];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%s/value", pin);
    int value_fd = open(value_path, O_RDONLY);
    if (value_fd < 0) {
        fprintf(stderr, "Failed to open value for button GPIO pin\n");
        exit(EXIT_FAILURE);
    }

    char value;
    if (read(value_fd, &value, 1) != 1) {
        fprintf(stderr, "Failed to read button GPIO pin value\n");
        close(value_fd);
        exit(EXIT_FAILURE);
    }
    close(value_fd);

    // Simple debouncing
    usleep(50000); // 50 ms
    return value == '0' ? 1 : 0;
}

int main() {
    setup_button(BUTTON1_PIN);
    setup_button(BUTTON2_PIN);

    while (1) {
        int button1_state = read_button_state(BUTTON1_PIN);
        int button2_state = read_button_state(BUTTON2_PIN);

        printf("Button1: %d, Button2: %d\n", button1_state, button2_state);
        sleep(1);
    }

    return 0;
}
