#ifndef GAME_RACE_H
#define GAME_RACE_H

#include "lvgl.h"
#include <Arduino.h>

// --- Configuration des broches pour la course ---
#define PIN_BTN_J1 D2  // J1 - NC (Actif HIGH) - Jaune
#define PIN_BTN_J2 D3  // J2 - NO (Actif LOW)  - Rouge
#define PIN_BTN_J3 D5  // J3 - NC (Actif HIGH) - Bleu
#define PIN_BTN_J4 D4  // J4 - NO (Actif LOW)  - Blanc

// Prototype pour appeler le Game Over dans main.cpp
extern void show_game_over_race(int winner);

// --- Variables internes de la course ---
static lv_obj_t * bar_j1;
static lv_obj_t * bar_j2;
static lv_obj_t * bar_j3;
static lv_obj_t * bar_j4;

static int score_j1 = 0;
static int score_j2 = 0;
static int score_j3 = 0;
static int score_j4 = 0;

// Mémoires pour éviter la triche (détection de clic unique)
static bool last_state_j1 = false;
static bool last_state_j2 = false;
static bool last_state_j3 = false;
static bool last_state_j4 = false;

const int MAX_CLICKS = 50;

static void reset_game_race() {
    score_j1 = 0;
    score_j2 = 0;
    score_j3 = 0;
    score_j4 = 0;

    lv_bar_set_value(bar_j1, 0, LV_ANIM_OFF);
    lv_bar_set_value(bar_j2, 0, LV_ANIM_OFF);
    lv_bar_set_value(bar_j3, 0, LV_ANIM_OFF);
    lv_bar_set_value(bar_j4, 0, LV_ANIM_OFF);
}

static void init_game_race(lv_obj_t * scr) {
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x222222), 0);

    // Paramètres d'espacement pour 4 joueurs
    int bar_w = 60;
    int bar_h = 180;
    int spacing = (480 - (4 * bar_w)) / 5;

    // --- J1 : JAUNE ---
    bar_j1 = lv_bar_create(scr);
    lv_obj_set_size(bar_j1, bar_w, bar_h);
    lv_obj_align(bar_j1, LV_ALIGN_BOTTOM_LEFT, spacing, -40);
    lv_bar_set_range(bar_j1, 0, MAX_CLICKS);
    lv_obj_set_style_bg_color(bar_j1, lv_color_hex(0xFFFF00), LV_PART_INDICATOR);
    
    lv_obj_t * lbl_1 = lv_label_create(scr);
    lv_label_set_text(lbl_1, "J1");
    lv_obj_set_style_text_color(lbl_1, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(lbl_1, bar_j1, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // --- J2 : ROUGE ---
    bar_j2 = lv_bar_create(scr);
    lv_obj_set_size(bar_j2, bar_w, bar_h);
    lv_obj_align(bar_j2, LV_ALIGN_BOTTOM_LEFT, spacing * 2 + bar_w, -40);
    lv_bar_set_range(bar_j2, 0, MAX_CLICKS);
    lv_obj_set_style_bg_color(bar_j2, lv_color_hex(0xFF0000), LV_PART_INDICATOR);

    lv_obj_t * lbl_2 = lv_label_create(scr);
    lv_label_set_text(lbl_2, "J2");
    lv_obj_set_style_text_color(lbl_2, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(lbl_2, bar_j2, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // --- J3 : BLEU ---
    bar_j3 = lv_bar_create(scr);
    lv_obj_set_size(bar_j3, bar_w, bar_h);
    lv_obj_align(bar_j3, LV_ALIGN_BOTTOM_LEFT, spacing * 3 + bar_w * 2, -40);
    lv_bar_set_range(bar_j3, 0, MAX_CLICKS);
    lv_obj_set_style_bg_color(bar_j3, lv_color_hex(0x0088FF), LV_PART_INDICATOR);

    lv_obj_t * lbl_3 = lv_label_create(scr);
    lv_label_set_text(lbl_3, "J3");
    lv_obj_set_style_text_color(lbl_3, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(lbl_3, bar_j3, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // --- J4 : BLANC ---
    bar_j4 = lv_bar_create(scr);
    lv_obj_set_size(bar_j4, bar_w, bar_h);
    lv_obj_align(bar_j4, LV_ALIGN_BOTTOM_LEFT, spacing * 4 + bar_w * 3, -40);
    lv_bar_set_range(bar_j4, 0, MAX_CLICKS);
    lv_obj_set_style_bg_color(bar_j4, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);

    lv_obj_t * lbl_4 = lv_label_create(scr);
    lv_label_set_text(lbl_4, "J4");
    lv_obj_set_style_text_color(lbl_4, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(lbl_4, bar_j4, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // Titre
    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "MARTELAGE INTENSIF !");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
}

static void game_race_loop() {
    // 1. Lecture des états physiques (avec adaptation NO/NC de tes boutons)
    bool cur_j1 = (digitalRead(PIN_BTN_J1) == HIGH);
    bool cur_j2 = (digitalRead(PIN_BTN_J2) == LOW);
    bool cur_j3 = (digitalRead(PIN_BTN_J3) == HIGH);
    bool cur_j4 = (digitalRead(PIN_BTN_J4) == LOW);

    // 2. Incrémentation uniquement lors d'un "clic" (False -> True)
    if (cur_j1 && !last_state_j1) {
        score_j1++;
        lv_bar_set_value(bar_j1, score_j1, LV_ANIM_OFF);
    }
    if (cur_j2 && !last_state_j2) {
        score_j2++;
        lv_bar_set_value(bar_j2, score_j2, LV_ANIM_OFF);
    }
    if (cur_j3 && !last_state_j3) {
        score_j3++;
        lv_bar_set_value(bar_j3, score_j3, LV_ANIM_OFF);
    }
    if (cur_j4 && !last_state_j4) {
        score_j4++;
        lv_bar_set_value(bar_j4, score_j4, LV_ANIM_OFF);
    }

    // 3. Sauvegarde de l'état pour la prochaine frame
    last_state_j1 = cur_j1;
    last_state_j2 = cur_j2;
    last_state_j3 = cur_j3;
    last_state_j4 = cur_j4;

    // 4. Vérification du gagnant
    if (score_j1 >= MAX_CLICKS) show_game_over_race(4);
    else if (score_j2 >= MAX_CLICKS) show_game_over_race(2);
    else if (score_j3 >= MAX_CLICKS) show_game_over_race(3);
    else if (score_j4 >= MAX_CLICKS) show_game_over_race(1);
}

#endif