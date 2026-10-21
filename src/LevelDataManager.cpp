#include "LevelDataManager.h"
#include "GameObjectManager.h"
#include "ObjectFactory.h"
#include "SpriteRenderer.h"
#include "Text.h"
#include "EditorMode.h"
#include "SceneManager.h"

#if EDITOR_MODE
#include "EditorModule.h"
#endif

#include <SDL3/SDL.h>
#include <unordered_set>
#include <algorithm>

using json = nlohmann::json;
LevelDataManager::LevelDataManager() {

}

LevelDataManager* LevelDataManager::P_SHARED_INSTANCE = nullptr;
void LevelDataManager::initialize()
{
	P_SHARED_INSTANCE = new LevelDataManager();
}

void LevelDataManager::destroy()
{
	delete P_SHARED_INSTANCE;
	P_SHARED_INSTANCE = nullptr;
}

LevelDataManager* LevelDataManager::getInstance()
{
	return P_SHARED_INSTANCE;
}

bool LevelDataManager::saveLevel(const std::string& fileName)
{
	if (!activeScene)
	{
		SDL_Log("[LevelData] no level scene is active");
		return false;
	}
#if EDITOR_MODE
	if (Editor::EditorModule::getInstance()->isPlaying()) {
		SDL_Log("[LevelData] stop play mode before saving");
		return false;
	}
#endif
	if (!isSafeFileName(fileName))
	{
		SDL_Log("[LevelData] bad file name '%s'", fileName.c_str());
		return false;
	}

	json level = serializeScene();
	level["genre"] = activeScene->getGenre();
	activeScene->writeLevelData(level);

	const std::string path = writableDir() + fileName;
	const std::string text = level.dump(2);
	if (!SDL_SaveFile(path.c_str(), text.data(), text.size()))
	{
		SDL_Log("[LevelData] save failed: %s", SDL_GetError());
		return false;
	}
	currentFile = fileName;
	currentFileGenre = activeScene->getGenre();
	SDL_Log("[LevelData] saved %s", path.c_str());
	return true;
}

void LevelDataManager::requestLoad(const std::string& fileName)
{
	pendingLoad = fileName;
}

void LevelDataManager::applyPending()
{
	if (pendingLoad.empty()) return;

	std::string name = std::move(pendingLoad);
	pendingLoad.clear();

	if (!loadLevel(name) && loadingTemplate)
	{
		loadingTemplate = false;
		newBlankLevel(templateGenre);
	}
}

static SDL_EnumerationResult enumerateLevelFile(void* userdata, const char* dirname, const char* fname)
{
	std::string n = fname;
	if (n.rfind("template_", 0) == 0) return SDL_ENUM_CONTINUE;
	if (n.size() > 5 && n.compare(n.size() - 5, 5, ".json") == 0)
		static_cast<std::vector<std::string>*>(userdata)->push_back(n);
	return SDL_ENUM_CONTINUE;
}

std::vector<std::string> LevelDataManager::listLevelFiles() const
{
	std::vector<std::string> files;
	SDL_EnumerateDirectory(writableDir().c_str(), enumerateLevelFile, &files);

	SDL_EnumerateDirectory(kBundledDir, enumerateLevelFile, &files);

	std::sort(files.begin(), files.end());
	files.erase(std::unique(files.begin(), files.end()), files.end());   // writable + bundled overlap
	return files;
}

bool LevelDataManager::loadLevel(const std::string& fileName)
{
	if (!isSafeFileName(fileName))
		return false;

	std::string text;
	if (!readFile(writableDir() + fileName, text) && !readFile(kBundledDir + fileName, text))
	{
		SDL_Log("[LevelData] '%s' not found", fileName.c_str());
		return false;
	}

	json level = json::parse(text, nullptr, false);
	if (level.is_discarded() || !level.contains("objects") || !level["objects"].is_array())
	{
		SDL_Log("[LevelData] '%s' is not a valid level", fileName.c_str());
		return false;
	}

	const std::string genre = level.value("genre", activeScene ? activeScene->getGenre() : std::string());
	if (genre.empty())
	{
		SDL_Log("[LevelData] '%s' has no genre", fileName.c_str());
		return false;
	}

	if (!activeScene || genre != activeScene->getGenre())
	{
		const SceneTag tag = sceneTagFor(genre);
		if (tag == SceneTag::NONE)
		{
			SDL_Log("[LevelData] '%s' has unknown genre '%s'", fileName.c_str(), genre.c_str());
			return false;
		}
		nextFile = fileName;
		SceneManager::getInstance()->loadScene(tag);
		return true;
	}

	activeScene->readLevelData(level);
	clearScene();
	if (!buildFromJson(level))
		return false;
	activeScene->rewire();

	currentFile = fileName;
	currentFileGenre = genre;

	// a template starts an untitled level: Save must go to Save As
	if (loadingTemplate)
	{
		currentFile.clear();
		currentFileGenre.clear();
		loadingTemplate = false;
	}
	return true;
}

SceneTag LevelDataManager::sceneTagFor(const std::string& genre)
{
	if (genre == "puzzle")     return SceneTag::PUZZLE_LOADED_SCENE;
	if (genre == "platformer") return SceneTag::PLATFORMER_LOADED_SCENE;
	return SceneTag::NONE;
}

void LevelDataManager::enterScene(LoadedScene* scene)
{
	activeScene = scene;

	if (!nextFile.empty())
	{
		requestLoad(nextFile);
		nextFile.clear();
	}
	else if (hasCurrentFile() && currentFileGenre == scene->getGenre())
		requestLoad(currentFile);            // re-enter the open file
	else
		currentFile.clear();                 // the open file belongs to another genre
}

void LevelDataManager::leaveScene(LoadedScene* scene)
{
	if (activeScene == scene) activeScene = nullptr;
}

json LevelDataManager::serializeScene() const
{
	json objs = json::array();

	for (AGameObject* o : GameObjectManager::getInstance()->getAllObjects())
	{
		json oj = serializeObject(o);
		if (!oj.is_null())
			objs.push_back(std::move(oj));
	}
	
	json level;
	level["version"] = 1;
	level["objects"] = std::move(objs);
	return level;
}

bool LevelDataManager::buildFromJson(const json& level)
{
	if (!level.contains("objects") || !level["objects"].is_array())
		return false;

	GameObjectManager* gom = GameObjectManager::getInstance();
	std::unordered_set<std::string> seen;

	for (const json& oj : level["objects"])
	{
		AGameObject* o = createFromJson(oj);
		if (!o) continue;
		if (!seen.insert(o->getName()).second)
		{
			SDL_Log("[LevelData] duplicate root name '%s', skipped", o->getName().c_str());
			delete o;
			continue;
		}

		gom->addObject(o);
		applyFields(o, oj);
		applyChildren(o, oj);
	}

	return true;
}

std::string LevelDataManager::writableDir()
{
	char* pref = SDL_GetPrefPath(kOrg, kApp); //creates app's folder
	if (!pref) {
		SDL_Log("[LevelData] SDL_GetPrefPath failed: %s", SDL_GetError());
		return "";
	}

	std::string dir = std::string(pref) + "levels/";
	SDL_free(pref);
	SDL_CreateDirectory(dir.c_str());
	return dir;
}

bool LevelDataManager::isSafeFileName(const std::string& name)
{
	return !name.empty() && name.find_first_of("/\\") ==
		std::string::npos && name.find("..") == std::string::npos;
}

bool LevelDataManager::readFile(const std::string& path, std::string& out)
{
	size_t size = 0;
	void* data = SDL_LoadFile(path.c_str(), &size);

	if (!data)
		return false;

	out.assign(static_cast<char*>(data), size);
	SDL_free(data);
	return true;
}

// conversions
json LevelDataManager::vecToJson(const Vector2D& v)
{
	return json::array({ v.x, v.y });
}

Vector2D LevelDataManager::jsonToVec(const json& a)
{
	return Vector2D(a.at(0).get<float>(), a.at(1).get<float>());
}

SpriteRenderer* LevelDataManager::spriteRendererOf(AGameObject* o)
{
	return static_cast<SpriteRenderer*>(o->findComponentByName("SpriteRenderer"));
}

//save
json LevelDataManager::serializeObject(AGameObject* o)
{
	const char* type = ObjectFactory::typeKeyOf(o);
	if (!type)
	{
		SDL_Log("[LevelData] skipping '%s': not a factory type", o->getName().c_str());
		return nullptr;
	}

	json j;
	j["type"] = type;
	j["name"] = o->getName();
	j["enabled"] = o->getEnabled();
	j["isScreenObject"] = o->getIsScreenObject();
	j["position"] = vecToJson(o->getLocalPos());
	j["scale"] = vecToJson(o->getLocalScale());
	j["rotation"] = o->getLocalRot();

	if (SpriteRenderer* sr = spriteRendererOf(o))
	{
		j["texture"] = sr->getTextureKey();

		SDL_Color c = sr->getColor();
		j["color"] = json::array({ c.r, c.g, c.b, c.a });
		j["flipX"] = sr->getflipX();
		j["flipY"] = sr->getFlipY();
		j["pivot"] = vecToJson(sr->getPivot());
	}

	if (Text* t = dynamic_cast<Text*>(o))
	{
		SDL_Color c = t->getColor();
		j["text"] = t->getMessage();
		j["fontSize"] = t->getFontSize();
		j["color"] = json::array({ c.r, c.g, c.b, c.a });
	}

	json children = json::array();
	for (AGameObject* c : o->getChildren())
	{
		json cj = serializeObject(c);
		if (!cj.is_null())
			children.push_back(std::move(cj));
	}

	if (!children.empty())
		j["children"] = std::move(children);
	return j;
}

//load
AGameObject* LevelDataManager::createFromJson(const json& j)
{
	const std::string type = j.value("type", "");
	const std::string name = j.value("name", "");
	AGameObject* o = ObjectFactory::createObject(type, name);
	
	if (!o)
		SDL_Log("[LevelData] unknown type '%s' (name '%s')", type.c_str(), name.c_str());
	
	return o;
}

void LevelDataManager::applyFields(AGameObject* o, const json& j)
{
	if (j.contains("position"))
		o->setLocalPos(jsonToVec(j["position"]));
	if (j.contains("scale"))
		o->setLocalScale(jsonToVec(j["scale"]));
	if (j.contains("rotation"))
		o->setLocalRot(j["rotation"].get<float>());

	if (j.contains("texture"))
	{
		if (SpriteRenderer* sr = spriteRendererOf(o))
		{
			if (j.contains("texture"))
				sr->setTextureKey(j["texture"].get<std::string>());

			if (j.contains("color"))
			{
				const json& c = j["color"];
				sr->setColor({ c.at(0).get<Uint8>(), c.at(1).get<Uint8>(),
							   c.at(2).get<Uint8>(), c.at(3).get<Uint8>() });
			}

			if (j.contains("flipX")) sr->setFlipX(j["flipX"].get<bool>());
			if (j.contains("flipY")) sr->setFlipY(j["flipY"].get<bool>());
			if (j.contains("pivot")) sr->setPivot(jsonToVec(j["pivot"]));
		}
	}

	if (Text* t = dynamic_cast<Text*>(o))
	{
		if (j.contains("fontSize"))
			t->setFontSize(j["fontSize"].get<int>());
		if (j.contains("text"))
			t->setMessage(j["text"].get<std::string>());
		if (j.contains("color"))
		{
			const json& c = j["color"];
			t->setColor({ c.at(0).get<Uint8>(), c.at(1).get<Uint8>(),
							c.at(2).get<Uint8>(), c.at(3).get<Uint8>() });
		}
	}

	if (j.contains("enabled") && j["enabled"].get<bool>() != o->getEnabled())
		o->setEnabled(j["enabled"].get<bool>());
}

void LevelDataManager::applyChildren(AGameObject* parent, const json& j)
{
	if (!j.contains("children")) return;

	std::vector<AGameObject*> existing = parent->getChildren();
	std::vector<bool> claimed(existing.size(), false);

	for (const json& cj : j["children"])
	{
		const std::string name = cj.value("name", "");
		AGameObject* child = nullptr;

		for (size_t i = 0; i < existing.size(); i++)
		{
			if (!claimed[i] && existing[i]->getName() == name)
			{
				claimed[i] = true;
				child = existing[i];
				break;
			}
		}

		if (!child)
		{
			child = createFromJson(cj);
			if (!child) continue;
			parent->attachChild(child);
		}
		applyFields(child, cj);
		applyChildren(child, cj);
	}
}

void LevelDataManager::clearScene()
{
#if EDITOR_MODE
	Editor::EditorModule::getInstance()->setSelectedObject(nullptr);
#endif
	GameObjectManager* gom = GameObjectManager::getInstance();
	for (AGameObject* o : gom->getAllObjects())
		if (ObjectFactory::typeKeyOf(o))      // only what the editor can recreate
			gom->deleteObject(o);             // queues only, safe while iterating
	gom->cleanUpDeletedObjects();
}

void LevelDataManager::newLevel(const std::string& genre)
{
	if (sceneTagFor(genre) == SceneTag::NONE)
	{
		SDL_Log("[LevelData] cannot create a new '%s' level: no scene for that genre", genre.c_str());
		return;
	}

	nextFile.clear();
	currentFile.clear();
	currentFileGenre.clear();

	loadingTemplate = true;
	templateGenre = genre;
	requestLoad("template_" + genre + ".json");
}

void LevelDataManager::newBlankLevel(const std::string& genre)
{
	const SceneTag tag = sceneTagFor(genre);
	if (tag == SceneTag::NONE)
	{
		SDL_Log("[LevelData] cannot create a new '%s' level: no scene for that genre", genre.c_str());
		return;
	}

	// a new level has no file until it is saved
	nextFile.clear();
	currentFile.clear();
	currentFileGenre.clear();

	// same genre: just empty the current scene
	if (activeScene && activeScene->getGenre() == genre)
	{
		clearScene();
		activeScene->rewire();
		return;
	}

	// different genre or no scene yet: load that genre's scene.
	// enterScene() finds no file to load, so the scene stays empty.
	SceneManager::getInstance()->loadScene(tag);
}

bool LevelDataManager::canCreate(const std::string& genre) const
{
	return sceneTagFor(genre) != SceneTag::NONE;
}

