#include "LevelDataManager.h"
#include "GameObjectManager.h"
#include "ObjectFactory.h"
#include "SpriteRenderer.h"
#include "Text.h"
#include "EditorMode.h"
#include "SceneManager.h"
#include "TileMap.h"
#include "TiledImporter.h"
#include "Background.h"

#if EDITOR_MODE
#include "EditorModule.h"
#endif

#include <SDL3/SDL.h>
#include <unordered_set>
#include <algorithm>

#include "Platform.h"
#include "PuzzleToken.h"
#include "Sprite.h"

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

//saves level given a filename
bool LevelDataManager::saveLevel(const std::string& fileName)
{
	//dont save if theres no active scene
	if (!activeScene)
	{
		SDL_Log("[LevelData] no level scene is active");
		return false;
	}
#if EDITOR_MODE
	//dont save when in playmode
	if (Editor::EditorModule::getInstance()->isPlaying()) {
		SDL_Log("[LevelData] stop play mode before saving");
		return false;
	}
#endif
	//dont save level with bad filename
	if (!isSafeFileName(fileName))
	{
		SDL_Log("[LevelData] bad file name '%s'", fileName.c_str());
		return false;
	}
	
	//serialize scene in a JSON
	json level = serializeScene(); 
	level["genre"] = activeScene->getGenre(); //get genre (either puzzle or platformer)
	
	//set file path
	const std::string path = writableDir() + fileName;
	//dump formed JSON to text
	const std::string text = level.dump(2);

	//save file via SDL
	if (!SDL_SaveFile(path.c_str(), text.data(), text.size()))
	{
		SDL_Log("[LevelData] save failed: %s", SDL_GetError());
		return false;
	}
	currentFile = fileName; //set current file to recently saved file
	currentFileGenre = activeScene->getGenre(); 
	SDL_Log("[LevelData] saved %s", path.c_str());
	return true;
}

//dont accept filename that is: (1) empty, (2) has slashes, (3) has ".."
// 2 and 3 are to prevent path traversal
bool LevelDataManager::isSafeFileName(const std::string& name)
{
	return !name.empty() && name.find_first_of("/\\") ==
		std::string::npos && name.find("..") == std::string::npos;
}

//Serialize scene in a JSON 
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
	if (!tilemapPath.empty())
		level["tilemap"] = tilemapPath;
	return level;
}

//Serialize a given object to JSON format
json LevelDataManager::serializeObject(AGameObject* o)
{
	if (o->isSystemObject()) //ignore subsystem
		return nullptr;

	const char* type = ObjectFactory::typeKeyOf(o); //get object's type key
	if (!type)
	{
		SDL_Log("[LevelData] skipping '%s': not a factory type", o->getName().c_str());
		return nullptr;
	}

	//create the json
	json j;
	j["type"] = type;
	j["name"] = o->getName();
	j["enabled"] = o->getEnabled();
	j["isScreenObject"] = o->getIsScreenObject();
	j["position"] = vecToJson(o->getLocalPos());
	j["scale"] = vecToJson(o->getLocalScale());
	j["rotation"] = o->getLocalRot();

	//serialize the sprite component, its color, flip state, and pivot location
	if (SpriteRenderer* sr = spriteRendererOf(o))
	{
		j["texture"] = sr->getTextureKey();

		SDL_Color c = sr->getColor();
		j["color"] = json::array({ c.r, c.g, c.b, c.a });
		j["flipX"] = sr->getflipX();
		j["flipY"] = sr->getFlipY();
		j["pivot"] = vecToJson(sr->getPivot());
	}

	//serialize text component, its color, message, font size, and text message
	if (Text* t = dynamic_cast<Text*>(o))
	{
		SDL_Color c = t->getColor();
		j["text"] = t->getMessage();
		j["fontSize"] = t->getFontSize();
		j["color"] = json::array({ c.r, c.g, c.b, c.a });
	}

	//serialize platform component's size
	if (Platform* p = dynamic_cast<Platform*>(o))
		j["size"] = vecToJson(p->getSize());

	//serialize 
	if (PuzzleToken* t = dynamic_cast<PuzzleToken*>(o))
	{
		if (t->hasTargetPos())
			j["targetPosition"] = vecToJson(t->getTargetPos());
		if (!t->getHint().empty())
			j["hint"] = t->getHint();
	}

	//serialize children recursively (if any)
	json children = json::array();
	for (AGameObject* c : o->getChildren())
	{
		json cj = serializeObject(c);
		if (!cj.is_null()) //push back child inside children array
			children.push_back(std::move(cj));
	}

	//base case: add the array as an object
	if (!children.empty())
		j["children"] = std::move(children);
	return j; //return resulting json
}


//load level given a filename
bool LevelDataManager::loadLevel(const std::string& fileName)
{
	if (!isSafeFileName(fileName)) //dont load if filename isnt safe
		return false;

	std::string text;
	//find and read file in writableDir or kBundledDir
	if (!readFile(writableDir() + fileName, text) && !readFile(kBundledDir + fileName, text))
	{
		SDL_Log("[LevelData] '%s' not found", fileName.c_str());
		return false;
	}

	//parse loaded text to a JSON
	json level = json::parse(text, nullptr, false);
	
	//JSON failed to load
	if (level.is_discarded() || !level.contains("objects") || !level["objects"].is_array())
	{
		SDL_Log("[LevelData] '%s' is not a valid level", fileName.c_str());
		return false;
	}

	std::string genre;
	//set genre if there is an active scene
	if (activeScene)
		genre = level.value("genre", activeScene->getGenre());
	else
		genre = std::string();

	if (genre.empty())
	{
		SDL_Log("[LevelData] '%s' has no genre", fileName.c_str());
		return false;
	}

	//set genre for new scene
	if (!activeScene || genre != activeScene->getGenre())
	{
		const SceneTag tag = sceneTagFor(genre);
		if (tag == SceneTag::NONE)
		{
			SDL_Log("[LevelData] '%s' has unknown genre '%s'", fileName.c_str(), genre.c_str());
			return false;
		}
		nextFile = fileName;
		SceneManager::getInstance()->loadScene(tag); //load a platformer/puzzle scene
		return true;
	}

	//clear current scene before loading new one
	clearScene();
	activeScene->resetSubsystems(); //reset subsystems becofe loading new scene

	//build the level from the parsed json
	if (!buildFromJson(level))
		return false;
	//
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

//read JSON file given file path
bool LevelDataManager::readFile(const std::string& path, std::string& out)
{
	size_t size = 0;
	void* data = SDL_LoadFile(path.c_str(), &size); //load file in the path

	if (!data)
		return false;

	//populate string out with the bytes of loaded data
	out.assign(static_cast<char*>(data), size);
	SDL_free(data); //free memory of data after loading
	return true;
}

SceneTag LevelDataManager::sceneTagFor(const std::string& genre)
{
	if (genre == "puzzle")     return SceneTag::PUZZLE_LOADED_SCENE;
	if (genre == "platformer") return SceneTag::PLATFORMER_LOADED_SCENE;
	return SceneTag::NONE;
}


bool LevelDataManager::buildFromJson(const json& level)
{
	if (!level.contains("objects") || !level["objects"].is_array())
		return false;

	GameObjectManager* gom = GameObjectManager::getInstance();
	std::unordered_set<std::string> seen;

	//iterate through all objects of the level
	for (const json& oj : level["objects"])
	{
		AGameObject* o = createFromJson(oj); //create object from json
		if (!o) 
			continue;
		//not seen 
		if (!seen.insert(o->getName()).second)
		{
			SDL_Log("[LevelData] duplicate root name '%s', skipped", o->getName().c_str());
			delete o;
			continue;
		}

		//set transform values of the object
		if (oj.contains("position")) 
			o->setLocalPos(jsonToVec(oj["position"]));
		if (oj.contains("scale"))    
			o->setLocalScale(jsonToVec(oj["scale"]));
		if (oj.contains("rotation")) 
			o->setLocalRot(oj["rotation"].get<float>());

		//if object is a platform, set its collision size
		if (Platform* p = dynamic_cast<Platform*>(o))
			if (oj.contains("size")) 
				p->setSize(jsonToVec(oj["size"]));

		//add object, apply fields, and apply children
		gom->addObject(o);
		applyFields(o, oj);
		applyChildren(o, oj);
	}

	//load tilemap if a tilemap is found in the JSON
	//JSON only stores the tilemap path.
	if (level.contains("tilemap"))
		loadTilemap(level["tilemap"].get<std::string>());
	return true;
}


//Create one object given a json
AGameObject* LevelDataManager::createFromJson(const json& j)
{
	const std::string type = j.value("type", "");
	const std::string name = j.value("name", "");
	AGameObject* o = ObjectFactory::createObject(type, name); //create object via ObjectFactory

	if (!o)
		SDL_Log("[LevelData] unknown type '%s' (name '%s')", type.c_str(), name.c_str()); 

	return o; //return object
}

//sets pending load to a given filename
void LevelDataManager::requestLoad(const std::string& fileName)
{
	pendingLoad = fileName;
}

//applies the values of the json to the gameobject given a key
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

	if (PuzzleToken* t = dynamic_cast<PuzzleToken*>(o))
	{
		if (j.contains("targetPosition")) t->setTargetPos(jsonToVec(j["targetPosition"]));
		if (j.contains("hint"))           t->setHint(j["hint"].get<std::string>());
	}

	if (j.contains("enabled") && j["enabled"].get<bool>() != o->getEnabled())
		o->setEnabled(j["enabled"].get<bool>());
}

//applyFields() but to a gameobject's children (recursive call)
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

//applies pending object creations, deletions, and rewires
void LevelDataManager::applyPending()
{
	if (restorePending)
	{
		restorePending = false;
		restoreSnapshot();
	}

	if (!pendingTilemap.empty() && activeScene)
	{
		//rebuild the scene with the new tilemap, the same way play mode restores its snapshot,
		//so PhysicsSystem never holds colliders that were already deleted
		json level = serializeScene();
		level["tilemap"] = std::move(pendingTilemap);
		pendingTilemap.clear();

		clearScene();
		activeScene->resetSubsystems();
		buildFromJson(level);
		activeScene->rewire();
	}

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

void LevelDataManager::enterScene(LoadedScene* scene)
{
	activeScene = scene;

	if (!nextFile.empty())
	{
		pendingLoad = nextFile;
		nextFile.clear();
	}
	else if (hasCurrentFile() && currentFileGenre == scene->getGenre())
		pendingLoad = currentFile;            // re-enter the open file
	else
		currentFile.clear();                 // the open file belongs to another genre
}

void LevelDataManager::leaveScene(LoadedScene* scene)
{
	if (activeScene == scene) {
		activeScene = nullptr;
		pTileMap = nullptr; //remove tilemap
		tilemapPath.clear();
	}
}


std::string LevelDataManager::writableDir()
{
	std::string dir = std::string(kBundledDir) + "saves/";
	SDL_CreateDirectory(dir.c_str());
	return dir;
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
void LevelDataManager::clearScene()
{
	EventBroadcaster::getInstance()->unregisterAllListeners();
#if EDITOR_MODE
	Editor::EditorModule::getInstance()->setSelectedObject(nullptr);
#endif
	GameObjectManager* gom = GameObjectManager::getInstance();
	for (AGameObject* o : gom->getAllObjects())
		if (!o->isSystemObject() && ObjectFactory::typeKeyOf(o))
			gom->deleteObject(o);

	if (pTileMap)
	{
		gom->deleteObject(pTileMap);
		pTileMap = nullptr;
		tilemapPath.clear();
	}
	gom->cleanUpDeletedObjects();
}

void LevelDataManager::newLevel(const std::string& genre)
{
	if (sceneTagFor(genre) == SceneTag::NONE)
	{
		SDL_Log("[LevelData] cannot create a new '%s' level: no scene for that genre", genre.c_str());
		return;
	}

	//clear currently loaded file before loading
	nextFile.clear(); 
	currentFile.clear();
	currentFileGenre.clear();

	//request a load
	loadingTemplate = true;
	templateGenre = genre;
	pendingLoad = "template_" + genre + ".json"; 
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

void LevelDataManager::beginPlay()
{
	if (!activeScene) return;
	playSnapshot = serializeScene();
	restorePending = true;
}

void LevelDataManager::endPlay()
{
	if (!activeScene || playSnapshot.is_null()) return;
	restorePending = true;
}

void LevelDataManager::restoreSnapshot()
{
	if (!activeScene || playSnapshot.is_null()) return;
	clearScene();
	activeScene->resetSubsystems();
	buildFromJson(playSnapshot);
	activeScene->rewire();
}

//creates level's tile layer from a Tiled JSON map, replacing any previous one
void LevelDataManager::loadTilemap(const std::string& mapPath)
{
	GameObjectManager* gom = GameObjectManager::getInstance();

	if (pTileMap)
	{
		gom->deleteObject(pTileMap);
		gom->cleanUpDeletedObjects();
		pTileMap = nullptr;
		tilemapPath.clear();
	}

	TileMap* pTiles = new TileMap("Tiles");
	gom->addObject(pTiles); //attach TileMapRenderer
	if (!importTiledMap(mapPath, pTiles))
	{
		gom->deleteObject(pTiles);
		gom->cleanUpDeletedObjects();
		return;
	}

	//objects draw in list order: put the tiles after the system managers and the backdrop
	//(backgrounds and plain sprites at the start of the list), before the first gameplay object
	int index = 0;
	const auto& roots = gom->getAllObjects();
	while (index < (int)roots.size() &&
		(roots[index]->isSystemObject() ||
			dynamic_cast<Background*>(roots[index]) ||
			dynamic_cast<Sprite*>(roots[index])))
		index++;
	gom->moveObject(pTiles, nullptr, index);

	pTileMap = pTiles;
	tilemapPath = mapPath;
}


void LevelDataManager::importTilemap(const std::string& mapPath)
{
	pendingTilemap = mapPath;
}