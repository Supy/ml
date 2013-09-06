#ifndef CCOLLISIONOBJECT_H
#define CCOLLISIONOBJECT_H
//#include "SVector2D.h"
class SVector2D;
class CCollisionObject
{
public:
	enum ObjectType { Mine, SuperMine, Rock };


	CCollisionObject();
	CCollisionObject(ObjectType objectType, SVector2D position);
	~CCollisionObject();

	void setPosition(SVector2D position);
	SVector2D getPosition();
	void setType(ObjectType objectType);
	ObjectType getType();
		
private:
	
	SVector2D * m_vPosition;
	ObjectType m_ObjectType;
};

#endif