#pragma once
#include "../Math/Vector3.h"

namespace ambergris {
	namespace PointCloudEngine {

		//Normal lookup table/compression (convert a 3-element float vector into a 14bit index and vice versa)
		static const int NumberOfNormalBins = 16225;
		static const int IndexNoNormal = 16224;
		class AgPointCloudNormalUtils
		{
		public:
			//set up normal lookup table
			static int indexForNormal(const Urho3D::Vector3& val);
			static const Urho3D::Vector3 normalForIndex(int val);
			static float* getNormals();

		private:
			static void initialize();
			static float mNormals[NumberOfNormalBins][3];
			static bool mInitialized;
		};
	}
}