#include <SFML/Graphics.hpp>
using namespace sf;

sf::View view;

void getplayercoordinateforview(float x, float y) {
	float tempX = x;
	float tempY = y;

	/*
	if (x < 160)
		tempX = 160;
	if (y < 120)
		tempY = 120;
	if (x > 5000)
		tempX = 1000;
	*/

	view.setCenter(tempX, tempY);
}