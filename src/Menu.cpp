#include "Menu.h"

RTC_DATA_ATTR uint8_t cursor=0;
RTC_DATA_ATTR uint8_t firstVisibleItem=0; // New variable to track the first visible item

void Menu::moveCursor(uint8_t direction) {
    if (direction == 0) { // reset
        cursor = 0;
        firstVisibleItem = 0;
    } else if (direction == MENU_DIR_DOWN) { // down
        cursor = (cursor + 1) % MENU_LENGTH;
        if (cursor >= firstVisibleItem + MENU_ITEMS_PER_PAGE) {
            firstVisibleItem = (firstVisibleItem + 1) % MENU_LENGTH;
        }else if (cursor == 0){
            firstVisibleItem = 0;
        }
    } else if (direction == MENU_DIR_UP) { // up
        if (cursor == 0) {
            cursor = MENU_LENGTH - 1;
            firstVisibleItem = (MENU_LENGTH - MENU_ITEMS_PER_PAGE) % MENU_LENGTH;
        } else {
            cursor = (cursor - 1) % MENU_LENGTH;
            if (cursor < firstVisibleItem) {
                firstVisibleItem = (firstVisibleItem - 1 + MENU_LENGTH) % MENU_LENGTH;
            }
        }
    }

}

MenuItem* Menu::getSelectedItem() {
    return menuItems[cursor];
}

void Menu::renderMenu(bool partialRefresh) {
    watchy->display.setFullWindow();
    watchy->display.fillScreen(GxEPD_BLACK);
    watchy->display.setFont(&FreeMonoBold9pt7b);

    int16_t x1, y1;
    uint16_t w, h;
    int16_t yPos;

    for (int i = 0; i < MENU_ITEMS_PER_PAGE; i++) {
        int itemIndex = (firstVisibleItem + i) % MENU_LENGTH;
        const char* itemName = menuItems[itemIndex]->getName();
        yPos = MENU_HEIGHT + (MENU_HEIGHT * i);
        watchy->display.setCursor(0, yPos);
        if (itemIndex == cursor) {
            watchy->display.getTextBounds(itemName, 0, yPos, &x1, &y1, &w, &h);
            watchy->display.fillRect(x1 - 1, y1 - 10, 200, h + 15, GxEPD_WHITE);
            watchy->display.setTextColor(GxEPD_BLACK);
            watchy->display.println(itemName);
        } else {
            watchy->display.setTextColor(GxEPD_WHITE);
            watchy->display.println(itemName);
        }
    }

    if (firstVisibleItem > 0) {
        watchy->display.setCursor(0, MENU_HEIGHT - 10);
        watchy->display.setTextColor(cursor == firstVisibleItem ? GxEPD_BLACK : GxEPD_WHITE);
        watchy->display.println("^");
    }
    if (firstVisibleItem + MENU_ITEMS_PER_PAGE < MENU_LENGTH) {
        watchy->display.setCursor(0, MENU_HEIGHT + (MENU_HEIGHT * MENU_ITEMS_PER_PAGE) - 12);
        watchy->display.setTextColor(GxEPD_WHITE);
        watchy->display.println("v");
    }

    watchy->display.display(partialRefresh);
}