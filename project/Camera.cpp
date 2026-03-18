#include "Camera.h"


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