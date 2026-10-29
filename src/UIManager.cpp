#include "UIManager.h"
#include "CustomStyles.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "EditorMode.h"
#include "InspectorScreen.h"
#include "HierarchyScreen.h"
#include "ObjectPalette.h"
#include "EditorToolbarScreen.h"
#include "Settings.h"

UIManager* UIManager::sharedInstance = nullptr;

UIManager* UIManager::getInstance()
{
	return sharedInstance;
}

void UIManager::initialize(SDL_Window* window, SDL_Renderer* renderer)
{
	sharedInstance = new UIManager(window, renderer);
}

void UIManager::destroy()
{
	if (!sharedInstance->uiList.empty())
	{
		for (int i = sharedInstance->uiList.size() - 1; i >= 0; i--)
			delete sharedInstance->uiList[i];

		sharedInstance->uiTable.clear();
		sharedInstance->uiList.clear();
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	delete sharedInstance;
}

void UIManager::newFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(uiWidth, uiHeight);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	if (SDL_GetMouseFocus() == this->pWindow)
	{
		float mx, my, lx, ly;
		SDL_GetMouseState(&mx, &my);
		SDL_RenderCoordinatesFromWindow(this->pRenderer, mx, my, &lx, &ly);
		io.AddMousePosEvent(lx, ly);   // queued last, so it wins
	}

    ImGui::NewFrame();
}

void UIManager::processEvent(const SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);

    switch (event->type)
    {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
        {
            float winX, winY;
            if (event->type == SDL_EVENT_FINGER_DOWN ||
                event->type == SDL_EVENT_FINGER_UP ||
                event->type == SDL_EVENT_FINGER_MOTION)
            {
                int winW, winH;
                SDL_GetWindowSize(this->pWindow, &winW, &winH);
                winX = event->tfinger.x * winW;
                winY = event->tfinger.y * winH;
            }
            else
            {
                winX = event->motion.x;
                winY = event->motion.y;
            }

            float logicalX, logicalY;
            SDL_RenderCoordinatesFromWindow(this->pRenderer, winX, winY, &logicalX, &logicalY);
            ImGui::GetIO().AddMousePosEvent(logicalX, logicalY);
            break;
        }
        default:
            break;
    }
}
void UIManager::drawAllUI(SDL_Renderer* renderer)
{
	//ImGui::ShowDemoWindow();
	for (int i = 0; i < this->uiList.size(); i++)
	{
		if (this->uiList[i]->enabled)
			this->uiList[i]->draw();
	}

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

	//ImGuiIO io = ImGui::GetIO();
	//SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
}

AUIScreen* UIManager::getUIScreen(std::string name)
{
	return this->uiTable[name];
}

bool UIManager::getEnabled(std::string name)
{
	AUIScreen* ui = this->uiTable[name];
	return ui ? ui->enabled : false;
}

void UIManager::setEnabled(std::string name, bool enabled)
{
	AUIScreen* ui = this->uiTable[name];
	if (ui) ui->enabled = enabled;
}

UIManager::UIManager(SDL_Window* window, SDL_Renderer* renderer)
{
    this->pWindow = window;
    this->pRenderer = renderer;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	CustomStyles styles;
	styles.StyleColorsCrimson();

	io.FontGlobalScale = 1.4f;

	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	UINames uiNames;

#if EDITOR_MODE
	InspectorScreen* inspectorScreen = new InspectorScreen();
	this->uiTable[uiNames.INSPECTOR_SCREEN] = inspectorScreen;
	this->uiList.push_back(inspectorScreen);

	HierarchyScreen* hierarchyScreen = new HierarchyScreen();
	this->uiTable[uiNames.HIERARCHY_SCREEN] = hierarchyScreen;
	this->uiList.push_back(hierarchyScreen);

	ObjectPalette* objectPalette = new ObjectPalette();
	this->uiTable[uiNames.OBJECT_PALETTE] = objectPalette;
	this->uiList.push_back(objectPalette);

    EditorToolbarScreen* toolbarScreen = new EditorToolbarScreen();
    this->uiTable[uiNames.EDITOR_TOOLBAR_SCREEN] = toolbarScreen;
    this->uiList.push_back(toolbarScreen);
#endif // EDITOR_MODE

}

UIManager::~UIManager()
{

}
