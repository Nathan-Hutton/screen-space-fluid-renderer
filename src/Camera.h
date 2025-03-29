#pragma once
#define _USE_MATH_DEFINES

#include "cyMatrix.h"
#include <cmath>

// defines the view and projection matricies for a scene
class Camera
{
private:
	cy::Matrix4f view;
	cy::Matrix4f projection;

	float fov;
	float nearClip;
	float farClip;

	float camDist;
	float zRot;
	float yRot;
	float xRot;

	int imWidth, imHeight;
	bool ortho;

	void calcView();
	void calcProj();

public:
	Camera(){}
	Camera(int width, int height);

	cy::Matrix4f GetView() { calcView(); return view; }
	cy::Matrix4f GetProj() { calcProj(); return projection; }
	cy::Matrix4f GetReflView(float height);	// height is y coordinate of our assumed horizontal relfection plane
	float GetFov() { return fov; }

	void ToggleOrtho() { ortho = !ortho; }

	void SetFov(float newFov) { fov = newFov; }
	void SetDist(float dist) { camDist = dist; }

	void UpdateXRot(float delta) { xRot += delta; }
	void UpdateYRot(float delta) { yRot += delta; }
	void UpdateZRot(float delta) { zRot += delta; }
	void UpdateDist(float delta) { camDist += delta; }
};

