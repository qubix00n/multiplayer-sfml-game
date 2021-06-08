#pragma once

#include <cstdint>
#include <array>

#include "one_file_qb_net.h"
#include <SFML/Graphics.hpp>

enum class GameMsg : uint32_t {
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignID,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_AddPlayer,
	Game_RemovePlayer,
	Game_UpdatePlayer,

	Chat_NewMessage,
};

struct PlayerDesc {
	uint32_t UUID = 0;

	uint16_t skinID = 0;

	float speed = .1f;
	sf::Vector2f velocity = { 0.f, 0.f };
	sf::Vector2f position = { 0.f, 0.f };

	float angularSpeed = .1f;
	float angularVelocity = 0.f;
	float rotation = 0.f;
};

inline bool operator == (const PlayerDesc& l, const PlayerDesc& r) {
	if(l.UUID != r.UUID) return false;
	if(l.skinID != r.skinID) return false;

	if(l.angularSpeed != r.angularSpeed) return false;
	if(l.angularVelocity != r.angularVelocity) return false;
	if(l.rotation != r.rotation) return false;

	if(l.speed != r.speed) return false;
	if(l.velocity != r.velocity) return false;
	if(l.position != r.position) return false;

	return true;
}

inline bool operator != (const PlayerDesc& l, const PlayerDesc& r) { return !(l == r); }

std::ostream& operator << (std::ostream& os, const sf::Vector2f& v) {
	os << "{ x: " << v.x << ", y: " << v.y << " }";
	return os;
}

std::ostream& operator << (std::ostream& os, const PlayerDesc& pd) {
	os << "UUID:" << pd.UUID
		<< " speed:" << pd.speed
		<< " angularSpeed:" << pd.angularSpeed
		<< " angularVelocity:" << pd.angularVelocity
		<< " rotation:" << pd.rotation
		<< " position:" << pd.position
		<< " velocity:" << pd.velocity
		;
	return os;
}