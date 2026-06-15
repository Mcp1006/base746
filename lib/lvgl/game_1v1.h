#ifndef GAME_1V1_H
#define GAME_1V1_H

#include "lvgl.h"
#include <Arduino.h>

// --- Configuration des broches ---
#define PIN_P1_BACK   D2
#define PIN_P1_ATTACK D3
#define PIN_P2_BACK   D4
#define PIN_P2_ATTACK D5

// Prototypes fournis par main.cpp
extern int p1_skin;
extern int p2_skin;
extern void show_game_over(int winner);

// --- Variables internes du combat ---
static lv_obj_t * player1;
static lv_obj_t * player2;
static lv_obj_t * farm1;      
static lv_obj_t * farm2;      

static lv_obj_t * p1_weapon_rest;     
static lv_obj_t * p1_weapon_atk;     
static lv_obj_t * p2_weapon_rest;     
static lv_obj_t * p2_weapon_atk;     

static lv_obj_t * hp_bar1;
static lv_obj_t * hp_bar2;
static lv_obj_t * round_label;
static lv_obj_t * p1_hearts_obj[3];
static lv_obj_t * p2_hearts_obj[3];

static int p1_x = 20;
static int p2_x = 400;
static int p1_hp = 100;
static int p2_hp = 100;
static int p1_hearts = 3;
static int p2_hearts = 3;
static int current_round = 1;

static int p1_attack_cooldown = 0;
static int p2_attack_cooldown = 0;
static int p1_anim_timer = 0;
static int p2_anim_timer = 0;

const int PLAYER_WIDTH = 45;       
const int SPEED_FORWARD = 1;
const int SPEED_BACKWARD = 4;
const int ATTACK_DAMAGE = 10;      
const int COOLDOWN_FRAMES = 15;
const int ATTACK_RANGE = 35;       

// --- Générateurs de visuels ---
static lv_obj_t* create_pixel_heart(lv_obj_t * parent) {
    lv_obj_t * heart = lv_obj_create(parent);
    lv_obj_set_size(heart, 13, 11);
    lv_obj_set_style_bg_opa(heart, 0, 0);
    lv_obj_set_style_border_width(heart, 0, 0);
    lv_obj_set_style_pad_all(heart, 0, 0);
    lv_obj_remove_flag(heart, LV_OBJ_FLAG_SCROLLABLE);

    lv_color_t c = lv_color_hex(0xFF1144);
    int rects[][4] = {{1,0,3,1}, {9,0,3,1}, {0,1,13,4}, {1,5,11,1}, {2,6,9,1}, {3,7,7,1}, {4,8,5,1}, {5,9,3,1}, {6,10,1,1}};

    for(int i = 0; i < 9; i++) {
        lv_obj_t * p = lv_obj_create(heart);
        lv_obj_set_size(p, rects[i][2], rects[i][3]);
        lv_obj_align(p, LV_ALIGN_TOP_LEFT, rects[i][0], rects[i][1]);
        lv_obj_set_style_bg_color(p, c, 0);
        lv_obj_set_style_border_width(p, 0, 0);
        lv_obj_set_style_radius(p, 0, 0);
        lv_obj_remove_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    }
    return heart;
}

// Générateur d'arme dynamique (Escrime, Chevalier, Viking, Ninja)
static lv_obj_t* create_weapon(lv_obj_t * parent, int skin_id, bool is_player1, bool is_attack) {
    lv_obj_t * wpn = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(wpn, 0, 0);
    lv_obj_set_style_border_width(wpn, 0, 0);
    lv_obj_set_style_pad_all(wpn, 0, 0);
    lv_obj_remove_flag(wpn, LV_OBJ_FLAG_SCROLLABLE);

    if (skin_id == 0) { // Fleuret d'escrime
        lv_obj_t * blade = lv_obj_create(wpn);
        lv_obj_set_style_bg_color(blade, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_border_width(blade, 0, 0);
        if (is_attack) {
            lv_obj_set_size(wpn, ATTACK_RANGE, 3);
            lv_obj_set_size(blade, ATTACK_RANGE, 3);
        } else {
            lv_obj_set_size(wpn, 3, ATTACK_RANGE);
            lv_obj_set_size(blade, 3, ATTACK_RANGE);
        }
        lv_obj_align(blade, LV_ALIGN_TOP_LEFT, 0, 0);
    } 
    else if (skin_id == 1) { // Épée de Chevalier
        lv_obj_t * blade = lv_obj_create(wpn);
        lv_obj_t * guard = lv_obj_create(wpn);
        lv_obj_set_style_bg_color(blade, lv_color_hex(0xDDDDDD), 0);
        lv_obj_set_style_border_width(blade, 0, 0);
        lv_obj_set_style_bg_color(guard, lv_color_hex(0xDAA520), 0);
        lv_obj_set_style_border_width(guard, 0, 0);

        if (is_attack) {
            lv_obj_set_size(wpn, ATTACK_RANGE, 12);
            lv_obj_set_size(blade, ATTACK_RANGE, 4);
            lv_obj_align(blade, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_size(guard, 4, 12);
            lv_obj_align(guard, is_player1 ? LV_ALIGN_LEFT_MID : LV_ALIGN_RIGHT_MID, is_player1 ? 4 : -4, 0);
        } else {
            lv_obj_set_size(wpn, 12, ATTACK_RANGE);
            lv_obj_set_size(blade, 4, ATTACK_RANGE);
            lv_obj_align(blade, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_size(guard, 12, 4);
            lv_obj_align(guard, LV_ALIGN_BOTTOM_MID, 0, -4);
        }
    }
    else if (skin_id == 2) { // Hache de Viking
        lv_obj_t * handle = lv_obj_create(wpn);
        lv_obj_t * head = lv_obj_create(wpn);
        lv_obj_set_style_bg_color(handle, lv_color_hex(0x5C4033), 0); 
        lv_obj_set_style_border_width(handle, 0, 0);
        lv_obj_set_style_bg_color(head, lv_color_hex(0x95A5A6), 0); 
        lv_obj_set_style_border_width(head, 0, 0);
        lv_obj_set_style_radius(head, 3, 0);

        if (is_attack) {
            lv_obj_set_size(wpn, ATTACK_RANGE, 20);
            lv_obj_set_size(handle, ATTACK_RANGE, 4);
            lv_obj_align(handle, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_size(head, 12, 20);
            lv_obj_align(head, is_player1 ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID, 0, 0);
        } else {
            lv_obj_set_size(wpn, 20, ATTACK_RANGE);
            lv_obj_set_size(handle, 4, ATTACK_RANGE);
            lv_obj_align(handle, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_size(head, 20, 12);
            lv_obj_align(head, LV_ALIGN_TOP_MID, is_player1 ? 6 : -6, 0); 
        }
    }
    else if (skin_id == 3) { // Katana de Ninja
        lv_obj_t * handle = lv_obj_create(wpn);
        lv_obj_t * guard = lv_obj_create(wpn);
        lv_obj_t * blade = lv_obj_create(wpn);
        
        lv_obj_set_style_bg_color(handle, lv_color_hex(0x111111), 0); 
        lv_obj_set_style_border_width(handle, 0, 0);
        lv_obj_set_style_bg_color(guard, lv_color_hex(0xDAA520), 0); 
        lv_obj_set_style_border_width(guard, 0, 0);
        lv_obj_set_style_bg_color(blade, lv_color_hex(0xEEEEEE), 0); 
        lv_obj_set_style_border_width(blade, 0, 0);

        if (is_attack) {
            lv_obj_set_size(wpn, ATTACK_RANGE, 8);
            lv_obj_set_size(handle, 8, 3);
            lv_obj_align(handle, is_player1 ? LV_ALIGN_LEFT_MID : LV_ALIGN_RIGHT_MID, 0, 0);
            lv_obj_set_size(guard, 3, 8);
            lv_obj_align(guard, is_player1 ? LV_ALIGN_LEFT_MID : LV_ALIGN_RIGHT_MID, is_player1 ? 8 : -8, 0);
            lv_obj_set_size(blade, ATTACK_RANGE - 11, 2);
            lv_obj_align(blade, is_player1 ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID, 0, 0);
        } else {
            lv_obj_set_size(wpn, 8, ATTACK_RANGE);
            lv_obj_set_size(handle, 3, 8);
            lv_obj_align(handle, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_obj_set_size(guard, 8, 3);
            lv_obj_align(guard, LV_ALIGN_BOTTOM_MID, 0, -8);
            lv_obj_set_size(blade, 2, ATTACK_RANGE - 11);
            lv_obj_align(blade, LV_ALIGN_TOP_MID, 0, 0);
        }
    }
    return wpn;
}

// Générateur de personnage unifié
static lv_obj_t* create_character(lv_obj_t * parent, int skin_id, bool is_player1, bool is_preview = false) {
    lv_obj_t * char_container = lv_obj_create(parent);
    lv_obj_set_size(char_container, PLAYER_WIDTH, 75);
    lv_obj_set_style_bg_opa(char_container, 0, 0);
    lv_obj_set_style_border_width(char_container, 0, 0);
    lv_obj_set_style_pad_all(char_container, 0, 0); 
    lv_obj_remove_flag(char_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_color_t team_c = is_player1 ? lv_color_hex(0x0055FF) : lv_color_hex(0xFF0000);
    lv_color_t cloth_c, armor_c, shoe_c = lv_color_hex(0x111111);

    if (skin_id == 0) { cloth_c = lv_color_hex(0xEEEEEE); armor_c = lv_color_hex(0xEEEEEE); } // Escrimeur
    else if (skin_id == 1) { cloth_c = team_c; armor_c = lv_color_hex(0x8899A6); } // Chevalier
    else if (skin_id == 2) { cloth_c = team_c; armor_c = lv_color_hex(0xE2A76F); shoe_c = lv_color_hex(0x5C4033); } // Viking
    else if (skin_id == 3) { cloth_c = lv_color_hex(0x2B2B2B); armor_c = lv_color_hex(0x1A1A1A); } // Ninja

    // Torse
    lv_obj_t * torso = lv_obj_create(char_container);
    lv_obj_set_size(torso, 16, 28); 
    lv_obj_set_style_bg_color(torso, (skin_id == 0) ? team_c : cloth_c, 0); 
    lv_obj_set_style_border_width(torso, 0, 0); 
    lv_obj_set_style_radius(torso, 2, 0); 
    lv_obj_align(torso, LV_ALIGN_TOP_MID, 0, 22); 
    lv_obj_remove_flag(torso, LV_OBJ_FLAG_SCROLLABLE);

    if(skin_id == 2) { // Ceinture Viking
        lv_obj_t * belt = lv_obj_create(torso);
        lv_obj_set_size(belt, 16, 4);
        lv_obj_set_style_bg_color(belt, lv_color_hex(0x3E2723), 0);
        lv_obj_set_style_border_width(belt, 0, 0);
        lv_obj_align(belt, LV_ALIGN_BOTTOM_MID, 0, -4);
    } 
    else if(skin_id == 3) { // Ceinture Ninja aux couleurs de l'équipe
        lv_obj_t * belt = lv_obj_create(torso);
        lv_obj_set_size(belt, 16, 5);
        lv_obj_set_style_bg_color(belt, team_c, 0);
        lv_obj_set_style_border_width(belt, 0, 0);
        lv_obj_align(belt, LV_ALIGN_CENTER, 0, -2);
    }

    // Tête
    lv_obj_t * hood = lv_obj_create(char_container);
    lv_obj_set_size(hood, 18, 22); 
    lv_obj_set_style_bg_color(hood, armor_c, 0); 
    lv_obj_set_style_border_width(hood, 0, 0); 
    lv_obj_set_style_radius(hood, 4, 0); 
    lv_obj_set_style_pad_all(hood, 0, 0); 
    lv_obj_align(hood, LV_ALIGN_TOP_MID, is_player1 ? 3 : -3, 0); 
    lv_obj_remove_flag(hood, LV_OBJ_FLAG_SCROLLABLE);

    // Détails du visage selon le skin
    if (skin_id == 0) { // Masque escrime
        lv_obj_t * mask = lv_obj_create(hood);
        lv_obj_set_size(mask, 10, 16); 
        lv_obj_set_style_bg_color(mask, lv_color_hex(0x111111), 0); 
        lv_obj_set_style_border_width(mask, 0, 0); 
        lv_obj_set_style_radius(mask, 2, 0); 
        lv_obj_align(mask, LV_ALIGN_CENTER, is_player1 ? 4 : -4, 0); 
    } else if (skin_id == 1) { // Heaume de chevalier
        lv_obj_t * visor = lv_obj_create(hood);
        lv_obj_set_size(visor, 14, 4); 
        lv_obj_set_style_bg_color(visor, lv_color_hex(0x111111), 0); 
        lv_obj_set_style_border_width(visor, 0, 0); 
        lv_obj_align(visor, LV_ALIGN_CENTER, is_player1 ? 3 : -3, -2);
    } else if (skin_id == 2) { // Tête Viking
        lv_obj_t * helm = lv_obj_create(hood);
        lv_obj_set_size(helm, 18, 10);
        lv_obj_set_style_bg_color(helm, lv_color_hex(0x7F8C8D), 0);
        lv_obj_set_style_border_width(helm, 0, 0);
        lv_obj_align(helm, LV_ALIGN_TOP_MID, 0, 0);
        
        lv_obj_t * h1 = lv_obj_create(helm); lv_obj_set_size(h1, 4, 8); lv_obj_set_style_bg_color(h1, lv_color_hex(0xFFFDD0), 0); lv_obj_set_style_border_width(h1, 0, 0); lv_obj_align(h1, LV_ALIGN_TOP_LEFT, -2, -4);
        lv_obj_t * h2 = lv_obj_create(helm); lv_obj_set_size(h2, 4, 8); lv_obj_set_style_bg_color(h2, lv_color_hex(0xFFFDD0), 0); lv_obj_set_style_border_width(h2, 0, 0); lv_obj_align(h2, LV_ALIGN_TOP_RIGHT, 2, -4);
        
        lv_obj_t * beard = lv_obj_create(hood);
        lv_obj_set_size(beard, 12, 10);
        lv_obj_set_style_bg_color(beard, lv_color_hex(0xD35400), 0);
        lv_obj_set_style_border_width(beard, 0, 0);
        lv_obj_align(beard, LV_ALIGN_BOTTOM_MID, is_player1 ? 4 : -4, 2);
    } else if (skin_id == 3) { // Masque Ninja
        lv_obj_t * face_slit = lv_obj_create(hood);
        lv_obj_set_size(face_slit, 14, 5); 
        lv_obj_set_style_bg_color(face_slit, lv_color_hex(0xFAD4B9), 0); // Peau
        lv_obj_set_style_border_width(face_slit, 0, 0); 
        lv_obj_align(face_slit, LV_ALIGN_TOP_MID, is_player1 ? 2 : -2, 4);

        lv_obj_t * eye = lv_obj_create(face_slit);
        lv_obj_set_size(eye, 3, 3);
        lv_obj_set_style_bg_color(eye, lv_color_hex(0x000000), 0);
        lv_obj_set_style_border_width(eye, 0, 0);
        lv_obj_align(eye, is_player1 ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID, is_player1 ? -2 : 2, 0);
        
        // Bandeau ninja
        lv_obj_t * headband = lv_obj_create(hood);
        lv_obj_set_size(headband, 18, 3);
        lv_obj_set_style_bg_color(headband, team_c, 0);
        lv_obj_set_style_border_width(headband, 0, 0);
        lv_obj_align(headband, LV_ALIGN_TOP_MID, 0, 1);
    }

    // Membres
    lv_obj_t * barm = lv_obj_create(char_container); lv_obj_set_size(barm, 6, 20); lv_obj_set_style_bg_color(barm, armor_c, 0); lv_obj_set_style_border_width(barm, 0, 0); lv_obj_align(barm, LV_ALIGN_TOP_MID, is_player1 ? -12 : 12, 6); lv_obj_remove_flag(barm, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * fleg = lv_obj_create(char_container); lv_obj_set_size(fleg, 7, 24); lv_obj_set_style_bg_color(fleg, armor_c, 0); lv_obj_set_style_border_width(fleg, 0, 0); lv_obj_align(fleg, LV_ALIGN_BOTTOM_MID, is_player1 ? 12 : -12, 0); lv_obj_remove_flag(fleg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * fshoe = lv_obj_create(char_container); lv_obj_set_size(fshoe, 14, 5); lv_obj_set_style_bg_color(fshoe, shoe_c, 0); lv_obj_set_style_border_width(fshoe, 0, 0); lv_obj_align_to(fshoe, fleg, LV_ALIGN_OUT_BOTTOM_MID, is_player1 ? 2 : -2, -2); lv_obj_remove_flag(fshoe, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * bleg = lv_obj_create(char_container); lv_obj_set_size(bleg, 7, 24); lv_obj_set_style_bg_color(bleg, armor_c, 0); lv_obj_set_style_border_width(bleg, 0, 0); lv_obj_align(bleg, LV_ALIGN_BOTTOM_MID, is_player1 ? -12 : 12, 0); lv_obj_remove_flag(bleg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * bshoe = lv_obj_create(char_container); lv_obj_set_size(bshoe, 14, 5); lv_obj_set_style_bg_color(bshoe, shoe_c, 0); lv_obj_set_style_border_width(bshoe, 0, 0); lv_obj_align_to(bshoe, bleg, LV_ALIGN_OUT_BOTTOM_MID, is_player1 ? -2 : 2, -2); lv_obj_remove_flag(bshoe, LV_OBJ_FLAG_SCROLLABLE);
    
    // Bras Avant Animé
    lv_obj_t * farm = lv_obj_create(char_container); lv_obj_set_size(farm, 10, 6); lv_obj_set_style_bg_color(farm, armor_c, 0); lv_obj_set_style_border_width(farm, 0, 0); lv_obj_align(farm, LV_ALIGN_TOP_MID, is_player1 ? 10 : -10, 26); lv_obj_remove_flag(farm, LV_OBJ_FLAG_SCROLLABLE);
    
    if(!is_preview) {
        if(is_player1) farm1 = farm; else farm2 = farm;
    }

    return char_container;
}

// --- Logique du 1v1 ---
static void reset_game_1v1() {
    p1_x = 20; p2_x = 400; p1_hp = 100; p2_hp = 100;
    p1_hearts = 3; p2_hearts = 3; current_round = 1;
    p1_attack_cooldown = 0; p2_attack_cooldown = 0;
    
    lv_label_set_text(round_label, "ROUND 1");
    lv_bar_set_value(hp_bar1, 100, LV_ANIM_OFF);
    lv_bar_set_value(hp_bar2, 100, LV_ANIM_OFF);
    for(int i = 0; i < 3; i++) {
        lv_obj_remove_flag(p1_hearts_obj[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(p2_hearts_obj[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void init_game_1v1(lv_obj_t * scr) {
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x332222), 0);
    lv_obj_t * piste = lv_obj_create(scr);
    lv_obj_set_size(piste, 480, 80); lv_obj_align(piste, LV_ALIGN_BOTTOM_MID, 0, 0); lv_obj_set_style_bg_color(piste, lv_color_hex(0xDDCCBB), 0); lv_obj_set_style_border_color(piste, lv_color_hex(0x2244AA), 0); lv_obj_set_style_border_width(piste, 5, 0); lv_obj_set_style_border_side(piste, LV_BORDER_SIDE_TOP, 0); lv_obj_set_style_radius(piste, 0, 0); lv_obj_remove_flag(piste, LV_OBJ_FLAG_SCROLLABLE);

    // Joueurs
    player1 = create_character(scr, p1_skin, true); 
    player2 = create_character(scr, p2_skin, false);
    lv_obj_align(player1, LV_ALIGN_BOTTOM_LEFT, p1_x, -30);
    lv_obj_align(player2, LV_ALIGN_BOTTOM_LEFT, p2_x, -30);

    // Armes
    p1_weapon_rest = create_weapon(scr, p1_skin, true, false);
    p1_weapon_atk  = create_weapon(scr, p1_skin, true, true);
    lv_obj_add_flag(p1_weapon_atk, LV_OBJ_FLAG_HIDDEN);

    p2_weapon_rest = create_weapon(scr, p2_skin, false, false);
    p2_weapon_atk  = create_weapon(scr, p2_skin, false, true);
    lv_obj_add_flag(p2_weapon_atk, LV_OBJ_FLAG_HIDDEN);

    // HUD
    hp_bar1 = lv_bar_create(scr); lv_obj_set_size(hp_bar1, 150, 18); lv_obj_align(hp_bar1, LV_ALIGN_TOP_LEFT, 20, 20); lv_bar_set_range(hp_bar1, 0, 100); lv_obj_set_style_bg_color(hp_bar1, lv_color_hex(0x00FF00), LV_PART_INDICATOR); lv_obj_set_style_radius(hp_bar1, 2, 0); lv_obj_set_style_radius(hp_bar1, 2, LV_PART_INDICATOR);
    hp_bar2 = lv_bar_create(scr); lv_obj_set_size(hp_bar2, 150, 18); lv_obj_align(hp_bar2, LV_ALIGN_TOP_RIGHT, -20, 20); lv_bar_set_range(hp_bar2, 0, 100); lv_obj_set_style_bg_color(hp_bar2, lv_color_hex(0x00FF00), LV_PART_INDICATOR); lv_obj_set_style_radius(hp_bar2, 2, 0); lv_obj_set_style_radius(hp_bar2, 2, LV_PART_INDICATOR);

    for(int i=0; i<3; i++) { p1_hearts_obj[i] = create_pixel_heart(scr); lv_obj_align(p1_hearts_obj[i], LV_ALIGN_TOP_LEFT, 20+(i*20), 45); }
    for(int i=0; i<3; i++) { p2_hearts_obj[i] = create_pixel_heart(scr); lv_obj_align(p2_hearts_obj[i], LV_ALIGN_TOP_RIGHT, -45-(i*20), 45); }

    round_label = lv_label_create(scr); lv_label_set_text(round_label, "ROUND 1"); lv_obj_set_style_text_color(round_label, lv_color_hex(0xFFFFFF), 0); lv_obj_align(round_label, LV_ALIGN_TOP_MID, 0, 20);
}

static void game_1v1_loop() {
    bool p1_back = (digitalRead(PIN_P1_BACK) == HIGH); bool p2_back = (digitalRead(PIN_P2_BACK) == HIGH);
    bool p1_atk  = (digitalRead(PIN_P1_ATTACK) == LOW); bool p2_atk  = (digitalRead(PIN_P2_ATTACK) == LOW);

    if (p1_back) p1_x -= SPEED_BACKWARD; else p1_x += SPEED_FORWARD;
    if (p2_back) p2_x += SPEED_BACKWARD; else p2_x -= SPEED_FORWARD;

    int overlap = (p1_x + PLAYER_WIDTH) - p2_x;
    if (overlap > 0) { p1_x -= overlap / 2; p2_x += overlap / 2 + (overlap % 2); }
    if (p1_x < 0) p1_x = 0;
    if (p2_x > 480 - PLAYER_WIDTH) p2_x = 480 - PLAYER_WIDTH;
    if (p1_x + PLAYER_WIDTH > p2_x) {
        if (p1_x == 0) p2_x = PLAYER_WIDTH;
        if (p2_x == 480 - PLAYER_WIDTH) p1_x = 480 - 2 * PLAYER_WIDTH;
    }

    bool in_range = (p2_x - (p1_x + PLAYER_WIDTH) <= ATTACK_RANGE);
    if (p1_attack_cooldown > 0) p1_attack_cooldown--;
    if (p2_attack_cooldown > 0) p2_attack_cooldown--;

    if (p1_atk && p1_attack_cooldown == 0) {
        p1_attack_cooldown = COOLDOWN_FRAMES; p1_anim_timer = 5; 
        if (in_range && p2_hp > 0) { p2_hp -= ATTACK_DAMAGE; if(p2_hp < 0) p2_hp = 0; lv_bar_set_value(hp_bar2, p2_hp, LV_ANIM_ON); }
    }
    if (p2_atk && p2_attack_cooldown == 0) {
        p2_attack_cooldown = COOLDOWN_FRAMES; p2_anim_timer = 5; 
        if (in_range && p1_hp > 0) { p1_hp -= ATTACK_DAMAGE; if(p1_hp < 0) p1_hp = 0; lv_bar_set_value(hp_bar1, p1_hp, LV_ANIM_ON); }
    }

    if (p1_hp <= 0 || p2_hp <= 0) {
        if (p1_hp <= 0) { p1_hearts--; if (p1_hearts >= 0) lv_obj_add_flag(p1_hearts_obj[p1_hearts], LV_OBJ_FLAG_HIDDEN); } 
        else { p2_hearts--; if (p2_hearts >= 0) lv_obj_add_flag(p2_hearts_obj[p2_hearts], LV_OBJ_FLAG_HIDDEN); }

        if (p1_hearts <= 0 || p2_hearts <= 0) {
            show_game_over(p1_hearts <= 0 ? 2 : 1);
        } else {
            p1_x = 20; p2_x = 400; p1_hp = 100; p2_hp = 100; p1_attack_cooldown = 0; p2_attack_cooldown = 0;
            current_round++; char buf[16]; lv_snprintf(buf, sizeof(buf), "ROUND %d", current_round);
            lv_label_set_text(round_label, buf);
            lv_bar_set_value(hp_bar1, 100, LV_ANIM_OFF); lv_bar_set_value(hp_bar2, 100, LV_ANIM_OFF);
        }
        return;
    }

    lv_obj_set_x(player1, p1_x); lv_obj_set_x(player2, p2_x);

    // Animation P1 
    if (p1_anim_timer > 0) {
        lv_obj_set_size(farm1, 22, 6); lv_obj_align(farm1, LV_ALIGN_TOP_MID, 16, 26);
        lv_obj_add_flag(p1_weapon_rest, LV_OBJ_FLAG_HIDDEN); lv_obj_clear_flag(p1_weapon_atk, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(p1_weapon_atk, farm1, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        p1_anim_timer--;
    } else {
        lv_obj_set_size(farm1, 10, 6); lv_obj_align(farm1, LV_ALIGN_TOP_MID, 10, 26);
        lv_obj_clear_flag(p1_weapon_rest, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(p1_weapon_atk, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(p1_weapon_rest, farm1, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);
    }

    // Animation P2
    if (p2_anim_timer > 0) {
        lv_obj_set_size(farm2, 22, 6); lv_obj_align(farm2, LV_ALIGN_TOP_MID, -16, 26);
        lv_obj_add_flag(p2_weapon_rest, LV_OBJ_FLAG_HIDDEN); lv_obj_clear_flag(p2_weapon_atk, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(p2_weapon_atk, farm2, LV_ALIGN_OUT_LEFT_MID, 0, 0);
        p2_anim_timer--;
    } else {
        lv_obj_set_size(farm2, 10, 6); lv_obj_align(farm2, LV_ALIGN_TOP_MID, -10, 26);
        lv_obj_clear_flag(p2_weapon_rest, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(p2_weapon_atk, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(p2_weapon_rest, farm2, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
    }
}

#endif