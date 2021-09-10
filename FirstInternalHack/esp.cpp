#include "pch.h"
#include "esp.h"
#include "geom.h"

bool ESP::isTeamGame() {
	if ((*gameMode == 0 || *gameMode == 4 || *gameMode == 5 || *gameMode == 7 || *gameMode == 11 || *gameMode == 13 ||
		*gameMode == 14 || *gameMode == 16 || *gameMode == 17 || *gameMode == 20 || *gameMode == 21))
		return true;
	else return false;
}

bool ESP::isEnemy(ent* e) {
	if (localPlayer->team == e->team) {
		return false;
	}

	return true;
}

bool ESP::isValidEnt(ent* e) {
	if (e) {
		if (e->health <= 0) return false;

		if (e->vTable == 0x4E4A98 || e->vTable == 0x4E4AC0) {
			return true;
		}
	}
	return false;
}

void ESP::DrawESPLifebar(ent* p, vec3 screen) {
	const GLubyte* color = rgb::black;

	float dist = localPlayer->pos.Distance(p->pos);

	float scale = (GAME_UNIT_MAGIC / dist) * (viewport[2] / VIRTUAL_SCREEN_WIDTH) * 2;

	float x = screen.x - scale - 16;
	float y = screen.y - scale * PLAYER_ASPECT_RATIO;

	float width = 10;
	float height = scale * PLAYER_ASPECT_RATIO * 2;

	GL::DrawOutlineRect(x, y, width, height, 2.0f, color);

	float healthBarHeight = height * (float)(p->health / 100.0);

	GL::DrawFilledRect(x, y, width, height, rgb::darkGreen);
	GL::DrawFilledRect(x, y - (healthBarHeight - height) , width, healthBarHeight, rgb::green);
}

void ESP::DrawESPBox(ent* p, vec3 screen, GL::Font& font, bool canRenderHealthBar) {
	const GLubyte* color = nullptr;

	if (isTeamGame() && !isEnemy(p)) {
		color = rgb::green;	
	}
	else {
		color = rgb::red;
	}

	float dist = localPlayer->pos.Distance(p->pos);

	float scale = (GAME_UNIT_MAGIC / dist) * (viewport[2] / VIRTUAL_SCREEN_WIDTH) * 2;

	float x = screen.x - scale;
	float y = screen.y - scale * PLAYER_ASPECT_RATIO;

	float width = scale * 2;
	float height = scale * PLAYER_ASPECT_RATIO * 2;


	GL::DrawOutlineRect(x, y, width, height, 2.0f, color);
	if (canRenderHealthBar) {
		DrawESPLifebar(p, screen);
	}

	float textX = font.centerText(x, width, strlen(p->name) * ESP_FONT_WIDTH);
	float textY = y - ESP_FONT_HEIGTH / 2;

	font.Print(textX, textY, color, "%s", p->name);
}

void ESP::Draw(GL::Font& font, bool canRenderHealthBar) {
	glGetIntegerv(GL_VIEWPORT, viewport);

	for (int i = 0; i < (*numOfPlayers); i++) {
		if (entlist && entlist->ents && isValidEnt(entlist->ents[i])) {
			ent* e = entlist->ents[i];
			vec3 center = e->head;
			center.z = center.z - EYE_HEIGHT + PLAYER_HEIGHT / 2;

			vec3 screenCoords;
			if (WorldToScreen(center, screenCoords, matrix, viewport[2], viewport[3])) {
				DrawESPBox(e, screenCoords, font, canRenderHealthBar);
			}
		}
	}
}