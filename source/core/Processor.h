/**
* Intermesh Processor
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#ifndef _MESHSMITH_PROCESSOR_H
#define _MESHSMITH_PROCESSOR_H

#include "library.h"

#include "math/Vector3T.h"
#include "math/Range3T.h"

struct aiScene;
struct aiMesh;

namespace meshsmith
{
	class INTERMESH_ENGINE_EXPORT Processor
	{
	protected:
		Processor() {};

	public:
		static void combine(const aiScene* pScene, const std::string& diffuseMap, const std::string& occlusionMap, const std::string& normalMap);

		static void center(const aiScene* pScene);
		
		static void translate(const aiScene* pScene, const flow::Vector3f& offset);
		static void translate(const aiMesh* pMesh, const flow::Vector3f& offset);

		static void scale(const aiScene* pScene, const float factor);
		static void scale(const aiMesh* pMesh, const float factor);

		static void swizzle(const aiScene* pScene, const std::string& order);
		static void swizzle(const aiMesh* pMesh, const std::string& order);

		static flow::Range3f calculateBoundingBox(const aiScene* pScene);
		static flow::Range3f calculateBoundingBox(const aiMesh* pMesh);
		
	protected:
	private:
	};
}
 
#endif // _MESHSMITH_PROCESSOR_H