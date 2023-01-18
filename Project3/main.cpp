#include "GameOfLife.h"

const int WIDTH = 1080, HEIGHT = 840;

int main()
{
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Game of Life", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	sf::Clock clock;

	GameOfLife game(70, 70, WIDTH, HEIGHT, 15.0f, sf::seconds(0.05f));

	while (window.isOpen())
	{
		float dt = clock.restart().asSeconds();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			game.checkEvents(event, window);
		}

		game.update(dt);

		window.clear(sf::Color::White);

		game.render(window);

		window.display();
	}

	return 0;
}