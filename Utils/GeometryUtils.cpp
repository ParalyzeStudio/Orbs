#include "OrbsPluginPCH.h"
#include "GeometryUtils.h"

float GeometryUtils::determinant(hkvVec2 u, hkvVec2 v, hkvVec2 w, bool normalize)
{
    if (normalize)
    {
        hkvVec2 vec1 = u - w;
        hkvVec2 vec2 = v - w;
        float determinant = u.x * v.y + v.x * w.y +  w.x * u.y - u.x * w.y - v.x * u.y - w.x * v.y;
		return determinant / (vec1.getLength() * vec2.getLength());
    }
    else
        return u.x * v.y + v.x * w.y +  w.x * u.y - u.x * w.y - v.x * u.y - w.x * v.y;
}

bool GeometryUtils::isDeterminantNull(float determinant)
{
	return (abs(determinant) < 0.00001f);
}

float GeometryUtils::arcDistance(float radius, hkvVec2 pointA, hkvVec2 pointB) //3D case
{
	float thetaA = pointA.x;
	float thetaB = pointB.x;
	float phiA = pointA.y;
	float phiB = pointB.y;

	hkvVec3 cartesianPointA = GeometryUtils::sphericalToCartesian(hkvVec3(radius, thetaA, phiA));
	hkvVec3 cartesianPointB = GeometryUtils::sphericalToCartesian(hkvVec3(radius, thetaB, phiB));

	float cosAB = cartesianPointA.dot(cartesianPointB) / pow(radius, 2);

	return (float) (2 * M_PI * radius * (acos(cosAB)  / (2 * M_PI)));
}

float GeometryUtils::arcDistance(float radius, float thetaA, float thetaB, bool signOfVariation) //2D case
{
	//thetaA and thetaB are suppose to be normalize between 0 and 2*pi
	float thetaVariation = 0;
	if (signOfVariation)
	{
		if (thetaB > thetaA)
			thetaVariation = thetaB - thetaA; 
		else if (thetaB < thetaA)
			thetaVariation = 360 - thetaA + thetaB;
		else
			return 0;
	}
	else
	{
		if (thetaA > thetaB)
			thetaVariation = thetaA - thetaB; 
		else if (thetaA < thetaB)
			thetaVariation = thetaA + 360 - thetaB;
		else
			return 0;
	}

	float thetaVariationRad = GeometryUtils::degreesToRadians(thetaVariation);
	return (float) (thetaVariationRad / (2 * M_PI) * radius);
}

hkvVec4 GeometryUtils::findPlaneEquation(hkvVec3 point, hkvVec3 normal)
{
	float d = -point.x * normal.x - point.y * normal.y - point.z * normal.z;
	return hkvVec4(normal.x, normal.y, normal.z, d);
}

int GeometryUtils::round(float number)
{
	return (int) floor(number + 0.5f);
}

//void GeometryUtils::rotateVectorAroundXAxis(hkvVec3 &vector, float angle)
//{
//	float vectorY = vector.y;
//	float vectorZ = vector.z;
//	vector.y = vectorY * cos(angle) + vectorZ * sin(angle);
//	vector.z = -vectorY * sin(angle) + vectorZ * cos(angle);
//}
//
//void GeometryUtils::rotateVectorAroundYAxis(hkvVec3 &vector, float angle)
//{
//	float vectorX = vector.x;
//	float vectorZ = vector.z;
//	vector.x = vectorX * cos(angle) - vectorZ * sin(angle);
//	vector.z = vectorX * sin(angle) + vectorZ * cos(angle);
//}
//
//void GeometryUtils::rotateVectorAroundZAxis(hkvVec3 &vector, float angle)
//{
//	float vectorX = vector.x;
//	float vectorY = vector.y;
//	vector.x = vectorX * cos(angle) + vectorY * sin(angle);
//	vector.y = -vectorX * sin(angle) + vectorY * cos(angle);
//}

bool GeometryUtils::arePointsEqual(hkvVec2 point1, hkvVec2 point2)
{
	return (point1.x == point2.x && point1.y == point2.y);
}

bool GeometryUtils::lineAndSphereIntersects(hkvVec3 lineStartPoint, hkvVec3 lineDir, hkvVec3 sphereCenter, float sphereRadius)
{
	//points are the solutions of the equation ax² + bx + c = 0
	float a = 1; //dot product of lineDir and itself, so 1 because lineDir is a unit vector
	float b = 2 * lineDir.dot(lineStartPoint - sphereCenter);
	hkvVec3 cameraToSphereSegment = lineStartPoint - sphereCenter;
	float c = cameraToSphereSegment.dot(cameraToSphereSegment) - pow(sphereRadius, 2);

	float determinant = pow(b, 2) - 4 * a * c;

	return (determinant >= 0);
}

hkvVec3 GeometryUtils::lineAndPlaneIntersection(hkvVec3 linePoint, hkvVec3 lineDirection, hkvVec4 planeEquation)
{
	float upperFactor = -(planeEquation.x * linePoint.x + planeEquation.y * linePoint.y + planeEquation.z * linePoint.z + planeEquation.w);

	float lowerFactor = planeEquation.x * lineDirection.x + planeEquation.y * lineDirection.y + planeEquation.z * lineDirection.z;

	if (lowerFactor != 0)
	{
		float t = upperFactor / lowerFactor;
		return hkvVec3(linePoint.x + t * lineDirection.x,
					   linePoint.y + t * lineDirection.y,
					   linePoint.z + t * lineDirection.z);
	}

	return hkvVec3(0,0,0);
}

bool GeometryUtils::twoSegmentsIntersect(hkvVec2 segment1Point1, hkvVec2 segment1Point2, hkvVec2 segment2Point1, hkvVec2 segment2Point2)
{
	//one segment is zero-length
	if (arePointsEqual(segment1Point1, segment1Point2) || 
		arePointsEqual(segment2Point1, segment2Point2))
		return false;

	//two segment have one point in common
	if (arePointsEqual(segment1Point1, segment2Point1) ||
		arePointsEqual(segment1Point1, segment2Point2) ||
		arePointsEqual(segment1Point2, segment2Point1) ||
		arePointsEqual(segment1Point2, segment2Point2))
		return false;

	float det1 = GeometryUtils::determinant(segment1Point1, segment1Point2, segment2Point1, true);
	float det2 = GeometryUtils::determinant(segment1Point1, segment1Point2, segment2Point2, true);

	if (GeometryUtils::isDeterminantNull(det1) || GeometryUtils::isDeterminantNull(det2))
		return true;
	if ((det1 * det2) > 0) //point are on the same half plane set by the first segment
		return false;

	float det3 = GeometryUtils::determinant(segment2Point1, segment2Point2, segment1Point1, true);
	float det4 = GeometryUtils::determinant(segment2Point1, segment2Point2, segment1Point2, true);

	if (GeometryUtils::isDeterminantNull(det3) || GeometryUtils::isDeterminantNull(det4) || (det3 * det4) >= 0) //point are on the same half plane set by the first segment
		return false;
	if ((det3 * det4) > 0) //point are on the same half plane set by the first segment
		return false;

	return true;
}

void GeometryUtils::project2D(hkvVec3 pos, float &x2d, float &y2d, hkvVec3 cameraPosition, hkvMat3 cameraRotationMatrix)
{
	hkvVec3 orthoVector = cameraRotationMatrix.transformDirection(hkvVec3(1,0,0));

	hkvVec3 cameraPosTo3DPos = pos - cameraPosition; //AM
	hkvVec3 planeProjectionTo3DPos = cameraPosTo3DPos.dot(orthoVector) * orthoVector; //PM

	float projX = pos.x - planeProjectionTo3DPos.x;
	float projY = pos.y - planeProjectionTo3DPos.y;
	float projZ = pos.z - planeProjectionTo3DPos.z;

	hkvVec3 localProj = hkvVec3(projX - cameraPosition.x, projY - cameraPosition.y, projZ - cameraPosition.z);
	hkvMat3 invertedCameraRotationMatrix = cameraRotationMatrix;
	invertedCameraRotationMatrix.invert();
	localProj = invertedCameraRotationMatrix.transformDirection(localProj);

	x2d = localProj.x;
	y2d = -localProj.z;

	x2d += 512;
	y2d += 384;
}