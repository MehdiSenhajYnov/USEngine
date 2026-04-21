#ifndef VDE__CORE__ASSETS__ASSET_H
#define VDE__CORE__ASSETS__ASSET_H
#pragma once

#include <core/assets/assetsource.h>
#include <util/plugin.h>
#include <util/pluginregistry.h>
#include <util/property.h>

namespace vde::core::assets
{
	template<typename T>
	class Asset
	{
		std::unique_ptr<AssetSource> m_source;
		std::unique_ptr<T>           m_value;

	public:
		explicit Asset()
			: m_source(nullptr)
		{
		}

		explicit Asset(std::unique_ptr<T> value)
			: m_source(nullptr)
			, m_value(std::move(value))
		{
		}

		void Reset()
		{
			m_source.reset();
			m_value.reset();
		}

		void SetValue(std::unique_ptr<T> value)
		{
			m_source.reset();
			m_value = std::move(value);
		}

		template<typename... ARGS>
		explicit Asset(std::unique_ptr<AssetSource> src, ARGS... args)
			: m_source(std::move(src))
		{
			Load(args...);
		}

		Asset& operator=(std::unique_ptr<AssetSource>&& src)
		{
			m_source = std::move(src);
			return *this;
		}

		template<typename... ARGS>
		void Load(ARGS... args);

		template<typename... ARGS>
		bool ReloadIfChanged(ARGS... args)
		{
			if (m_source->HasChanged())
			{
				Load(args...);
				return true;
			}

			return false;
		}

		bool HasValue() const { return m_value != nullptr; }
		T& Value() { return *m_value; }
		operator T& () { return Value(); }
	};
}

#endif /* VDE__CORE__ASSETS__ASSET_H */
