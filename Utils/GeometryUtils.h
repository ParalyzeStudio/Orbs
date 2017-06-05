#pragma once

//light struct to hold 3-coordinates point
//class float3
//{
//public:
//	float3() : x(0), y(0), z(0){};
//	float3(float x, float y, float z) : x(x), y(y), z(z) {};
//
//	inline hkvVec3 tohkvVec3(0,0,0)
//	{
//		return hkvVec3(x, y, z);
//	}
//
//	inline float getLength()
//	{
//		return sqrt(getLengthSquared());
//	}
//
//	inline float getLengthSquared()
//	{
//		return pow(x, 2) + pow(y, 2) + pow(z, 2);
//	}
//
//	inline void normalize()
//	{
//		float length = getLength();
//		x /= length;
//		y /= length;
//		z /= length;
//	}
//
//	
//
//	float x;
//	float y;
//	float z;
//};
//
/////   Returns a temporary vector that is the sum of lhs and rhs.
//inline const float3 operator+ (const float3& lhs, const float3& rhs);
//
///// \brief
/////   Returns a temporary vector that is the subtraction of lhs and rhs.
//inline const float3 operator- (const float3& lhs, const float3& rhs);
//
///// \brief
/////   Returns a temporary vector that is lhs multiplied by f.
//inline const float3 operator* (const float3& lhs, float f);
//
///// \brief
/////   Returns a temporary vector that is lhs multiplied by f.
//inline const float3 operator* (float f, const float3& rhs);
//
///// \brief
/////   Returns a temporary vector that is lhs divided by f.
//inline const float3 operator/ (const float3& lhs, float f);
//
//inline const float3 operator+ (const float3& lhs, const float3& rhs)
//{
//	float3 temp (lhs);
//	temp = temp + rhs;
//	return temp;
//}
//
//inline const float3 operator- (const float3& lhs, const float3& rhs)
//{
//	float3 temp (lhs);
//	temp = temp - rhs;
//	return temp;
//}
//
//inline const float3 operator* (const float3& lhs, float f)
//{
//	float3 temp (lhs);
//	temp = temp * f;
//	return temp;
//}
//
//inline const float3 operator* (float f, const float3& rhs)
//{
//	float3 temp (rhs);
//	temp = temp * f;
//	return temp;
//}
//
//inline const float3 operator/ (const float3& lhs, float f)
//{
//	float3 temp (lhs);
//	temp = temp / f;
//	return temp;
//}

class GeometryUtils
{
public:
	static int round(float number);

	//determinant
	static float determinant(hkvVec2 u, hkvVec2 v, hkvVec2 w, bool normalize);
	static bool isDeterminantNull(float determinant);

	//angles
	static inline float radiansToDegrees(float angleInRadians)
	{
		return (float) (angleInRadians * 360.0f / (2 * M_PI));
	}

	static inline float degreesToRadians(float angleInDegrees)
	{
		return (float) (angleInDegrees / 360.0f * (2* M_PI));
	}

	static inline void normalizeAngle(float &angle)
	{
		if (angle < 0)
			angle += 360;
		else if (angle >= 360)
			angle -= 360;
	}

	//Spherical trigonometry
	static float arcDistance(float radius, hkvVec2 pointA, hkvVec2 pointB); //3D
	static float arcDistance(float radius, float thetaA, float thetaB, bool signOfVariation); //2D

	//plane related functions
	static hkvVec4 findPlaneEquation(hkvVec3 point, hkvVec3 normal);

	//intersections
	static bool arePointsEqual(hkvVec2 point1, hkvVec2 point2);
	static bool lineAndSphereIntersects(hkvVec3 lineStartPoint, hkvVec3 lineDir, hkvVec3 sphereCenter, float sphereRadius);
	static hkvVec3 lineAndPlaneIntersection(hkvVec3 linePoint, hkvVec3 lineDirection, hkvVec4 planeEquation);
	static bool twoSegmentsIntersect(hkvVec2 segment1Point1, hkvVec2 segment1Point2, hkvVec2 segment2Point1, hkvVec2 segment2Point2);

	//cartesian, spherical coordinates
	static inline hkvVec3 cartesianToSpherical(hkvVec3 cartesianCoords)
	{
		float x = cartesianCoords.x;
		float y = cartesianCoords.y;
		float z = cartesianCoords.z;
		float r = sqrt(pow(x,2) + pow(y,2) + pow(z,2));
		float phi = atan2(y, x);
		float theta = acos(z/r);

		return hkvVec3(r, theta, phi);
	}

	static inline hkvVec3 sphericalToCartesian(hkvVec3 sphericalCoords)
	{
		float r = sphericalCoords.x;
		float theta = sphericalCoords.y;
		float phi = sphericalCoords.z;
		return hkvVec3(r * cos(phi) * sin(theta),
			r * sin(phi) * sin(theta),
			r * cos(theta));
	}

	//cartesian, polar coordinates
	static inline hkvVec2 polarToCartesian(float radius, float theta)
	{
		return hkvVec2(radius * cos(theta), radius * sin(theta));
	}

	//CCW rotation around center
	static inline void rotate2DPoint(hkvVec2 &point, hkvVec2 rotationCenter, float angle)
	{
		point.operator-=(rotationCenter); //makes the point be in rotationCenter local space
		angle = degreesToRadians(angle); //convert the angle in radians
		point = hkvVec2(point.x * cos(angle) + point.y * sin(angle), -point.x * sin(angle) + point.y * cos(angle)) + rotationCenter; //apply rotation and go back to global space
	}

	//project2D
	static void project2D(hkvVec3 pos, float &x2d, float &y2d, hkvVec3 cameraPosition, hkvMat3 cameraRotationMatrix);
};