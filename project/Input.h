#pragma once
#include <windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Input
{
public:
	//namespace省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	//初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	//更新
	void Update();

	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

	bool ReleaseKey(BYTE keyNumber);
private:
	//メンバ変数
	//キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	//全キーの状態
	BYTE key[256] = {};
	//前回の全キ―の状態
	BYTE keyPre[256] = {};
	//DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput = nullptr;
};
