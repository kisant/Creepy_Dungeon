#include <SFML/Network.hpp>
#include <string>
#include <iostream>

using namespace std;

class Player
{
public:
	int x;
	int y;
	int id;
	
	Player(int ID, int X, int Y)
	{
		x = X;
		y = Y;
		id = ID;
	}

	int getId()
	{
		return id;
	}

	int getXCoord()
	{
		return x;
	}

	int getYCoord()
	{
		return y;
	}

	void setCoord(int X, int Y)
	{
		x = X;
		y = Y;
	}

	void setId(int ID)
	{
		id = ID;
	}
};

enum SIGNAL_SEND
{
	PLAYER_LIST_REQUEST,
	CLIENT_POSITION
};

enum SIGNAL_RECEIVE
{
	CLIENT_ID,
	PLAYER_LIST,
	ENEMY_POSITION
};

int main()
{
	std::vector<Player> m_enemies;
	Player m_player(0,0,0);

	sf::TcpSocket socketToServer;
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();

	sf::Socket::Status status = socketToServer.connect(ip, 2000);

	int x;
	int y;
	int signal;
	cin >> x >> y;
	m_player.setCoord(x, y);

	sf::Packet received_packet;

	if (socketToServer.receive(received_packet) == sf::Socket::Done)
	{
		received_packet >> signal;

		switch (signal)
		{
		case SIGNAL_RECEIVE::CLIENT_ID:
		{
			int id;
			received_packet >> id;
			m_player.setId(id);

		}
		default:
			break;
		}
	}

	while (true)
	{
		sf::Packet packet;
		packet << SIGNAL_SEND::CLIENT_POSITION;
		packet << m_player.getId();
		packet << m_player.x << m_player.y;
		socketToServer.send(packet);

		if (socketToServer.send(packet) != sf::Socket::Done)
			std::cout << "Error sending getPlayerList to server" << std::endl;

		sf::Packet temp;
		temp << SIGNAL_SEND::PLAYER_LIST_REQUEST;
		temp << m_player.getId();

		if (socketToServer.send(temp) != sf::Socket::Done)
			std::cout << "Error sending getPlayerList to server" << std::endl;

		if (socketToServer.receive(received_packet) == sf::Socket::Done)
		{
			received_packet >> signal;

			switch (signal)
			{
			case SIGNAL_RECEIVE::CLIENT_ID:
			{
				int id;
				received_packet >> id;
				m_player.setId(id);

			}
			break;
			case SIGNAL_RECEIVE::PLAYER_LIST: // Received a list of players
			{
				int enemy_count;
				received_packet >> enemy_count;

				for (int i = 0; i < enemy_count; ++i)
				{
					int enemy_id;
					int x;
					int y;
					received_packet >> enemy_id;
					received_packet >> x;
					received_packet >> y;

					// Check if we already have that enemy

					bool createEnemy = true;
					for (auto& enemy : m_enemies)
					{
						if (enemy.getId() == enemy_id)
							createEnemy = false;
					}
					if (enemy_id != m_player.getId() && createEnemy)
					{
						std::cout << "New user connected with id " << enemy_id << std::endl;
						m_enemies.push_back(Player(enemy_id, x, y));
					}
				}
				std::cout << "Number of players connected: " << m_enemies.size() << std::endl;
				for (auto& enemy : m_enemies)
				{
				std:cout << "Friend id: "<< enemy.id << " Friend x: "<<enemy.x << " Friend y: "<< enemy.y << std::endl;
				}

			}
			break;
			case SIGNAL_RECEIVE::ENEMY_POSITION: // Received other player position
			{
				int num;
				received_packet >> num;

				// Loop through received positions and adjust the enemy position
				for (int i = 0; i < num; ++i)
				{
					int e_id;
					int x;
					int y;
					received_packet >> e_id;
					received_packet >> x;
					received_packet >> y;

					for (auto& enm : m_enemies)
						if (enm.getId() == e_id)
						{
							enm.setCoord(x, y);
						}
				}
			}
			break;
			default:
				break;
			}
		}
	}



	return 0;
}