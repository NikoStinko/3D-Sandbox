#include "SandBoxUI.h"
#include "UiOverlay.h"
#include "ModelManager.h"
#include "Model.h"
#include "Mesh.h"
#include "EditorState.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

SandBoxUI::SandBoxUI(ModelManager* m, EditorState* e, Camera* c)
    : models(m), editor(e), camera(c) {}

void SandBoxUI::openModelBrowser(UiOverlay& overlay) {
    if (!editor || !models) return;
    if (!editor->isInMap) {
        editor->setStatusMessage("Ouvrez/Créez une map avant d'accéder aux modèles", 3.0f);
        return;
    }
    overlay.showOnlyModelBrowser();
}

void SandBoxUI::openScenePanel(UiOverlay& overlay) {
    if (!editor) return;
    overlay.showOnlyScenePanel();
}

void SandBoxUI::openMapPanel(UiOverlay& overlay) {
    overlay.showOnlyMapPanel();
}

void SandBoxUI::draw(GLFWwindow* window) {
    if (!editor || !models || !camera || !window) return;

    // Clic droit: sélectionner sous curseur par triangulation
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        size_t hitIdx = 0;
        glm::vec3 hitPoint(0.0f);
        if (pickUnderCursor(window, hitIdx, hitPoint)) {
            const auto* e = models->getModel(hitIdx);
            if (e) {
                std::string name = e->path;
                auto pos = name.find_last_of("/\\");
                if (pos != std::string::npos) name = name.substr(pos + 1);
                editor->selectObject(hitIdx, name, e->position, e->rotation, e->scale, 1.0f);
                editor->setStatusMessage(std::string("Sélection: ") + name, 2.0f);
                contextIndex = hitIdx;
                ImGui::OpenPopup("ObjectContext");
            }
        } else {
            editor->clearSelection();
        }
    }

    // Popup contextuel
    if (ImGui::BeginPopup("ObjectContext")) {
        if (ImGui::MenuItem("Resize")) {
            showResizePopup = true;
            if (editor->selectedObject) tmpScale = editor->selectedObject->scale;
        }
        if (ImGui::MenuItem("Rotate")) {
            showRotatePopup = true;
            if (editor->selectedObject) tmpRotation = editor->selectedObject->rotation;
        }
        ImGui::EndPopup();
    }

    if (showResizePopup) {
        ImGui::SetNextWindowSize(ImVec2(320, 160), ImGuiCond_Appearing);
        if (ImGui::Begin("Resize Object", &showResizePopup, ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text("Scale");
            ImGui::DragFloat3("##scale", &tmpScale.x, 0.01f, 0.01f, 100.0f);
            if (ImGui::Button("Apply")) {
                if (contextIndex < models->getModelCount() && editor->selectedObject) {
                    auto sel = *editor->selectedObject;
                    models->updateModel(contextIndex, sel.position, sel.rotation, tmpScale, sel.opacity);
                    editor->selectedObject->scale = tmpScale;
                }
                showResizePopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) { showResizePopup = false; }
            ImGui::End();
        }
    }

    if (showRotatePopup) {
        ImGui::SetNextWindowSize(ImVec2(320, 160), ImGuiCond_Appearing);
        if (ImGui::Begin("Rotate Object", &showRotatePopup, ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text("Rotation (deg)");
            ImGui::DragFloat3("##rot", &tmpRotation.x, 0.5f, -360.0f, 360.0f);
            if (ImGui::Button("Apply")) {
                if (contextIndex < models->getModelCount() && editor->selectedObject) {
                    auto sel = *editor->selectedObject;
                    models->updateModel(contextIndex, sel.position, tmpRotation, sel.scale, sel.opacity);
                    editor->selectedObject->rotation = tmpRotation;
                }
                showRotatePopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) { showRotatePopup = false; }
            ImGui::End();
        }
    }
}

bool SandBoxUI::screenRay(GLFWwindow* window, glm::vec3& outOrigin, glm::vec3& outDir) const {
    int width = 1, height = 1;
    glfwGetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0) return false;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    // NDC
    float x = float(2.0 * mx / width - 1.0);
    float y = float(1.0 - 2.0 * my / height);
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // Matrices
    float aspect = float(width) / float(height);
    glm::mat4 proj = glm::perspective(glm::radians(camera->Zoom), aspect, 0.1f, 1000.0f);
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 invPV = glm::inverse(proj * view);

    glm::vec4 rayEye = glm::inverse(proj) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec4 rayWorld4 = glm::inverse(view) * rayEye;
    glm::vec3 rayWorld = glm::normalize(glm::vec3(rayWorld4));

    outOrigin = camera->Position;
    outDir = rayWorld;
    return true;
}

bool SandBoxUI::intersectRayTriangle(const glm::vec3& orig, const glm::vec3& dir,
                                     const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                     float& t, float& u, float& v) {
    const float EPS = 1e-6f;
    glm::vec3 e1 = v1 - v0;
    glm::vec3 e2 = v2 - v0;
    glm::vec3 p = glm::cross(dir, e2);
    float det = glm::dot(e1, p);
    if (fabs(det) < EPS) return false;
    float invDet = 1.0f / det;
    glm::vec3 s = orig - v0;
    u = glm::dot(s, p) * invDet;
    if (u < 0.0f || u > 1.0f) return false;
    glm::vec3 q = glm::cross(s, e1);
    v = glm::dot(dir, q) * invDet;
    if (v < 0.0f || u + v > 1.0f) return false;
    t = glm::dot(e2, q) * invDet;
    return t > EPS;
}

bool SandBoxUI::pickUnderCursor(GLFWwindow* window, size_t& outIndex, glm::vec3& outHitPoint) {
    glm::vec3 ro, rd;
    if (!screenRay(window, ro, rd)) return false;

    float closestT = std::numeric_limits<float>::max();
    bool hit = false;
    size_t bestIdx = 0;

    const size_t count = models->getModelCount();
    for (size_t i = 0; i < count; ++i) {
        const auto* entry = models->getModel(i);
        if (!entry || !entry->model) continue;

        // Model matrix (comme dans draw)
        glm::mat4 M(1.0f);
        M = glm::translate(M, entry->position);
        M = glm::rotate(M, glm::radians(entry->rotation.x), glm::vec3(1,0,0));
        M = glm::rotate(M, glm::radians(entry->rotation.y), glm::vec3(0,1,0));
        M = glm::rotate(M, glm::radians(entry->rotation.z), glm::vec3(0,0,1));
        M = glm::scale(M, entry->scale == glm::vec3(0.0f) ? glm::vec3(1.0f) : entry->scale);

        for (const auto& mesh : entry->model->meshes) {
            const auto& idx = mesh.indices;
            const auto& verts = mesh.vertices;
            for (size_t k = 0; k + 2 < idx.size(); k += 3) {
                glm::vec3 v0 = glm::vec3(M * glm::vec4(verts[idx[k + 0]].Position, 1.0f));
                glm::vec3 v1 = glm::vec3(M * glm::vec4(verts[idx[k + 1]].Position, 1.0f));
                glm::vec3 v2 = glm::vec3(M * glm::vec4(verts[idx[k + 2]].Position, 1.0f));
                float t, u, v;
                if (intersectRayTriangle(ro, rd, v0, v1, v2, t, u, v)) {
                    if (t < closestT) {
                        closestT = t;
                        bestIdx = i;
                        outHitPoint = ro + rd * t;
                        hit = true;
                    }
                }
            }
        }
    }

    if (hit) outIndex = bestIdx;
    return hit;
}