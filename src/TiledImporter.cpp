#include "TiledImporter.h"
#include "TileMap.h"
#include "TileMapRenderer.h"
#include "TextureManager.h"
#include "Platform.h"
#include "PhysicsSystem.h"
#include <vector>
#include <string>

#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

// Adds one collider to the tilemap as a child Platform.
// Center is an offset from the tilemap in world units; size is in tile pixels
// (the tilemap's scale is applied to its children automatically).
static void addTileCollider(TileMap* pTileMap, int& nColliders,
	float centerX, float centerY, float sizeX, float sizeY)
{
	Platform* pPlatform = new Platform("Tile_Collider_" + std::to_string(nColliders),
		Vector2D(centerX, centerY), Vector2D(sizeX, sizeY), 0.f);
	pTileMap->attachChild(pPlatform);
	nColliders++;
}

bool importTiledMap(const std::string& mapPath, TileMap* pTileMap)
{
	TileMapRenderer* pTMR = (TileMapRenderer*)pTileMap->findComponentByName("TileMapRenderer");
	if (!pTMR)
		return false;

	//read and parse file
	std::ifstream file("Assets/" + mapPath);
	if (!file)
	{
		SDL_Log("[Tiled] can't open %s", mapPath.c_str());
		return false;
	}

	std::stringstream ss;
	ss << file.rdbuf();
	json map = json::parse(ss.str(), nullptr, false);
	if (map.is_discarded())
	{
		SDL_Log("[Tiled] %s is not a valid JSON", mapPath.c_str());
		return false;
	}

	//set tile size
	float tileW = map["tilewidth"].get<float>();
	float tileH = map["tileheight"].get<float>();
	pTMR->setTileSize(tileW, tileH);

	//match the grid to the map's size
	pTMR->setGridSize(map["width"].get<Uint64>(), map["height"].get<Uint64>());

	//optional custom map property set in Tiled (Map > Map Properties)
	float scale = 1.0f;
	if (map.contains("properties"))
	{
		for (const json& prop : map["properties"])
		{
			if (prop["name"] == "scale")
				scale = prop["value"].get<float>();
		}
	}
	pTileMap->setScale(Vector2D(scale));
	
	//size of one cell in the world after scaling
	float cellW = tileW * scale;
	float cellH = tileH * scale;

	//tile image paths are relative to the map file
	std::string mapDir = mapPath.substr(0, mapPath.find_last_of('/') + 1);

	//build a lookup from global tile ID (gid) to texture
	std::unordered_map<int, SDL_Texture*> gidToTexture;
	TextureManager* tm = TextureManager::getInstance();

	//loads texture and record tile number for each texture
	for (const json& tileset : map["tilesets"])
	{
		int firstGid = tileset["firstgid"].get<int>();
		if (!tileset.contains("tiles")) continue;

		for (const json& tile : tileset["tiles"])
		{
			//Tiled writes image paths relative to the map, but switches to absolute paths
			//when the map is saved on another drive.
			std::string image = tile["image"].get<std::string>();
			std::string imagePath;
			size_t assetsPos = image.find("Assets/");
			if (assetsPos != std::string::npos)
				imagePath = image.substr(assetsPos + 7);   //7 = length of "Assets/"
			else
				imagePath = mapDir + image;

			if (!tm->get(imagePath))
				tm->load(imagePath, imagePath);   //the path doubles as the texture key

			SDL_Texture* pTex = tm->get(imagePath);
			if (!pTex)
				SDL_Log("[Tiled] missing tile image: Assets/%s", imagePath.c_str());
			gidToTexture[firstGid + tile["id"].get<int>()] = pTex;
		}
	}

	
	bool bPhysics = PhysicsSystem::getInstance() != nullptr;
	int nColliders = 0;
	int nTiles = 0;
	
	for (const json& layer : map["layers"])
	{
		//set up for this layer
		if (layer["type"] != "tilelayer")
			continue;

		int width = layer["width"].get<int>();
		int height = layer["height"].get<int>();
		const json& data = layer["data"];
		
		//every tile collides unless the layer has a custom bool property collision = false
		bool bCollision = bPhysics;
		if (layer.contains("properties"))
		{
			for (const json& prop : layer["properties"])
			{
				if (prop["name"] == "collision")
					bCollision = bCollision && prop["value"].get<bool>();
			}
		}

		//used to mark solid cells
		std::vector<std::vector<bool>> solid(width, std::vector<bool>(height, false));

		//For each tile layer, it places each tile in its cell,
		//flipping the rows, and then merges the solid tiles into as 
		//few big colliders as possible.
		for (int i = 0; i < (int)data.size(); i++)
		{
			int gid = (int)(data[i].get<Uint32>() & 0x1FFFFFFF);   //strip Tiled's flip flags
			if (gid == 0) continue;                                  //0 = empty cell

			//lookup texture via gID
			auto it = gidToTexture.find(gid);
			if (it == gidToTexture.end() || !it->second) continue;

			//Tiled counts rows down from the top, the game counts them up from the bottom
			int c = i % width;
			int r = height - 1 - i / width;
			pTMR->addTile(c, r, it->second);
			nTiles++;

			if (!bCollision) continue;

			SDL_Texture* tex = it->second;
			if (tex->w == tileW && tex->h == tileH)
				solid[c][r] = true; 
			else
				//odd sized tile
				addTileCollider(pTileMap, nColliders, (c + 0.5f) * cellW, (r + 0.5f) * cellH, (float)tex->w, (float)tex->h);
		}

		//merge solid cells into as few rectangles as possible:
		//grow right as far as possible, then grow up while the whole row is solid
		std::vector<std::vector<bool>> used(width, std::vector<bool>(height, false));
		for (int r = 0; r < height; r++)
		{
			for (int c = 0; c < width; c++)
			{
				if (!solid[c][r] || used[c][r])
					continue;

				int c1 = c;
				//grow as far right as possible
				while (c1 + 1 < width && solid[c1 + 1][r] && !used[c1 + 1][r])
					c1++;

				int r1 = r;
				bool bGrow = true;
				while (bGrow && r1 + 1 < height)
				{
					for (int x = c; x <= c1; x++)
					{
						if (!solid[x][r1+ 1 ] || used[x][r1 + 1])
						{
							bGrow = false;
							break;
						}
					}
					if (bGrow)
						r1++;
				}

				for (int x = c; x <= c1; x++)
				{
					for (int y = r; y <= r1; y++)
						used[x][y] = true;
				}

				addTileCollider(pTileMap, nColliders, (c + c1 + 1) * 0.5f * cellW, (r + r1 + 1) * 0.5f * cellH,
					(c1 - c + 1) * tileW, (r1 - r + 1) * tileH);
			}
		}
	}

	SDL_Log("[Tiled] imported %s: %d textures, %d tiles, %d colliders",
		mapPath.c_str(), (int)gidToTexture.size(), nTiles, nColliders);
	return true;
}
