#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/utsname.h>

char LED1_PIN[5];
char LED2_PIN[5];
char BUTTON1_PIN[5];
char BUTTON2_PIN[5];
char BUZZER_PIN[5];
#define PWM_EXPORT_PATH "/sys/class/pwm/pwmchip4/export"
#define PWM_PERIOD_PATH "/sys/class/pwm/pwmchip4/pwm-4:0/period"
#define PWM_DUTY_CYCLE_PATH "/sys/class/pwm/pwmchip4/pwm-4:0/duty_cycle"
#define PWM_ENABLE_PATH "/sys/class/pwm/pwmchip4/pwm-4:0/enable"

// Function prototypes
void setup_gpio(const char *pin, const char *direction);
void write_gpio(const char *pin, const char *value);
void write_to_file(const char *file_path, const char *value);
int read_gpio(const char *pin);
void setup_pwm();
void control_servo(int position);
void *train_sensor_thread(void *arg);
void *led_control_thread(void *arg);
void *servo_control_thread(void *arg);
void *buzzer_control_thread(void *arg);

// Global variables
int train_approaching = 0;
int train_leaving = 0;
int collision_scenario = 0;
pthread_mutex_t lock;

int main() {
    pthread_t threads[4];

    // Prompt user for GPIO pin numbers
    printf("Enter GPIO pin number for LED1: ");
    scanf("%s", LED1_PIN);
    printf("Enter GPIO pin number for LED2: ");
    scanf("%s", LED2_PIN);
    printf("Enter GPIO pin number for Button1: ");
    scanf("%s", BUTTON1_PIN);
    printf("Enter GPIO pin number for Button2: ");
    scanf("%s", BUTTON2_PIN);
    printf("Enter GPIO pin number for Buzzer: ");
    scanf("%s", BUZZER_PIN);

    // Initialize GPIO pins
    setup_gpio(LED1_PIN, "out");
    setup_gpio(LED2_PIN, "out");
    setup_gpio(BUTTON1_PIN, "in");
    setup_gpio(BUTTON2_PIN, "in");
    setup_gpio(BUZZER_PIN, "out");

    // Initialize PWM for servo
    setup_pwm();

    // Initialize mutex
    pthread_mutex_init(&lock, NULL);

    // Create threads
    pthread_create(&threads[0], NULL, train_sensor_thread, NULL);
    pthread_create(&threads[1], NULL, led_control_thread, NULL);
    pthread_create(&threads[2], NULL, servo_control_thread, NULL);
    pthread_create(&threads[3], NULL, buzzer_control_thread, NULL);

    // Join threads
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy mutex
    pthread_mutex_destroy(&lock);

    return 0;
}

void setup_gpio(const char *pin, const char *direction) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s", pin);

    // Export the pin if not already exported
    if (access(path, F_OK) == -1) {
        int export_fd = open("/sys/class/gpio/export", O_WRONLY);
        write(export_fd, pin, strlen(pin));
        close(export_fd);
    }

    // Set the pin direction
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", pin);
    int direction_fd = open(path, O_WRONLY);
    write(direction_fd, direction, strlen(direction));
    close(direction_fd);
}

void write_gpio(const char *pin, const char *value) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
    int value_fd = open(path, O_WRONLY);
    write(value_fd, value, strlen(value));
    close(value_fd);
}

int read_gpio(const char *pin) {
    char path[50];
    char value[3];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
    int value_fd = open(path, O_RDONLY);
    read(value_fd, value, 3);
    close(value_fd);
    return atoi(value);
}

void setup_pwm() {
    // Export the PWM channel
    write_to_file(PWM_EXPORT_PATH, "0");

    // Set the period (20ms)
    write_to_file(PWM_PERIOD_PATH, "20000000");

    // Enable the PWM signal
    write_to_file(PWM_ENABLE_PATH, "1");
}

void control_servo(int position) {
    // Calculate the duty cycle for the given position
    int duty_cycle = 1000000 + (position * 5555); // 0 degrees at 1ms, 180 degrees at 2ms

    // Set the duty cycle
    char duty_cycle_str[20];
    sprintf(duty_cycle_str, "%d", duty_cycle);
    write_to_file(PWM_DUTY_CYCLE_PATH, duty_cycle_str);
}

void *train_sensor_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        int new_train_approaching = !read_gpio(BUTTON1_PIN);  // Invert the logic
        int new_train_leaving = !read_gpio(BUTTON2_PIN);      // Invert the logic
        if (new_train_approaching != train_approaching || new_train_leaving != train_leaving) {
            train_approaching = new_train_approaching;
            train_leaving = new_train_leaving;
            collision_scenario = train_approaching && train_leaving;  // Collision when both buttons are pressed
        }
        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}

void *led_control_thread(void *arg) {
    int led_state = 0;
    while (1) {
        pthread_mutex_lock(&lock);
        if (collision_scenario) {
            write_gpio(LED1_PIN, "1");
            write_gpio(LED2_PIN, "1");
        } else if (train_approaching || train_leaving) {
            led_state = !led_state;
            write_gpio(LED1_PIN, led_state ? "1" : "0");
            write_gpio(LED2_PIN, led_state ? "0" : "1");
        } else {
            if (led_state) {
                // Ensure LEDs are turned off and stay off for 1 second
                write_gpio(LED1_PIN, "0");
                write_gpio(LED2_PIN, "0");
                led_state = 0;
                pthread_mutex_unlock(&lock);
                usleep(1000000); // 1 second
                continue;
            }
        }
        pthread_mutex_unlock(&lock);
        usleep(500000); // 500 ms for blinking
    }
    return NULL;
}

void *servo_control_thread(void *arg) {
    int guard_raised = 0;
    while (1) {
        pthread_mutex_lock(&lock);
        if (collision_scenario || train_approaching || train_leaving) {
            control_servo(0); // Lower the crossing guard
            guard_raised = 0;
        } else {
            if (!guard_raised) {
                pthread_mutex_unlock(&lock);
                usleep(1000000); // Wait 1 second before raising
                pthread_mutex_lock(&lock);
                control_servo(90); // Raise the crossing guard
                guard_raised = 1;
            }
        }
        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}

void *buzzer_control_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (collision_scenario) {
            write_gpio(BUZZER_PIN, "1");
        } else {
            write_gpio(BUZZER_PIN, "0");
        }
        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}

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