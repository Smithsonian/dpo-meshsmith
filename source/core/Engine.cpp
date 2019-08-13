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
 
#include "Engine.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>

using namespace meshsmith;

namespace meshsmith
{
	struct _engineImpl_t
	{
		Assimp::Importer* _pImporter;
		Assimp::Exporter* _pExporter;
		uint32_t refCount;
	};
}

Engine::Engine()
{
	_createRef();
}

Engine::Engine(const Engine& other)
{
	_pImpl = other._pImpl;
	_addRef();
}

Engine::~Engine()
{
	_releaseRef();
}

Engine& Engine::operator=(const Engine& other)
{
	if (this == &other)
		return *this;

	_releaseRef();
	_pImpl = other._pImpl;
	_addRef();
	return *this;
}

void Engine::_createRef()
{
	_pImpl = new _engineImpl_t();
	_pImpl->refCount = 1;
	_pImpl->_pImporter = new Assimp::Importer();
	_pImpl->_pExporter = new Assimp::Exporter();
}

void Engine::_addRef()
{
	if (_pImpl)
		_pImpl->refCount++;
}

void Engine::_releaseRef()
{
	if (_pImpl) {
		_pImpl->refCount--;
		if (_pImpl->refCount == 0) {
			delete _pImpl->_pImporter;
			delete _pImpl->_pExporter;
			delete _pImpl;
		}

		_pImpl = nullptr;
	}
}
