#include <util/pluginregistry.h>

#include <unordered_map>
#include <vector>

#if defined(_WIN32)
# define WIN32_LEAN_AND_MEAN
# define WIN32_EXTRA_LEAN
# include <Windows.h>
#else
# include <dlfcn.h>
#endif

using namespace vde::util;

struct PluginRegistry::Impl
{
#if defined(_WIN32)
	using ModuleHandle = HMODULE;
#else
	using ModuleHandle = void*;
#endif

	struct Module
	{
		ModuleHandle hModule;
		PluginBase* plugin;
	};

	std::unordered_map<EPluginFamily, std::vector<Module>> modules;
};

PluginRegistry::PluginRegistry()
	: m_pImpl(new Impl)
{
}

PluginRegistry::~PluginRegistry() noexcept
{
	for (auto& [_, plugins] : m_pImpl->modules)
	{
		for (auto& p : plugins)
		{
			delete p.plugin;
#if defined(_WIN32)
			FreeLibrary(p.hModule);
#else
			dlclose(p.hModule);
#endif
		}
	}
}

PluginContext& PluginRegistry::Context()
{
	return m_context;
}

void PluginRegistry::Load(const std::filesystem::path& path)
{
	Impl::ModuleHandle module = nullptr;

#if defined(_WIN32)
	module = LoadLibraryW(path.c_str());
#else
	module = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
	if (module)
	{
#if defined(_WIN32)
		CreatePluginFn createFn = reinterpret_cast<CreatePluginFn>(GetProcAddress(module, "CreatePlugin"));
#else
		CreatePluginFn createFn = reinterpret_cast<CreatePluginFn>(dlsym(module, "CreatePlugin"));
#endif
		if (createFn)
		{
			PluginBase* plugin = createFn(&m_context);
			m_pImpl->modules[plugin->Family()].push_back({ module, plugin });
			return;
		}

#if defined(_WIN32)
		FreeLibrary(module);
#else
		dlclose(module);
#endif
	}
}

void PluginRegistry::LoadAllFromDirectory(const std::filesystem::path& directory)
{
	if (!std::filesystem::is_directory(directory))
		return;

	static constexpr auto kModuleExtension =
#if defined(_WIN32)
		".dll";
#elif defined(__APPLE__)
		".dylib";
#else
		".so";
#endif

	for (const auto& e : std::filesystem::directory_iterator(directory))
		if (e.is_regular_file() && e.path().extension() == kModuleExtension)
			Load(e.path());
}

std::vector<PluginBase*> PluginRegistry::ListFamily(EPluginFamily family) const
{
	if (!m_pImpl->modules.contains(family))
		return {};

	std::vector<PluginBase*> result;

	for (auto& m : m_pImpl->modules[family])
		result.push_back(m.plugin);

	return result;
}
