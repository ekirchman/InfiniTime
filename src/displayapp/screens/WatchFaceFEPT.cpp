#include "displayapp/screens/WatchFaceFEPT.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

LV_IMG_DECLARE(bg_clock);

WatchFaceFEPT::WatchFaceFEPT(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController,
                                   Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    statusIcons(batteryController, bleController) {

  lfs_file f = {};
  // filesys = *filesystem;
  //Add background
 /*
  background_img = lv_img_create(lv_scr_act(), nullptr);
  if (filesystem.FileOpen(&f, "/images/FEPT/BGNight.bin", LFS_O_RDONLY) >= 0) {
      lv_img_set_src(background_img, "F:/images/FEPT/BGNight.bin");
      filesystem.FileClose(&f);
  }

  if(background_img != nullptr){
    lv_img_set_zoom(background_img, 256);
    // lv_img_set_auto_size(background_img, true);
  }
  */
  //Add enemy
  enemy_img = lv_img_create(lv_scr_act(), nullptr);
  if (filesystem.FileOpen(&f, "/images/FEPT/Enemy.bin", LFS_O_RDONLY) >= 0) {
      lv_img_set_src(enemy_img, "F:/images/FEPT/Enemy.bin");
      filesystem.FileClose(&f);
  }

  if(enemy_img != nullptr){
    // lv_img_set_auto_size(enemy_img, true);
    lv_img_set_zoom(enemy_img, 512);
  }
  lv_obj_align(enemy_img, nullptr, LV_ALIGN_IN_LEFT_MID, 10, 50);

  //Add hero
  hero_img = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(hero_img, &hero);

  lv_obj_align(hero_img, nullptr, LV_ALIGN_IN_RIGHT_MID, 0, 00);

  statusIcons.Create();

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));



  //Set time label and time font
  label_time = lv_label_create(lv_scr_act(), nullptr);

  if (filesystem.FileOpen(&f, "/fonts/Outline.bin", LFS_O_RDONLY) >= 0) {
      filesystem.FileClose(&f);
      font_outline = lv_font_load("F:/fonts/Outline.bin");
  }

  if(font_outline != nullptr){
    lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_outline);
  }else{
    lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  }

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  //This will cause Refresh to be called every 20 ms
  // taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceFEPT::~WatchFaceFEPT() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceFEPT::Refresh() {
  statusIcons.Update();
  /*
  enemy_img = lv_img_create(lv_scr_act(), nullptr);
  if (filesystem1.FileOpen(&f, "/images/FEPT/Enemy.bin", LFS_O_RDONLY) >= 0) {
      lv_img_set_src(enemy_img, "F:/images/FEPT/Enemy.bin");
      filesystem.FileClose(&f);
  }*/

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -30);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, -30);
    }

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date,
                              "%s %d %s %d",
                              dateTimeController.DayOfWeekShortToString(),
                              day,
                              dateTimeController.MonthShortToString(),
                              year);
      } else {
        lv_label_set_text_fmt(label_date,
                              "%s %s %d %d",
                              dateTimeController.DayOfWeekShortToString(),
                              dateTimeController.MonthShortToString(),
                              day,
                              year);
      }
      lv_obj_realign(label_date);
    }

    updateAnimation();
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }
}

void WatchFaceFEPT::updateAnimation() {
  uint8_t second = dateTimeController.Seconds();
  if(second != mSecond){
    mSecond = second;
    heroFrameCnt++;
    if(heroFrameCnt >= LYN_MAX_FRAME_COUNT){
      //Reset hero animation back to img 0, with the next frame of animation being 1
      heroFrameCnt = 1;
    }else{
      //Update to next frame
    }
  }
}

void WatchFaceFEPT::loadFrames(Pinetime::Controllers::FS& filesystem) {
  lfs_file f = {};
  for(uint8_t frameCnt =  1; frameCnt < LYN_MAX_FRAME_COUNT; frameCnt++){
    if(11 == frameCnt){
      //skip img to save on flash storage
      //img 11 is the same as 9
      continue;
    }
    if(14 == frameCnt){
      //skip img to save on flash storage
      //img 14 is the same as 12
      continue;
    }
    if(17 == frameCnt){
      //skip img to save on flash storage
      //img 17 is the same as 15
      continue;
    }
    if(19 == frameCnt){
      //skip img to save on flash storage
      //img 19 is the same as 16
      continue;
    }
    if(23 == frameCnt){
      //skip img to save on flash storage
      //img 23 is the same as 21
      continue;
    }
    if(29 == frameCnt){
      //skip img to save on flash storage
      //img 29 is the same as 27
      continue;
    }
    if(35 == frameCnt){
      //skip img to save on flash storage
      //img 35 is the same as 31, but flipped vertically
      continue;
    }
    if(36 == frameCnt){
      //skip img to save on flash storage
      //img 36 is the same as 32, but flipped vertically
      continue;
    }
    if(28 == frameCnt || 30 == frameCnt || 34 == frameCnt){
      //These are invisible frames
      //skip img to save on flash storage
      continue;
    }
    if(40 == frameCnt){
      //skip img to save on flash storage
      //img 40 is a all white image
      continue;
    }
    if(44 == frameCnt){
      //skip img to save on flash storage
      continue;
    }
    if(49 == frameCnt){
      //skip img to save on flash storage
      //img 49 is the same as 48, but moved slightly to the right
      continue;
    }
    if(65 == frameCnt){
      //skip img to save on flash storage
      //img 65 is the same as 00
      continue;
    }
    std::string fd1 = "/images/FEPT/LYN" + std::to_string(frameCnt) + ".bin";
    std::string fd2 = "F:/images/FEPT/LYN" + std::to_string(frameCnt) + ".bin";

    // lv_obj_align(enemy_img, nullptr, LV_ALIGN_IN_LEFT_MID, 0, 60);
  }
  filesystem.FileClose(&f);
}
