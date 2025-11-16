#include "Logger.h"

namespace Logger
{
	void Log(std::ostream& os,const std::string& message)
	{
		os << message << std::endl;
		OutputDebugStringA(message.c_str());
	}
}