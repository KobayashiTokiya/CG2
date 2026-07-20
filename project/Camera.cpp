#include "Camera.h"
#include "Input.h"

void Camera::Update()
{
	worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale,transform.rotate,transform.translate);
	viewMatrix = MatrixMath::Inverse(worldMatrix);

	projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(fovY,aspectRatio,nearClip,farClip);

	viewProjectionMatrix = MatrixMath::Multiply(viewMatrix, projectionMatrix);
}

Camera::Camera()
	: transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(MatrixMath::Inverse(worldMatrix))
	, projectionMatrix(MatrixMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(MatrixMath::Multiply(viewMatrix, projectionMatrix))
{

}

void Camera::DebugUpdate(Input* input)
{
	// ─── 1. 矢印キーでカメラの回転（向き）を変更 ───
	if (input->PushKey(DIK_UP)) { transform.rotate.x -= rotateSpeed; }
	if (input->PushKey(DIK_DOWN)) { transform.rotate.x += rotateSpeed; }
	if (input->PushKey(DIK_LEFT)) { transform.rotate.y -= rotateSpeed; }
	if (input->PushKey(DIK_RIGHT)) { transform.rotate.y += rotateSpeed; }

	// ─── 2. カメラの現在の向きから「前・右・上」の方向ベクトルを計算 ───
	Matrix4x4 rotationXMatrix = MatrixMath::MakeRotateXMatrix(transform.rotate.x);
	Matrix4x4 rotationYMatrix = MatrixMath::MakeRotateYMatrix(transform.rotate.y);
	Matrix4x4 rotationZMatrix = MatrixMath::MakeRotateZMatrix(transform.rotate.z);
	
	Matrix4x4 rotationMatrix = MatrixMath::Multiply(rotationXMatrix, rotationYMatrix);
	rotationMatrix = MatrixMath::Multiply(rotationMatrix, rotationZMatrix);


	// ローカルな方向ベクトル（Z前、X右、Y上）に回転を適用する
	Vector3 forward = { rotationMatrix.m[2][0], rotationMatrix.m[2][1], rotationMatrix.m[2][2] }; // Z方向
	Vector3 right = { rotationMatrix.m[0][0], rotationMatrix.m[0][1], rotationMatrix.m[0][2] }; // X方向
	Vector3 up = { rotationMatrix.m[1][0], rotationMatrix.m[1][1], rotationMatrix.m[1][2] }; // Y方向

	// ─── 3. WASDキー（およびQEで上下）で移動 ───
	Vector3 moveDir = { 0.0f, 0.0f, 0.0f };

	if (input->PushKey(DIK_W))
	{ // 前進
		moveDir.x += forward.x; moveDir.y += forward.y; moveDir.z += forward.z;
	}
	if (input->PushKey(DIK_S))
	{ // 後退
		moveDir.x -= forward.x; moveDir.y -= forward.y; moveDir.z -= forward.z;
	}
	if (input->PushKey(DIK_D))
	{ // 右移動
		moveDir.x += right.x; moveDir.y += right.y; moveDir.z += right.z;
	}
	if (input->PushKey(DIK_A))
	{ // 左移動
		moveDir.x -= right.x; moveDir.y -= right.y; moveDir.z -= right.z;
	}
	if (input->PushKey(DIK_E))
	{ // 上昇
		moveDir.x += up.x; moveDir.y += up.y; moveDir.z += up.z;
	}
	if (input->PushKey(DIK_Q))
	{ // 下降
		moveDir.x -= up.x; moveDir.y -= up.y; moveDir.z -= up.z;
	}

	// ベクトルに速度を掛けて座標に加算
	transform.translate.x += moveDir.x * moveSpeed;
	transform.translate.y += moveDir.y * moveSpeed;
	transform.translate.z += moveDir.z * moveSpeed;

	// ─── 4. 最後に通常のUpdateを呼んで行列を再計算する ───
	Update();
}