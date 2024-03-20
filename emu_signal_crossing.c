#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/utsname.h>

#define LED1_PIN "45"
#define LED2_PIN "44"
#define BUTTON1_PIN "68"
#define BUTTON2_PIN "67"
#define BUTTON3_PIN "60"
#define BUTTON4_PIN "69"
#define BUZZER_PIN "66"
#define SERVO_PIN "PWM"  // Assuming a placeholder for the servo control

void setup_gpio(const char *pin, const char *direction);
int read_gpio(const char *pin);
void write_gpio(const char *pin, const char *value);
void control_servo(int angle);
void *train_sensor_thread(void *arg);
void *led_control_thread(void *arg);
void *buzzer_control_thread(void *arg);

int train_from_left = 0;
int train_from_right = 0;
int collision_scenario = 0;
pthread_mutex_t lock;

int main() {
    pthread_t threads[3];
    struct utsname unameData;
    uname(&unameData);
    printf("Running on %s\n", unameData.machine);

    setup_gpio(LED1_PIN, "out");
    setup_gpio(LED2_PIN, "out");
    setup_gpio(BUTTON1_PIN, "in");
    setup_gpio(BUTTON2_PIN, "in");
    setup_gpio(BUTTON3_PIN, "in");
    setup_gpio(BUTTON4_PIN, "in");
    setup_gpio(BUZZER_PIN, "out");

    pthread_mutex_init(&lock, NULL);

    pthread_create(&threads[0], NULL, train_sensor_thread, NULL);
    pthread_create(&threads[1], NULL, led_control_thread, NULL);
    pthread_create(&threads[2], NULL, buzzer_control_thread, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);

    return 0;
}

void setup_gpio(const char *pin, const char *direction) {
    // GPIO setup code for the target system
}

int read_gpio(const char *pin) {
    // GPIO read code for the target system
    return rand() % 2;  // Randomly return 0 or 1 for testing
}

void write_gpio(const char *pin, const char *value) {
    // GPIO write code for the target system
    printf("GPIO %s set to %s\n", pin, value);
}

void control_servo(int angle) {
    // Servo control code for the target system
    printf("Servo set to %d degrees\n", angle);
}

void *train_sensor_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        if (read_gpio(BUTTON1_PIN)) {
            printf("Button 1 pressed\n");
            if (!train_from_left) {
                train_from_left = 1;
                printf("Train coming from left\n");
            }
        }
        if (read_gpio(BUTTON2_PIN)) {
            printf("Button 2 pressed\n");
            if (train_from_left) {
                train_from_left = 0;
                printf("Train from left passed\n");
            }
        }
        if (read_gpio(BUTTON3_PIN)) {
            printf("Button 3 pressed\n");
            if (!train_from_right) {
                train_from_right = 1;
                printf("Train coming from right\n");
            }
        }
        if (read_gpio(BUTTON4_PIN)) {
            printf("Button 4 pressed\n");
            if (train_from_right) {
                train_from_right = 0;
                printf("Train from right passed\n");
            }
        }

        if (train_from_left && train_from_right) {
            collision_scenario = 1;
            printf("Collision detected!\n");
        }

        pthread_mutex_unlock(&lock);
        usleep(100000); // Check every 100 ms
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
        } else if (train_from_left || train_from_right) {
            led_state = !led_state;
            write_gpio(LED1_PIN, led_state ? "1" : "0");
            write_gpio(LED2_PIN, led_state ? "0" : "1");
        } else {
            write_gpio(LED1_PIN, "0");
            write_gpio(LED2_PIN, "0");
        }
        pthread_mutex_unlock(&lock);
        usleep(500000); // Check every 500 ms
    }
    return NULL;
}

void *buzzer_control_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (collision_scenario) {
            write_gpio(BUZZER_PIN, "1"); // Sound the buzzer
        } else {
            write_gpio(BUZZER_PIN, "0"); // Turn off the buzzer
        }
        pthread_mutex_unlock(&lock);
        usleep(100000); // 100 ms
    }
    return NULL;
}  
