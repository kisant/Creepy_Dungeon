/*
#include <SFML/Network.hpp>
#include <string>
#include <iostream>
#include <list>
#include <thread>
#include "Client.h"
using namespace std;

enum SIGNAL_SEND
{
	CLIENT_ID,
	CLIENT_POSITION,
};

enum SIGNAL_RECEIVE
{
	CLIENT_NAME,
};

int main()
{
	int playersCount = 0;
	int playersId = 0;

	std::queue<sf::Packet> receivedPackets;

	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::TcpListener listener;
	sf::SocketSelector selector;
	listener.listen(2000);
	//sf::TcpSocket socketToClient;
	std::list<Client>* clients = nullptr;
	sf::Packet outPacket;

	selector.add(listener);

	while (true)
	{
		if (selector.wait())
		{
			if (selector.isReady(listener))
			{
				sf::TcpSocket tempSocket;

				if (listener.accept(tempSocket) == sf::Socket::Done)
				{
					clients->push_back(Client(tempSocket, playersId));
					selector.add(*clients->back().getSocket());
					++playersCount;

					outPacket << SIGNAL_SEND::CLIENT_ID;
					outPacket << playersId;

					if (clients->back().getSocket()->send(outPacket) != sf::Socket::Done)
						std::cout << "Error sending player index" << std::endl;

					++playersId;
				}
			}
			else
			{
				for (auto& client : *clients)
				{
					if (selector.isReady(*client.getSocket()))
					{
						sf::Packet received;
						if (client.getSocket()->receive(received) == sf::Socket::Done)
						{
							receivedPackets.push(received);
						}
					}
				}

				/*
				* Update
				
				if (receivedPackets.size() > 0)
				{
					sf::Packet received = receivedPackets.front();

					int id, signal;
					received >> signal;
					received >> id;

					Client* currClient = nullptr;
					for (auto& itr : *clients)
					{
						if (itr.getId() == id)
							currClient = &itr;
					}

					switch (signal)
					{
					case SIGNAL_RECEIVE::CLIENT_NAME:
					{
						std::cout << "New client connected - " << " ID: " << id << std::endl;
						break;
					}
					default:
						break;
					}

					receivedPackets.pop();
				}
			}
		}
	}
	
	/*sf::Uint16 x;
	std::string s;
	double d;

	return 0;
}
*/

#include <iostream>
#include <queue>
#include <memory>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>

#include "Client.h"

#include <SFML/Network.hpp>
#include <SFML/Graphics/Rect.hpp>

enum SIGNAL_SEND
{
	CLIENT_ID,
	PLAYER_LIST,
	ENEMY_POSITION
};

enum SIGNAL_RECEIVE
{
	PLAYER_LIST_REQUEST,
	CLIENT_POSITION
};
int main()
{
	bool m_running;

	sf::TcpListener m_listener;
	sf::SocketSelector m_selector;


	// Server settings
	unsigned short m_port;
	unsigned short m_maxPlayers;

	// Maybe add here "ticks" like in the counter strike.
	// Then the user should get the number of ticks and run it on the same speed on his simulation


	// Variables
	int m_playersConnected;
	int m_currentPlayerId;

	std::vector<Client> m_playerList;
	std::queue<sf::Packet> m_receivedPackets;

	char m_tmp[1400];

	// Server cpp
	m_running = true;

	if (m_listener.listen(2000) == sf::Socket::Done)
		std::cout << "Server is started on port: " << 2000 << ". Waiting for clients.\n";
	else
		std::cout << "Error - Failed to bind the port " << 2000 << std::endl;

	m_selector.add(m_listener);

	m_maxPlayers = 5;

	m_playersConnected = 0;
	m_currentPlayerId = 0;

	while (m_running)
	{
		if (m_selector.wait())
		{
			if (m_selector.isReady(m_listener))
			{
				// Creating the socket that will wait for new connections
				std::unique_ptr<sf::TcpSocket> tempSocket = std::make_unique<sf::TcpSocket>();

				// If a new client is connected this code executes
				if (m_listener.accept(*tempSocket) == sf::Socket::Done)
				{
					if (m_playersConnected < m_maxPlayers) //if server is not full
					{
						m_playerList.push_back(Client(&tempSocket, m_currentPlayerId));
						m_selector.add(*m_playerList.back().getSocket());
						++m_playersConnected;

						// TODO Create a function that send the id
						sf::Packet outPacket;
						outPacket << SIGNAL_SEND::CLIENT_ID;
						outPacket << m_currentPlayerId;

						//Send client id
						if (m_playerList.back().getSocket()->send(outPacket) != sf::Socket::Done)
							std::cout << "Error sending player index" << std::endl;

						++m_currentPlayerId;
					}
				}
			}
			else
			{
				for (auto& player : m_playerList)
				{
					if (m_selector.isReady(*player.getSocket()))
					{
 						sf::Packet received;
						if (player.getSocket()->receive(received) == sf::Socket::Done)
						{
							// If mutex is locked here it doesn't work for the first but it works for all other
							//std::lock_guard<std::mutex> guard2(m_mutex);
							// Add the element on the end of the queue
							m_receivedPackets.push(received);
						}
					}
				}
			}
			if (m_receivedPackets.size() > 0)
			{
				sf::Packet received = m_receivedPackets.front();

				int signal, id;
				received >> signal;
				received >> id;

				// Get the player from which we received data
				Client* currentPlayer = nullptr;
				for (auto& itr : m_playerList)
				{
					if (itr.getId() == id)
						currentPlayer = &itr;
				}

				switch (signal)
				{
				case SIGNAL_RECEIVE::CLIENT_POSITION:
				{
					float x;
					float y;
					received >> x >> y;
					currentPlayer->setCoordinate(x, y);
				}
				break;
				case SIGNAL_RECEIVE::PLAYER_LIST_REQUEST:  // Received request for player list
				{
					sf::Packet namePacket;
					namePacket << SIGNAL_SEND::PLAYER_LIST;
					namePacket << m_playersConnected;

					for (auto& itr : m_playerList)
					{
						namePacket << itr.getId();
						namePacket << itr.getXCoord();
						namePacket << itr.getYCoord();
					}

					for (auto& player : m_playerList)
					{
						if (player.getSocket()->send(namePacket) != sf::Socket::Done)
						{
							std::cout << "Error - Failed to send a name list packet" << std::endl;
						}
					}
					break;
				}
				default:
					break;
				}
				m_receivedPackets.pop();
			}

			// Sending the enemy position
			// Send to each player, but skip that player to which you are sending
			if (m_playerList.size() > 1)
			{
				for (auto& player : m_playerList)
				{
					sf::Packet p;
					p << SIGNAL_SEND::ENEMY_POSITION;
					p << int(m_playerList.size() - 1);

					for (auto& enemy : m_playerList)
					{
						int id = enemy.getId();

						if (id != player.getId())
						{
							p << id;
							p << enemy.getXCoord();
							p << enemy.getYCoord();
						}
					}
					player.getSocket()->send(p);
				}
			}
		}
	}
}