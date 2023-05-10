#pragma once
#include <glm/glm.hpp>

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 color;

	float size;
	float life;
	float cameradistance;

	bool operator<(Particle& that) {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};
