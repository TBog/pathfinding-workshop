#include "pch.h"

#include "World.h"

#include "..\Extern\tinyxml2.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Object.h"
#include "..\Utils\Entity.h"

#include "..\Render\RenderManager.h"
#include "..\Render\ObjectsManager.h"
#include "..\Render\EntitiesManager.h"

#include "..\Render\PostProcess.h"
#include "..\Render\Sky.h"
#include "..\Render\Clouds.h"
#include "..\Render\Terrain.h"

//-------------------------------------------------------------------

World::World(const WCHAR* _szFileName)
	: m_szFileName(NULL)
	, m_center(0.f, 0.f, 0.f)
	, m_size(0.f)
	, m_entitiesList(1024, 1024)
	, m_terrain(NULL)
{
	Load(_szFileName);
}

//-------------------------------------------------------------------

World::~World()
{
	Unload();
}

//-------------------------------------------------------------------

bool World::Load(const WCHAR* _szFileName)
{
	Unload();

	m_szFileName = _wcsdup(_szFileName);

	// convert file name to char*
	char szFileNameChar[256];
	ConvertWdieStringToString(m_szFileName, szFileNameChar);

	// load the world xml file
	tinyxml2::XMLDocument worldFile;
	worldFile.LoadFile(szFileNameChar);
	if (worldFile.ErrorID() != 0)
	{
		WCHAR msg[512];
		swprintf_s(msg, 512, L"World::Load() - \"%s\" failed!", m_szFileName);
		myAssert(false, msg);

		return false;
	}

	// load world parameters
	{
		tinyxml2::XMLElement* worldElement = worldFile.FirstChildElement("world");

		if (worldElement)
		{
			worldElement->QueryFloatAttribute("centerX", &m_center.x);
			worldElement->QueryFloatAttribute("centerY", &m_center.y);
			worldElement->QueryFloatAttribute("centerZ", &m_center.z);

			worldElement->QueryFloatAttribute("size", &m_size);
		}
	}

	// load terrain
	{
		tinyxml2::XMLElement* terrainElement = worldFile.FirstChildElement("terrain");

		if (terrainElement)
		{
			const char* szTerrainFileName = terrainElement->Attribute("terrainFileName");

			WCHAR wszTerrainFileName[256];
			ConvertStringToWideString(szTerrainFileName, wszTerrainFileName);

			m_terrain = new Terrain(wszTerrainFileName);
		}
	}

	// load post process params
	{
		tinyxml2::XMLElement* postProcessElement = worldFile.FirstChildElement("postProcess");

		if (postProcessElement)
		{
			const char* szPostProcessFileName = postProcessElement->Attribute("postProcessFileName");

			WCHAR wszPostProcessFileName[256];
			ConvertStringToWideString(szPostProcessFileName, wszPostProcessFileName);

			g_renderManager->GetPostProcess()->Load(wszPostProcessFileName);
		}
	}

	// load sky
	{
		tinyxml2::XMLElement* skyElement = worldFile.FirstChildElement("sky");

		if (skyElement)
		{
			const char* szSkyFileName = skyElement->Attribute("skyFileName");

			WCHAR wszSkyFileName[256];
			ConvertStringToWideString(szSkyFileName, wszSkyFileName);

			g_renderManager->GetSky()->Load(wszSkyFileName);
		}
	}

	// load clouds
	{
		tinyxml2::XMLElement* cloudsElement = worldFile.FirstChildElement("clouds");

		if (cloudsElement)
		{
			const char* szCloudsFileName = cloudsElement->Attribute("cloudsFileName");

			WCHAR wszCloudsFileName[256];
			ConvertStringToWideString(szCloudsFileName, wszCloudsFileName);

			g_renderManager->GetClouds()->Load(wszCloudsFileName);
		}
	}

	// load objects
	{
		tinyxml2::XMLElement* entitiesListElement = worldFile.FirstChildElement("entities");

		if (entitiesListElement)
		{
			tinyxml2::XMLElement* entityElement = entitiesListElement->FirstChildElement();

			while (entityElement)
			{
				// get the entity name
				const char* szEntityName = entityElement->Name();

				// load the object
				const char* szObjectFileName = entityElement->Attribute("objectFileName");

				float objectScale = 1.f;
				entityElement->QueryFloatAttribute("objectScale", &objectScale);

				Object* object = g_objectsManager->Load(szObjectFileName, objectScale);

				// construct the world matrix for the entity
				float x = 0.f, y = 0.f, z = 0.f;
				entityElement->QueryFloatAttribute("x", &x);
				entityElement->QueryFloatAttribute("y", &y);
				entityElement->QueryFloatAttribute("z", &z);

				float rotation_degrees_yaw = 0.f, rotation_degrees_pitch = 0.f, rotation_degrees_roll = 0.f;
				entityElement->QueryFloatAttribute("yaw", &rotation_degrees_yaw);
				entityElement->QueryFloatAttribute("pitch", &rotation_degrees_pitch);
				entityElement->QueryFloatAttribute("roll", &rotation_degrees_roll);

				D3DXMATRIX worldMatrix, tmpMatrix;
				D3DXMatrixRotationYawPitchRoll(&worldMatrix, (float)D3DXToRadian(rotation_degrees_yaw), (float)D3DXToRadian(rotation_degrees_pitch), (float)D3DXToRadian(rotation_degrees_roll));
				D3DXMatrixTranslation(&tmpMatrix, x, y, z);
				worldMatrix *= tmpMatrix;

				// create the entity
				Entity* entity = g_entitiesManager->CreateEntity(szEntityName, object, worldMatrix);

				m_entitiesList.Add(entity);

				entityElement = entityElement->NextSiblingElement();
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------

void World::Unload()
{
	for (int i = 0; i < m_entitiesList.GetSize(); i++)
	{
		SAFE_RELEASE(m_entitiesList[i]);
	}
	m_entitiesList.Clear();

	SAFE_DELETE(m_terrain);

	SAFE_FREE(m_szFileName);
}

//-------------------------------------------------------------------
