#include "Menu.h"

void Menu::moveCursor(uint8_t direction) {
    if (direction == 0) { // reset
        cursor = 0;
    } else if (direction == MENU_DIR_DOWN) { // down
        cursor = (cursor + 1) % MENU_LENGTH;
    } else if (direction == MENU_DIR_UP) { // up
        cursor = (cursor + (MENU_LENGTH-1)) % MENU_LENGTH;
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

  for (int i = 0; i < MENU_LENGTH; i++) {
    const char* itemName = menuItems[i]->getName();
    yPos = MENU_HEIGHT + (MENU_HEIGHT * i);
    watchy->display.setCursor(0, yPos);
    if (i == cursor) {
      watchy->display.getTextBounds(itemName, 0, yPos, &x1, &y1, &w, &h);
      watchy->display.fillRect(x1 - 1, y1 - 10, 200, h + 15, GxEPD_WHITE);
      watchy->display.setTextColor(GxEPD_BLACK);
      watchy->display.println(itemName);
    } else {
      watchy->display.setTextColor(GxEPD_WHITE);
      watchy->display.println(itemName);
    }
  }

  watchy->display.display(partialRefresh);
}