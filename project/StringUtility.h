#pragma once
#include <string>
#include <Windows.h>

//文字コードユーティリティ
namespace StringUtility
{
	//stringをwstringに変更する
	std::wstring ConvertString(const std::string& str);

	//wstringをstringに変換する
	std::string ConvertString(const std::wstring& str);
};

