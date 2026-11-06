#include "GUI.h"
#include <stdint.h>



#define MAKE_COLOR(R, G, B)   (((uint32_t)(B) << 16) | ((uint32_t)(G) << 8) | (R))

/* ---------------- FOND & BANDEAUX ---------------- */
#define COLOR_BG                 MAKE_COLOR(0x0A, 0x0E, 0x17) // Fond général bleu-nuit très sombre
#define COLOR_TOP_HEADER_BG      MAKE_COLOR(0x1B, 0x24, 0x36) // Rectangle du haut (titre + score)
#define COLOR_TOP_HEADER_BORDER  MAKE_COLOR(0x00, 0xF0, 0xFF) // Bordure néon du bandeau supérieur

/* ---------------- TEXTES ---------------- */
#define COLOR_TEXT_TITLE         MAKE_COLOR(0xFF, 0xFF, 0xFF) // Titre de la musique (Blanc pur)
#define COLOR_TEXT_SCORE         MAKE_COLOR(0x00, 0xF0, 0xFF) // Score en chiffre (Cyan vif)

/* ---------------- TUILES (NOTES) ---------------- */
#define COLOR_TILE_FALLING       MAKE_COLOR(0x00, 0xC8, 0xFF) // Note qui descend (Cyan très visible)
#define COLOR_TILE_PRESSED_OK    MAKE_COLOR(0x39, 0xFF, 0x14) // Note appuyée avec succès (Vert néon flash)
#define COLOR_TILE_PRESSED_MISS  MAKE_COLOR(0xFF, 0x00, 0x55) // Erreur / mauvais timing (Rouge écarlate)
#define COLOR_TILE_INACTIVE      MAKE_COLOR(0x13, 0x1A, 0x27) // Fond de colonne / piste vide

/* ---------------- LIGNES DE SÉPARATION ---------------- */
#define COLOR_LANE_LINE          MAKE_COLOR(0x24, 0x30, 0x47) // Séparation entre les 4 colonnes
#define COLOR_HIT_LINE           MAKE_COLOR(0xFF, 0xE6, 0x00) // Ligne cible en bas (Jaune)


typedef struct
{
    uint8_t piste_num;
    int height;
    int y; // position vertical
} DrawNote_t;


void display_init_screen();
void display_note(DrawNote_t val,int y);
void display_note_section();
void display_header();
void display_musique_name(char * musiqueName);
void display_score(int score);
void clear_note(DrawNote_t val);
void Draw_note_line();
void Draw_bottom_line();
