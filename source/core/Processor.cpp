/**
* Intermesh Processor
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "Processor.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>

#include <iostream>

using namespace meshsmith;
using namespace flow;


void Processor::combine(const aiScene* pScene, const std::string& diffuseMap, const std::string& occlusionMap, const std::string& normalMap)
{

}


void Processor::center(const aiScene* pScene)
{
	Range3f boundingBox = Processor::calculateBoundingBox(pScene);
	Vector3f offset = boundingBox.center();

	Processor::translate(pScene, -offset);
}

void Processor::translate(const aiScene* pScene, const Vector3f& offset)
{
	for (uint32_t i = 0; i < pScene->mNumMeshes; ++i) {
		Processor::translate(pScene->mMeshes[i], offset);
	}
}

void Processor::translate(const aiMesh* pMesh, const Vector3f& offset)
{
	uint32_t count = pMesh->mNumVertices;

	for (uint32_t i = 0; i < count; ++i) {
		aiVector3D* p = &pMesh->mVertices[i];
		p->x += offset.x;
		p->y += offset.y;
		p->z += offset.z;
	}
}

void Processor::scale(const aiScene* pScene, const float factor)
{
	for (uint32_t i = 0; i < pScene->mNumMeshes; ++i) {
		Processor::scale(pScene->mMeshes[i], factor);
	}
}

void Processor::scale(const aiMesh* pMesh, const float factor)
{
	uint32_t count = pMesh->mNumVertices;

	for (uint32_t i = 0; i < count; ++i) {
		aiVector3D* p = &pMesh->mVertices[i];
		p->x *= factor;
		p->y *= factor;
		p->z *= factor;
	}
}

void Processor::swizzle(const aiScene* pScene, const std::string& order)
{
	for (uint32_t i = 0; i < pScene->mNumMeshes; ++i) {
		Processor::swizzle(pScene->mMeshes[i], order);
	}
}

void Processor::swizzle(const aiMesh* pMesh, const std::string& order)
{
	float factors[3] = { 1, 1, 1 };
	size_t indices[3] = { 0, 1, 2 };

	size_t index = 0;
	int pos = -1;
	for (size_t i = 0; i < order.size(); ++i) {
		char c = order[i];
		if (c == 'x' || c == 'X') {
			index = 0;
			indices[++pos] = index;
		}
		else if (c == 'y' || c == 'Y') {
			index = 1;
			indices[++pos] = index;
		}
		else if (c == 'z' || c == 'Z') {
			index = 2;
			indices[++pos] = index;
		}
		else if (c == '-') {
			factors[pos] = -1;
		}
	}

	uint32_t count = pMesh->mNumVertices;

	for (uint32_t i = 0; i < count; ++i) {
		float* p = (float*)(&(pMesh->mVertices[i]));
		float x = p[indices[0]] * factors[0];
		float y = p[indices[1]] * factors[1];
		float z = p[indices[2]] * factors[2];
		p[0] = x; p[1] = y; p[2] = z;
	}
}

Range3f Processor::calculateBoundingBox(const aiScene* pScene)
{
	Range3f boundingBox;
	boundingBox.invalidate();

	uint32_t count = pScene->mNumMeshes;

	for (uint32_t i = 0; i < count; ++i) {
		boundingBox.uniteWith(Processor::calculateBoundingBox(pScene->mMeshes[i]));
	}

	return boundingBox;
}

Range3f Processor::calculateBoundingBox(const aiMesh* pMesh)
{
	Range3f boundingBox;
	boundingBox.invalidate();
	uint32_t count = pMesh->mNumVertices;

	for (uint32_t i = 0; i < count; ++i) {
		boundingBox.include(Vector3f((float*)&(pMesh->mVertices[i])));
	}

	return boundingBox;
}

