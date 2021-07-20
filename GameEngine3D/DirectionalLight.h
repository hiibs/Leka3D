#pragma once
#include "Object.h"
#include <glm.hpp>

class DirectionalLight : public Object {
public:
	DirectionalLight(
		Scene* scene,
		std::string name = "Directional Light",
		glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
		glm::vec3 rotation = glm::vec3(0.f, 0.f, 0.f),
		glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f),
		Object* parent = nullptr,
		float intensity = 1.f,
		glm::vec3 color = glm::vec3(1.f, 1.f, 1.f)
	);

	float intensity;
	glm::vec3 color;
};

