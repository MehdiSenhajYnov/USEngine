#pragma once
#include "../../List.h"

namespace vde::core::gpu
{
	class Buffer;
}

struct Renderable {
	Renderable() = default;

	Renderable(const USList<vde::core::gpu::Buffer*>& all_vertex_buffers, vde::core::gpu::Buffer* index_buffer, int index_count)
		: AllVertexBuffers(all_vertex_buffers),
		  IndexBuffer(index_buffer),
		  IndexCount(index_count)
	{
	}

	USList<vde::core::gpu::Buffer*> AllVertexBuffers; // Faut que ca soit dans meme ordre que le pipeline, actuellement {Vb, Uvb}
	vde::core::gpu::Buffer* IndexBuffer;
	int IndexCount;
};
