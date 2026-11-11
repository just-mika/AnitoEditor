#include "EditorMode.h"
#if EDITOR_MODE

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

#include "Gem.h"
#include "LevelDataManager.h"
#include "MetricsManager.h"
#include "Settings.h"
#include "TileMap.h"


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

void InspectorScreen::draw()
{
    // Pull this frame's selection from the shared source of truth (EditorModule),
    // rather than tracking our own independent copy.
    this->selectedObject = Editor::EditorModule::getInstance()->getSelectedObject();

    // the selection changed while typing: apply the text to the object it was typed for,
	// before syncBuffers() replaces the buffer with the new selection's text
    if (this->pEditingText && this->pEditingText != this->selectedObject)
    {
        if (this->pEditingText->getMessage() != textBuf)
            this->pEditingText->setMessage(textBuf);
        this->pEditingText = nullptr;
    }

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
	// these objects are identified by their name, so renaming them would break them:
	// the tilemap (found by name, saved as a path), gems and statues (name = which gem/statue)
    const char* nameKey = ObjectFactory::typeKeyOf(selectedObject);
    bool bLockName = dynamic_cast<TileMap*>(selectedObject) != nullptr ||
        (nameKey && (std::string(nameKey) == "gem" || std::string(nameKey) == "statue"));

    ImGui::BeginDisabled(bLockName);
    ImGui::SetNextItemWidth(190.0f);
    bool commit = ImGui::InputText("##name", nameBuf, sizeof(nameBuf),
        ImGuiInputTextFlags_EnterReturnsTrue);
    commit |= ImGui::IsItemDeactivatedAfterEdit();

    // explain why the name is locked
    // (disabled items don't report hovering unless AllowWhenDisabled is passed)
    if (bLockName && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        if (dynamic_cast<TileMap*>(selectedObject))
            ImGui::SetTooltip("The tilemap is found by this name, so it can't be renamed.");
        else
            ImGui::SetTooltip("This name decides which %s this is (e.g. %s), so it can't be renamed.",
                nameKey, selectedObject->getName().c_str());
    }
    ImGui::EndDisabled();
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
    memset(gemTitleBuf, 0, sizeof(gemTitleBuf));
    memset(gemTextBuf, 0, sizeof(gemTextBuf));

    if (!selectedObject) return;

    strncpy_s(nameBuf, selectedObject->getName().c_str(), sizeof(nameBuf) - 1);
    if (Text* t = dynamic_cast<Text*>(selectedObject))
    {
        strncpy_s(textBuf, t->getMessage().c_str(), sizeof(textBuf) - 1);
        fontSizeBuf = t->getFontSize();
    }
    if (Gem* g = dynamic_cast<Gem*>(selectedObject))
    {
        strncpy_s(gemTitleBuf, g->getPickupTitle().c_str(), sizeof(gemTitleBuf) - 1);
        strncpy_s(gemTextBuf, g->getPickupText().c_str(), sizeof(gemTextBuf) - 1);
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

    //show the file the texture came from (scene keys are an internal detail)
    const std::string key = sr->getTextureKey();
    std::string shown = TextureManager::getInstance()->getPathOfKey(key);
    if (shown.empty()) shown = key;
    ImGui::LabelText("Texture", "%s", shown.empty() ? "(none)" : shown.c_str());

    if (ImGui::Button("Browse...", ImVec2(-FLT_MIN, 0)))
        assetBrowser.open();
    assetBrowser.draw();
}

// Read-only: reparenting is done by dragging in the Hierarchy. Unparent is a quick escape hatch.
void InspectorScreen::showParent()
{
    AGameObject* current = selectedObject->getParent();
    ImGui::LabelText("Parent", "%s", current ? current->getName().c_str() : "None");
    if (current)
    {
        ImGui::SameLine();
        ImGui::BeginDisabled(ObjectFactory::isInsideComposite(selectedObject));
        if (ImGui::SmallButton("Unparent"))
            GameObjectManager::getInstance()->reparentObject(selectedObject, current->getParent());
        ImGui::EndDisabled();
    }
}

void InspectorScreen::showDelete()
{
    if (selectedObject->isSystemObject())
        return;

    if (ObjectFactory::isInsideComposite(selectedObject))
    {
        ImGui::TextDisabled("Part of a built-in screen: edit its properties above");
        return;
    }

    //gems and statue are unique and named after their texture, so they cant be copied
    const char* key = ObjectFactory::typeKeyOf(selectedObject);
    bool bCanDuplicate = true;
    for (const auto& info : ObjectFactory::getRegisteredTypes())
        if (key && info.typeKey == key && !info.variants.empty())
            bCanDuplicate = false;

    if (ObjectFactory::isInsideComposite(selectedObject))
        bCanDuplicate = false;

    float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

    ImGui::BeginDisabled(!bCanDuplicate);
    if (ImGui::Button("Duplicate", ImVec2(half, 0.0f)))
    {
        if (AGameObject* pCopy = LevelDataManager::getInstance()->duplicateObject(selectedObject))
            Editor::EditorModule::getInstance()->setSelectedObject(pCopy);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.70f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    bool clicked = ImGui::Button("Delete", ImVec2(-1.0f, 0.0f));
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
        ImGui::InputTextMultiline("Text", textBuf, sizeof(textBuf), ImVec2(-FLT_MIN, 200));
        if (ImGui::IsItemActive())
            this->pEditingText = t;   //remember who we're typing for
        // setMessage() rebuilds the font texture, so apply on click-away, not every keystroke
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            t->setMessage(textBuf);
            this->pEditingText = nullptr;
        }

        ImGui::DragInt("Font Size", &fontSizeBuf, 1.0f, 4, 512);
        if (ImGui::IsItemDeactivatedAfterEdit())
            t->setFontSize(fontSizeBuf);

        int wrap = t->getWrapWidth();
        ImGui::DragInt("Wrap Width", &wrap, 5.0f, 0, 4000);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Break lines wider than this (0 = only at line breaks)");
        if (ImGui::IsItemDeactivatedAfterEdit())
            t->setWrapWidth(wrap);

        // the fonts in Assets/Fonts
        static const char* kFonts[] = { "JainiPurva-Regular.ttf", "Maragsa.otf" };
        if (ImGui::BeginCombo("Font", t->getFontName().c_str()))
        {
            for (const char* f : kFonts)
                if (ImGui::Selectable(f, t->getFontName() == f))
                    t->setFontName(f);
            ImGui::EndCombo();
        }

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
        if (ImGui::Button("Stop"))
        {
            cur->stopAtEnd();
            anim->perform();   //apply the frame to the sprite right away (not playing, so it doesn't advance)
        }
    }
    else if (k == "puzzle_token")
    {
        PuzzleToken* t = static_cast<PuzzleToken*>(selectedObject);

        // the sprite this token must be placed on to count as correct
        std::string current = t->getTargetName();
        if (ImGui::BeginCombo("Target tile", current.empty() ? "(none)" : current.c_str()))
        {
            if (ImGui::Selectable("(none)", current.empty()))
                t->setTargetName("");

            for (AGameObject* o : GameObjectManager::getInstance()->getAllObjectsRecursively())
            {
                const char* key = ObjectFactory::typeKeyOf(o);
                if (!key || (std::string(key) != "sprite" && std::string(key) != "empty")) continue;
                if (ImGui::Selectable(o->getName().c_str(), o->getName() == current))
                    t->setTargetName(o->getName());
            }

            ImGui::EndCombo();
        }

        char buf[1024];
        snprintf(buf, sizeof(buf), "%s", t->getHint().c_str());
        if (ImGui::InputTextMultiline("Hint", buf, sizeof(buf))) t->setHint(buf);
    }
    else if (k == "gem_input_token")
    {
        GemInputToken* t = static_cast<GemInputToken*>(selectedObject);
        bool dropped = t->getDropped();
        if (ImGui::Checkbox("Dropped", &dropped)) t->setDropped(dropped);
        ImGui::TextWrapped("Name is also this token's texture key.");
    }
    else if (k == "gem")
    {
        Gem* g = static_cast<Gem*>(selectedObject);
        ImGui::TextDisabled("Shown when picked up (empty = default)");

        // filled from the gem every frame, like the token's Hint box above;
        // plain strings (no texture to rebuild), so apply on every keystroke
        char titleBuf[128];
        snprintf(titleBuf, sizeof(titleBuf), "%s", g->getPickupTitle().c_str());
        if (ImGui::InputText("Title", titleBuf, sizeof(titleBuf)))
            g->setPickupTitle(titleBuf);

        char textBuf[1024];
        snprintf(textBuf, sizeof(textBuf), "%s", g->getPickupText().c_str());
        if (ImGui::InputTextMultiline("Pickup Text", textBuf, sizeof(textBuf), ImVec2(-FLT_MIN, 80)))
            g->setPickupText(textBuf);
    }
    else if (k == "platform")
    {
        ImGui::TextWrapped("Platform size is the collider size under the BoxCollider component below.");
    }
    else if (k == "gui_button")
    {
        GUIToggle* t = (GUIToggle*)selectedObject->findComponentByName("GUIToggle");
        const char* current = "Nothing";

        if (t && GUIToggle::nameOf(t->getKey()))
            current = GUIToggle::nameOf(t->getKey());


        if (ImGui::BeginCombo("On click", current))
        {
            if (ImGui::Selectable("Nothing", t == nullptr) && t)
            {
                selectedObject->detachComponent(t);
                delete t; //detachComponent only removes it from the list
                t = nullptr;
            }

            for (int i = 0; i < GUIToggle::OPTION_COUNT; i++)
            {
                const GUIToggle::ClickOption& opt = GUIToggle::OPTIONS[i];
                bool bSelected = (t && t->getKey() == opt.key);
                if (ImGui::Selectable(opt.name, bSelected))
                {
                    if (t)
                        t->setKey(opt.key);
                    else
                        selectedObject->attachComponent(new GUIToggle(opt.key));
                }
            }

            ImGui::EndCombo();
        }
    }
}

void InspectorScreen::showComponents()
{
    //screen obj: editable on root objects w/o children
    bool screenObj = selectedObject->getIsScreenObject();
    bool bCanSwitch = !selectedObject->getParent() && selectedObject->getChildren().empty();

    ImGui::BeginDisabled(!bCanSwitch);
    if (ImGui::Checkbox("Screen Object", &screenObj))
    {
        //keep it where it is on screen: convert its position to the other space
        Camera* pCam = CameraManager::getInstance()->getCurrentCamera();
        Vector2D pos = selectedObject->getPos();

        if (screenObj)
            pos = pCam->worldToCanvasPoint(pos); //world -> screen space
        else
            pos = pCam->canvasToWorldPoint(pos); //screen space -> world

        selectedObject->setIsScreenObject(screenObj);
        selectedObject->setPos(pos);
    }
    ImGui::EndDisabled();

    if (!bCanSwitch && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Only objects without a parent or children can switch");

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

#endif