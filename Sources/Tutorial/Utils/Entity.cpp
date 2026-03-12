#include "pch.h"

#include "Entity.h"

#include "Utils.h"
#include "Object.h"

//-------------------------------------------------------------------

Entity::Entity(const WCHAR* _szName, Object* _object, const D3DXMATRIX& _worldMatrix)
	: m_szName(NULL)
	, m_object(_object)
	, m_worldMatrix(_worldMatrix)
	, m_boundingSphere_center(0.f, 0.f, 0.f)
	, m_boundingSphere_radius(0.f)
	, m_boundingAABB_min(0.f, 0.f, 0.f)
	, m_boundingAABB_max(0.f, 0.f, 0.f)
{
	m_szName = _wcsdup(_szName);

	m_object->AddRef();

	_UpdateBoundingAABB();
}

//-------------------------------------------------------------------

Entity::~Entity()
{
	SAFE_FREE(m_szName);
	SAFE_RELEASE(m_object);
}

//-------------------------------------------------------------------

void Entity::_UpdateBoundingAABB()
{
	D3DXVECTOR4 worldBB_min;
	D3DXVECTOR4 worldBB_max;
	D3DXVec3Transform(&worldBB_min, &m_object->GetLocalBoundingAABB_min(), &m_worldMatrix);
	D3DXVec3Transform(&worldBB_max, &m_object->GetLocalBoundingAABB_max(), &m_worldMatrix);

	D3DXVECTOR4 tmp;
	D3DXVec4Minimize(&tmp, &worldBB_min, &worldBB_max);
	m_boundingAABB_min = D3DXVECTOR3(tmp.x, tmp.y, tmp.z);
	D3DXVec4Maximize(&tmp, &worldBB_min, &worldBB_max);
	m_boundingAABB_max = D3DXVECTOR3(tmp.x, tmp.y, tmp.z);

	m_boundingSphere_center = (m_boundingAABB_min + m_boundingAABB_max) * 0.5f;
	D3DXVECTOR3 diag = m_boundingAABB_max - m_boundingAABB_min;
	m_boundingSphere_radius = D3DXVec3Length(&diag) * 0.5f;
}

//-------------------------------------------------------------------
