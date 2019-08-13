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

#if defined(WIN32) && defined(_DEBUG)
#include <Windows.h>
#include <iostream>
#include <sstream>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace meshsmith
{
	class DebugStream : public std::stringbuf
	{
	public:
		~DebugStream() { sync(); }

		int sync()
		{
			::OutputDebugStringA(str().c_str());
			str(std::string()); // Clear the string buffer
			return 0;
		}
	};

	class DebugStreamEnabler
	{
	public:
		DebugStreamEnabler()
		{
			_pStream = new DebugStream();
			std::cout.rdbuf(_pStream);
		}

		~DebugStreamEnabler()
		{
			delete _pStream;
		}

	private:
		DebugStream* _pStream;
	};

}
#else
namespace meshsmith
{
	class DebugStreamEnabler
	{
	};
}
#endif
