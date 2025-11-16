#include "Input.h"
#include <cassert>

//ヘッダに引っ越し
//#include <wrl.h>
//using namespace Microsoft::WRL;
//#define DIRECTINPUT_VERSION 0x0800
//#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(WinApp* winApp)
{
	HRESULT result;

	//借りてきたWinAppのインスタントを記録
	this->winApp_ = winApp;

	//DirectInputの初期化
	// 引っ越し
	//ComPtr<IDirectInput8> directInput = nullptr;
	result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	//メンバ変数として宣言したためローカル変数のkeyboardは削除する
	//ComPtr<IDirectInputDevice8> keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));

	//排他制限レベルのセット
	result = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
	HRESULT result;

	//前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));

	//キーボード情報の取得開始
	result = keyboard->Acquire();

	//全キーの入力状態を取得する
	//引っ越しする
	//BYTE key[256] = {};
	result = keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber])
	{
		return true;
	}
	//そうでなければfalseを返す
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (!keyPre[keyNumber] && key[keyNumber])
	{
		return true;
	};
	return false;
}

bool Input::ReleaseKey(BYTE keyNumber)
{
	if (keyPre[keyNumber] && !key[keyNumber])
	{
		return true;
	}
	return false;
}
