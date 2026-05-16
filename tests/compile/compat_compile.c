#include "glcp/glcp_compat.h"

int main(void)
{
	void* glcp_compat_legacy_symbols[] = {
		(void*)glPushAttrib,
		(void*)glPopAttrib,
		(void*)glBegin
	};
#if defined(_WIN32)
	void* glcp_compat_loader_symbols[] = {
		(void*)glBindBuffer
	};
	(void)glcp_compat_loader_symbols;
#endif
	return glcp_compat_legacy_symbols[0] == 0 ? 0 : 0;
}
