#include "common.h"

#include <unordered_map>

using namespace std::chrono_literals;

sf::RenderWindow window(sf::VideoMode(1000, 1000), "cool GEJm");

class Game : public qb::net::client_interface<GameMsg> {
protected:

	const std::string host;
	const int16_t port;

	std::thread m_thread;

protected:

	float deltaTime = 15;
	int fps = 0;

public:

	Game(const std::string _host, const int16_t _port) : host(_host), port(_port) {
		if(!start()) exit("Couldn't start the program\n");

		unsigned long lastTime = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

		m_thread = std::thread([&]() {
			int i = 0;
			while(window.isOpen()) {
				sf::Event event;
				while(window.pollEvent(event)) {
					if(event.type == sf::Event::Closed)
						exit("Exiting the program\n");
					if(!handleEvent(event))
						exit("Couldn't handle event during the program\n");
				}

				if(!update())
					exit("Couldn't update the program\n");

				unsigned long currentTime = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
				deltaTime = (currentTime - lastTime);
				lastTime = currentTime;

				fps = int(1000.f / deltaTime);

				std::this_thread::sleep_for(15ms);

				if(i++ % 60 == 0) window.setTitle("kremconcy sie boryc - fps: " + std::to_string(fps));
			}
		});

		m_thread.join();
	}

protected:

	std::unordered_map<uint32_t, PlayerDesc> mapPlayers;
	uint32_t playerID = 0;

	bool focused = true;
	bool waitingForConnection = true;

	void exit(const std::string& msg = "") {
		std::cerr << msg;
		window.close();
	}

protected:

	sf::Sprite playerSprite;
	std::array<sf::Texture, 16> arrTextures;

	bool start() {
		window.setKeyRepeatEnabled(false);
		{
			sf::Image icon;
			icon.loadFromFile("icon.png");
			uint8_t pixels[icon.getSize().x * icon.getSize().y * 4];
			int i = 0;
			for(int y = 0; y < icon.getSize().y; y++) {
				for(int x = 0; x < icon.getSize().x; x++) {
					pixels[i++] = icon.getPixel(x, y).r;
					pixels[i++] = icon.getPixel(x, y).g;
					pixels[i++] = icon.getPixel(x, y).b;
					pixels[i++] = icon.getPixel(x, y).a;
				}
			}
			window.setIcon(icon.getSize().x, icon.getSize().y, pixels);
		}
		{
			int i = 0;
			arrTextures[i++].loadFromFile("avatars/boryc.png");
			arrTextures[i++].loadFromFile("avatars/bozenka.png");
			arrTextures[i++].loadFromFile("avatars/charytanowicz.png");
			arrTextures[i++].loadFromFile("avatars/jackowska.png");
			arrTextures[i++].loadFromFile("avatars/jajko.png");
			arrTextures[i++].loadFromFile("avatars/kalbarczyk.png");
			arrTextures[i++].loadFromFile("avatars/kiwal.png");
			arrTextures[i++].loadFromFile("avatars/krupa.png");
			arrTextures[i++].loadFromFile("avatars/kusznerczyk.png");
			arrTextures[i++].loadFromFile("avatars/marek.png");
			arrTextures[i++].loadFromFile("avatars/mozgawa.png");
			arrTextures[i++].loadFromFile("avatars/orlowski.png");
			arrTextures[i++].loadFromFile("avatars/potapczuk.png");
			arrTextures[i++].loadFromFile("avatars/rogal.png");
			arrTextures[i++].loadFromFile("avatars/sadurski.png");
			arrTextures[i++].loadFromFile("avatars/white.png");

			auto size = arrTextures[0].getSize();
			playerSprite.setOrigin(size.x / 2, size.y / 2);
		}

		if(Connect(host, port)) {
			std::cerr << "Conencted to " << host << ":" << port << "\n";
			return true;
		}

		return false;
	}

	PlayerDesc previousDesc;

	bool update() {
		if(IsConnected()) {
			while(!Incoming().empty()) {
				auto msg = Incoming().pop_front().msg;

				switch(msg.header.id) {
					case GameMsg::Client_Accepted: {
						std::cout << "Server accepted you in!" << "\n";
						qb::net::message<GameMsg> msg;
						msg.header.id = GameMsg::Client_RegisterWithServer;

						PlayerDesc descPlayer;
						descPlayer.position = { float(rand() % window.getSize().x), float(rand() % window.getSize().y) };
						descPlayer.skinID = rand() % arrTextures.size();
						msg << descPlayer;
						Send(msg);
					} break;
					case GameMsg::Client_AssignID: {
						msg >> playerID;
						std::cout << "Assigned Client ID: " << playerID << "\n";
					} break;
					case GameMsg::Game_AddPlayer: {
						PlayerDesc desc;
						msg >> desc;
						mapPlayers.insert_or_assign(desc.UUID, desc);

						if(desc.UUID == playerID) {
							waitingForConnection = false;
						}
					} break;
					case GameMsg::Game_RemovePlayer: {
						uint32_t removalID = 0;
						msg >> removalID;
						mapPlayers.erase(removalID);
					} break;
					case GameMsg::Game_UpdatePlayer: {
						PlayerDesc desc;
						msg >> desc;
						mapPlayers.insert_or_assign(desc.UUID, desc);
					} break;
					case GameMsg::Chat_NewMessage: {
						std::string m;
						uint32_t d = 0;
						msg >> m >> d;
						std::cout << "[" << d << "] : " << m << "\n";
					} break;
				}
			}
		}

		if(waitingForConnection) {
			return true;
		}

		if(focused) {

			mapPlayers[playerID].velocity = {
				sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A),
				sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W)
			};
			mapPlayers[playerID].velocity *= mapPlayers[playerID].speed;

			mapPlayers[playerID].angularVelocity = sf::Keyboard::isKeyPressed(sf::Keyboard::V) - sf::Keyboard::isKeyPressed(sf::Keyboard::C);
			mapPlayers[playerID].angularVelocity *= mapPlayers[playerID].angularSpeed;

		}

		for(auto& player : mapPlayers) {
			sf::Vector2f potentialPos = player.second.position + player.second.velocity * deltaTime;
			float potentialRot = player.second.rotation + player.second.angularVelocity * deltaTime;

			// HANDLE WALLS, COLLISIONS, ETC...

			player.second.position = potentialPos;
			player.second.rotation = potentialRot;
		}

		// draw

		window.clear();

		for(auto& player : mapPlayers) {
			playerSprite.setTexture(arrTextures[player.second.skinID]);

			playerSprite.setPosition(player.second.position);
			playerSprite.setRotation(player.second.rotation);
			window.draw(playerSprite);
		}

		window.display();

	  	bool shouldSend = previousDesc != mapPlayers[playerID];

		if(shouldSend) {
			qb::net::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_UpdatePlayer;
			msg << mapPlayers[playerID];
			Send(msg);
		}

		previousDesc = mapPlayers[playerID];

		return true;
	}

	bool handleEvent(sf::Event& event) {
		switch(event.type) {
			case sf::Event::GainedFocus: {
				focused = true;
			} break;
			case sf::Event::LostFocus: {
				focused = false;
			} break;
			case sf::Event::KeyPressed: {
				switch(event.key.code) {
					case sf::Keyboard::Up: {
						mapPlayers[playerID].skinID++;
						mapPlayers[playerID].skinID %= arrTextures.size();
					} break;
					case sf::Keyboard::Down: {
						mapPlayers[playerID].skinID--;
						mapPlayers[playerID].skinID %= arrTextures.size();
					} break;
				}
			} break;
		}
		return true;
	}
	
};

int main(int argc, char const *argv[]) {
	if(argc == 3) {
		srand(time(0));

		std::string host(argv[1]);
		int16_t port = atoi(argv[2]);

		std::cout << host << ":" << port << "\n";

		Game game = Game(host, port);
	} else {
		std::cerr << "usage: '" << argv[0] << " HOST_NAME PORT_NUMBER'" << "\n";
	}
	return 0;
}