#pragma once
#include <string>
#include <memory>
#include "Vector.h"

class Object3d;
class Camera;

class Player
{
public:
	Player();
	~Player();

	void Initialize(const std::string& modelFilePath);
	void Update();
	void Draw();

	const Vector3& GetPosition() const { return position_; }

	void SetCamera(Camera* camera) { camera_ = camera; }
private:
	Vector3 position_;
	float speed_;
	
	float velocityY_ = 0.0f;
	const float jumpInitialSpeed_ = 0.5f;
	const float gravity_ = 0.02f;
	bool isJumping_ = false;

	std::unique_ptr<Object3d> object3d_ = nullptr;
	Camera* camera_ = nullptr;
};

