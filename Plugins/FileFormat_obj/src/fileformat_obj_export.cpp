#include <fileformat_obj.h>

extern "C" VDE_PLUGIN_EXPORT vde::util::PluginBase* CreatePlugin(vde::util::PluginContext* context)
{
	return new FileFormat_OBJ(context->graphicsContext);
}
