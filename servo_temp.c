#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PWM_EXPORT_PATH "/sys/class/pwm/pwmchip4/export"
#define PWM_UNEXPORT_PATH "/sys/class/pwm/pwmchip4/unexport"
#define PWM_PERIOD_PATH "/sys/class/pwm/pwmchip4/pwm-4:0/period"
#define PWM_DUTY_CYCLE_PATH "/sys/class/pwm/pwmchip4/pwm-4:0/duty_cycle"
#define PWM_ENABLE_PATH "/sys/class/pwm/pwmchip4/pwm-4:0/enable"

// Function to write a value to a specified file
void write_to_file(const char *file_path, const char *value) {
    int fd = open(file_path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    if (write(fd, value, strlen(value)) < 0) {
        perror("Failed to write to file");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// Function to control the servo motor
void control_servo(int position) {
    // Calculate the duty cycle for the given position
    // Middle position (0 degrees) = 1.5ms, Right (90 degrees) = 2ms, Left (-90 degrees) = 1ms
    int duty_cycle = 1500000 + (position * 5000); // Position in degrees (-90 to 90)

    // Set the period (20ms)
    write_to_file(PWM_PERIOD_PATH, "20000000");

    // Set the duty cycle
    char duty_cycle_str[20];
    sprintf(duty_cycle_str, "%d", duty_cycle);
    write_to_file(PWM_DUTY_CYCLE_PATH, duty_cycle_str);

    // Enable the PWM signal
    write_to_file(PWM_ENABLE_PATH, "1");

    // Keep the servo at the position for 1 second
    sleep(1);

    // Disable the PWM signal
    write_to_file(PWM_ENABLE_PATH, "0");
}

int main() {
    // Export the PWM channel
    write_to_file(PWM_EXPORT_PATH, "0");

    // Control the servo motor
    control_servo(-90); // Move to -90 degrees (left)
    control_servo(0);   // Move to 0 degrees (middle)
    control_servo(90);  // Move to 90 degrees (right)

    // Unexport the PWM channel
    write_to_file(PWM_UNEXPORT_PATH, "0");

    return 0;
}