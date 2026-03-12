#include "pch.h"

#include "EntitiesManager.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Entity.h"
#include "..\Utils\Object.h"

//-------------------------------------------------------------------

EntitiesManager* EntitiesManager::s_Instance = NULL;

EntitiesManager::EntitiesManager()
	: m_entitiesList(1024, 1024)
{
}

//-------------------------------------------------------------------

EntitiesManager::~EntitiesManager()
{
	DeleteAllEntities();
}

//-------------------------------------------------------------------

void EntitiesManager::Create()
{
	if (s_Instance)
	{
		myAssert(false, L"ObjectsManager::Create() already called !");
		return;
	}

	s_Instance = new EntitiesManager();
}

//-------------------------------------------------------------------

void EntitiesManager::Destroy()
{
	SAFE_DELETE(s_Instance);
}

//-------------------------------------------------------------------

void EntitiesManager::DeleteAllEntities()
{
	for (int i = 0; i < m_entitiesList.GetSize(); i++)
	{
		SAFE_RELEASE(m_entitiesList[i]);
	}

	m_entitiesList.Clear();
}

//-------------------------------------------------------------------

Entity* EntitiesManager::CreateEntity(const char* _szName, Object* _object, const D3DXMATRIX& _worldMatrix)
{
	WCHAR wszName[256];
	ConvertStringToWideString(_szName, wszName);

	return CreateEntity(wszName, _object, _worldMatrix);
}

//-------------------------------------------------------------------

Entity* EntitiesManager::CreateEntity(const WCHAR* _szName, Object* _object, const D3DXMATRIX& _worldMatrix)
{
	Entity* entity = new Entity(_szName, _object, _worldMatrix);
	entity->AddRef();  // add ref for keeping it in the global entities list

	m_entitiesList.Add(entity);

	entity->AddRef();  // add ref for the caller that will have the responsibility to release it

	return entity;
}

//-------------------------------------------------------------------
