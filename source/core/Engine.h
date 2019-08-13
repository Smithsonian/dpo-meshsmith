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

#ifndef _MESHSMITH_ENGINE_H
#define _MESHSMITH_ENGINE_H

#include "library.h"
#include <string>

namespace meshsmith
{
	struct _engineImpl_t;

	class MESHSMITH_CORE_EXPORT Engine
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
