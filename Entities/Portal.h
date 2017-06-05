#pragma once


#define PORTAL_RADIUS 40

//class Portal
//{
//public:
//	Portal(void) : fromNodeSphere(NULL), 
//		toNodeSphere(NULL), 
//		distanceToNodeSphereCenter(0), 
//		direction(hkvVec3(0,0,0)), 
//		sphericalCoordinates(hkvVec3(0,0,0)),
//		planeEquation(hkvVec4(0,0,0,0)){};
//	~Portal(void){};
//
//	void init(NodeSphere* fromNodeSphere, NodeSphere* toNodeSphere);
//
//	void setFromNodeSphere(NodeSphere* fromNodeSphere){this->fromNodeSphere = fromNodeSphere;};
//	NodeSphere* getFromNodeSphere(){return this->fromNodeSphere;};
//	void setToNodeSphere(NodeSphere* toNodeSphere){this->toNodeSphere = toNodeSphere;};
//	NodeSphere* getToNodeSphere(){return this->toNodeSphere;};
//	void setDistanceToNodeSphereCenter(float distanceToNodeSphereCenter){this->distanceToNodeSphereCenter = distanceToNodeSphereCenter;};
//	float getDistanceToNodeSphereCenter(){return this->distanceToNodeSphereCenter;};
//	void setDirection(hkvVec3 direction){this->direction = direction;};
//	hkvVec3 getDirection(){return this->direction;};
//
//	hkvVec3 getCartesianCoordinates(){return this->cartesianCoordinates;};
//	hkvVec3 getSphericalCoordinates(){return this->sphericalCoordinates;};
//	hkvVec4 getPlaneEquation(){return this->planeEquation;};
//
//private:
//	NodeSphere* fromNodeSphere; //node sphere the portal belongs to
//	NodeSphere* toNodeSphere; //node sphere the portal points to
//	float distanceToNodeSphereCenter; //distance between the center of the portal and the center of the sphere
//
//	hkvVec3 direction; //direction of the portal (from/to)
//
//	hkvVec3 cartesianCoordinates; //cartesian coordinates of the portal related to the node sphere
//	hkvVec3 sphericalCoordinates; //spherical coordinates of the portal related to the node sphere
//
//	//equation of the plane containing this portal ax + by + cz + d = 0
//	hkvVec4 planeEquation;
//};

