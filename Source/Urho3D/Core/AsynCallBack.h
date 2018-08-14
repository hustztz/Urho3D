#pragma once
#include "../Container/Ptr.h"
#include <type_traits>
#include <functional>
#include <utility>
namespace Urho3D
{
	class AsynCallBackHandler : public RefCounted
	{
	public:
		virtual void Execute() {}
	};

	class ResourceAsynCallBackHandler : public AsynCallBackHandler
	{
	public:
		virtual void SetArg(void* arg) {}
	};

	template <typename THostClass1, typename TArg>
	class ResourceAsynCallBackHandlerImpl : public ResourceAsynCallBackHandler
	{
	public:
		typedef TArg Arg;
		typedef THostClass1 HostClass;
		typedef void (HostClass::*CallBackPtr)(Arg*);

		ResourceAsynCallBackHandlerImpl(HostClass* callobject, CallBackPtr callback)
			:callobject_(SharedPtr<HostClass>(callobject))
			, arg_(SharedPtr<Arg>(nullptr))
			, callback_(callback)
		{
			assert(callobject_);
			assert(callback_);
		}

		void Execute()
		{
			if (callobject_.Refs() > 1)
			{
				auto* obj = callobject_.Get();
				(obj->*callback_)(arg_.Get());
			}
		}

		void SetArg(void* arg)
		{
			arg_ = SharedPtr<Arg>(static_cast<Arg*>(arg));
		}
	private:
		CallBackPtr callback_;
		SharedPtr<Arg> arg_;
		SharedPtr<HostClass> callobject_;
	};

	template <typename THostClass>
	class AsynCallBackHandler11Impl : public AsynCallBackHandler
	{
	public:
		typedef THostClass HostClass;
		AsynCallBackHandler11Impl(HostClass* callobject, std::function<void(void)> callback)
			:callobject_(SharedPtr<HostClass>(callobject))
			, callback_(std::move(callback))
		{
			assert(callobject_);
			assert(callback_);
		}

		void Execute()
		{
			if (callobject_.Refs() > 1)
			{
				callback_();
			}
		}
	private:
		std::function<void(void)> callback_;
		SharedPtr<HostClass> callobject_;
	};

	template<typename TClass, typename Arg>
	ResourceAsynCallBackHandler* NewResAsynCallBackWarp(TClass* obj, void(TClass::*callback)(Arg))
	{
		return new Urho3D::ResourceAsynCallBackHandlerImpl<TClass, typename std::remove_pointer<typename std::remove_const<Arg>::type>::type>(obj, callback);
	}

#define URHO3D_ASYNLOADRESCALLBACK(className, function) (NewResAsynCallBackWarp(this, &className::function))
#define URHO3D_ASYNCALLBACK_LAMD(function, ...) (new Urho3D::AsynCallBackHandler11Impl<RefCounted>(this, std::bind(function, ##__VA_ARGS__)))
}
