/**
* Intermesh Processor
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
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

		static flow::Range3f calculateBoundingBox(const aiScene* pScene);
		static flow::Range3f calculateBoundingBox(const aiMesh* pMesh);
		
	protected:
		static flow::Vector3f getOffset(const flow::Range3f& boundingBox, Align alignX, Align alignY, Align alignZ);

	private:
	};
}
 
#endif // _MESHSMITH_PROCESSOR_H