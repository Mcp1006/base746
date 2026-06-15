#include "lvgl.h"
#include <Arduino.h>
#include "game_1v1.h"
#include "game_race.h"

#ifdef ARDUINO
#include "lvglDrivers.h"

// --- États globaux ---
enum GameState {
    STATE_MAIN_MENU,
    STATE_SKIN_SELECT,
    STATE_PLAYING,
    STATE_PAUSED,       
    STATE_GAMEOVER
};
GameState current_state = STATE_MAIN_MENU;

enum ActiveGame {
    GAME_NONE,
    GAME_1V1,
    GAME_RACE
};
ActiveGame current_game = GAME_NONE;

#ifndef PIN_BUZZER
#define PIN_BUZZER D6
#endif

// Variables pour le choix des skins (0=Escrime, 1=Chevalier, 2=Viking, 3=Ninja)
int p1_skin = 0; 
int p2_skin = 0;

// Écrans LVGL
lv_obj_t * scr_main_menu;
lv_obj_t * scr_skin_select;
lv_obj_t * scr_game = NULL; 

// Éléments d'interface dynamiques
lv_obj_t * game_over_container;
lv_obj_t * win_label;
lv_obj_t * pause_container; 

// Conteneurs et objets de preview visuel
lv_obj_t * p1_preview_container;
lv_obj_t * p2_preview_container;
lv_obj_t * p1_preview_char = NULL;
lv_obj_t * p1_preview_wpn = NULL;
lv_obj_t * p2_preview_char = NULL;
lv_obj_t * p2_preview_wpn = NULL;

// --- Prototypes ---
void build_main_menu();
void build_skin_select();
void build_game_over_overlay();
void build_pause_overlay();

// --- Fonctions de rafraîchissement des Previews (1v1) ---
void update_p1_preview() {
    if (p1_preview_char != NULL) lv_obj_delete(p1_preview_char);
    if (p1_preview_wpn != NULL) lv_obj_delete(p1_preview_wpn);
    p1_preview_char = create_character(p1_preview_container, p1_skin, true, true);
    lv_obj_center(p1_preview_char);
    p1_preview_wpn = create_weapon(p1_preview_container, p1_skin, true, false);
    lv_obj_align_to(p1_preview_wpn, p1_preview_char, LV_ALIGN_OUT_RIGHT_MID, -10, -5);
}

void update_p2_preview() {
    if (p2_preview_char != NULL) lv_obj_delete(p2_preview_char);
    if (p2_preview_wpn != NULL) lv_obj_delete(p2_preview_wpn);
    p2_preview_char = create_character(p2_preview_container, p2_skin, false, true);
    lv_obj_center(p2_preview_char);
    p2_preview_wpn = create_weapon(p2_preview_container, p2_skin, false, false);
    lv_obj_align_to(p2_preview_wpn, p2_preview_char, LV_ALIGN_OUT_LEFT_MID, 10, -5);
}

// --- Callbacks (Actions des boutons) ---
static void btn_combat_1v1_cb(lv_event_t * e) {
    current_game = GAME_1V1;
    current_state = STATE_SKIN_SELECT;
    update_p1_preview();
    update_p2_preview();
    lv_scr_load(scr_skin_select);
}

static void btn_course_cb(lv_event_t * e) {
    current_game = GAME_RACE;
    if (scr_game != NULL) lv_obj_delete(scr_game);
    scr_game = lv_obj_create(NULL);
    lv_obj_remove_flag(scr_game, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr_game, LV_OBJ_FLAG_CLICKABLE); 
    
    // Ajout de la pause au clic
    lv_obj_add_event_cb(scr_game, [](lv_event_t* e){
        if (current_state == STATE_PLAYING) {
            current_state = STATE_PAUSED;
            lv_obj_clear_flag(pause_container, LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_CLICKED, NULL);

    init_game_race(scr_game); 
    build_game_over_overlay(); 
    build_pause_overlay(); 

    reset_game_race();
    current_state = STATE_PLAYING;
    lv_scr_load(scr_game);
}

static void skin_btn_cb(lv_event_t * e) {
    int dir = (int)(intptr_t)lv_event_get_user_data(e);
    if (dir == 1) { p1_skin = (p1_skin + 1) % 4; update_p1_preview(); }
    else if (dir == -1) { p1_skin = (p1_skin == 0) ? 3 : p1_skin - 1; update_p1_preview(); }
    else if (dir == 2) { p2_skin = (p2_skin + 1) % 4; update_p2_preview(); }
    else if (dir == -2) { p2_skin = (p2_skin == 0) ? 3 : p2_skin - 1; update_p2_preview(); }
}

static void start_1v1_game_cb(lv_event_t * e) {
    if (scr_game != NULL) lv_obj_delete(scr_game);
    scr_game = lv_obj_create(NULL);
    lv_obj_remove_flag(scr_game, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr_game, LV_OBJ_FLAG_CLICKABLE); 
    
    lv_obj_add_event_cb(scr_game, [](lv_event_t* e){
        if (current_state == STATE_PLAYING) {
            current_state = STATE_PAUSED;
            lv_obj_clear_flag(pause_container, LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_CLICKED, NULL);

    init_game_1v1(scr_game); 
    build_game_over_overlay(); 
    build_pause_overlay(); 

    reset_game_1v1();
    current_state = STATE_PLAYING;
    lv_scr_load(scr_game);
}

static void resume_cb(lv_event_t * e) {
    lv_obj_add_flag(pause_container, LV_OBJ_FLAG_HIDDEN);
    current_state = STATE_PLAYING;
}

static void restart_cb(lv_event_t * e) {
    lv_obj_add_flag(pause_container, LV_OBJ_FLAG_HIDDEN);
    if (current_game == GAME_1V1) reset_game_1v1();
    else if (current_game == GAME_RACE) reset_game_race();
    current_state = STATE_PLAYING;
}

static void quit_cb(lv_event_t * e) {
    lv_obj_add_flag(pause_container, LV_OBJ_FLAG_HIDDEN);
    current_state = STATE_MAIN_MENU;
    lv_scr_load(scr_main_menu);
}

static void btn_replay_cb(lv_event_t * e) {
    lv_obj_add_flag(game_over_container, LV_OBJ_FLAG_HIDDEN);
    if (current_game == GAME_1V1) reset_game_1v1();
    else if (current_game == GAME_RACE) reset_game_race();
    current_state = STATE_PLAYING;
}

// Fonction de Game Over pour le 1v1
void show_game_over(int winner) {
    current_state = STATE_GAMEOVER;
    lv_obj_clear_flag(game_over_container, LV_OBJ_FLAG_HIDDEN);
    
    tone(PIN_BUZZER, 1000, 500); // Son de victoire éclatant !

    if (winner == 1) {
        lv_label_set_text(win_label, "BLUE WIN");
        lv_obj_set_style_text_color(win_label, lv_color_hex(0x4444FF), 0);
    } else {
        lv_label_set_text(win_label, "RED WIN");
        lv_obj_set_style_text_color(win_label, lv_color_hex(0xFF4444), 0);
    }
}

// Fonction de Game Over pour la Course
void show_game_over_race(int winner) {
    current_state = STATE_GAMEOVER;
    lv_obj_clear_flag(game_over_container, LV_OBJ_FLAG_HIDDEN);
    
    tone(PIN_BUZZER, 1000, 500); // Son de victoire éclatant !

    if (winner == 1) {
        lv_label_set_text(win_label, "JAUNE WIN");
        lv_obj_set_style_text_color(win_label, lv_color_hex(0xFFFF00), 0);
    } else if (winner == 2) {
        lv_label_set_text(win_label, "ROUGE WIN");
        lv_obj_set_style_text_color(win_label, lv_color_hex(0xFF0000), 0);
    } else if (winner == 3) {
        lv_label_set_text(win_label, "BLEU WIN");
        lv_obj_set_style_text_color(win_label, lv_color_hex(0x0088FF), 0);
    } else if (winner == 4) {
        lv_label_set_text(win_label, "BLANC WIN");
        lv_obj_set_style_text_color(win_label, lv_color_hex(0xFFFFFF), 0);
    }
}

// Routeur Physique du jeu
static void main_timer_cb(lv_timer_t * timer) {
    if (current_state == STATE_PLAYING) {
        if (current_game == GAME_1V1) {
            game_1v1_loop(); 
        } else if (current_game == GAME_RACE) {
            game_race_loop();
        }
    }
}

// --- Construction des Écrans (UI) ---
void build_main_menu() {
    scr_main_menu = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_main_menu, lv_color_hex(0x222222), 0);

    lv_obj_t * title = lv_label_create(scr_main_menu);
    lv_label_set_text(title, "MENU PRINCIPAL");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Bouton Jeu 1 (Combat 1v1)
    lv_obj_t * btn_combat = lv_button_create(scr_main_menu);
    lv_obj_set_size(btn_combat, 200, 50);
    lv_obj_align(btn_combat, LV_ALIGN_CENTER, 0, -10);
    lv_obj_add_event_cb(btn_combat, btn_combat_1v1_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_combat = lv_label_create(btn_combat);
    lv_label_set_text(lbl_combat, LV_SYMBOL_PLAY " Combat 1v1");
    lv_obj_center(lbl_combat);

    // Bouton Jeu 2 (Course)
    lv_obj_t * btn_course = lv_button_create(scr_main_menu);
    lv_obj_set_size(btn_course, 200, 50);
    lv_obj_align(btn_course, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(btn_course, btn_course_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_course = lv_label_create(btn_course);
    lv_label_set_text(lbl_course, LV_SYMBOL_PLAY " Course 4 Joueurs");
    lv_obj_center(lbl_course);
}

void build_skin_select() {
    scr_skin_select = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_skin_select, lv_color_hex(0x333333), 0);

    lv_obj_t * bg_btn = lv_button_create(scr_skin_select);
    lv_obj_set_size(bg_btn, 480, 272);
    lv_obj_set_style_bg_opa(bg_btn, 0, 0); 
    lv_obj_remove_flag(bg_btn, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_add_event_cb(bg_btn, start_1v1_game_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * title = lv_label_create(scr_skin_select);
    lv_label_set_text(title, "Choix des Skins\n(Appuyez sur le fond pour demarrer)");
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    // Panel J1
    lv_obj_t * p1_panel = lv_obj_create(scr_skin_select);
    lv_obj_set_size(p1_panel, 160, 200); 
    lv_obj_align(p1_panel, LV_ALIGN_LEFT_MID, 20, 20);

    lv_obj_t * p1_title = lv_label_create(p1_panel);
    lv_label_set_text(p1_title, "J1"); 
    lv_obj_align(p1_title, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t * p1_up = lv_button_create(p1_panel);
    lv_obj_set_size(p1_up, 40, 25); lv_obj_align(p1_up, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_add_event_cb(p1_up, skin_btn_cb, LV_EVENT_CLICKED, (void*)1);
    lv_obj_t * l1u = lv_label_create(p1_up); lv_label_set_text(l1u, LV_SYMBOL_UP); lv_obj_center(l1u);

    p1_preview_container = lv_obj_create(p1_panel);
    lv_obj_set_size(p1_preview_container, 80, 80);
    lv_obj_set_style_bg_opa(p1_preview_container, 0, 0);
    lv_obj_set_style_border_width(p1_preview_container, 0, 0);
    lv_obj_align(p1_preview_container, LV_ALIGN_CENTER, 0, 0); 

    lv_obj_t * p1_down = lv_button_create(p1_panel);
    lv_obj_set_size(p1_down, 40, 25); lv_obj_align(p1_down, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(p1_down, skin_btn_cb, LV_EVENT_CLICKED, (void*)-1);
    lv_obj_t * l1d = lv_label_create(p1_down); lv_label_set_text(l1d, LV_SYMBOL_DOWN); lv_obj_center(l1d);

    // Panel J2
    lv_obj_t * p2_panel = lv_obj_create(scr_skin_select);
    lv_obj_set_size(p2_panel, 160, 200);
    lv_obj_align(p2_panel, LV_ALIGN_RIGHT_MID, -20, 20);

    lv_obj_t * p2_title = lv_label_create(p2_panel);
    lv_label_set_text(p2_title, "J2");
    lv_obj_align(p2_title, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t * p2_up = lv_button_create(p2_panel);
    lv_obj_set_size(p2_up, 40, 25); lv_obj_align(p2_up, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_add_event_cb(p2_up, skin_btn_cb, LV_EVENT_CLICKED, (void*)2);
    lv_obj_t * l2u = lv_label_create(p2_up); lv_label_set_text(l2u, LV_SYMBOL_UP); lv_obj_center(l2u);

    p2_preview_container = lv_obj_create(p2_panel);
    lv_obj_set_size(p2_preview_container, 80, 80);
    lv_obj_set_style_bg_opa(p2_preview_container, 0, 0);
    lv_obj_set_style_border_width(p2_preview_container, 0, 0);
    lv_obj_align(p2_preview_container, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * p2_down = lv_button_create(p2_panel);
    lv_obj_set_size(p2_down, 40, 25); lv_obj_align(p2_down, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(p2_down, skin_btn_cb, LV_EVENT_CLICKED, (void*)-2);
    lv_obj_t * l2d = lv_label_create(p2_down); lv_label_set_text(l2d, LV_SYMBOL_DOWN); lv_obj_center(l2d);
}

void build_game_over_overlay() {
    game_over_container = lv_obj_create(scr_game); 
    lv_obj_set_size(game_over_container, 300, 180);
    lv_obj_center(game_over_container);
    lv_obj_set_style_bg_color(game_over_container, lv_color_hex(0x111111), 0);
    lv_obj_set_style_bg_opa(game_over_container, 230, 0);
    lv_obj_add_flag(game_over_container, LV_OBJ_FLAG_HIDDEN); 

    win_label = lv_label_create(game_over_container);
    lv_obj_set_style_text_font(win_label, &lv_font_montserrat_24, 0); 
    lv_obj_align(win_label, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t * btn_replay = lv_button_create(game_over_container);
    lv_obj_set_size(btn_replay, 110, 50);
    lv_obj_align(btn_replay, LV_ALIGN_BOTTOM_LEFT, 10, -20);
    lv_obj_add_event_cb(btn_replay, btn_replay_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_rep = lv_label_create(btn_replay);
    lv_label_set_text(lbl_rep, "Rejouer");
    lv_obj_center(lbl_rep);

    lv_obj_t * btn_menu = lv_button_create(game_over_container);
    lv_obj_set_size(btn_menu, 110, 50);
    lv_obj_align(btn_menu, LV_ALIGN_BOTTOM_RIGHT, -10, -20);
    lv_obj_add_event_cb(btn_menu, quit_cb, LV_EVENT_CLICKED, NULL); 
    lv_obj_t * lbl_menu = lv_label_create(btn_menu);
    lv_label_set_text(lbl_menu, "Menu");
    lv_obj_center(lbl_menu);
}

void build_pause_overlay() {
    pause_container = lv_obj_create(scr_game); 
    lv_obj_set_size(pause_container, 300, 240); 
    lv_obj_center(pause_container);
    lv_obj_set_style_bg_color(pause_container, lv_color_hex(0x111111), 0);
    lv_obj_set_style_bg_opa(pause_container, 230, 0);
    lv_obj_add_flag(pause_container, LV_OBJ_FLAG_HIDDEN); 

    lv_obj_t * title = lv_label_create(pause_container);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0); 
    lv_label_set_text(title, "PAUSE");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * btn_res = lv_button_create(pause_container);
    lv_obj_set_size(btn_res, 180, 45);
    lv_obj_align(btn_res, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_add_event_cb(btn_res, resume_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_res = lv_label_create(btn_res);
    lv_label_set_text(lbl_res, "Continuer");
    lv_obj_center(lbl_res);

    lv_obj_t * btn_rest = lv_button_create(pause_container);
    lv_obj_set_size(btn_rest, 180, 45);
    lv_obj_align(btn_rest, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_add_event_cb(btn_rest, restart_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_rest = lv_label_create(btn_rest);
    lv_label_set_text(lbl_rest, "Recommencer");
    lv_obj_center(lbl_rest);

    lv_obj_t * btn_quit = lv_button_create(pause_container);
    lv_obj_set_size(btn_quit, 180, 45);
    lv_obj_align(btn_quit, LV_ALIGN_TOP_MID, 0, 170);
    lv_obj_add_event_cb(btn_quit, quit_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_quit = lv_label_create(btn_quit);
    lv_label_set_text(lbl_quit, "Quitter");
    lv_obj_center(lbl_quit);
}

void mySetup()
{
    pinMode(PIN_P1_BACK, INPUT_PULLUP);
    pinMode(PIN_P1_ATTACK, INPUT_PULLUP);
    pinMode(PIN_P2_BACK, INPUT_PULLUP);
    pinMode(PIN_P2_ATTACK, INPUT_PULLUP);
    
    pinMode(PIN_BUZZER, OUTPUT); // Configuration de la broche du Buzzer

    build_main_menu();
    build_skin_select();

    lv_scr_load(scr_main_menu);
    lv_timer_create(main_timer_cb, 33, NULL);
}

void loop() {}

void myTask(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

#else
#include "app_hal.h"
#include <cstdio>

int main(void)
{
  printf("LVGL Simulator\n");
  fflush(stdout);
  lv_init();
  hal_setup();
  hal_loop();
  return 0;
}
#endif