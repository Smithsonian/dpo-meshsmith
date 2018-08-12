/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "VCGMesh.h"
#include "vcg/complex/complex.h"

#include <vector>

using namespace meshsmith;
using namespace flow;


namespace meshsmith
{


	struct _vcgmesh_impl_t : public SharedImpl
	{
		//vcgMesh_t mesh;
	};
}

VCGMesh* VCGMesh::create(const aiMesh* pMesh)
{
	return nullptr;
}

VCGMesh::VCGMesh()
{
	_pImpl = new _vcgmesh_impl_t();
}

VCGMesh::VCGMesh(const VCGMesh& other) :
	SharedT<VCGMesh>(other)
{
}

VCGMesh& VCGMesh::operator=(const VCGMesh& other)
{
	return SharedT<VCGMesh>::operator=(other);
}

json VCGMesh::inspect() const
{
	return json();
}

aiMesh* VCGMesh::toMesh() const
{
	return nullptr;
}
