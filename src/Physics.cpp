#include "Physics.h"

void Physics::update(float deltaTime) {
	for (BoxHull* box : boxHulls) {
		box->isGrounded = false;
		box->contactNormals.clear();
		for (Mesh* mesh : meshes) {
			if (!mesh->enableCollision)
				continue;

			const glm::vec3* bounds = mesh->getBounds();

			glm::vec3 meshMin = bounds[0];
			glm::vec3 meshMax = bounds[1];

			glm::vec3 boxMin = -box->halfExtents * box->getScale() + box->getPosition();
			glm::vec3 boxMax = box->halfExtents * box->getScale() + box->getPosition();

			std::vector<glm::vec3> boxCorners = {
				box->getModelMatrix() * glm::vec4(box->halfExtents.x, box->halfExtents.y, box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(box->halfExtents.x, box->halfExtents.y, -box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(box->halfExtents.x, -box->halfExtents.y, box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(-box->halfExtents.x, box->halfExtents.y, box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(-box->halfExtents.x, box->halfExtents.y, -box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(-box->halfExtents.x, -box->halfExtents.y, box->halfExtents.z, 1.f),
				box->getModelMatrix() * glm::vec4(-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z, 1.f)
			};

			// Do a AABB test

			if (boxMin.x < meshMax.x && boxMax.x > meshMin.x &&
				boxMin.y < meshMax.y && boxMax.y > meshMin.y &&
				boxMin.z < meshMax.z && boxMax.z > meshMin.z) {


				std::vector<glm::vec3> meshPoints;
				for (glm::vec3 vert : mesh->getVertices())
					meshPoints.push_back(mesh->getModelMatrix() * glm::vec4(vert, 1.f));


				// test AABB against every triangle
				float floorNormalZ(-1.f);
				for (int i = 0; i < mesh->getIndices().size(); i += 3) {
					glm::vec3 c(0.f);
					glm::vec3 minCorrection(1000.f);
					bool collisionFound = true;

					std::vector<glm::vec3> verts = {
						meshPoints[mesh->getIndices()[i]],
						meshPoints[mesh->getIndices()[i + 1]],
						meshPoints[mesh->getIndices()[i + 2]],
					};

					glm::vec3 edges[] = {
						verts[1] - verts[0],
						verts[2] - verts[1],
						verts[0] - verts[2]
					};

					glm::vec3 origin = (verts[0] + verts[1] + verts[2]) / 3.f;
					glm::vec3 faceNormal = glm::normalize(glm::cross(edges[0], edges[1]));

					glm::vec3 edgeOrigins[] = {
						(verts[1] + verts[0]) / 2.f,
						(verts[2] + verts[1]) / 2.f,
						(verts[0] + verts[2]) / 2.f
					};

					glm::vec3 edgeNormals[3];
					for (int j = 0; j < 3; j++) {
						edgeNormals[j] = glm::normalize(edgeOrigins[j] - origin);
					}

					glm::vec3 axes[] = {
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(0.f, 0.f, 1.f)
					};

					bool validFaceNormal = false;
					for (int i = 0; i < 8; i++) {
						if (glm::dot(boxCorners[i] - origin, faceNormal) > 0.f) {
							validFaceNormal = true;
						}
					}
					if (!validFaceNormal)
						goto nextFace;

					// Test for overlaps on cross products of edge normals and aabb axes
					for (int j = 0; j < 3; j++) {
						for (int k = 0; k < 3; k++) {
							glm::vec3 n = glm::normalize(glm::cross(edgeNormals[j], axes[k]));
							if (testSatCollision(boxCorners, verts, n, c)) {
								if (glm::length(c) < glm::length(minCorrection))
									minCorrection = c;
							}
							else
								collisionFound = false;
						}
					}

					// Test for overlaps on Box axes
					for (int j = 0; j < 3; j++) {
						if (testSatCollision(boxCorners, verts, axes[j], c)) {
							if (glm::length(c) < glm::length(minCorrection))
								minCorrection = c;
						}
						else
							collisionFound = false;
					}

					// Test for overlaps on Face normal axis
					if (testSatCollision(boxCorners, verts, faceNormal, c)) {
						if (glm::length(c) < glm::length(minCorrection))
							minCorrection = c;
					}
					else
						collisionFound = false;


					if (collisionFound) {
						box->move(glm::length(minCorrection)* faceNormal, false);

						float pVel = glm::dot(box->velocity, faceNormal);
						box->velocity -= pVel * faceNormal;

						if (faceNormal.z > 0.7f) {
							box->isGrounded = true;
						}
					}
				nextFace:;
				}
			}
		}
	}
}

bool Physics::lineTrace(glm::vec3 start, glm::vec2 end) {
	return false;
}

void Physics::addMesh(Mesh* object) {
	for (Mesh* m : meshes) {
		if (object == m)
			return;
	}
	
	meshes.push_back(object);
}

void Physics::removeMesh(Mesh* object) {
	for (int i = 0; i < meshes.size(); i++) {
		if (object == meshes[i]) {
			delete object;
			meshes.erase(meshes.begin() + i);
		}
	}
}

void Physics::addBoxHull(BoxHull* object) {
	for (BoxHull* b : boxHulls) {
		if (object == b) {
			return;
		}
	}
	
	boxHulls.push_back(object);
}

void Physics::removeBoxHull(BoxHull* object) {
	for (int i = 0; i < boxHulls.size(); i++) {
		if (object == boxHulls[i]) {
			delete object;
			boxHulls.erase(boxHulls.begin() + i);
		}
	}
}

bool Physics::testVertexOverlap(glm::vec3 boxMin, glm::vec3 boxMax, const std::vector<glm::vec3>& points, glm::vec3& correction) {
	for (glm::vec3 point : points) {
		if (point.x < boxMax.x && point.x > boxMin.x &&
			point.y < boxMax.y && point.y > boxMin.y &&
			point.z < boxMax.z && point.z > boxMin.z) {

			glm::vec3 corrections[] = {
				glm::vec3(boxMin.x - point.x, 0.f, 0.f),
				glm::vec3(boxMax.x - point.x, 0.f, 0.f),
				glm::vec3(0.f, boxMin.y - point.y, 0.f),
				glm::vec3(0.f, boxMax.y - point.y, 0.f),
				glm::vec3(0.f, 0.f, boxMin.z - point.z),
				glm::vec3(0.f, 0.f, boxMax.z - point.z)
			};

			correction = corrections[0];
			for (int i = 1; i < 6; i++) {
				if (glm::length(corrections[i]) < glm::length(correction))
					correction = corrections[i];
			}

			//box->move(correction, true);

			return true;
		}
	}
	return false;
}

bool Physics::testSatCollision(const std::vector<glm::vec3>& shapeA, const std::vector<glm::vec3>& shapeB, glm::vec3 normal, glm::vec3& correction) {

	float aMin = glm::dot(shapeA[0], normal);
	float aMax = aMin;

	for (glm::vec3 point : shapeA) {
		float dot = glm::dot(point, normal);
		if (dot < aMin)
			aMin = dot;
		if (dot > aMax)
			aMax = dot;
	}

	float bMin = glm::dot(shapeB[0], normal);
	float bMax = bMin;

	for (glm::vec3 point : shapeB) {
		float dot = glm::dot(point, normal);
		if (dot < bMin)
			bMin = dot;
		if (dot > bMax)
			bMax = dot;
	}

	if (aMin < bMax && aMax > bMin) {
		float c = abs(aMax - bMin) < abs(aMin - bMax) ? -abs(aMax - bMin) : abs(aMin - bMax);
		correction = c * normal;
		return true;
	}
	else
		return false;
}
