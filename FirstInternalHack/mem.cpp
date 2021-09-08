#include "pch.h"
#include "mem.h"

void mem::Patch(BYTE* dst, BYTE* src, unsigned int size) {
	DWORD oldprotected;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotected);

	memcpy( dst, src, size);

	VirtualProtect(dst, size, oldprotected, &oldprotected);

}

void mem::Nop(BYTE* dst, unsigned int size) {
	DWORD oldprotected;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotected);

	memset(dst, 0x90, size);

	VirtualProtect(dst, size, oldprotected, &oldprotected);
}

uintptr_t mem::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); i++) {
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}

	return addr;
}

void mem::ChangeValueOfMemoryValue(MemoryTypes type, uintptr_t adress) {
	switch (type) {
		case MemoryTypes::Integer: {
			int* newValue = (int*)adress;
			*newValue = 1337;
			break;
		}
		case MemoryTypes::Float: {
			float* newValue = (float*)adress;
			*newValue = 1337;
			break;
		}
	}
}
