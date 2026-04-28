#include "Camera.h"
#include <cmath>

Camera::Camera() : m_x(0), m_y(0) {}

void Camera::update(float targetX, float targetY, float dt) {
    // Smooth lerp toward player center
    float speed = LERP * dt;
    if (speed > 1.0f) speed = 1.0f;
    m_x += (targetX - m_x) * speed;
    m_y += (targetY - m_y) * speed;
}
