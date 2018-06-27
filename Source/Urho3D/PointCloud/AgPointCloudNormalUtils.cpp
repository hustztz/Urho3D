#include "AgPointCloudNormalUtils.h"

#include "../Math/MathDefs.h"

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;

//static variable initialization
float AgPointCloudNormalUtils::mNormals[NumberOfNormalBins][3];
bool AgPointCloudNormalUtils::mInitialized = false;

int AgPointCloudNormalUtils::indexForNormal(const Vector3& n) {
	if (!mInitialized) initialize();

	if (n.Length() == 0)
		return IndexNoNormal;

	/* surface index: */
	int si;
	if (fabsf(n.x_) > fabsf(n.y_)) {
		if (fabsf(n.z_) > fabsf(n.x_)) {
			si = 2;
		}
		else {
			si = 0;
		}

	}
	else {

		if (fabsf(n.z_) > fabsf(n.y_)) {
			si = 2;
		}
		else {
			si = 1;
		}
	}

	float xl, xh, xm;
	switch (si) {
	case 0:
		xl = n.y_;
		xh = n.z_;
		break;
	case 1:
		xl = n.x_;
		xh = n.z_;
		break;
	case 2:
		xl = n.x_;
		xh = n.y_;
		break;
	default:
		return -1;
	}
	float sd = *(n.Data() + si);

	xm = fabsf(sd);

	if (sd < 0.0f) si = 2 * si + 1;
	else si = 2 * si;

	return int(52 * 52 * si + round(25.5f + 25.9f * 4 * atanf(xl / xm) / M_PI)
		+ 52 * round(25.5f + 25.9f * 4 * atanf(xh / xm) / M_PI));
}

const Vector3 AgPointCloudNormalUtils::normalForIndex(int val) {
	if (!mInitialized) initialize();

	if (val == IndexNoNormal || val >= NumberOfNormalBins || val < 0)
		return Vector3(0.0f, 0.0f, 0.0f);
	else
		return Vector3(mNormals[val][0], mNormals[val][1], mNormals[val][2]);
}

void AgPointCloudNormalUtils::initialize() {
	float phi, psi;
	float x, y, z, len;

	for (float j = 0.5f; j < 26; j += 1.0f) {
		for (float i = 0.5f; i < 26; i += 1.0f) {
			phi = i * M_PI / (4 * 26.0f);
			x = tan(phi);
			psi = j * M_PI / (4 * 26.0f);
			y = tan(psi);
			z = 1.0f;

			/* normalize */
			len = sqrt(x*x + y*y + z*z);
			x = x / len;
			y = y / len;
			z = z / len;
			//symmetric values for each quadrant on each surface

			//surface 1: x direction
			mNormals[52 * 52 * 0 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][0] = z;
			mNormals[52 * 52 * 0 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][1] = x;
			mNormals[52 * 52 * 0 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 0 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][0] = z;
			mNormals[52 * 52 * 0 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][1] = -x;
			mNormals[52 * 52 * 0 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 0 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][0] = z;
			mNormals[52 * 52 * 0 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][1] = x;
			mNormals[52 * 52 * 0 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][2] = -y;

			mNormals[52 * 52 * 0 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][0] = z;
			mNormals[52 * 52 * 0 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][1] = -x;
			mNormals[52 * 52 * 0 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][2] = -y;

			//surface 2: -x direction
			mNormals[52 * 52 * 1 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][0] = -z;
			mNormals[52 * 52 * 1 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][1] = x;
			mNormals[52 * 52 * 1 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 1 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][0] = -z;
			mNormals[52 * 52 * 1 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][1] = -x;
			mNormals[52 * 52 * 1 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 1 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][0] = -z;
			mNormals[52 * 52 * 1 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][1] = x;
			mNormals[52 * 52 * 1 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][2] = -y;

			mNormals[52 * 52 * 1 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][0] = -z;
			mNormals[52 * 52 * 1 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][1] = -x;
			mNormals[52 * 52 * 1 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][2] = -y;

			//surface 3: y direction
			mNormals[52 * 52 * 2 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][0] = x;
			mNormals[52 * 52 * 2 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][1] = z;
			mNormals[52 * 52 * 2 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 2 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][0] = -x;
			mNormals[52 * 52 * 2 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][1] = z;
			mNormals[52 * 52 * 2 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 2 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][0] = x;
			mNormals[52 * 52 * 2 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][1] = z;
			mNormals[52 * 52 * 2 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][2] = -y;

			mNormals[52 * 52 * 2 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][0] = -x;
			mNormals[52 * 52 * 2 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][1] = z;
			mNormals[52 * 52 * 2 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][2] = -y;

			//surface 4: -y direction
			mNormals[52 * 52 * 3 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][0] = x;
			mNormals[52 * 52 * 3 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][1] = -z;
			mNormals[52 * 52 * 3 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 3 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][0] = -x;
			mNormals[52 * 52 * 3 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][1] = -z;
			mNormals[52 * 52 * 3 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][2] = y;

			mNormals[52 * 52 * 3 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][0] = x;
			mNormals[52 * 52 * 3 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][1] = -z;
			mNormals[52 * 52 * 3 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][2] = -y;

			mNormals[52 * 52 * 3 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][0] = -x;
			mNormals[52 * 52 * 3 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][1] = -z;
			mNormals[52 * 52 * 3 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][2] = -y;

			//surface 5: z direction
			mNormals[52 * 52 * 4 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][0] = x;
			mNormals[52 * 52 * 4 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][1] = y;
			mNormals[52 * 52 * 4 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][2] = z;

			mNormals[52 * 52 * 4 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][0] = -x;
			mNormals[52 * 52 * 4 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][1] = y;
			mNormals[52 * 52 * 4 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][2] = z;

			mNormals[52 * 52 * 4 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][0] = x;
			mNormals[52 * 52 * 4 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][1] = -y;
			mNormals[52 * 52 * 4 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][2] = z;

			mNormals[52 * 52 * 4 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][0] = -x;
			mNormals[52 * 52 * 4 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][1] = -y;
			mNormals[52 * 52 * 4 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][2] = z;

			//surface 6: -z direction
			mNormals[52 * 52 * 5 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][0] = x;
			mNormals[52 * 52 * 5 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][1] = y;
			mNormals[52 * 52 * 5 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 + j)][2] = -z;

			mNormals[52 * 52 * 5 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][0] = -x;
			mNormals[52 * 52 * 5 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][1] = y;
			mNormals[52 * 52 * 5 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 + j)][2] = -z;

			mNormals[52 * 52 * 5 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][0] = x;
			mNormals[52 * 52 * 5 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][1] = -y;
			mNormals[52 * 52 * 5 + static_cast<int>(26 + i) + 52 * static_cast<int>(26 - j)][2] = -z;

			mNormals[52 * 52 * 5 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][0] = -x;
			mNormals[52 * 52 * 5 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][1] = -y;
			mNormals[52 * 52 * 5 + static_cast<int>(26 - i) + 52 * static_cast<int>(26 - j)][2] = -z;
		}
	}

	mNormals[IndexNoNormal][0] = 0.0f;
	mNormals[IndexNoNormal][1] = 0.0f;
	mNormals[IndexNoNormal][2] = 0.0f;

	mInitialized = true;
}

float* AgPointCloudNormalUtils::getNormals() {
	if (!mInitialized) initialize();

	return &mNormals[0][0];
}