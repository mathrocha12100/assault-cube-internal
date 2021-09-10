#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

enum class MemoryTypes { Float, Integer };

namespace mem {
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	void Nop(BYTE* dst, unsigned int size);
	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
	void ChangeValueOfMemoryValue(MemoryTypes type, uintptr_t adress);

	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len);
	BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);
}