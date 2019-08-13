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

#ifndef _MESHSMITH_PROCESSOR_H
#define _MESHSMITH_PROCESSOR_H

#include "library.h"

#include "math/Vector3T.h"
#include "math/Matrix4T.h"
#include "math/Range3T.h"

struct aiScene;
struct aiMesh;

namespace meshsmith
{
	enum Align { None, Start, Center, End };

	class MESHSMITH_CORE_EXPORT Processor
	{
	protected:
		Processor() {};

	public:
		static void combine(const aiScene* pScene, const std::string& diffuseMap, const std::string& occlusionMap, const std::string& normalMap);

		static void transform(const aiScene* pScene, const flow::Matrix4f& matrix);
		static void transform(const aiMesh* pMesh, const flow::Matrix4f& matrix);

		static void translate(const aiScene* pScene, const flow::Vector3f& offset);
		static void translate(const aiMesh* pMesh, const flow::Vector3f& offset);

		static void scale(const aiScene* pScene, float factor);
		static void scale(const aiMesh* pMesh, float factor);

		static void align(const aiScene* pScene, Align alignX, Align alignY, Align alignZ);
		static void align(const aiMesh* pMesh, Align alignX, Align alignY, Align alignZ);

		static void swizzle(const aiScene* pScene, const std::string& order);
		static void swizzle(const aiMesh* pMesh, const std::string& order);

		static void flipUVs(const aiScene* pScene, bool flipX, bool flipY);
		static void flipUVs(const aiMesh* pMesh, bool flipX, bool flipY);

		static flow::Range3f calculateBoundingBox(const aiScene* pScene);
		static flow::Range3f calculateBoundingBox(const aiMesh* pMesh);
		
	protected:
		static flow::Vector3f getOffset(const flow::Range3f& boundingBox, Align alignX, Align alignY, Align alignZ);

	private:
	};
}
 
#endif // _MESHSMITH_PROCESSOR_H
