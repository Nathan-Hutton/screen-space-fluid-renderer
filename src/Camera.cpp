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
    fov = M_PI / 3;
    nearClip = 0.001;
    farClip = 1000.0;

    camDist = 30;
    zRot = 0;
    //xRot = -M_PI/2;
    yRot = 0;
    xRot = 0;

    imWidth = width;
    imHeight = height;

    ortho = false;

    calcView();
    calcProj();
}

void Camera::calcView() {
    // camera transform
    cy::Matrix4f camTrans = cy::Matrix4f();
    camTrans.SetTranslation(cy::Vec3f(xpos, ypos, zpos));

    cy::Matrix4f camXRot = cy::Matrix4f();
    camXRot.SetRotationX(xRot);

    cy::Matrix4f camYRot = cy::Matrix4f();
    camYRot.SetRotationY(yRot);

    cy::Matrix4f camZRot = cy::Matrix4f();
    camZRot.SetRotationZ(zRot);

    view = camTrans * camXRot * camYRot * camZRot;
}

void Camera::calcProj() {
    projection.Zero();

    if (ortho) {
        float r = 2 * camDist * std::tan(fov / 2);
        float l = -r;
        float t = r * imHeight / imWidth;
        float b = -t;

        projection[0] = 2 / (r - l);
        projection[5] = 2 / (t - b);
        projection[10] = -2 / (farClip - nearClip);
        projection[15] = 1;

        projection[14] = -((farClip + nearClip) / (farClip - nearClip));

    }
    else {
        float aspect = float(imWidth) / imHeight;
        projection.SetPerspective(fov, aspect, nearClip, farClip);
    }
}

cy::Matrix4f Camera::GetReflView(float height)
{
    cy::Matrix4f flip = cy::Matrix4f().Identity();
    flip.cell[5] = -1.0f;

    cy::Matrix4f camTrans = cy::Matrix4f();
    camTrans.SetTranslation(cy::Vec3f(0, 0, -camDist));

    cy::Matrix4f camXRot = cy::Matrix4f();
    camXRot.SetRotationX(xRot);

    cy::Matrix4f camYRot = cy::Matrix4f();
    camYRot.SetRotationY(yRot);

    cy::Matrix4f camZRot = cy::Matrix4f();
    camZRot.SetRotationZ(zRot);
    
    cy::Matrix4f postTrans = cy::Matrix4f();
    postTrans.SetTranslation(cy::Vec3f(0, 2*height, 0));

    return camTrans * camXRot * camYRot * camZRot * postTrans * flip;
}
