#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/utsname.h>

// GPIO pin numbers
#define LED1_PIN "45"
#define LED2_PIN "44"
#define BUTTON1_PIN "68"
#define BUTTON2_PIN "67"
#define BUTTON3_PIN "60"
#define BUTTON4_PIN "69"
#define BUZZER_PIN "66"

// PWM paths
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
int train_from_left = 0; // Flag to check if a train is coming from the left
int train_from_right = 0; // Flag to check if a train is coming from the right
int collision_scenario = 0; // Flag to check if a collision scenario has been detected

pthread_mutex_t lock; // Mutex to protect shared resources

int main() {
    pthread_t threads[4]; // Array of threads, 4 because we have 4
    char led1_pin[10], led2_pin[10], button1_pin[10], button2_pin[10], button3_pin[10], button4_pin[10], buzzer_pin[10], pwm_chip_num[10], pwm_channel_num[10]; // User input variables

    // Prompt user for GPIO pin numbers
    printf("Enter GPIO pin number for LED1: ");
    scanf("%s", led1_pin);
    printf("Enter GPIO pin number for LED2: ");
    scanf("%s", led2_pin);
    printf("Enter GPIO pin number for Button1: ");
    scanf("%s", button1_pin);
    printf("Enter GPIO pin number for Button2: ");
    scanf("%s", button2_pin);
    printf("Enter GPIO pin number for Button3: ");
    scanf("%s", button3_pin);
    printf("Enter GPIO pin number for Button4: ");
    scanf("%s", button4_pin);
    printf("Enter GPIO pin number for Buzzer: ");
    scanf("%s", buzzer_pin);
    printf("Enter PWM chip number: ");
    scanf("%s", pwm_chip_num);
    printf("Enter PWM channel number: ");
    scanf("%s", pwm_channel_num);

    // Initialize GPIO pins
    setup_gpio(LED1_PIN, "out");
    setup_gpio(LED2_PIN, "out");
    setup_gpio(BUTTON1_PIN, "in");
    setup_gpio(BUTTON2_PIN, "in");
    setup_gpio(BUTTON3_PIN, "in");
    setup_gpio(BUTTON4_PIN, "in");
    setup_gpio(BUZZER_PIN, "out");

    // Initialize PWM for servo
    setup_pwm();

    // Initialize mutex
    pthread_mutex_init(&lock, NULL);

    // Create threads
    pthread_create(&threads[0], NULL, train_sensor_thread, NULL); // Create the train sensor thread
    pthread_create(&threads[1], NULL, led_control_thread, NULL); // Create the LED thread
    pthread_create(&threads[2], NULL, servo_control_thread, NULL); // Create the servo thread
    pthread_create(&threads[3], NULL, buzzer_control_thread, NULL); // Create the buzzer thread

    // Join threads
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy mutex
    pthread_mutex_destroy(&lock);

    return 0;
}

// Function to setup a GPIO pin
void setup_gpio(const char *pin, const char *direction) {
    char path[50]; // Path to the GPIO pin
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s", pin); // Check if the pin is already exported

    // Export the pin if not already exported
    if (access(path, F_OK) == -1) {
        int export_fd = open("/sys/class/gpio/export", O_WRONLY); // Open the export file
        write(export_fd, pin, strlen(pin));
        close(export_fd);
    }

    // Set the pin direction
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", pin);
    int direction_fd = open(path, O_WRONLY); // Open the direction file
    write(direction_fd, direction, strlen(direction));
    close(direction_fd); // Close the file
}

// Function to write a value to a GPIO pin
void write_gpio(const char *pin, const char *value) {
    char path[50]; // Path to the GPIO pin
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin); // Path to the GPIO pin
    int value_fd = open(path, O_WRONLY); // Open the value file
    write(value_fd, value, strlen(value)); // Write the value to the file
    close(value_fd);
}

// Function to read a value from a GPIO pin
int read_gpio(const char *pin) {
    char path[50]; // Path to the GPIO pin
    char value[3]; // Buffer to store the value
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin); // Path to the GPIO pin
    int value_fd = open(path, O_RDONLY); // Open the value file
    read(value_fd, value, 3); // Read the value from the file
    close(value_fd); // Close the file
    return atoi(value); // Convert the value to an integer and return
}

// Function to setup the PWM for the servo
void setup_pwm() {
    // Export the PWM channel
    write_to_file(PWM_EXPORT_PATH, "0");

    // Set the period (20ms)
    write_to_file(PWM_PERIOD_PATH, "20000000");

    // Enable the PWM signal
    write_to_file(PWM_ENABLE_PATH, "1");
}

// Function to control the servo motor
void control_servo(int position) {
    // Calculate the duty cycle for the given position
    int duty_cycle = 1500000 + (position * 5000); // 0 degrees at 1ms, 180 degrees at 2ms

    // Set the duty cycle
    char duty_cycle_str[20]; // Buffer to store the duty cycle
    sprintf(duty_cycle_str, "%d", duty_cycle); // Convert the duty cycle to a string
    write_to_file(PWM_DUTY_CYCLE_PATH, duty_cycle_str); // Write the duty cycle to the file
}

// Function to control the train sensor
void *train_sensor_thread(void *arg) {
    int button1_pressed = 0; // Flag to check if button 1 is pressed
    int button2_pressed = 0; // Flag to check if button 2 is pressed
    int button3_pressed = 0; // Flag to check if button 3 is pressed
    int button4_pressed = 0; // Flag to check if button 4 is pressed

    while (1) {
        pthread_mutex_lock(&lock);
        int button1 = !read_gpio(BUTTON1_PIN); // Read the state of button 1
        int button2 = !read_gpio(BUTTON2_PIN); // Read the state of button 2
        int button3 = !read_gpio(BUTTON3_PIN); // Read the state of button 3
        int button4 = !read_gpio(BUTTON4_PIN); // Read the state of button 4

        // Check for button presses after a collision has been detected
        if (collision_scenario && (button1 || button2 || button3 || button4)) {
            collision_scenario = 0; // Reset the collision scenario
            write_gpio(LED1_PIN, "0");
            write_gpio(LED2_PIN, "0");
            write_gpio(BUZZER_PIN, "0");
            control_servo(90); // Raise the crossing guard
        }

        // Train coming from left (1 then 2)
        if (button1 && !button1_pressed && !button2_pressed) {
            button1_pressed = 1;
            printf("Button 1 pressed\n");
        } else if (button2 && button1_pressed && !button2_pressed) {
            button2_pressed = 1;
            printf("Button 2 pressed\n");
            train_from_left = 1;
            printf("Train coming from left\n");
            write_gpio(LED1_PIN, "1"); // Turn on LED1
            control_servo(-90); // Lower the crossing guard
        }

        // Train coming from right (4 then 3)
        if (button4 && !button4_pressed && !button3_pressed) {
            button4_pressed = 1;
            printf("Button 4 pressed\n");
        } else if (button3 && button4_pressed && !button3_pressed) {
            button3_pressed = 1;
            printf("Button 3 pressed\n");
            train_from_right = 1;
            printf("Train coming from right\n");
            write_gpio(LED2_PIN, "1"); // Turn on LED2
            control_servo(-90); // Lower the crossing guard
        }

        // Train passing from left (3 then 4)
        if (button3 && train_from_left && !button3_pressed && !button4_pressed) {
            button3_pressed = 1;
            printf("Button 3 pressed\n");
        } else if (button4 && button3_pressed && !button4_pressed) {
            button4_pressed = 1;
            printf("Button 4 pressed\n");
            printf("Train from left passed\n");
            // Wait for 1 second before turning off the LED
            pthread_mutex_unlock(&lock);
            sleep(1);
            pthread_mutex_lock(&lock);
        
            write_gpio(LED1_PIN, "0"); // Turn off LED1
            
            // Wait for an additional 1 second before raising the crossing guard
            pthread_mutex_unlock(&lock);
            sleep(1);
            pthread_mutex_lock(&lock);
        
            control_servo(90); // Raise the crossing guard
            button1_pressed = button2_pressed = button3_pressed = button4_pressed = train_from_left = train_from_right = 0;
        }

        // Train passing from right (2 then 1)
        if (button2 && train_from_right && !button2_pressed && !button1_pressed) {
            button2_pressed = 1;
            printf("Button 2 pressed\n");
        } else if (button1 && button2_pressed && !button1_pressed) {
            button1_pressed = 1;
            printf("Button 1 pressed\n");
            printf("Train from right passed\n");
            // Wait for 1 second before turning off the LED
            pthread_mutex_unlock(&lock);
            sleep(1);
            pthread_mutex_lock(&lock);
        
            write_gpio(LED2_PIN, "0"); // Turn off LED2
        
            // Wait for an additional 1 second before raising the crossing guard
            pthread_mutex_unlock(&lock);
            sleep(1);
            pthread_mutex_lock(&lock);
        
            control_servo(90); // Raise the crossing guard
            button1_pressed = button2_pressed = button3_pressed = button4_pressed = train_from_left = train_from_right = 0;
        }

        // Collision scenario
        if (train_from_left && train_from_right) {
            collision_scenario = 1;
            printf("Collision detected!\n");
            write_gpio(LED1_PIN, "1"); // Turn on LED1
            write_gpio(LED2_PIN, "1"); // Turn on LED2
            write_gpio(BUZZER_PIN, "1"); // Turn on buzzer
            control_servo(0); // Lower the crossing guard
            pthread_mutex_unlock(&lock);
            sleep(2);
            pthread_mutex_lock(&lock);
            write_gpio(LED1_PIN, "0"); // Turn on LED1
            write_gpio(LED2_PIN, "0"); // Turn on LED2
            write_gpio(BUZZER_PIN, "0"); // Turn on buzzer
            control_servo(90);
            collision_scenario = 0;
        }

        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}

// Function to control the LED
void *led_control_thread(void *arg) {
    int led_state = 0; // Flag to keep track of the LED state
    while (1) {
        pthread_mutex_lock(&lock);
        if (collision_scenario) { // Rapidly blink the LEDs during a collision scenario
            while (collision_scenario) {
                write_gpio(LED1_PIN, led_state ? "1" : "0");
                write_gpio(LED2_PIN, led_state ? "1" : "0");
                led_state = !led_state; // Toggle the LED state
                usleep(250000); // Blink every 250 ms for rapid blinking
            }
        } else if (train_from_left || train_from_right) {
            // Keep blinking until the train has passed
            write_gpio(LED1_PIN, led_state ? "1" : "0");
            write_gpio(LED2_PIN, led_state ? "0" : "1");
            pthread_mutex_unlock(&lock);
            usleep(500000); // Blink every 500 ms
        } else {
            write_gpio(LED1_PIN, "0");
            write_gpio(LED2_PIN, "0");
            pthread_mutex_unlock(&lock);
            usleep(100000); // Check the flag every 100 ms
        }
        led_state = !led_state;
    }
    return NULL;
}

// Function to control the servo
void *servo_control_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (train_from_left || train_from_right || collision_scenario) {
            control_servo(-90); // Lower the crossing guard
        } else {
            control_servo(90); // Raise the crossing guard
        }
        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}

// Function to control the buzzer
void *buzzer_control_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (collision_scenario) { // Keep the buzzer on during a collision scenario
            while (collision_scenario) {
                write_gpio(BUZZER_PIN, "1"); // Keep the buzzer on
                usleep(100000); // Check every 100 ms
            }
        } else {
            write_gpio(BUZZER_PIN, "0"); // Turn off the buzzer
        }
        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}

// Function to write a value to a specified file
void write_to_file(const char *file_path, const char *value) {
    int fd = open(file_path, O_WRONLY); // Open the file
    if (fd < 0) { // Check if the file was opened successfully
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    if (write(fd, value, strlen(value)) < 0) { // Write the value to the file
        perror("Failed to write to file");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd); // Close the file
}