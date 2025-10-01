//
// Created by ghima on 28-09-2025.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GizmoDragController {
public:
    struct DragState {
        bool active = false;
        int axis = 0;                     // 1=X, 2=Y, 3=Z
        glm::vec3 startObjectPos;         // Position at drag start
        glm::vec3 startObjectScale;       // Scale at the start of the drag;
        glm::vec3 startObjectRotation;    // Rotation at the start of the drag;
        glm::vec3 startVector;            // Start vector for rotation
        glm::vec3 axisDir;                // Axis direction (world space)
        glm::vec3 planeNormal;            // Plane normal for intersection
        float startT = 0.0f;              // Initial axis coordinate
    };

    GizmoDragController() = default;

    void BeginDrag(int axis, const glm::vec3 &objectPos, const glm::vec3 &startScale, const glm::vec3 &startRotation,
                   const glm::mat4 &view, const glm::mat4 &proj,
                   float mouseX, float mouseY, float vpWidth, float vpHeight,
                   const glm::vec3 &camForward) {
        m_drag.active = true;
        m_drag.axis = axis;
        m_drag.startObjectPos = objectPos;
        m_drag.startObjectScale = startScale;
        m_drag.startObjectRotation = startRotation;

        // World-space axis directions
        m_drag.axisDir = (axis == 1) ? glm::vec3(1, 0, 0)
                                     : (axis == 2) ? glm::vec3(0, 1, 0)
                                                   : glm::vec3(0, 0, 1);

        // Plane normal = camera forward projected perpendicular to axis
        glm::vec3 V = glm::normalize(camForward);
        glm::vec3 Vperp = V - m_drag.axisDir * glm::dot(m_drag.axisDir, V);
        if (glm::length(Vperp) < 1e-8f) // Fallback if camera is aligned
            Vperp = glm::cross(m_drag.axisDir, glm::vec3(0, 1, 0));

        m_drag.planeNormal = glm::normalize(Vperp);

        // Mouse ray → intersection with plane
        glm::vec3 rayOrigin, rayDir;
        BuildMouseRay(mouseX, mouseY, vpWidth, vpHeight, view, proj, rayOrigin, rayDir);
        glm::vec3 P0 = RayPlaneIntersection(rayOrigin, rayDir, objectPos, m_drag.planeNormal);
        m_drag.startVector = glm::normalize(P0 - objectPos);

        m_drag.startT = glm::dot(P0 - objectPos, m_drag.axisDir);
    }

    glm::vec3 UpdateDrag(float mouseX, float mouseY, float vpWidth, float vpHeight,
                         const glm::mat4 &view, const glm::mat4 &proj) {
        if (!m_drag.active) return m_drag.startObjectPos;

        glm::vec3 rayOrigin, rayDir;
        BuildMouseRay(mouseX, mouseY, vpWidth, vpHeight, view, proj, rayOrigin, rayDir);

        glm::vec3 P = RayPlaneIntersection(rayOrigin, rayDir, m_drag.startObjectPos, m_drag.planeNormal);
        float t = glm::dot(P - m_drag.startObjectPos, m_drag.axisDir);

        float delta = m_drag.axis == 2 ? m_drag.startT - t : t - m_drag.startT;
        return m_drag.startObjectPos + delta * m_drag.axisDir;
    }

    glm::vec3 UpdateScale(float mouseX, float mouseY, float vpWidth, float vpHeight,
                          const glm::mat4 &view, const glm::mat4 &proj) {
        if (!m_drag.active) return m_drag.startObjectScale;

        glm::vec3 rayOrigin, rayDir;
        BuildMouseRay(mouseX, mouseY, vpWidth, vpHeight, view, proj, rayOrigin, rayDir);

        glm::vec3 P = RayPlaneIntersection(rayOrigin, rayDir, m_drag.startObjectPos, m_drag.planeNormal);
        float t = glm::dot(P - m_drag.startObjectPos, m_drag.axisDir);

        float delta = m_drag.axis == 2 ? m_drag.startT - t : t - m_drag.startT;
        float factor = 1.0f + delta;
        glm::vec3 newScale = m_drag.startObjectScale;
        if (m_drag.axis == 1) newScale.x *= factor;
        if (m_drag.axis == 2) newScale.y *= factor;
        if (m_drag.axis == 3) newScale.z *= factor;

        return newScale;
    }

    glm::vec3 UpdateRotation(float mouseX, float mouseY, float vpWidth, float vpHeight,
                             const glm::mat4 &view, const glm::mat4 &proj) {
        if (!m_drag.active) return m_drag.startObjectRotation;

        // Mouse ray → plane intersection
        glm::vec3 rayOrigin, rayDir;
        BuildMouseRay(mouseX, mouseY, vpWidth, vpHeight, view, proj, rayOrigin, rayDir);
        glm::vec3 P = RayPlaneIntersection(rayOrigin, rayDir, m_drag.startObjectPos, m_drag.planeNormal);

        glm::vec3 currentVector = glm::normalize(P - m_drag.startObjectPos);

        // Compute angle between start and current vectors
        float cosTheta = glm::dot(m_drag.startVector, currentVector);
        cosTheta = glm::clamp(cosTheta, -1.0f, 1.0f);
        float angle = acosf(cosTheta); // radians

        // Determine sign of angle using cross product
        glm::vec3 cross = glm::cross(m_drag.startVector, currentVector);
        float sign = glm::dot(cross, m_drag.axisDir) < 0 ? -1.0f : 1.0f;

        float deltaAngle = angle * sign; // radians

        // Apply delta to object’s start rotation
        glm::vec3 newRotation = m_drag.startObjectRotation;
        if (m_drag.axis == 1) newRotation.x += glm::degrees(deltaAngle) * 2;
        if (m_drag.axis == 2) newRotation.y += glm::degrees(deltaAngle) * 2;
        if (m_drag.axis == 3) newRotation.z += glm::degrees(deltaAngle) * 2;

        return newRotation;
    }

    void EndDrag() { m_drag.active = false; }

    bool IsDragging() const { return m_drag.active; }

private:
    DragState m_drag;

    // Mouse → world ray
    void BuildMouseRay(float mouseX, float mouseY, float vpWidth, float vpHeight,
                       const glm::mat4 &view, const glm::mat4 &proj,
                       glm::vec3 &rayOrigin, glm::vec3 &rayDir) {
        float xNdc = 2.0f * (mouseX / vpWidth) - 1.0f;
        float yNdc = 1.0f - 2.0f * (mouseY / vpHeight);

        glm::mat4 invVP = glm::inverse(proj * view);
        glm::vec4 nearH = invVP * glm::vec4(xNdc, yNdc, 0, 1);
        glm::vec4 farH = invVP * glm::vec4(xNdc, yNdc, 1, 1);

        glm::vec3 nearW = glm::vec3(nearH) / nearH.w;
        glm::vec3 farW = glm::vec3(farH) / farH.w;

        rayOrigin = nearW;
        rayDir = glm::normalize(farW - nearW);
    }

    // Ray-plane intersection
    glm::vec3 RayPlaneIntersection(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir,
                                   const glm::vec3 &planePoint, const glm::vec3 &planeNormal) {
        float denom = glm::dot(planeNormal, rayDir);
        if (fabs(denom) < 1e-6f) return rayOrigin;
        float t = glm::dot(planeNormal, planePoint - rayOrigin) / denom;
        return rayOrigin + t * rayDir;
    }
};
