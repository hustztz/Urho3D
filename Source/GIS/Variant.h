#pragma once

#include <new>
#include <variant>

namespace GIS {

	template <typename...T>
	struct Variant : public std::variant<T...> {
	public:

		template <typename R>
		const R & Get() const {
			return std::get<R>(*this);
		}

		template <typename R>
		R & Get() {
			return std::get<R>(*this);
		}

	};

}