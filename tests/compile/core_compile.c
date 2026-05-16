#include "glcp/glcp.h"

int main(void)
{
	void* glcp_core_compile_symbols[] = {
		(void*)glCreateShader,
		(void*)glBindBuffer,
		(void*)glVertexAttribPointer
	};
	return glcp_core_compile_symbols[0] == 0 ? 0 : 0;
}
