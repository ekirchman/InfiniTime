#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Doom : public Screen {
      public:
        Doom(DisplayApp* app);
        ~Doom() override;
      };
    }
  }
}
