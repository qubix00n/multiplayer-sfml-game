#include <iostream>
#include <unordered_map>

#include "common.h"

class Server : public qb::net::server_interface<GameMsg> {
public:

	std::unordered_map<uint32_t, PlayerDesc> mapPlayers;
	std::vector<uint32_t> garbageIDs;
	std::thread dupa;

	Server(uint16_t nPort) : qb::net::server_interface<GameMsg>(nPort) {
		dupa = std::thread([&]() {
			using namespace std::chrono_literals;
			while(true) {
				std::this_thread::sleep_for(1000ms);

				// qb::net::message<GameMsg> msg;
				// msg.header.id = GameMsg::Chat_NewMessage;
				// uint32_t d = 10;
				// msg << d << std::string("dupa");
				// std::cout << msg;
				// MessageAllClients(msg);

				// for(const auto& player : mapPlayers) {
				// 	std::cout << player.second << "\n";
				// 	for(const auto& client : m_deqConnections) {
				// 		if(player.second.UUID == player.second.UUID) continue;

				// 		qb::net::message<GameMsg> msg;
				// 		msg.header.id = GameMsg::Game_UpdatePlayer;
				// 		msg << player.second;

				// 		client->Send(msg);
				// 	}
				// }
			}
		});
	}

protected:

	bool OnClientConnect(std::shared_ptr<qb::net::connection<GameMsg>> client) override {
		return true;
	}

	void OnClientValidated(std::shared_ptr<qb::net::connection<GameMsg>> client) override {
		qb::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Client_Accepted;
		client->Send(msg);
	}

	void OnClientDisconnect(std::shared_ptr<qb::net::connection<GameMsg>> client) override {
		std::cout << "Removing client [" << client->GetID() << "]\n";

		if(client) {
			if(mapPlayers.find(client->GetID()) == mapPlayers.end()) {
				// nevermind
			} else {
				auto& pd = mapPlayers[client->GetID()];
				std::cout << "[UNGRACEFUL REMOVAL]:" << std::to_string(pd.UUID) << "\n";
				mapPlayers.erase(client->GetID());
				garbageIDs.push_back(client->GetID());
			}
		}
	}

	void OnMessage(std::shared_ptr<qb::net::connection<GameMsg>> client, qb::net::message<GameMsg>& msg) override {
		if(!garbageIDs.empty()) {
			for(auto& pid : garbageIDs) {
				qb::net::message<GameMsg> m;
				m.header.id = GameMsg::Game_RemovePlayer;
				m << pid;
				std::cout << "Removing " << pid << "\n";
				MessageAllClients(m);
			}
			garbageIDs.clear();
		}

		switch (msg.header.id) {
			case GameMsg::Client_RegisterWithServer: {
				PlayerDesc desc;
				msg >> desc;
				desc.UUID = client->GetID();
				mapPlayers.insert_or_assign(desc.UUID, desc);

				qb::net::message<GameMsg> msgToSendID;
				msgToSendID.header.id = GameMsg::Client_AssignID;
				msgToSendID << desc.UUID;
				MessageClient(client, msgToSendID);

				qb::net::message<GameMsg> msgToAddPlayer;
				msgToAddPlayer.header.id = GameMsg::Game_AddPlayer;
				msgToAddPlayer << desc;
				MessageAllClients(msgToAddPlayer);

				for(const auto& player : mapPlayers) {
					qb::net::message<GameMsg> msgToAddOtherPlayers;
					msgToAddOtherPlayers.header.id = GameMsg::Game_AddPlayer;
					msgToAddOtherPlayers << player.second;
					MessageClient(client, msgToAddOtherPlayers);
				}
			} break;
			case GameMsg::Client_UnregisterWithServer: {
				
			} break;
			case GameMsg::Game_UpdatePlayer: {
				MessageAllClients(msg, client);
			} break;
		}
	}
};

int main() {
	Server server(69420); 
	server.Start();

	while(true) {
		server.Update(-1, true);
	}

	return 0;
}