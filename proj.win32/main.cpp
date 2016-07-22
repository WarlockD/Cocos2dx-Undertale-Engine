#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"
#include "UndertaleResourceNode.h"
#include <string>
#include <sstream>
#include <codecvt>
#include <iostream>
#include <algorithm>
#include <iterator>

USING_NS_CC;
std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}


void LoadResources(LPTSTR lpCmdLine) {
	std::string cmdline = ws2s(lpCmdLine);
	std::stringstream iss(cmdline);
	std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
	if (tokens.size() ==0) exit(1);
	std::string filename = tokens[0];
	if (!Undertale::loadDataWin(filename)) {
		exit(-1);
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
   // UNREFERENCED_PARAMETER(lpCmdLine);
   //Undertale::UndertaleFile dataWin;
	LoadResources(lpCmdLine);


    // create the application instance
    AppDelegate app;
    return Application::getInstance()->run();
}
