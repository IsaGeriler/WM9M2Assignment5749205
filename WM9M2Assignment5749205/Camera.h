#pragma once

#include "MyMath.h"

// For FPS use inverted lookAt & translate/scale etc
class Camera {
public:
	// Camera Attributes (Transform from world to camera)
	Vec3 position = Vec3(0.f, 0.f, 0.f);
	Vec3 from = Vec3(1.f, 0.f, 0.f);  // Given a position
	Vec3 to = Vec3(0.f, 0.f, 1.f);	  // Target Point
	Vec3 up = Vec3(0.f, 1.f, 0.f);    // Up Vector

	// Frustrum Attributes
	int width{}, height{};
	float zFar{}, zNear{}, aspectRatio{}, fovTheta{}, windowHeightNear{}, windowHeightFar{};

	// View and Projection Matrices
	Matrix view = Matrix::identity();
	Matrix projection = Matrix::identity();

	// Update View Matrix
	void updateViewMatrix() { view = Matrix::lookAt(position, to, up); }

	// Set Camera Lense
	void setLense(float _width, float _height, float _zFar, float _zNear, float _fovTheta = 90.f) {
		width = _width;
		height = _height;
		zFar = _zFar;
		zNear = _zNear;
		fovTheta = _fovTheta;

		windowHeightFar = 2.f * zFar * tanf(0.5f * fovTheta * 0.5f * M_PI / 180.f);
		windowHeightNear = 2.f * zNear * tanf(0.5f * fovTheta * 0.5f * M_PI / 180.f);
		projection = Matrix::projection(width, height, zFar, zNear, fovTheta);
	}
};