/**
 * Intermesh Engine
 *
 * @author Ralph Wiedemeier <ralph@framefactory.ch>
 * @copyright (c) 2018 Frame Factory GmbH.
 */

#ifndef _MESHSMITH_ENGINE_H
#define _MESHSMITH_ENGINE_H

#include "library.h"
#include <string>

namespace meshsmith
{
	struct _engineImpl_t;

	class INTERMESH_ENGINE_EXPORT Engine
	{
	public:
		Engine();
		Engine(const Engine& other);

		~Engine();

		Engine& operator=(const Engine& other);

	public:
		//Scene loadScene(const std::string& fileName);

	private:
		void _createRef();
		void _addRef();
		void _releaseRef();

		_engineImpl_t* _pImpl;
	};
}
 
#endif // _MESHSMITH_ENGINE_H