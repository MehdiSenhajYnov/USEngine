#include <fileformat_stbimage.h>

extern "C" VDE_PLUGIN_EXPORT vde::util::PluginBase* CreatePlugin(vde::util::PluginContext* context)
{
	return new FileFormat_stbimage(context->graphicsContext);
}
