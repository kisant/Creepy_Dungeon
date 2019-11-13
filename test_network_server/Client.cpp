/*#include "Client.h"
#include <SFML/Network.hpp>
#include <string>
#include <iostream>
#include <list>
#include <queue>

Client::Client(sf::TcpSocket someSocket, int someId)
{
	id = someId;
	socketToServer = someSocket;
}

int Client::getId()
{
	return id;
}

int Client::getXCoordinate()
{
	return x;
}

int Client::getYCoordinate()
{
	return y;
}

sf::TcpSocket* Client::getSocket()
{
	return socketToServer;
}

Client::~Client()
{
}
*/

#include "Client.h"

Client::Client(std::unique_ptr<sf::TcpSocket>* socket, int id)
{
	m_id = id;
	m_socket = std::move(*socket);
	x = 0;
	y = 0;
}

//mine
void Client::setCoordinate(float X, float Y)
{
	x = X;
	y = Y;
}

//mine
float Client::getXCoord()
{
	return x;
}
float Client::getYCoord()
{
	return y;
}

sf::TcpSocket* Client::getSocket()
{
	return m_socket.get();
}

int Client::getId()
{
	return m_id;
}

void Client::update(float dt)
{

}