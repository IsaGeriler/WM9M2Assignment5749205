#pragma once

#include "MyMath.h"

class Camera {
public:
	// Camera Attributes
	Vec3 position = Vec3(0.f, 0.f, 0.f);  // Camera Position (eye/from)
	Vec3 right = Vec3(1.f, 0.f, 0.f);
	Vec3 up = Vec3(0.f, 1.f, 0.f);
	Vec3 dir = Vec3(0.f, 0.f, 1.f);

	// Frustrum Attributes
	int width{}, height{};
	float zFar{}, zNear{}, aspectRatio{}, fovTheta{};

	// View and Projection Matrices
	Matrix view = Matrix::identity();
	Matrix projection = Matrix::identity();

	// Update View Matrix
	void updateViewMatrix() {
		Vec3 _right = right;
		Vec3 _up = up;
		Vec3 _dir = dir;

		_dir = _dir.normalize();
		_up = Cross(_dir, _right).normalize();
		_right = Cross(_up, _dir);

		right = _right;
		dir = _dir;
		up = _up;

		view = Matrix::lookAt(position, right, up, dir);
	}

	// Set Camera
	void setCamera(int _width, int _height, float _zFar, float _zNear, float _fovTheta = 90.f) {
		width = _width;
		height = _height;
		zFar = _zFar;
		zNear = _zNear;
		fovTheta = _fovTheta;
		projection = Matrix::projection(width, height, zFar, zNear, fovTheta);
	}

	void walk(float d) { position += dir * d; position.y = 0.f; }	   // W and S key movements (walk forward/backward)
	void strafe(float d) { position += right * d; position.y = 0.f; }  // A and D key movements (strafe left/right)

	// Rotete up and dir/to vector about the right/from vector
	void pitch(float angle) {
		Matrix R = Matrix::rotateAxis(right, angle);
		up = R.mulVec(up).normalize();
		dir = R.mulVec(dir).normalize();
	}

	// Rotate the basis vectors about world y-axis
	void rotateY(float angle) {
		Matrix R = Matrix::rotateOnYAxis(angle);
		right = R.mulVec(right).normalize();
		up = R.mulVec(up).normalize();
		dir = R.mulVec(dir).normalize();
	}
	
	// Reset Camera
	void resetCamera() {
		position = Vec3(0.f, 0.f, 0.f);
		right = Vec3(1.f, 0.f, 0.f);
		up = Vec3(0.f, 1.f, 0.f);
		dir = Vec3(0.f, 0.f, 1.f);
	}
};