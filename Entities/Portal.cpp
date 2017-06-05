#include "OrbsPluginPCH.h"
#include "Portal.h"

//void Portal::init(NodeSphere* fromNodeSphere, NodeSphere* toNodeSphere)
//{
//	this->fromNodeSphere = fromNodeSphere;
//	this->toNodeSphere = toNodeSphere;
//	this->distanceToNodeSphereCenter = 1.2f * this->fromNodeSphere->getOuterRadius();
//
//	direction = toNodeSphere->GetPosition() - fromNodeSphere->GetPosition();
//	direction.normalize();
//
//	this->cartesianCoordinates = direction * this->distanceToNodeSphereCenter; //position of the portal with the center of fromNodeSphere as the origin of coordinate system
//	this->sphericalCoordinates = GeometryUtils::cartesianToSpherical(this->cartesianCoordinates);
//
//	//equation of the plane
//	//direction is a normal to this plane
//	//aboslute cartesian coordinates of the portal is a point contained in this plane
//	float a = direction.x;
//	float b = direction.y;
//	float c = direction.z;
//	hkvVec3 planePoint = fromNodeSphere->GetPosition() + this->cartesianCoordinates;
//	float d = -(a * planePoint.x + b * planePoint.y + c * planePoint.z);
//	planeEquation.x = a;
//	planeEquation.y = b;
//	planeEquation.z = c;
//	planeEquation.w = d;
//}