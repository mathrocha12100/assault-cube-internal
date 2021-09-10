#include "pch.h"
#include "hook.h"


bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return false;

	DWORD curProctection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProctection);

	uintptr_t relativeAdress = dst - src - 5;

	*src = 0xE9; // jmp instruction

	*(uintptr_t*)(src + 1) = relativeAdress;

	VirtualProtect(src, len, curProctection, &curProctection);

	return true;
}

BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return 0;

	// Create Gateway
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// writen the stolen bytes to the gateway

	memcpy_s(gateway, len, src, len);

	//get the gateway to destination adress

	uintptr_t gatewayRelativeAddr = src - gateway - 5;

	// add the jmp code to the end of gateway

	*(gateway + len) = 0xE9; //jmp instruction

	// Write the adress of the gateway to the jmp

	*(uintptr_t*)(uintptr_t)(gateway + len + 1) = gatewayRelativeAddr;

	// Perform the detour

	Detour32(src, dst, len);

	return gateway;
}

Hook::Hook(BYTE* src, BYTE* dst, BYTE* ptrToGatewayFcPtr, uintptr_t len) {
	this->src = src;
	this->dst = dst;
	this->ptrToGatewayFcPtr = ptrToGatewayFcPtr;
	this->len = len;
}

Hook::Hook(const char* exportName, const char* modName, BYTE* dst, BYTE* ptrToGatewayFcPtr, uintptr_t len) {

	HMODULE hMod = GetModuleHandleA(modName);

	this->src = (BYTE*)GetProcAddress(hMod, exportName);

	this->src = src;
	this->dst = dst;
	this->ptrToGatewayFcPtr = ptrToGatewayFcPtr;
	this->len = len;
}

void Hook::Enable() {
	memcpy(originalBytes, src, len);
	*(uintptr_t*)ptrToGatewayFcPtr = (uintptr_t)TrampHook32(src, dst, len);
	bStatus = true;
}

void Hook::Disable() {
	mem::Patch(src, originalBytes, len);
	bStatus = false;
}

void Hook::Toggle() {
	if (bStatus) Disable();
	else Enable();
}