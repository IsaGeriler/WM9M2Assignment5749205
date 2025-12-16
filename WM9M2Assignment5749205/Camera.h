#pragma once

#include "MyMath.h"

class Camera {
public:
	// Camera Attributes (Transform from world to camera)
	Vec3 from = Vec3(1.f, 0.f, 0.f);  // Given a position (eye)
	Vec3 to = Vec3(0.f, 0.f, 1.f);	  // Target Point (look)
	Vec3 up = Vec3(0.f, 1.f, 0.f);    // Up Vector (up)

	// Frustrum Attributes
	int width{}, height{};
	float zFar{}, zNear{}, aspectRatio{}, fovTheta{}, windowHeightNear{}, windowHeightFar{};

	// View and Projection Matrices
	Matrix view = Matrix::identity();
	Matrix projection = Matrix::identity();

	// Update View Matrix
	void updateViewMatrix() { view = Matrix::lookAt(from, to, up); }

	// Set Camera Lense
	void setLense(int _width, int _height, float _zFar, float _zNear, float _fovTheta = 90.f) {
		width = _width;
		height = _height;
		zFar = _zFar;
		zNear = _zNear;
		fovTheta = _fovTheta;

		windowHeightFar = 2.f * zFar * tanf(0.5f * fovTheta * 0.5f * M_PI / 180.f);
		windowHeightNear = 2.f * zNear * tanf(0.5f * fovTheta * 0.5f * M_PI / 180.f);
		projection = Matrix::projection(width, height, zFar, zNear, fovTheta);
	}

	float fovX() const { return 2.f * atanf(0.5 * windowHeightNear / zNear); }
	float getNearWindowWidth() const { return aspectRatio * windowHeightNear; }
	float getFarWindowWidth() const { return aspectRatio * windowHeightFar; }

	// W and S key movements (walk forward/backward)
	void walk(float d) {
		Vec3 dir = (to - from).normalize();
		from += dir * d;
		to += dir * d;
	}

	// A and D key movements (strafe left/right)
	void strafe(float d) {
		Vec3 dir = (to - from).normalize();
		Vec3 right = Cross(up, dir).normalize();
		from += right * d;
		to += right * d;
	}

	// Rotete up and to vector about the from vector
	void pitch(float angle) {
		Vec3 dir = (to - from).normalize();
		Vec3 right = Cross(up, dir).normalize();
		Matrix R = Matrix::rotateAxis(right, angle);
		up = R.mulVec(up).normalize();
		to = R.mulVec(to).normalize();
	}

	//// Rotate the basis vectors about world y-axis
	void rotateY(float angle) {
		Matrix R = Matrix::rotateOnYAxis(angle);
		from = R.mulVec(from).normalize();
		up = R.mulVec(up).normalize();
		to = R.mulVec(to).normalize();
	}
};