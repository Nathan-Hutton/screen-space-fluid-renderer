#include "Camera.h"

//Camera::Camera() {
//    fov = M_PI / 3;
//    nearClip = 0.5;
//    farClip = 1000.0;
//
//    camDist = 30;
//    zRot = 0;
//    //xRot = -M_PI/2;
//    yRot = 0;
//    xRot = 0;
//
//    imWidth = -1;
//    imHeight = -1;
//
//    ortho = false;
//
//    calcView();
//    calcProj();
//}

Camera::Camera(int width, int height) {
    m_fov = M_PI / 3;
    m_nearClip = 0.001;
    m_farClip = 1000.0;

    m_camDist = 30;
    m_zRot = 0;
    //xRot = -M_PI/2;
    m_yRot = 0;
    m_xRot = 0;

    m_imWidth = width;
    m_imHeight = height;

    m_ortho = false;

    calcView();
    calcProj();
}

void Camera::calcView() {
    // camera transform
    cy::Matrix4f camTrans = cy::Matrix4f();
    camTrans.SetTranslation(cy::Vec3f(m_xPos, m_yPos, m_zPos));

    cy::Matrix4f camXRot = cy::Matrix4f();
    camXRot.SetRotationX(m_xRot);

    cy::Matrix4f camYRot = cy::Matrix4f();
    camYRot.SetRotationY(m_yRot);

    cy::Matrix4f camZRot = cy::Matrix4f();
    camZRot.SetRotationZ(m_zRot);

    m_view = camTrans * camXRot * camYRot * camZRot;
}

void Camera::calcProj() {
    m_projection.Zero();

    if (m_ortho) {
        const float r = 2 * m_camDist * std::tan(m_fov / 2);
        const float l = -r;
        const float t = r * m_imHeight / m_imWidth;
        const float b = -t;

        m_projection[0] = 2 / (r - l);
        m_projection[5] = 2 / (t - b);
        m_projection[10] = -2 / (m_farClip - m_nearClip);
        m_projection[15] = 1;

        m_projection[14] = -((m_farClip + m_nearClip) / (m_farClip - m_nearClip));

    }
    else {
        const float aspect = float(m_imWidth) / m_imHeight;
        m_projection.SetPerspective(m_fov, aspect, m_nearClip, m_farClip);
    }
}

cy::Matrix4f Camera::GetReflView(float height)
{
    cy::Matrix4f flip = cy::Matrix4f().Identity();
    flip.cell[5] = -1.0f;

    cy::Matrix4f camTrans = cy::Matrix4f();
    camTrans.SetTranslation(cy::Vec3f(0, 0, -m_camDist));

    cy::Matrix4f camXRot = cy::Matrix4f();
    camXRot.SetRotationX(m_xRot);

    cy::Matrix4f camYRot = cy::Matrix4f();
    camYRot.SetRotationY(m_yRot);

    cy::Matrix4f camZRot = cy::Matrix4f();
    camZRot.SetRotationZ(m_zRot);
    
    cy::Matrix4f postTrans = cy::Matrix4f();
    postTrans.SetTranslation(cy::Vec3f(0, 2*height, 0));

    return camTrans * camXRot * camYRot * camZRot * postTrans * flip;
}
