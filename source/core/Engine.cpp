/**
* Intermesh Engine
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
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
