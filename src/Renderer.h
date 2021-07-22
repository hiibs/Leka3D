#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>

class Renderer
{
public:
	void drawMeshes();

	void addMesh(const Mesh* mesh);
	void removeMesh(const Mesh* mesh);

	const Scene* scene;
	Camera* camera;
private:
	std::vector<const Mesh*> meshes;
};
