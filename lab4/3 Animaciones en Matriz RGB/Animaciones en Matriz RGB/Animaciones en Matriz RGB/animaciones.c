#include "animaciones.h"
#include <stdlib.h>
#include <stdint.h>

#define LED_COUNT 64
#define MAT_W 8
#define MAT_H 8

static uint8_t modo_actual = MODO_OFF;

// ===== Estado animación 1 (corazón palpitando) =====
static uint32_t ultimo_tiempo_anim1 = 0;
static uint8_t heart_phase = 0;

// ===== Estado animación 2 (LLUVIA CON RAYOS MEJORADA) =====
#define NUM_DROPS 8

static uint32_t ultimo_tiempo_lluvia = 0;
static uint8_t drop_row[NUM_DROPS];
static int8_t  drop_col[NUM_DROPS];
static uint8_t lluvia_inicializada = 0;

// Rayos
static uint8_t  rayo_activo = 0;
static uint8_t  rayo_fase   = 0;
static uint32_t ultimo_tiempo_rayo = 0;
static uint32_t proximo_rayo_ms = 0;

// ===================================================

static void fb_clear(uint8_t *fb) {
	for (uint16_t i = 0; i < LED_COUNT * 3; i++) {
		fb[i] = 0;
	}
}

// Orden GRB para WS2812
static void fb_set_pixel(uint8_t *fb, uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
	if (index >= LED_COUNT) return;
	uint16_t base = index * 3;
	fb[base + 0] = g;
	fb[base + 1] = r;
	fb[base + 2] = b;
}

static void fb_set_xy(uint8_t *fb, uint8_t x, uint8_t y,
uint8_t r, uint8_t g, uint8_t b) {
	if (x >= MAT_W || y >= MAT_H) return;
	uint8_t index = y * MAT_W + x;
	fb_set_pixel(fb, index, r, g, b);
}

// ===== NUEVO: Color sólido para toda la matriz =====
void animaciones_color_solido(uint8_t *fb, uint8_t r, uint8_t g, uint8_t b) {
	for (uint16_t i = 0; i < LED_COUNT; i++) {
		fb_set_pixel(fb, i, r, g, b);
	}
}

// ===== Bitmaps del corazón (para anim1) =====

static const uint8_t heart_border[MAT_H] = {
	0b00000000,
	0b01100110,
	0b10011001,
	0b10000001,
	0b10000001,
	0b01000010,
	0b00100100,
	0b00011000
};

static const uint8_t heart_inner[MAT_H] = {
	0b00000000,
	0b00000000,
	0b01100110,
	0b01111110,
	0b01111110,
	0b00111100,
	0b00011000,
	0b00000000
};

static void draw_heart(uint8_t *fb,
uint8_t border_r, uint8_t border_g, uint8_t border_b,
uint8_t inner_r,  uint8_t inner_g,  uint8_t inner_b) {

	fb_clear(fb);

	for (uint8_t y = 0; y < MAT_H; y++) {
		uint8_t row_b = heart_border[y];
		uint8_t row_i = heart_inner[y];

		for (uint8_t x = 0; x < MAT_W; x++) {
			uint8_t mask = (1 << (7 - x));

			if (row_i & mask) {
				fb_set_xy(fb, x, y, inner_r, inner_g, inner_b);
			}
			else if (row_b & mask) {
				fb_set_xy(fb, x, y, border_r, border_g, border_b);
			}
		}
	}
}

// ===================================================

void animaciones_init(uint8_t *fb) {
	modo_actual = MODO_OFF;
	fb_clear(fb);

	// Corazón
	ultimo_tiempo_anim1 = 0;
	heart_phase = 0;

	// Lluvia
	ultimo_tiempo_lluvia = 0;
	lluvia_inicializada = 0;
	rayo_activo = 0;
	rayo_fase = 0;
	ultimo_tiempo_rayo = 0;
	proximo_rayo_ms = 3000;
}

void animaciones_set_modo(uint8_t modo, uint8_t *fb) {
	modo_actual = modo;

	if (modo_actual == MODO_OFF) {
		fb_clear(fb);
	}

	if (modo_actual == MODO_ANIM1) {
		heart_phase = 0;
		ultimo_tiempo_anim1 = 0;
	}

	if (modo_actual == MODO_ANIM2) {
		ultimo_tiempo_lluvia = 0;
		lluvia_inicializada = 0;
		rayo_activo = 0;
		rayo_fase = 0;
		ultimo_tiempo_rayo = 0;
		proximo_rayo_ms = 2000 + (rand() % 8000); // 2 a 10 segundos
	}
}

uint8_t animaciones_update(uint32_t t_ms, uint8_t *fb) {

	if (modo_actual == MODO_OFF) {
		fb_clear(fb);
		return 1;
	}

	// ===== MODO 1: CORAZÓN =====
	if (modo_actual == MODO_ANIM1) {

		if (t_ms - ultimo_tiempo_anim1 >= 120) {
			ultimo_tiempo_anim1 = t_ms;

			const uint8_t border_levels[6] = {80, 140, 200, 255, 200, 140};
			const uint8_t inner_levels[6]  = {40, 100, 160, 220, 160, 100};

			uint8_t idx = heart_phase % 6;

			draw_heart(fb,
			border_levels[idx], 0, 0,
			inner_levels[idx], inner_levels[idx]/4, inner_levels[idx]/8
			);

			heart_phase++;
			return 1;
		}
		return 0;
	}

	// ===== MODO 2: LLUVIA MEJORADA =====
	if (modo_actual == MODO_ANIM2) {

		// Inicialización de gotas
		if (!lluvia_inicializada) {
			for (uint8_t k = 0; k < NUM_DROPS; k++) {
				if (k < 4) {
					drop_row[k] = rand() % 3;
					} else {
					drop_row[k] = 3 + (rand() % 5);
				}
				drop_col[k] = rand() % 8;
			}
			lluvia_inicializada = 1;
		}

		// --- Control de rayos ---
		if (!rayo_activo) {
			if (t_ms - ultimo_tiempo_rayo >= proximo_rayo_ms) {
				rayo_activo = 1;
				rayo_fase = 0;
				ultimo_tiempo_rayo = t_ms;
			}
			} else {
			if (t_ms - ultimo_tiempo_rayo >= 120) {
				ultimo_tiempo_rayo = t_ms;
				rayo_fase++;
				if (rayo_fase >= 8) {
					rayo_activo = 0;
					proximo_rayo_ms = 2000 + (rand() % 8000);
				}
			}
		}

		// --- Frame de lluvia ---
		if (t_ms - ultimo_tiempo_lluvia >= 100) {
			ultimo_tiempo_lluvia = t_ms;

			fb_clear(fb);

			// 1) Dibujar gotas
			for (uint8_t k = 0; k < NUM_DROPS; k++) {
				for (uint8_t seg = 0; seg < 3; seg++) {
					int8_t rr = drop_row[k] + seg;
					int8_t cc = drop_col[k] + seg;

					if (rr >= 0 && rr < MAT_H && cc >= 0 && cc < MAT_W) {
						fb_set_xy(fb, cc, rr, 0, 0, 180);
					}
				}
			}

			// 2) Colores de nube
			uint8_t nr = 80, ng = 80, nb = 80;
			if (rayo_activo) {
				uint8_t intensidad = 50 + rayo_fase * 20;
				nr = 180 + intensidad;
				ng = 180 + intensidad;
				nb = 20;
			}

			// Dibujar nube
			for (uint8_t y = 0; y < 3; y++) {
				for (uint8_t x = 0; x < 8; x++) {
					fb_set_xy(fb, x, y, nr, ng, nb);
				}
			}

			// 3) Actualizar gotas
			for (uint8_t k = 0; k < NUM_DROPS; k++) {
				drop_row[k]++;
				drop_col[k]++;

				if (drop_row[k] >= MAT_H || drop_col[k] >= MAT_W) {
					drop_row[k] = rand() % MAT_H;
					drop_col[k] = rand() % MAT_W;
				}
			}

			return 1;
		}

		return 0;
	}

	return 0;
}
