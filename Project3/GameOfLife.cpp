#include "GameOfLife.h"

Button::Button() {}

Button::Button(sf::FloatRect rect, sf::Color color, std::string str, sf::Font& font, void(*func)(GameOfLife*), GameOfLife* game)
{
	this->rect = rect;

	this->shape.setPosition(rect.left, rect.top);
	this->shape.setSize(sf::Vector2f(rect.width, rect.height));
	this->shape.setFillColor(color);

	this->text.setFont(font);
	this->text.setCharacterSize(14);
	this->text.setString(str);
	this->text.setFillColor(sf::Color::Black);
	this->text.setPosition(rect.left, rect.top);

	this->func = func;
	this->game = game;
}

bool Button::check(sf::Vector2f click_pos)
{
	if (this->rect.contains(click_pos))
	{
		this->func(this->game);
		return true;
	}
	return false;
}

void Button::render(sf::RenderWindow& window)
{
	window.draw(this->shape);
	window.draw(this->text);
}

//-----------------------------------------------------------------------------------

void pause(GameOfLife* game)
{
	game->state = PAUSED;
}

void unpause(GameOfLife* game)
{
	game->state = PLAYING;
}

void clear(GameOfLife* game)
{
	for (uint16_t i = 0; i < game->height; i++)
	{
		for (uint16_t j = 0; j < game->width; j++)
			game->delLife(j, i);
	}
}

void random(GameOfLife* game)
{
	for (uint16_t i = 0; i < game->height; i++)
	{
		for (uint16_t j = 0; j < game->width; j++)
			game->current_world[i][j] = rand() % 2;
	}
}

GameOfLife::GameOfLife(uint16_t width, uint16_t height, int wWidth, int wHeight, float space, sf::Time step_time)
{
	this->width = width;
	this->height = height;
	this->space = space;

	srand(time(0));

	this->camera_pos = sf::Vector2f(width * space / 2, height * space / 2);
	this->camera_speed = 7.5f;

	this->view.setSize(wWidth, wHeight);
	this->default_view.reset(sf::FloatRect(0.0f, 0.0f, wWidth, wHeight));
	
	this->current_world = new bool* [this->height];
	for (uint16_t i = 0; i < this->height; i++)
	{
		this->current_world[i] = new bool[this->width];
		for (uint16_t j = 0; j < this->width; j++)
			this->current_world[i][j] = 0;
	}

	this->next_world = new bool* [this->height];
	for (uint16_t i = 0; i < this->height; i++)
	{
		this->next_world[i] = new bool[this->width];
		for (uint16_t j = 0; j < this->width; j++)
			this->next_world[i][j] = 0;
	}

	this->lines = sf::VertexArray(sf::Lines, 2 * (this->width + this->height) + 4);

	for (uint16_t i = 0; i <= 2 * this->width; i += 2)
	{
		this->lines[i] = sf::Vertex(sf::Vector2f(i / 2 * this->space, 0.0f), sf::Color::Black);
		this->lines[i + 1] = sf::Vertex(sf::Vector2f(i / 2 * this->space, this->height * this->space), sf::Color::Black);
	}

	for (uint16_t i = 0; i <= 2 * this->height; i += 2)
	{
		this->lines[2 * this->width + 2 + i] = sf::Vertex(sf::Vector2f(0.0f, i / 2 * this->space), sf::Color::Black);
		this->lines[2 * this->width + 3 + i] = sf::Vertex(sf::Vector2f(this->width * this->space, i / 2 * this->space), sf::Color::Black);
	}

	this->quads = sf::VertexArray(sf::Quads, this->width * this->height * 4);

	for (uint16_t i = 0; i < this->width * this->height * 4; i += 4)
	{
		uint32_t qp = i / 4;

		this->quads[i] = sf::Vertex(sf::Vector2f((qp % this->width) * this->space, (int)(qp / this->height) * this->space), sf::Color::White);
		this->quads[i + 1] = sf::Vertex(sf::Vector2f((qp % this->width + 1) * this->space, (int)(qp / this->height) * this->space), sf::Color::White);
		this->quads[i + 2] = sf::Vertex(sf::Vector2f((qp % this->width + 1) * this->space, (int)(qp / this->height + 1) * this->space), sf::Color::White);
		this->quads[i + 3] = sf::Vertex(sf::Vector2f((qp % this->width) * this->space, (int)(qp / this->height + 1) * this->space), sf::Color::White);
	}

	this->state = PAUSED;

	this->font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
	this->pause_button = Button(sf::FloatRect(40.0f, 750.0f, 70.0f, 45.0f), sf::Color(200, 200, 200), "pause", this->font, pause, this);
	this->unpause_button = Button(sf::FloatRect(135.0f, 750.0f, 70.0f, 45.0f), sf::Color(200, 200, 200), "resume", this->font, unpause, this);
	this->clear_button = Button(sf::FloatRect(230.0f, 750.0f, 70.0f, 45.0f), sf::Color(200, 200, 200), "clear", this->font, clear, this);
	this->random_button = Button(sf::FloatRect(325.0f, 750.0f, 70.0f, 45.0f), sf::Color(200, 200, 200), "random", this->font, random, this);

	this->step_time = step_time;
	this->elapsed_time = sf::seconds(0.0f);
}

void GameOfLife::addLife(uint16_t xpos, uint16_t ypos)
{
	this->current_world[ypos][xpos] = 1;
}

void GameOfLife::delLife(uint16_t xpos, uint16_t ypos)
{
	this->current_world[ypos][xpos] = 0;
}

uint16_t GameOfLife::neighbours(uint16_t xpos, uint16_t ypos)
{
	xpos += this->width * (xpos == 0);
	ypos += this->height * (ypos == 0);

	uint16_t n = 0;

	for (uint16_t y = ypos - 1; y <= ypos + 1; y++)
	{
		for (uint16_t x = xpos - 1; x <= xpos + 1; x++)
		{
			if (x != xpos || y != ypos)
			{
				if (this->current_world[y % this->height][x % this->width])
					n++;
			}
		}
	}

	return n;

	/*return this->current_world[(xpos - 1) % this->width][(ypos - 1) % this->height] + this->current_world[(xpos) % this->width][(ypos - 1) % this->height] +
		this->current_world[(xpos + 1) % this->width][(ypos - 1) % this->height] + this->current_world[(xpos - 1) % this->width][(ypos) % this->height] +
		this->current_world[(xpos + 1) % this->width][(ypos) % this->height] + this->current_world[(xpos - 1) % this->width][(ypos + 1) % this->height] +
		this->current_world[(xpos) % this->width][(ypos + 1) % this->height] + this->current_world[(xpos + 1) % this->width][(ypos + 1) % this->height];*/
}

void GameOfLife::step()
{
	for (uint16_t ypos = 0; ypos < this->height; ypos++)
	{
		for (uint16_t xpos = 0; xpos < this->width; xpos++)
		{
			uint16_t nnum = this->neighbours(xpos, ypos);

			if (this->current_world[ypos][xpos])
				this->next_world[ypos][xpos] = (nnum == 2 || nnum == 3);
			else
				this->next_world[ypos][xpos] = (nnum == 3);
		}
	}

	for (uint16_t i = 0; i < this->height; i++)
	{
		for (uint16_t j = 0; j < this->width; j++)
		{
			this->current_world[i][j] = this->next_world[i][j];
			this->next_world[i][j] = 0;
		}
	}
}

void GameOfLife::checkEvents(sf::Event& event, sf::RenderWindow& window)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		sf::Vector2f mpos = sf::Vector2f(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);

		if (event.mouseButton.button == sf::Mouse::Left)
		{
			if (!this->pause_button.check(mpos) &&
				!this->unpause_button.check(mpos) &&
				!this->clear_button.check(mpos) &&
				!this->random_button.check(mpos))
			{
				sf::Vector2f rmpos = sf::Vector2f(mpos.x + (this->camera_pos.x - window.getSize().x / 2), mpos.y + (this->camera_pos.y - window.getSize().y / 2));

				if (0.0f <= rmpos.x && rmpos.x < this->width * this->space &&
					0.0f <= rmpos.y && rmpos.y <= this->height * this->space)
				{
					uint16_t xpos = rmpos.x / space;
					uint16_t ypos = rmpos.y / space;
					this->addLife(xpos, ypos);
				}
			}
		}
		else if (event.mouseButton.button == sf::Mouse::Right)
		{
			sf::Vector2f rmpos = sf::Vector2f(mpos.x + (this->camera_pos.x - window.getSize().x / 2), mpos.y + (this->camera_pos.y - window.getSize().y / 2));

			if (0.0f <= rmpos.x && rmpos.x < this->width * this->space &&
				0.0f <= rmpos.y && rmpos.y <= this->height * this->space)
			{
				uint16_t xpos = rmpos.x / space;
				uint16_t ypos = rmpos.y / space;
				this->delLife(xpos, ypos);
			}
		}
	}
	if (event.type == sf::Event::MouseWheelScrolled)
	{
		//view.zoom(1.0f - event.mouseWheelScroll.delta / 50.0f);
	}
}

void GameOfLife::checkKeys()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		this->camera_pos.y -= this->camera_speed;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		this->camera_pos.x -= this->camera_speed;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		this->camera_pos.y += this->camera_speed;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		this->camera_pos.x += this->camera_speed;

	this->view.setCenter(this->camera_pos);
}

void GameOfLife::playing_update(float dt)
{
	if (this->elapsed_time >= this->step_time)
	{
		this->step();
		this->elapsed_time = sf::seconds(0.0f);
	}
	else
		this->elapsed_time += sf::seconds(dt);
}

void GameOfLife::update(float dt)
{
	this->checkKeys();

	switch (this->state)
	{
	case PLAYING:
		this->playing_update(dt);
		break;

	case PAUSED:
		break;

	default:
		break;
	}
}

void GameOfLife::render(sf::RenderWindow& window)
{
	for (uint16_t i = 0; i < this->width * this->height * 4; i += 4)
	{
		uint16_t qp = i / 4;

		uint16_t ypos = qp / this->height;
		uint16_t xpos = qp % this->width;

		if (this->current_world[ypos][xpos])
		{
			this->quads[i].color = sf::Color::Black;
			this->quads[i + 1].color = sf::Color::Black;
			this->quads[i + 2].color = sf::Color::Black;
			this->quads[i + 3].color = sf::Color::Black;
		}
		else
		{
			this->quads[i].color = sf::Color::White;
			this->quads[i + 1].color = sf::Color::White;
			this->quads[i + 2].color = sf::Color::White;
			this->quads[i + 3].color = sf::Color::White;
		}
	}

	window.setView(this->view);

	window.draw(this->quads);
	window.draw(this->lines);

	window.setView(this->default_view);

	this->pause_button.render(window);
	this->unpause_button.render(window);
	this->clear_button.render(window);
	this->random_button.render(window);
}