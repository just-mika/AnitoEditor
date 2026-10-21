#include "InspectorScreen.h"
#include "SceneManager.h"
#include "GameObjectManager.h"
#include "CameraManager.h"
#include "EditorModule.h"
#include <cmath>

#include "ObjectFactory.h"

#include "Text.h"

#include "PuzzleToken.h"
#include "PuzzleHint.h"

#include "GemInputToken.h"

#include "SpriteAnimator.h"
#include "Animation.h"
#include "SpriteRenderer.h"
#include "TextureManager.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include "Settings.h"


void InspectorScreen::setMousePos(Vector2D mousePos)
{
    this->mousePos = mousePos;
}

InspectorScreen::InspectorScreen() : AUIScreen("INSPECTOR_SCREEN")
{
    this->selectedObject = NULL;
}

InspectorScreen::~InspectorScreen()
{

}

void InspectorScreen::DrawUI()
{
    // Pull this frame's selection from the shared source of truth (EditorModule),
    // rather than tracking our own independent copy.
    this->selectedObject = Editor::EditorModule::getInstance()->getSelectedObject();

    // Floating overlay: only exists while something is selected
    if (!this->selectedObject)
    {
        this->lastSelected = nullptr;
        return;
    }

    if (this->selectedObject != this->lastSelected)
    {
        this->lastSelected = this->selectedObject;
        syncBuffers();
    }

    // Appears at the top-right (below the toolbar) each time it opens,
    // then the user can drag it anywhere.
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + vp->WorkSize.x - 12.0f, vp->WorkPos.y + 68.0f),
        ImGuiCond_Appearing, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(440.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.95f);

    ImGui::Begin("Inspector", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

    // Name + Enabled
    ImGui::SetNextItemWidth(190.0f);
    bool commit = ImGui::InputText("##name", nameBuf, sizeof(nameBuf),
                                   ImGuiInputTextFlags_EnterReturnsTrue);
    commit |= ImGui::IsItemDeactivatedAfterEdit();
    if (commit) applyName();

    ImGui::SameLine();
    bool enabled = selectedObject->getEnabled();
    if (ImGui::Checkbox("Enabled", &enabled))
        selectedObject->setEnabled(enabled);

    ImGui::Spacing();
    ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;
    showTransform(childFlags);      // your existing Position / Scale / Rotation

    showTexture();
    showParent();
    showObjectType();

    showTypeSpecific();

    showChildrenList();   // only draws when the object actually has children

    ImGui::Separator();
    showComponents();

    ImGui::Separator();
    showDelete();

    ImGui::End();

    // Deleting inside the window would leave the rest of this frame using a dead selection
    if (pendingDelete) performDelete();
}

void InspectorScreen::showTransform(ImGuiChildFlags childFlags)
{
    ImGui::BeginChild("Transform", ImVec2(0, 0), childFlags);
    if (this->selectedObject == NULL)
    {
        ImGui::Text("No game object selected.");
    }
    else
    {
        std::string objectText;
        if (this->selectedObject->getIsScreenObject())
            objectText = "Screen Object: " + this->selectedObject->getName();
        else
            objectText = "Game Object: " + this->selectedObject->getName();

        ImGui::Text("%s", objectText.c_str());
        Vector2D pos = this->selectedObject->getPos();
        float rot = this->selectedObject->getRot();
        Vector2D scale = this->selectedObject->getScale();

        float scenePos[] = { pos.x, pos.y };
        float sceneRot = rot;
        float sceneScale[] = { scale.x, scale.y };

        if (ImGui::DragFloat2("Position", scenePos))
        {
            this->selectedObject->setPos(Vector2D(scenePos[0], scenePos[1]));
        }

        if (ImGui::DragFloat2("Scale", sceneScale, 0.01f, 0.0f, 0.0f))
        {
            this->selectedObject->setScale(Vector2D(sceneScale[0], sceneScale[1]));
        }

        if (ImGui::DragFloat("Rotation", &sceneRot))
        {
            this->selectedObject->setRot(sceneRot);
        }
    }
    ImGui::EndChild();
}

// Maps a live object back to its ObjectFactory type key (null if it isn't a palette type).
// None of these classes derive from each other, so test order doesn't matter.

void InspectorScreen::syncBuffers()
{
    memset(nameBuf, 0, sizeof(nameBuf));
    memset(textBuf, 0, sizeof(textBuf));
    if (!selectedObject) return;

    strncpy_s(nameBuf, selectedObject->getName().c_str(), sizeof(nameBuf) - 1);
    if (Text* t = dynamic_cast<Text*>(selectedObject))
    {
        strncpy_s(textBuf, t->getMessage().c_str(), sizeof(textBuf) - 1);
        fontSizeBuf = t->getFontSize();
    }
}

void InspectorScreen::applyName()
{
    GameObjectManager* gom = GameObjectManager::getInstance();
    std::string newName = nameBuf;

    // Reject empty or already-used names, and revert the field
    if (newName.empty() || newName == selectedObject->getName() ||
        gom->findObjectByName(newName))
    {
        syncBuffers();
        return;
    }

    if (selectedObject->getParent()) selectedObject->setName(newName);
    else gom->setObjectName(selectedObject->getName(), newName);
}

void InspectorScreen::showTexture()
{
    SpriteRenderer* sr = (SpriteRenderer*)selectedObject->findComponentByName("SpriteRenderer");
    if (!sr) return;   // Text objects etc. have no texture

    const std::string current = sr->getTextureKey();
    if (ImGui::BeginCombo("Texture", current.c_str()))
    {
        for (const std::string& n : TextureManager::getInstance()->getTextureNames())
        {
            bool sel = (n == current);
            if (ImGui::Selectable(n.c_str(), sel)) sr->setTextureKey(n);
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

// Read-only: reparenting is done by dragging in the Hierarchy. Unparent is a quick escape hatch.
void InspectorScreen::showParent()
{
    AGameObject* current = selectedObject->getParent();
    ImGui::LabelText("Parent", "%s", current ? current->getName().c_str() : "None");
    if (current)
    {
        ImGui::SameLine();
        if (ImGui::SmallButton("Unparent"))
            GameObjectManager::getInstance()->reparentObject(selectedObject, nullptr);
    }
}

void InspectorScreen::showDelete()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.70f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    bool clicked = ImGui::Button("Delete Object", ImVec2(-1.0f, 0.0f));
    ImGui::PopStyleColor(3);
    if (clicked) ImGui::OpenPopup("Delete Object?");

    if (ImGui::BeginPopupModal("Delete Object?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete \"%s\"?", selectedObject->getName().c_str());
        if (!selectedObject->getChildren().empty())
            ImGui::TextDisabled("Its %d child object(s) will be deleted too.", (int)selectedObject->getChildren().size());

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
            pendingDelete = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void InspectorScreen::performDelete()
{
    pendingDelete = false;
    AGameObject* obj = selectedObject;

    // Drop every reference to the selection first; actual deletion is deferred by the manager.
    Editor::EditorModule::getInstance()->setSelectedObject(nullptr);
    this->selectedObject = nullptr;
    this->lastSelected = nullptr;
    if (!obj) return;

    GameObjectManager* gom = GameObjectManager::getInstance();
    // The manager only deletes top-level objects, so lift children out of their parent first.
    if (obj->getParent()) gom->reparentObject(obj, nullptr);
    gom->deleteObject(obj);
}

void InspectorScreen::showObjectType()
{
    const char* key = ObjectFactory::typeKeyOf(selectedObject);
    if (key) ImGui::LabelText("Object Type", "%s", ObjectFactory::getDisplayName(key).c_str());
    else     ImGui::LabelText("Object Type", "%s", "Object");
}

// Lists direct children as buttons so composite UI objects (built in initialize())
// can be drilled into and edited with the generic panels.
void InspectorScreen::showChildrenList()
{
    std::vector<AGameObject*> kids = selectedObject->getChildren();
    if (kids.empty()) return;
    char header[64];
    snprintf(header, sizeof(header), "Children (%d)###ChildrenList", (int)kids.size());

    // Closed by default (no DefaultOpen flag); the ### keeps its open/closed state stable as the count changes
    if (!ImGui::CollapsingHeader(header)) return;

    for (AGameObject* child : kids)
    {
        ImGui::PushID(child);   // names may repeat
        if (ImGui::Selectable(child->getName().c_str()))
        {
            Editor::EditorModule::getInstance()->setSelectedObject(child);
            ImGui::PopID();
            return;   // selection changed; stop using this frame's state
        }
        ImGui::PopID();
    }
}

// Only things the generic panels (transform / texture / parent / component inspectors) can't cover.
// Color, flip and pivot come from SpriteRenderer::drawInspector; size, offset and trigger from
// BoxCollider::drawInspector (which Platform, Gem, Statue and Gate interactables all use).
void InspectorScreen::showTypeSpecific()
{
    const char* key = ObjectFactory::typeKeyOf(selectedObject);
    if (!key) return;
    const std::string k = key;

    ImGui::Separator();

    if (k == "text")
    {
        Text* t = static_cast<Text*>(selectedObject);
        ImGui::InputTextMultiline("Text", textBuf, sizeof(textBuf), ImVec2(0, 70));
        // setMessage() rebuilds the font texture, so apply on click-away, not every keystroke
        if (ImGui::IsItemDeactivatedAfterEdit())
            t->setMessage(textBuf);

        ImGui::DragInt("Font Size", &fontSizeBuf, 1.0f, 4, 512);
        if (ImGui::IsItemDeactivatedAfterEdit())
            t->setFontSize(fontSizeBuf);

        SDL_Color c = t->getColor();
        float col[4] = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
        if (ImGui::ColorEdit4("Text Color", col))
            t->setColor({ (Uint8)(col[0] * 255), (Uint8)(col[1] * 255), (Uint8)(col[2] * 255), (Uint8)(col[3] * 255) });
    }
    else if (k == "animated_sprite")
    {
        SpriteAnimator* anim = (SpriteAnimator*)selectedObject->findComponentByName("SpriteAnimator");
        Animation* cur = anim ? anim->getCurrentAnimation() : nullptr;
        if (!cur) { ImGui::TextDisabled("No animation loaded."); return; }

        int fps = (int)cur->getFrameRate();
        if (ImGui::DragInt("Frame Rate", &fps, 0.25f, 1, 120))
            cur->setFrameRate((Uint8)fps);

        static const char* kTypes[] = { "Once", "Loop", "Yoyo", "Yoyo Once" };
        int type = (int)cur->getType();
        if (ImGui::Combo("Play Mode", &type, kTypes, IM_ARRAYSIZE(kTypes)))
            cur->setType((AnimationType)type);

        if (ImGui::Button("Play"))  anim->play();
        ImGui::SameLine();
        if (ImGui::Button("Pause")) anim->pause();
        ImGui::SameLine();
        if (ImGui::Button("Stop"))  anim->stop();
    }
    else if (k == "puzzle_token")
    {
        PuzzleToken* t = static_cast<PuzzleToken*>(selectedObject);
        int slot = t->getSlot();
        if (ImGui::InputInt("Slot", &slot)) t->setSlot(slot);
        bool dropped = t->getDropped();
        if (ImGui::Checkbox("Dropped", &dropped)) t->setDropped(dropped);
        ImGui::TextWrapped("Name is also this token's texture key.");
    }
    else if (k == "gem_input_token")
    {
        GemInputToken* t = static_cast<GemInputToken*>(selectedObject);
        bool dropped = t->getDropped();
        if (ImGui::Checkbox("Dropped", &dropped)) t->setDropped(dropped);
        ImGui::TextWrapped("Name is also this token's texture key.");
    }
    else if (k == "gem" || k == "statue")
    {
        ImGui::TextWrapped("Name is also this object's texture key (e.g. Gem_Cyan). "
                           "Pickup area is under the collectable/pickup component below.");
    }
    else if (k == "platform")
    {
        ImGui::TextWrapped("Platform size is the collider size under the BoxCollider component below.");
    }
}

void InspectorScreen::showComponents()
{
    // Object-level flags
    bool screenObj = selectedObject->getIsScreenObject();
    ImGui::BeginDisabled();   // changing this mid-edit would break positions, so read-only
    ImGui::Checkbox("Screen Object", &screenObj);
    ImGui::EndDisabled();

    // Components
    int idx = 0;
    for (AComponent* comp : selectedObject->getAllComponents())
    {
        ImGui::PushID(idx++);

        if (ImGui::CollapsingHeader(comp->getName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool en = comp->getEnabled();
            if (ImGui::Checkbox("Enabled", &en)) comp->setEnabled(en);
            comp->drawInspector();
        }

        ImGui::PopID();
    }
}