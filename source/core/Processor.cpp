/**
 * 3D Foundation Project
 * Copyright 2019 Smithsonian Institution
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

void Processor::transform(const aiScene* pScene, const Matrix4f& matrix)
{
	for (uint32_t i = 0; i < pScene->mNumMeshes; ++i) {
		Processor::transform(pScene->mMeshes[i], matrix);
	}
}

void Processor::transform(const aiMesh* pMesh, const Matrix4f& matrix)
{
	uint32_t count = pMesh->mNumVertices;

	for (uint32_t i = 0; i < count; ++i) {
		aiVector3D* p = &pMesh->mVertices[i];
		Vector4f t = matrix * Vector4f(p->x, p->y, p->z, 1);
		t.homogenize();
		p->x += t.x;
		p->y += t.y;
		p->z += t.z;
	}
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

void Processor::align(const aiScene* pScene, Align alignX, Align alignY, Align alignZ)
{
	Range3f boundingBox = Processor::calculateBoundingBox(pScene);
	Vector3f offset = Processor::getOffset(boundingBox, alignX, alignY, alignZ);

	Processor::translate(pScene, offset);
}

void Processor::align(const aiMesh* pMesh, Align alignX, Align alignY, Align alignZ)
{
	Range3f boundingBox = Processor::calculateBoundingBox(pMesh);
	Vector3f offset = Processor::getOffset(boundingBox, alignX, alignY, alignZ);

	Processor::translate(pMesh, offset);
}

Vector3f Processor::getOffset(const Range3f& boundingBox, Align alignX, Align alignY, Align alignZ)
{
	Vector3f lowerBound = boundingBox.lowerBound();
	Vector3f upperBound = boundingBox.upperBound();
	Vector3f center = boundingBox.center();
	Vector3f offset(0.0f, 0.0f, 0.0f);

	switch (alignX) {
	case Align::Start:
		offset.x = -lowerBound.x;
		break;
	case Align::Center:
		offset.x = -center.x;
		break;
	case Align::End:
		offset.x = -upperBound.x;
		break;
	}

	switch (alignY) {
	case Align::Start:
		offset.y = -lowerBound.y;
		break;
	case Align::Center:
		offset.y = -center.y;
		break;
	case Align::End:
		offset.y = -upperBound.y;
		break;
	}

	switch (alignZ) {
	case Align::Start:
		offset.z = -lowerBound.z;
		break;
	case Align::Center:
		offset.z = -center.z;
		break;
	case Align::End:
		offset.z = -upperBound.z;
		break;
	}

	return offset;
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

void Processor::flipUVs(const aiScene* pScene, bool flipX, bool flipY)
{
	for (uint32_t i = 0; i < pScene->mNumMeshes; ++i) {
		Processor::flipUVs(pScene->mMeshes[i], flipX, flipY);
	}
}

void Processor::flipUVs(const aiMesh* pMesh, bool flipX, bool flipY)
{
	uint32_t channels = pMesh->GetNumUVChannels();
	uint32_t count = pMesh->mNumVertices;

	for (uint32_t c = 0; c < channels; ++c) {
		aiVector3D* pCoords = pMesh->mTextureCoords[c];
		for (uint32_t i = 0; i < count; ++i) {
			aiVector3D& uv = pCoords[i];
			uv[0] = flipX ? 1 - uv[0] : uv[0];
			uv[1] = flipY ? 1 - uv[1] : uv[1];
		}
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

