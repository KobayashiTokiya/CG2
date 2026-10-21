#pragma once
#include "Vector.h"
#include <algorithm>


class Sprite;

class FadeManager
{
public:
	enum class Status
	{
		kNone,
		kFadeIn,
		kFadeOut,
	};

	static FadeManager* GetInstance();

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	//フェード開始命令
	void StartFadeOut(float duration);
	void StartFadeIn(float duration );
	
	// 状態確認
	bool IsFading() const { return status != Status::kNone; }
	bool IsFadeFinished() const { return isFinished; }
	Status GetStatus() const { return status; }

private:
	FadeManager() = default;
	~FadeManager() = default;

	Sprite* fadeSprite = nullptr;
	Status status = Status::kNone;

	float duration_ = 0.5f;
	float timer = 0.0f;
	float alpha = 0.0f;
	bool isFinished = false;
};

