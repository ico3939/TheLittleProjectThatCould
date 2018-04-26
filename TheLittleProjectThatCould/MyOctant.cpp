#include "MyOctant.h"

using namespace Simplex;

uint MyOctant::m_uMaxLevel = 1;
uint MyOctant::m_uIdealEntityCount = 5;
uint MyOctant::m_uOctantCount = 0;

Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uLevel = 0;
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	int entity_count = m_pEntityMngr->GetEntityCount();

	for (int i = 0; i < entity_count; i++) {
		m_EntityList.push_back(i);
		vector3 max = m_pEntityMngr->GetRigidBody(i)->GetMaxGlobal();
		vector3 min = m_pEntityMngr->GetRigidBody(i)->GetMinGlobal();
		if (max.x > m_v3Max.x) m_v3Max.x = max.x;
		if (min.x < m_v3Min.x) m_v3Min.x = min.x;
		if (max.y > m_v3Max.y) m_v3Max.y = max.y;
		if (min.y < m_v3Min.y) m_v3Min.y = min.y;
		if (max.z > m_v3Max.z) m_v3Max.z = max.z;
		if (min.z < m_v3Min.z) m_v3Min.z = min.z;
	}
	m_v3Center = (m_v3Max + m_v3Min) / 2.0f;
	m_v3Center.y -= 2.2f;
	m_pRoot = this;
	m_uChildren = 0;
	m_uID = m_uOctantCount++;
	vector3 temp = m_v3Max - m_v3Min;
	m_fSize = std::max(temp.x, temp.y);
	m_fSize = std::max(m_fSize, temp.z);
	m_fSize /= 2.0f;
	m_v3Max = m_v3Center + vector3(m_fSize, m_fSize, m_fSize);
	m_v3Min = m_v3Center - vector3(m_fSize, m_fSize, m_fSize);


	ConstructTree(m_uLevel);
	ConstructList();
	AssignIDtoEntity();

}

Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
}

Simplex::MyOctant::MyOctant(MyOctant const & other)
{
	m_v3Center = other.m_v3Center;
	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_v3Max = other.m_v3Max;
	m_v3Min = other.m_v3Min;
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;
	if (m_uChildren != 0) {
		for (uint i = 0; i < m_uChildren; i++) {
			m_pChild[i] = other.m_pChild[i];
		}
	}
	m_pMeshMngr = other.m_pMeshMngr;
	m_pEntityMngr = other.m_pEntityMngr;
	m_pParent = other.m_pParent;
	m_EntityList = other.m_EntityList;
	m_pRoot = other.m_pRoot;
}

MyOctant & Simplex::MyOctant::operator=(MyOctant const & other)
{
	if (this != &other) {
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

Simplex::MyOctant::~MyOctant(void)
{
	Release();
}

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_pChild, other.m_pChild);
	std::swap(m_EntityList, other.m_EntityList);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_v3Max, other.m_v3Max);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pRoot, other.m_pRoot);
}

float Simplex::MyOctant::GetSize(void)
{
	return m_fSize;
}

vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	vector3 max = m_pEntityMngr->GetRigidBody(a_uRBIndex)->GetMaxGlobal();
	vector3 min = m_pEntityMngr->GetRigidBody(a_uRBIndex)->GetMinGlobal();
	if (min.x > m_v3Max.x) return false;
	if (max.x < m_v3Min.x) return false;
	if (min.y > m_v3Max.y) return false;
	if (max.y < m_v3Min.y) return false;
	if (min.z > m_v3Max.z) return false;
	if (max.z < m_v3Min.z) return false;
	return true;

}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex) {
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_v3Center) *
			glm::scale(vector3(m_fSize * 2.0f)), a_v3Color);
	}
	else {
		for (uint i = 0; i < m_uChildren; i++) {
			m_pChild[i]->Display(a_nIndex, a_v3Color);
		}
	}

}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	DisplayLeafs(a_v3Color);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	for (MyOctant* octant : m_lChild) {
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(octant->m_v3Center) *
			glm::scale(vector3(octant->m_fSize * 2.0f)), a_v3Color);

	}
}

void Simplex::MyOctant::ClearEntityList(void)
{
	m_EntityList.clear();
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ClearEntityList();
	}
}

void Simplex::MyOctant::Subdivide(void)
{
	float size = m_fSize / 2.0f;

	m_pChild[0] = new MyOctant(m_v3Center + vector3(size, size, size), size);
	m_pChild[1] = new MyOctant(m_v3Center + vector3(size, size, -size), size);
	m_pChild[2] = new MyOctant(m_v3Center + vector3(size, -size, size), size);
	m_pChild[3] = new MyOctant(m_v3Center + vector3(size, -size, -size), size);
	m_pChild[4] = new MyOctant(m_v3Center + vector3(-size, size, size), size);
	m_pChild[5] = new MyOctant(m_v3Center + vector3(-size, size, -size), size);
	m_pChild[6] = new MyOctant(m_v3Center + vector3(-size, -size, size), size);
	m_pChild[7] = new MyOctant(m_v3Center + vector3(-size, -size, -size), size);

}

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild)
{
	if (m_uChildren > 0 && a_nChild < 8) {
		return m_pChild[a_nChild];
	}
	return nullptr;
}

MyOctant * Simplex::MyOctant::GetParent(void)
{
	if (m_pParent)
		return m_pParent;
	return nullptr;
}

bool Simplex::MyOctant::IsLeaf(void)
{
	return m_uChildren == 0;
}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	bool temp = m_EntityList.size() > a_nEntities;
	return temp;
}

void Simplex::MyOctant::KillBranches(void)
{
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->KillBranches();
		m_pChild[i] = nullptr;
	}

}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	if (a_nMaxLevel < m_uMaxLevel && ContainsMoreThan(m_uIdealEntityCount)) {
		m_uChildren = 8;

		Subdivide();
		for (uint i = 0; i < m_uChildren; i++) {
			m_pChild[i]->m_pParent = this;
			m_pChild[i]->Init();
			m_pChild[i]->ConstructTree(m_uLevel + 1);
		}

	}
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
	for (MyOctant* MyOctant : m_lChild) {
		for (uint id : MyOctant->m_EntityList) {
			m_pEntityMngr->AddDimension(id, MyOctant->m_uID);
		}
	}
}

void Simplex::MyOctant::UpdateIdForEntity(uint id)
{
	MyEntity *entity = m_pEntityMngr->GetEntity(id);
	entity->ClearDimensionSet();
	for (MyOctant* MyOctant : m_lChild) {
		if (MyOctant->IsColliding(id)) {
			m_pEntityMngr->AddDimension(id, MyOctant->m_uID);
		}
	}
}

uint Simplex::MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void Simplex::MyOctant::Release(void)
{
	KillBranches();
}

void Simplex::MyOctant::Init(void)
{
	m_uChildren = 0;
	m_uID = m_uOctantCount++;

	m_v3Max = m_v3Center + vector3(m_fSize, m_fSize, m_fSize);
	m_v3Min = m_v3Center - vector3(m_fSize, m_fSize, m_fSize);

	m_pEntityMngr = m_pParent->m_pEntityMngr;
	m_pMeshMngr = m_pParent->m_pMeshMngr;
	m_pRoot = m_pParent->m_pRoot;
	m_uLevel = m_pParent->m_uLevel + 1;

	for (uint i : m_pParent->m_EntityList) {
		if (IsColliding(i)) {
			m_EntityList.push_back(i);
		}
	}
}

void Simplex::MyOctant::ConstructList(void)
{
	if (IsLeaf()) {
		m_pRoot->m_lChild.push_back(this);
	}
	else {
		for (uint i = 0; i < m_uChildren; i++) {
			m_pChild[i]->ConstructList();
		}
	}
}
