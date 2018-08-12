/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#ifndef _MESHSMITH_VCGMESHTYPE_H
#define _MESHSMITH_VCGMESHTYPE_H

#include "library.h"
#include "vcg/complex/complex.h"
#include <vector>

namespace meshsmith
{
	class VCGVertexType;
	class VCGEdgeType;
	class VCGFaceType;

	struct VCGUsedTypes : public vcg::UsedTypes<
		vcg::Use<VCGVertexType>::AsVertexType,
		vcg::Use<VCGEdgeType>::AsEdgeType,
		vcg::Use<VCGFaceType>::AsFaceType> {};

	class VCGVertexType : public vcg::Vertex<VCGUsedTypes,
		vcg::vertex::InfoOcf,
		vcg::vertex::Coord3f,
		vcg::vertex::Normal3f,
		vcg::vertex::Qualityf,
		vcg::vertex::Color4b,
		vcg::vertex::VFAdjOcf,
		vcg::vertex::MarkOcf,
		vcg::vertex::TexCoordfOcf,
		vcg::vertex::CurvaturefOcf,
		vcg::vertex::CurvatureDirfOcf,
		vcg::vertex::RadiusfOcf,
		vcg::vertex::BitFlags>
	{};

	class VCGEdgeType : public vcg::Edge<VCGUsedTypes,
		vcg::edge::EVAdj,
		vcg::edge::EEAdj,
		vcg::edge::BitFlags>
	{};

	class VCGFaceType : public vcg::Face<VCGUsedTypes,
		vcg::face::InfoOcf,
		vcg::face::VertexRef,
		vcg::face::Normal3f,
		vcg::face::QualityfOcf,
		vcg::face::MarkOcf,
		vcg::face::Color4bOcf,
		vcg::face::FFAdjOcf,
		vcg::face::VFAdjOcf,
		vcg::face::CurvatureDirfOcf,
		vcg::face::WedgeTexCoordfOcf,
		vcg::face::BitFlags>
	{};

	enum class VCGAttrib
	{
		NONE = 0x00000000,
		VERTCOORD = 0x00000001,
		VERTNORMAL = 0x00000002,
		VERTFLAG = 0x00000004,
		VERTCOLOR = 0x00000008,
		VERTQUALITY = 0x00000010,
		VERTMARK = 0x00000020,
		VERTFACETOPO = 0x00000040,
		VERTCURV = 0x00000080,
		VERTCURVDIR = 0x00000100,
		VERTRADIUS = 0x00000200,
		VERTTEXCOORD = 0x00000400,
		VERTNUMBER = 0x00000800,

		FACEVERT = 0x00001000,
		FACENORMAL = 0x00002000,
		FACEFLAG = 0x00004000,
		FACECOLOR = 0x00008000,
		FACEQUALITY = 0x00010000,
		FACEMARK = 0x00020000,
		FACEFACETOPO = 0x00040000,
		FACENUMBER = 0x00080000,
		FACECURVDIR = 0x00100000,

		WEDGTEXCOORD = 0x00200000,
		WEDGNORMAL = 0x00400000,
		WEDGCOLOR = 0x00800000,

		// 	Selection
		VERTFLAGSELECT = 0x01000000,
		FACEFLAGSELECT = 0x02000000,

		// Per Mesh Stuff....
		CAMERA = 0x08000000,
		TRANSFMATRIX = 0x10000000,
		COLOR = 0x20000000,
		POLYGONAL = 0x40000000,

		// unknown - will raise exceptions, to be avoided, here just for compatibility
		UNKNOWN = 0x80000000,

		// geometry change (for filters that remove stuff or modify geometry or topology, but not touch face/vertex color or face/vertex quality)
		GEOMETRY_AND_TOPOLOGY_CHANGE = 0x431e7be7,

		// everything - dangerous, will add unwanted data to layer (e.g. if you use MM_ALL it could means that it could add even color or quality)
		MM_ALL = 0xffffffff
	};

	class VCGMesh : public vcg::tri::TriMesh<std::vector<VCGVertexType>, std::vector<VCGFaceType>, std::vector<VCGEdgeType>>
	{

	};
}

#endif // _MESHSMITH_VCGMESHTYPE_H