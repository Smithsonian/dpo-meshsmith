/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#ifndef _MESHSMITH_VCG_MESH_H
#define _MESHSMITH_VCG_MESH_H

#include "library.h"
#include "core/json.h"
#include "core/SharedT.h"


struct aiMesh;

namespace meshsmith
{
	struct _vcgmesh_impl_t;

	class MESHSMITH_CORE_EXPORT VCGMesh : public flow::SharedT<VCGMesh>
	{
	public:
		static VCGMesh* create(const aiMesh* pMesh);

		VCGMesh();
		VCGMesh(const VCGMesh& other);
		VCGMesh& operator=(const VCGMesh& other);

		flow::json inspect() const;
		aiMesh* toMesh() const;

	protected:
		_vcgmesh_impl_t* _pImpl;
	};
}

#endif // _MESHSMITH_VCG_MESH_H