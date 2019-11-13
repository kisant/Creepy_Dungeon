/*#pragma once
#include <SFML/Network.hpp>
#include <string>
#include <iostream>
#include <list>
#include <queue>
class Client
{
public:
	Client(sf::TcpSocket someSocket, int id);

	int getId();
	int getXCoordinate();
	int getYCoordinate();

	sf::TcpSocket* getSocket();

	~Client();
private:
	int id;
	int x;
	int y;
	sf::TcpSocket socketToServer;
};*/

#pragma once

#include <memory>
#include <string>
#include <queue>

#include <SFML/System/Vector2.hpp>
#include <SFML/Network/TcpSocket.hpp>

class Client
{
public:
	Client(std::unique_ptr<sf::TcpSocket>* socket, int id);

	void setCoordinate(float x, float y);//mine

	sf::TcpSocket* getSocket();

	int getId();

	float getXCoord();//mine
	float getYCoord();//mine

	void update(float dt);
private:
	
	std::unique_ptr<sf::TcpSocket> m_socket = nullptr;
	bool m_connected;
	int m_id;

	float x;//mine
	float y;//mine
};