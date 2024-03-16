#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define LED1_PIN "67"  // GPIO 67 for LED1
#define LED2_PIN "68"  // GPIO 68 for LED2

void setup_led(const char *pin) {
    // Check if the pin is already exported
    char export_path[50];
    snprintf(export_path, sizeof(export_path), "/sys/class/gpio/gpio%s", pin);
    if (access(export_path, F_OK) == -1) {
        // Export the pin
        int export_fd = open("/sys/class/gpio/export", O_WRONLY);
        if (export_fd < 0) {
            fprintf(stderr, "Failed to open export for LED GPIO pin\n");
            exit(EXIT_FAILURE);
        }
        if (write(export_fd, pin, strlen(pin)) != strlen(pin)) {
            fprintf(stderr, "Failed to export LED GPIO pin\n");
            close(export_fd);
            exit(EXIT_FAILURE);
        }
        close(export_fd);
    }

    // Set the pin direction to output
    char direction_path[50];
    snprintf(direction_path, sizeof(direction_path), "/sys/class/gpio/gpio%s/direction", pin);
    int direction_fd = open(direction_path, O_WRONLY);
    if (direction_fd < 0) {
        fprintf(stderr, "Failed to open direction for LED GPIO pin\n");
        exit(EXIT_FAILURE);
    }
    if (write(direction_fd, "out", 3) != 3) {
        fprintf(stderr, "Failed to set LED GPIO pin direction\n");
        close(direction_fd);
        exit(EXIT_FAILURE);
    }
    close(direction_fd);
}

void set_led_state(const char *pin, int state) {
    char value_path[50];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%s/value", pin);
    int value_fd = open(value_path, O_WRONLY);
    if (value_fd < 0) {
        fprintf(stderr, "Failed to open value for LED GPIO pin\n");
        exit(EXIT_FAILURE);
    }
    if (write(value_fd, state ? "1" : "0", 1) != 1) {
        fprintf(stderr, "Failed to set LED GPIO pin value\n");
        close(value_fd);
        exit(EXIT_FAILURE);
    }
    close(value_fd);
}

int main() {
    setup_led(LED1_PIN);
    setup_led(LED2_PIN);

    while (1) {
        printf("LED1 ON, LED2 OFF\n");
        set_led_state(LED1_PIN, 1);
        set_led_state(LED2_PIN, 0);
        sleep(1);

        printf("LED1 OFF, LED2 ON\n");
        set_led_state(LED1_PIN, 0);
        set_led_state(LED2_PIN, 1);
        sleep(1);
    }

    return 0;
}