#include <OneButton.h>

#include "controller.h"

#define BUTTON_PIN 12

namespace Controller {
  namespace {
    OneButton button(BUTTON_PIN, true);
  };

  void setup(void (*buttonClicked)(void), void (*buttonLongPressed)(void)) {
    button.attachClick(buttonClicked);
    button.attachLongPressStart(buttonLongPressed);
  };

  void tick() {
    button.tick();
  };
};
