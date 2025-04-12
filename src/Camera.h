#pragma once
#define _USE_MATH_DEFINES

#include "cyMatrix.h"
#include <cmath>

// defines the view and projection matricies for a scene
class Camera
{
private:
	cy::Matrix4f m_view;
	cy::Matrix4f m_projection;

	float m_fov;
	float m_nearClip;
	float m_farClip;

	float m_camDist;
	float m_xPos;
	float m_yPos;
	float m_zPos;
	float m_zRot;
	float m_yRot;
	float m_xRot;

	int m_imWidth, m_imHeight;
	bool m_ortho;

	// parameters needed for calculating the normal map in the fragment shader
	float m_nearW;
	float m_farW;
	float m_nearH;
	float m_farH;

	void calcView();
	void calcProj();

public:
	Camera(){}
	Camera(int width, int height);

	cy::Matrix4f GetView() { calcView(); return m_view; }
	cy::Matrix4f GetProj() { calcProj(); return m_projection; }
	cy::Matrix4f GetReflView(float height);	// height is y coordinate of our assumed horizontal relfection plane
	float GetFov() const { return m_fov; }

	void ToggleOrtho() { m_ortho = !m_ortho; }

	void SetFov(float newFov) { m_fov = newFov; }
	void SetPos(float x, float y, float z) { m_camDist = z; m_xPos = x; m_yPos = y; m_zPos = z; }

	void UpdateXRot(float delta) { m_xRot += delta; }
	void UpdateYRot(float delta) { m_yRot += delta; }
	void UpdateZRot(float delta) { m_zRot += delta; }
	void UpdateDist(float delta) { m_camDist += delta; }

	float GetImgWidth() { return m_imWidth; }
	float GetImgHeight() { return m_imHeight; }
};

