#include "Console.h"

Console::Console(void)
	: mImp(*reinterpret_cast<ConsoleImplementation*>(NULL))
{
}

Console::~Console(void)
{
}

void Console::update(void)
{
}

void Console::write(const ExitGames::Common::JString& str)
{
#if defined _EG_MARMALADE_PLATFORM
	std::cout << str.UTF8Representation().cstr();
#else
	std::wcout << str;
#endif
}

void Console::writeLine(const ExitGames::Common::JString& str)
{
#if defined _EG_MARMALADE_PLATFORM
	std::cout << str.UTF8Representation().cstr() << std::endl;
#else
	std::wcout << str << std::endl;
#endif
}

Console& Console::get(void)
{
	static Console console;
	return console;
}