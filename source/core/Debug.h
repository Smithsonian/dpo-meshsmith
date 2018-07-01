/**
* Intermesh Engine
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
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