/*
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

// This file contains a simple Hello World app which you can base you own
// native Badge apps on.

#include "main.h"
#include "rp2040.h"

static pax_buf_t buf;
xQueueHandle buttonQueue;

#include <esp_log.h>
static const char *TAG = "addon-name-tag";

// Updates the screen with the latest buffer.
void disp_flush() {
    ili9341_write(get_ili9341(), buf.buf);
}

// Exits the app, returning to the launcher.
void exit_to_launcher() {
    REG_WRITE(RTC_CNTL_STORE0_REG, 0);
    esp_restart();
}

void app_main() {
  
    
    ESP_LOGI(TAG, "This is a nametag with working SOA");

    // Initialize the screen, the I2C and the SPI busses.
    bsp_init();

    // Initialize the RP2040 (responsible for buttons, etc).
    bsp_rp2040_init();
    
    // This queue is used to receive button presses.
    buttonQueue = get_rp2040()->queue;
    
    // Initialize graphics for the screen.
    pax_buf_init(&buf, NULL, 320, 240, PAX_BUF_16_565RGB);

     // Initialize NVS.
    nvs_flash_init();

    bool led = false;
    bool didLedLastCycle = false;

    RP2040* rp2040 = get_rp2040();
    rp2040_set_gpio_dir(rp2040, 0, RP2040_REG_GPIO_OUT);
    rp2040_set_gpio_dir(rp2040, 1, RP2040_REG_GPIO_OUT);

    int ledMode = 0;


    while (1) {
        // Pick a random background color.
        int hue = esp_random() & 255;
        pax_col_t col = pax_col_hsv(hue, 255 /*saturation*/, 255 /*brighness*/);
        
        // Greet the World in front of a random background color! 
        // Fill the background with the random color.
        pax_background(&buf, col);
        
        // This text is shown on screen.
        char             *text = "Hello, MCH2022!";
        
        // Pick the font (Saira is the only one that looks nice in this size).
        const pax_font_t *font = pax_font_saira_condensed;

        // Determine how the text dimensions so we can display it centered on
        // screen.
        pax_vec1_t        dims = pax_text_size(font, font->default_size, text);

        // Draw the centered text.
        pax_draw_text(
            &buf, // Buffer to draw to.
            0xff000000, // color
            font, font->default_size, // Font and size to use.
            // Position (top left corner) of the app.
            (buf.width  - dims.x) / 2.0,
            (buf.height - dims.y) / 2.0,
            // The text to be rendered.
            text
        );

        // Draws the entire graphics buffer to the screen.
        disp_flush();
        
        // Wait for button presses and do another cycle.
        
        // Structure used to receive data.
        rp2040_input_message_t message;
        
        // Wait forever for a button press (because of portMAX_DELAY)
        xQueueReceive(buttonQueue, &message, portMAX_DELAY);
        
        // Which button is currently pressed?
        if (message.input == RP2040_INPUT_BUTTON_HOME && message.state) {
            // If home is pressed, exit to launcher.
            exit_to_launcher();
        }

        if (message.input == RP2040_INPUT_BUTTON_SELECT && message.state) {
            if (ledMode < 2) {
                ledMode += 1;
            }
            else {
                ledMode = 0;
            }
        }

        switch (ledMode) {
            case 0:
                if (didLedLastCycle == false) {
                    led = !led;
                    rp2040_set_gpio_value(rp2040, 0, led);
                    rp2040_set_gpio_value(rp2040, 1, !led);
                    didLedLastCycle = true;
                } else {
                    didLedLastCycle = false;
                }
                break;
            case 1:
                    rp2040_set_gpio_value(rp2040, 0, true);
                    rp2040_set_gpio_value(rp2040, 1, true);
                break;
            case 2:
                    rp2040_set_gpio_value(rp2040, 0, false);
                    rp2040_set_gpio_value(rp2040, 1, false);
                break;
        }
    }
}
