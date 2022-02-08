
#include "menu.h"
#include "color.h"
#include "input.h"
#include <vpad/input.h>

Menu::Menu(Screen *screen) : screen(screen) {
    currentOption = LaunchDisk;
    message       = nullptr;
}

Menu::Option Menu::show() {
    while (true) {
        redraw();
        uint32_t buttons = WaitInput(VPAD_BUTTON_A | VPAD_BUTTON_DOWN | VPAD_BUTTON_UP);
        if (buttons & VPAD_BUTTON_A) return (Option) currentOption;
        else if (buttons & VPAD_BUTTON_DOWN) {
            if (currentOption < 2) currentOption++;
        } else if (buttons & VPAD_BUTTON_UP) {
            if (currentOption > 0) currentOption--;
        }
    }
}

void Menu::setMessage(const char *message) {
    this->message = message;
    redraw();
}

void Menu::redraw() {
    screen->clear(COLOR_BLUE);
    Screen::drawText(5, 5, "Wii U Debugger");
    Screen::drawText(5, 7, "Choose an option:");
    Screen::drawText(8, 9, "Install and launch disc");
    Screen::drawText(8, 10, "Install and return to system menu");
    Screen::drawText(8, 11, "Exit without installing");
    Screen::drawText(5, 9 + currentOption, ">");
    if (message) {
        Screen::drawText(5, 13, message);
    }
    Screen::flip();
}
