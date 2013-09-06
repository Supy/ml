#include "CCollisionObject.h"
#include "SVector2D.h"
CCollisionObject::CCollisionObject()
{
	m_ObjectType = ObjectType::Mine;
	m_vPosition = new SVector2D(0,0);
}
CCollisionObject::CCollisionObject(ObjectType objectType, SVector2D position)
{
	m_ObjectType = objectType;
	m_vPosition = new SVector2D(position.x,position.y);
}
CCollisionObject::~CCollisionObject()
{

}

void CCollisionObject::setPosition(SVector2D position)
{
	m_vPosition = new SVector2D(position.x,position.y);
	//*m_vPosition = position;
}
SVector2D CCollisionObject::getPosition()
{
	return *m_vPosition;
}
void CCollisionObject::setType(ObjectType objectType)
{
	m_ObjectType = objectType;
}

CCollisionObject::ObjectType CCollisionObject::getType()
{
	return m_ObjectType;
}