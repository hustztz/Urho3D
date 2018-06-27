#pragma once
#include <stdint.h>

namespace ambergris {
	namespace PointCloudEngine {

		struct AgCompactColor {
			AgCompactColor() {
				r_ = g_ = b_ = a_ = 0;
			}
			AgCompactColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
				: r_(r), g_(g), b_(b), a_(a)
			{
			}
			uint8_t                            r_;
			uint8_t                            g_;
			uint8_t                            b_;
			uint8_t                            a_;
		};
	}
}
