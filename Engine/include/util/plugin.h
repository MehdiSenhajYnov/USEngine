#ifndef VDE__UTIL__PLUGIN_H
#define VDE__UTIL__PLUGIN_H
#pragma once

#include <memory>
#include <core/assets/assetsource.h>

namespace vde::core
{
	class GraphicsContext;
}

namespace vde::util
{
#if defined(_WIN32)
# define VDE_PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
# define VDE_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
# define VDE_PLUGIN_EXPORT
#endif

	enum class EPluginFamily
	{
		FileFormat = 0,

		Count
	};

	struct PluginContext
	{
		vde::core::GraphicsContext* graphicsContext = nullptr;
	};

	class PluginBase
	{
	public:
		virtual ~PluginBase() noexcept = default;
		virtual EPluginFamily Family() const = 0;
	};

	template<EPluginFamily T>
	class Plugin
		: public PluginBase
	{
	public:
		static constexpr EPluginFamily kFAMILY = T;
		EPluginFamily Family() const override { return kFAMILY; }
	};

	class FileFormatPluginBase
		: public Plugin<EPluginFamily::FileFormat>
	{
	public:
		virtual bool CanLoad(const std::string& path) = 0;
	};

	template<typename T>
	class FileFormatPlugin
		: public FileFormatPluginBase
	{
	public:
		virtual std::unique_ptr<T> Load(const void* data, size_t size) = 0;
	};

	using CreatePluginFn = PluginBase*(*)(PluginContext*);
}

#endif /* VDE__UTIL__PLUGIN_H */
