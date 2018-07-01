/**
 * Intermesh Engine
 *
 * @author Ralph Wiedemeier <ralph@framefactory.io>
 * @copyright (c) 2018 Frame Factory GmbH.
 */
 
#ifndef _MESHSMITH_SCENE_H
#define _MESHSMITH_SCENE_H

#include "library.h"
#include <string>

struct aiMesh;

namespace meshsmith
{
	struct _sceneImpl_t;

	class INTERMESH_ENGINE_EXPORT Scene
	{
	public:
		static std::string getJsonExportFormats();
		static std::string getJsonError(const std::string& message);

	public:
		Scene();
		Scene(const Scene& other);
		~Scene();

		Scene& operator=(const Scene& other);

	public:
		void setVerbose(bool enabled);
		bool load(const std::string& fileName, bool stripNormals, bool stripUVs);
		bool save(const std::string& fileName, const std::string& formatId, bool joinVertices, bool stripNormals, bool stripUVs) const;

		void swizzle(const std::string& order);
		void center();
		void scale(float factor);

		void dump() const;
		bool isValid() const;
		bool hasError() const;
		const std::string& getLastError() const;
		std::string getJsonReport() const;
		std::string getJsonStatus() const;

	private:
		void _dumpMesh(const aiMesh* pMesh) const;

		void _createRef();
		void _addRef();
		void _releaseRef();

		_sceneImpl_t* _pImpl;
	};
}

#endif // _MESHSMITH_SCENE_H