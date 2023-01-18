#pragma once

#define DELTA_TIME (1.0f / 60.0f)

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>

enum GameState
{
	PLAYING,
	PAUSED
};

class GameOfLife;

class Button
{
public:
	Button();
	Button(sf::FloatRect rect, sf::Color color, std::string str, sf::Font& font, void (*func)(GameOfLife*), GameOfLife* game);

	bool check(sf::Vector2f click_pos);
	void render(sf::RenderWindow& window);

private:
	sf::FloatRect rect;
	sf::RectangleShape shape;
	sf::Text text;

	void (*func)(GameOfLife*);
	GameOfLife* game;
};

class GameOfLife
{
public:
	GameOfLife(uint16_t width, uint16_t height, int wWidth, int wHeight, float space, sf::Time step_time);

	friend void pause(GameOfLife* game);
	friend void unpause(GameOfLife* game);
	friend void clear(GameOfLife* game);
	friend void random(GameOfLife* game);

	void checkEvents(sf::Event& event, sf::RenderWindow& window);
	void update(float dt);
	void render(sf::RenderWindow& window);

private:
	uint16_t width, height;
	float space;

	bool** current_world;
	bool** next_world;

	sf::VertexArray lines, quads;

	sf::View view, default_view;

	sf::Vector2f camera_pos;
	float camera_speed;

	sf::Time step_time, elapsed_time;
	GameState state;

	sf::Font font;
	Button pause_button, unpause_button, clear_button, random_button;

	void addLife(uint16_t xpos, uint16_t ypos);
	void delLife(uint16_t xpos, uint16_t ypos);

	uint16_t neighbours(uint16_t xpos, uint16_t ypos);
	void step();

	void checkKeys();

	void playing_update(float dt);
};

