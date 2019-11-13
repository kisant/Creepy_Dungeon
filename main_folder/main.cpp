#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <sstream>
#include "mission.h"
#include "map.h"
#include "view.h"
#include "level.h"
#include <vector>
#include <list>
#include "tinyxml/tinyxml.h"

using namespace sf;
using namespace std;

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

class Entity {
public:
	std::vector <Object> obj;
	float dx, dy, x, y, speed, moveTimer, CurrentFrame;
	int w, h, health;
	bool life;
	Texture texture;
	Sprite sprite;
	String name;
	Entity(Image &image, float X, float Y, int W, int H, String Name) {
		x = X; y = Y; w = W; h = H; name = Name; moveTimer = 0;
		speed = 0; health = 1; dx = 0; dy = 0;
		life = true;

		texture.loadFromImage(image);
		sprite.setTexture(texture);
		sprite.setOrigin(w / 2, h / 2);	
	}

	FloatRect getRect(){
		return FloatRect(x, y, w, h);
	}

	virtual void update(float time) = 0;
};
/*-----------------------------------------Player Class-------------------------------------------------------------*/
class Player :public Entity {
public:
	enum { left, right, up, down, stay } state;
	int PlayerScore;
	int id;//new varuable

	Player(Image &image, Level &lev, float X, float Y, int W, int H, String Name, int ID):Entity(image,X,Y,W,H,Name) {
		PlayerScore = 0; state = stay; obj = lev.GetAllObjects(); health = 5;

		if (name == "Knight")
			sprite.setTextureRect(IntRect(0, 0, w, h));
		id = ID;
	}

	void setId(int ID)
	{
		id = ID;
	}

	void setCoordinateus(float X, float Y)
	{
		x = X;
		y = Y;
	}

	void update(float time) {
		control(time);
		switch (state)
		{
		case right: dx = speed; dy = 0; break;
		case left: dx = -speed; dy = 0; break;
		case down: dx = 0; dy = speed; break;
		case up: dx = 0; dy = -speed; break;
		case stay: dx = 0; dy = 0; break;
		}

		x += dx * time;
		y += dy * time;

		speed = 0;
		sprite.setPosition(x+w/2, y+h/2	);

		interactionWithMap(dx, dy);

		if (health <= 0)
			life = false;

		if (life)
			getplayercoordinateforview(x, y);
		else
			speed = 0;
	}	

	float getplayercoordinateX() {
		return x;
	}

	float getplayercoordinateY() {
		return y;
	}

	void control(float time) {
		if (Keyboard::isKeyPressed(Keyboard::A))
		{
			state = left;
			speed = 0.06;
			CurrentFrame += 0.01*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(20 * int(CurrentFrame) + 20, 0, -20, 30));
			getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
			
		}
		if (Keyboard::isKeyPressed(Keyboard::Space))
		{
			/*
			if (state == left) {
				//state = stay;
				speed = 0;
				for (CurrentFrame = 0; CurrentFrame < 2; CurrentFrame += 0.005*time) {//FIX
					sprite.setTextureRect(IntRect(35 * int(CurrentFrame), 90, 35, 30));
					getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
				}
				CurrentFrame -= 2;
			}
			else 
			*/
				state = stay;
				speed = 0;
				for (CurrentFrame = 0; CurrentFrame < 2; CurrentFrame += 0.005*time) {
					sprite.setTextureRect(IntRect(35 * int(CurrentFrame), 60, 35, 30));
					getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
				}
				CurrentFrame -= 2;
		}
		if (Keyboard::isKeyPressed(Keyboard::D))
		{
			state = right;
			speed = 0.06;
			CurrentFrame += 0.01*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(20 * int(CurrentFrame), 0, 20, 30));
			getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
		}
		if (Keyboard::isKeyPressed(Keyboard::W))
		{
			state = up;
			speed = 0.06;
			CurrentFrame += 0.01*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(20 * int(CurrentFrame), 0, 20, 30));
			getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
		}
		if (Keyboard::isKeyPressed(Keyboard::S)) 
		{
			state = down;
			speed = 0.06;
			CurrentFrame += 0.01*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(20 * int(CurrentFrame), 0, 20, 30));
			getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
		}
		if ((!Keyboard::isKeyPressed(Keyboard::A)) && (!Keyboard::isKeyPressed(Keyboard::D))&&
			(!Keyboard::isKeyPressed(Keyboard::W)) && (!Keyboard::isKeyPressed(Keyboard::S))&&
			(!Keyboard::isKeyPressed(Keyboard::Space)))
		{
			if (state == left) {
				//state = stay;
				speed = 0;
				CurrentFrame += 0.005*time;
				if (CurrentFrame > 4)
					CurrentFrame -= 4;
				sprite.setTextureRect(IntRect(60 - 20 * int(CurrentFrame), 120, 20, 30));
				getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
			}
			else {
				state = stay;
				speed = 0;
				CurrentFrame += 0.005*time;
				if (CurrentFrame > 4)
					CurrentFrame -= 4;
				sprite.setTextureRect(IntRect(20 * int(CurrentFrame), 30, 20, 30));
				getplayercoordinateforview(getplayercoordinateX(), getplayercoordinateY());
			}
		}
	}

	void interactionWithMap(float Dx, float Dy) {
		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect)) {
				if (obj[i].name == "Solid") {
					if (Dy > 0) 
						y = obj[i].rect.top - h;
					if (Dy < 0) 
						y = obj[i].rect.top + obj[i].rect.height;	
					if (Dx > 0) 
						x = obj[i].rect.left - w;
					if (Dx < 0) 
					x = obj[i].rect.left + obj[i].rect.width;
				}
					
			}
	}
};
/*-----------------------------------------End of Player Class-----------------------------------------------------*/

/*--------------------------------------------Enemy Class----------------------------------------------------------*/
class Enemy : public Entity {
	int zq;
public:
	enum { left, right, up, down, stay } state;
	
	Enemy(Image &image, Level &lev, float X, float Y, int W, int H, String Name) : Entity(image, X, Y, W, H, Name) {
		obj = lev.GetAllObjects(); zq = 1; 

		if (name == "Necro") {
			health = 15;
			sprite.setTextureRect(IntRect(0, 0, w, h));
			dx = 0;
			dy = 0;
		}
		if (name == "Poo") {
			health = 15;
			sprite.setTextureRect(IntRect(0, 20, w, h));
			dx = 0;
			dy = 0;
		}
		if (name == "Chert") {
			health = 15;
			sprite.setTextureRect(IntRect(0, 40, w, h));
			dx = 0;
			dy = 0;
		}
		if (name == "Heal") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
			dx = 0;
			dy = 0;
		}
		if (name == "Demon") {
			sprite.setTextureRect(IntRect(0, 60, w, h));
			health = 200;
			dx = 0;
			dy = 0;
		}
	}

	void interactionWithMap(float Dx, float Dy) {
		for (int i = 0; i < obj.size(); i++)
			if (getRect().intersects(obj[i].rect)) {
				if (obj[i].name == "Solid") {
					if (Dy > 0) {
						y = obj[i].rect.top - h;
						dy = 0;
					}
					if (Dy < 0) {
						y = obj[i].rect.top + obj[i].rect.height;
						dy = 0;
					}
					if (Dx > 0) {
						x = obj[i].rect.left - w;
						dx = 0;
					}
					if (Dx < 0) {
						x = obj[i].rect.left + obj[i].rect.width;
						dx = 0;
					}
				}
			}
	}
	void update(float time)
	{
		if (name != "Heal")
			control(dx, dy, time, name);
		//if (name == "Necro") {
			
			//moveTimer += time;if (moveTimer>3000){ dx *= -1; moveTimer = 0; }//меняет направление примерно каждые 3 сек
			interactionWithMap(dx,dy);

			x += dx * time;
			y += dy * time;

			sprite.setPosition(x+w/2, y+h/2); 

			if (health <= 0) 
				life = false; 
		//}
	}
	void control(float Dx, float Dy, float time, String name) {
		int ycontrol;
		int xcontrol;
		int wcontrol;
		int hcontrol;

		if (name == "Necro") {
			xcontrol = 16;
			ycontrol = 0;
			wcontrol = 16;
			hcontrol = 20;
		}
		if (name == "Poo") {
			ycontrol = 20;
			xcontrol = 16;
			wcontrol = 16;
			hcontrol = 20;
		}
		if (name == "Chert") {
			ycontrol = 40;
			xcontrol = 16;
			wcontrol = 16;
			hcontrol = 20;
		}
		if (name == "Demon") {
			xcontrol = 32;
			ycontrol = 60;
			wcontrol = 32;
			hcontrol = 36;
		}

		if (dx < 0)
		{
			CurrentFrame += 0.005*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(xcontrol * int(CurrentFrame) + xcontrol, ycontrol, -wcontrol, hcontrol));
		}
		if (dx > 0)
		{
			CurrentFrame += 0.005*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(xcontrol * int(CurrentFrame), ycontrol, wcontrol, hcontrol));
		}
		if (dy > 0)
		{
			CurrentFrame += 0.005*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(xcontrol * int(CurrentFrame) + xcontrol, ycontrol, -wcontrol, hcontrol));
		}
		if (dy < 0)
		{
			CurrentFrame += 0.005*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(xcontrol * int(CurrentFrame) + xcontrol, ycontrol, -wcontrol, hcontrol));
		}
		if ((dx == 0) && (dy == 0))
		{
			CurrentFrame += 0.005*time;
			if (CurrentFrame > 4)
				CurrentFrame -= 4;
			sprite.setTextureRect(IntRect(xcontrol * int(CurrentFrame) + xcontrol, ycontrol, -wcontrol, hcontrol));
		}

	}
};
/*----------------------------------------End of Enemy Class-------------------------------------------------------*/

int main() {
	/*
	* Network-part
	*/
	std::list<Player*> m_enemies;

	sf::TcpSocket socketToServer;
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();

	sf::Socket::Status status = socketToServer.connect(ip, 2000);

	sf::Packet received_packet;

	int signal;
	/*
	* Main-part
	*/
	RenderWindow window(sf::VideoMode(1080, 900), "DUNGEON");
	view.reset(sf::FloatRect(0, 0, 270, 225));

	Level lvl;
	lvl.LoadFromFile("D://visual//Kursachik//kursovik//kursovik//Map//TiledMap.tmx");

	SoundBuffer hitbuffer;
	hitbuffer.loadFromFile("D://visual//Kursachik//kursovik//kursovik//Hit.ogg");
	Sound hitsound(hitbuffer);

	Music music;
	music.openFromFile("Danger.ogg");		
	music.play();
	music.setLoop(true);

	Image knightimage;
	knightimage.loadFromFile("images/Knight/knight_anim_full2.png");
	knightimage.createMaskFromColor(Color(255, 255, 255));

	Image enemyimage;
	enemyimage.loadFromFile("images/Enemy/Enemies.png");

	Image healimage;
	healimage.loadFromFile("images/Map/flask_big_red.png");

	std::list<Entity*> entities;
	std::list<Player*> players;
	std::list<Entity*>::iterator it;
	std::list<Entity*>::iterator it2;

	Object player = lvl.GetObject("Player");
	std::vector<Object> enemyn = lvl.GetObjects("Necro");
	std::vector<Object> enemyp = lvl.GetObjects("Poo");
	std::vector<Object> enemyc = lvl.GetObjects("Chert");
	std::vector<Object> enemyd = lvl.GetObjects("Demon");

	std::vector<Object> heal = lvl.GetObjects("Heal");

	for (int i = 0; i < enemyn.size(); i++) 
		entities.push_back(new Enemy(enemyimage, lvl, enemyn[i].rect.left, enemyn[i].rect.top, 16.0, 20.0, "Necro"));
	for (int i = 0; i < enemyp.size(); i++)
		entities.push_back(new Enemy(enemyimage, lvl, enemyp[i].rect.left, enemyp[i].rect.top, 16.0, 20.0, "Poo"));
	for (int i = 0; i < enemyc.size(); i++)
		entities.push_back(new Enemy(enemyimage, lvl, enemyc[i].rect.left, enemyc[i].rect.top, 16.0, 20.0, "Chert"));
	for (int i = 0; i < enemyd.size(); i++)
		entities.push_back(new Enemy(enemyimage, lvl, enemyd[i].rect.left, enemyd[i].rect.top, 32.0, 36.0, "Demon"));
	for (int i = 0; i < heal.size(); i++)
		entities.push_back(new Enemy(healimage, lvl, heal[i].rect.left, heal[i].rect.top, 16.0, 16.0, "Heal"));

	Player knight(knightimage, lvl, player.rect.left, player.rect.top, 20.0, 30.0, "Knight", 0);

	if (socketToServer.receive(received_packet) == sf::Socket::Done)
	{
		received_packet >> signal;

		switch (signal)
		{
		case SIGNAL_RECEIVE::CLIENT_ID:
		{
			int id;
			received_packet >> id;
			knight.setId(id);

		}
		default:
			break;
		}
	}

	Font font;
	font.loadFromFile("Fonts/12510.ttf");
	Text text("", font, 10);
	//text.setColor(Color::Black);
	
	Clock clock;
	float CurrentFrame = 0;
	int timer = 0;

	while (window.isOpen())
	{
		float time = clock.getElapsedTime().asMicroseconds();
		clock.restart();
		time = time / 800;

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
					
			std::ostringstream PlayerHealthString;
			PlayerHealthString << knight.health;
	
			text.setString("Health: " + PlayerHealthString.str());	
		}

		if (knight.health > 0) {
			knight.update(time);
			for (auto& enm : m_enemies)
				enm->update(time);

			/*
			* Network-part-again
			*/
			sf::Packet packet;
			packet << SIGNAL_SEND::CLIENT_POSITION;
			packet << knight.id;
			packet << knight.x << knight.y;

			if (socketToServer.send(packet) != sf::Socket::Done)
				std::cout << "Error sending position" << std::endl;

			sf::Packet temp;
			temp << SIGNAL_SEND::PLAYER_LIST_REQUEST;
			temp << knight.id;

			if (socketToServer.send(temp) != sf::Socket::Done)
				std::cout << "Error sending getPlayerList to server" << std::endl;

			if (socketToServer.receive(received_packet) == sf::Socket::Done)
			{
				received_packet >> signal;

				switch (signal)
				{
				case SIGNAL_RECEIVE::PLAYER_LIST: // Received a list of players
				{
					int enemy_count;
					received_packet >> enemy_count;

					for (int i = 0; i < enemy_count; ++i)
					{
						int enemy_id;
						float x;
						float y;
						received_packet >> enemy_id;
						received_packet >> x;
						received_packet >> y;

						// Check if we already have that enemy

						bool createEnemy = true;
						for (auto& enemy : m_enemies)
						{
							if (*&enemy->id == enemy_id)
								createEnemy = false;
						}
						if (enemy_id != knight.id && createEnemy)
						{
							std::cout << "New user connected with id " << enemy_id << std::endl;
							m_enemies.push_back(new Player(knightimage, lvl, player.rect.left, player.rect.top, 20.0, 30.0, "Knight", enemy_id));
						}
					}
					std::cout << "Number of players connected: " << m_enemies.size() << std::endl;
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
						float x;
						float y;
						received_packet >> e_id;
						received_packet >> x;
						received_packet >> y;

						for (auto& enm : m_enemies)
							if (*&enm->id == e_id)
							{
								enm->setCoordinateus(x, y);
							}
					}
				}
				break;
				default:
					break;
				}
			}
			/*
			* End of this shit
			*/
			
			if (Keyboard::isKeyPressed(Keyboard::Space))
				hitsound.play();

			for (it = entities.begin(); it != entities.end();)
			{
				Entity *b = *it;
				b->update(time);
				if (b->life == false) {
					it = entities.erase(it);
					delete(b);
				}
				else
					it++;
			}

			for (it = entities.begin(); it != entities.end(); it++) {
				if ((*it)->getRect().intersects(knight.getRect())) {
					if ((*it)->name == "Necro" || (*it)->name == "Poo" || (*it)->name == "Chert" || (*it)->name == "Demon") {
						if ((*it)->dx > 0) {
							(*it)->x = knight.x - (*it)->w;
							(*it)->dx = 0;
						}
						if ((*it)->dx < 0) {
							(*it)->x = knight.x + knight.w;
							(*it)->dx = 0;
						}
						if (knight.dx < 0) {
							knight.x = (*it)->x + (*it)->w;
							knight.dx = 0;
						}
						if (knight.dx > 0) {
							knight.x = (*it)->x - knight.w;
							knight.dx = 0;
						}
					}
					if ((*it)->name == "Heal") {
						(*it)->life = false;
						knight.health = knight.health + 2;
					}
				}


				if (((*it)->y - knight.y) < 20 && ((*it)->y - knight.y) > -5 &&
					((*it)->x - knight.x) > 0 && ((*it)->x - knight.x) < 33) {
					if (((*it)->y - knight.y) < 20 && ((*it)->y - knight.y) > -5 &&
						((*it)->x - knight.x) < 0 && ((*it)->x - knight.x) > -33) {
						if ((*it)->name == "Necro" || (*it)->name == "Poo" || (*it)->name == "Chert" || (*it)->name == "Demon") {
							timer += time;
							if (timer > 2000) {
								knight.health--;
								timer = 0;
							}
						}
					}
					else {
						if ((*it)->name == "Necro" || (*it)->name == "Poo" || (*it)->name == "Chert" || (*it)->name == "Demon") {
							timer += time;
							if (timer > 2000) {
								knight.health--;
								timer = 0;
							}
						}
					}
					if (Keyboard::isKeyPressed(Keyboard::Space))
						(*it)->health--;
				}
				text.setPosition(view.getCenter().x - 155, view.getCenter().y + 105);



				//follow enemies
				Entity *e = *it;
				//right down
				if (e->name != "Heal")
					if ((e->y - knight.y < 50 && e->y - knight.y > 7) &&
						(e->x - knight.x < 75 && e->x - knight.x > 0)) {

						if (e->x - knight.x > 23) {
							e->dx = -0.02;
							e->dy = 0;
						}
						else
							if (e->y - knight.y > 7) {
								e->dy = -0.02;
								e->dx = 0;
							}
					}
					else {
						// right up
						if ((e->y - knight.y > -50 && e->y - knight.y < 7) &&
							(e->x - knight.x < 75 && e->x - knight.x > 0)) {

							if (e->x - knight.x > 23) {
								e->dx = -0.02;
								e->dy = 0;
							}
							else
								if (e->y - knight.y < 7) {
									e->dy = 0.02;
									e->dx = 0;
								}
						}
						else {
							// left up
							if ((e->y - knight.y > -50 && e->y - knight.y < 7) &&
								(e->x - knight.x > -75 && e->x - knight.x < 0)) {

								if (e->x - knight.x < -23) {
									e->dx = 0.02;
									e->dy = 0;
								}
								else
									if (e->y - knight.y < 7) {
										e->dy = 0.02;
										e->dx = 0;
									}
							}
							else {
								//left down
								if ((e->y - knight.y < 50 && e->y - knight.y > 7) &&
									(e->x - knight.x > -75 && e->x - knight.x < 0)) {

									if (e->x - knight.x < -23) {
										e->dx = 0.02;
										e->dy = 0;
									}
									else
										if (e->y - knight.y > 7) {
											e->dy = -0.02;
											e->dx = 0;
										}
								}
								else {
									e->dy = 0;
									e->dx = 0;
								}
							}
						}
					}
				for (it2 = entities.begin(); it2 != entities.end(); it2++) {
					if ((*it)->getRect() != (*it2)->getRect())
						if (((*it)->getRect().intersects((*it2)->getRect())) && (((*it)->name == "Necro") && ((*it2)->name == "Poo") ||
							((*it)->name == "Necro") && ((*it2)->name == "Necro") || ((*it)->name == "Necro") && ((*it2)->name == "Chert") ||
							((*it)->name == "Chert") && ((*it2)->name == "Poo") || ((*it)->name == "Chert") && ((*it2)->name == "Chert") ||
							((*it)->name == "Poo") && ((*it2)->name == "Poo"))) {

							(*it)->dx = 0;
							(*it)->dy = 0;
						}
				}
			}

			window.setView(view);
			window.clear();

			lvl.Draw(window);

			for (auto& enm : m_enemies)
				window.draw(enm->sprite);

			window.draw(knight.sprite);

			for (it = entities.begin(); it != entities.end(); it++)
				window.draw((*it)->sprite);

			text.setPosition(view.getCenter().x - 135, view.getCenter().y + 100);
			window.draw(text);

			window.display();
		}
		
	}

	return 0;
}
