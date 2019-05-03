#include "Hooks.h"
#include "../Directx/Directx.h"

Hooks    g_Hooks;
bool firstTime = true;
void Hooks::Init()
{
	logF("Initting Hooks");
	// GameMode::tick Signature
	// CC 8B 41 ?? 89 41 ?? C3
	//    ^^ Function starts here
	void* func = reinterpret_cast<void*>(Utils::FindSignature("CC 8B 41 ?? 89 41 ?? C3") + 1);
	g_Hooks.gameMode_tickHook = std::make_unique<FuncHook>(func, Hooks::GameMode_tick);
	g_Hooks.gameMode_tickHook->init();

	// ChatScreenController::_sendChatMessage
	// 40 57 48 83 EC 20 48 83 B9 ?? ?? ?? ?? 00 48 8B F9 0F 85
	// ^^ 
	void* _sendChatMessage = reinterpret_cast<void*>(Utils::FindSignature("40 57 48 83 EC 20 48 83 B9 ?? ?? ?? ?? 00 48 8B F9 0F 85"));
	g_Hooks.chatScreen_sendMessageHook = std::make_unique<FuncHook>(_sendChatMessage, Hooks::ChatScreenController_sendChatMessage);
	g_Hooks.chatScreen_sendMessageHook->init();

	//IDXGISwapChain::present;
	// using vtable found with dummy thing
	void** swapChainVtable = static_cast<void**>(getSwapChain());
	void* presentFunc = swapChainVtable[8];
	g_Hooks.d3d11_presentHook = std::make_unique<FuncHook>(presentFunc, Hooks::d3d11_present);
	g_Hooks.d3d11_presentHook->init();

	// 
	void* _shit = reinterpret_cast<void*>(Utils::FindSignature("30 5F C3 CC 48 8B C4 55 56 57 41 54") + 4);
	g_Hooks.renderTextHook = std::make_unique<FuncHook>(_shit, Hooks::renderText);
	g_Hooks.renderTextHook->init();

	// I8n::get doesnt want to work
	//void *_shitshikt = reinterpret_cast<void*>(g_Data.getModule()->ptrBase + 0xB577C0);
	//g_Hooks.I8n_getHook = std::make_unique<FuncHook>(_shitshikt, Hooks::I8n_get);
	//g_Hooks.I8n_getHook->init();

	//void *shat = reinterpret_cast<void*>(g_Data.getModule()->ptrBase + 0x6908A0);
	//g_Hooks.Options_getVersionStringHook = std::make_unique<FuncHook>(shat, Hooks::Options_getVersionString);
	//g_Hooks.Options_getVersionStringHook->init();

	void* boii = reinterpret_cast<void*>(Utils::FindSignature("0F 28 C2 C7 42 0C 00 00 80 3F F3"));
	g_Hooks.Dimension_getFogColorHook = std::make_unique<FuncHook>(boii, Hooks::Dimension_getFogColor);
	g_Hooks.Dimension_getFogColorHook->init();

	void* destroyBlok = reinterpret_cast<void*>(Utils::FindSignature("55 57 41 56 48 8D 68 ?? 48 81 EC ?? ?? ?? ?? 48 C7 45 ?? FE FF FF FF 48 89 58 ?? 48 89 70 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 ?? 45 0F B6 F0") - 3);
	g_Hooks.GameMode_destroyBlockHook = std::make_unique <FuncHook>(destroyBlok, Hooks::GameMode_destroyBlock);
	g_Hooks.GameMode_destroyBlockHook->init();

	//logF("Hooks hooked");
}

void Hooks::Restore()
{
	g_Hooks.gameMode_tickHook->Restore();
	g_Hooks.chatScreen_sendMessageHook->Restore();
	g_Hooks.d3d11_presentHook->Restore();
	g_Hooks.renderTextHook->Restore();
	g_Hooks.Options_getVersionStringHook->Restore();
	//g_Hooks.I8n_getHook->Restore();
	g_Hooks.GameMode_destroyBlockHook->Restore();
}

void __fastcall Hooks::GameMode_tick(C_GameMode * _this)
{
	static auto oTick = g_Hooks.gameMode_tickHook->GetOriginal<GameMode_tick_t>();
	oTick(_this); // Call Original Func
	GameData::updateGameData(_this);
}

void __fastcall Hooks::ChatScreenController_sendChatMessage(uint8_t * _this)
{
	static auto oSendMessage = g_Hooks.chatScreen_sendMessageHook->GetOriginal<ChatScreen_sendChatMessage_t>();
	uintptr_t* idk = reinterpret_cast<uintptr_t*>(_this + 0x688);
	if (*idk) {
		char* message = reinterpret_cast<char*>(_this + 0x678);
		if (*reinterpret_cast<__int64*>(_this + 0x690) >= 0x10)
			message = *reinterpret_cast<char**>(message);
		__int64* v6 = reinterpret_cast<__int64*>(_this + 0x690);

		if (*message == '.') {
			logF("Command: %s", message);
			logF("Yote %llX", message);
			//*message = 0x0; // Remove command in textbox
			//*v6 = 0x0;
			//*idk = 0x0;
			return;
		}
	}
	oSendMessage(_this);
}

HRESULT __stdcall Hooks::d3d11_present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	static auto oPresent = g_Hooks.d3d11_presentHook->GetOriginal<d3d11_present_t>();
	return oPresent(pSwapChain, SyncInterval, Flags);
}

__int64 __fastcall Hooks::renderText(__int64 yeet, C_MinecraftUIRenderContext* renderCtx) // I have no idea what this function is, only thing i know is that screencontext is in yote
{
	static auto oText = g_Hooks.renderTextHook->GetOriginal<renderText_t>();

	__int64 retval = oText(yeet, renderCtx);

	std::string textStr = std::string("Horion");
	TextHolder* text = new TextHolder(textStr);

	uintptr_t font = reinterpret_cast<uintptr_t>(g_Data.getClientInstance()->getFont());
	static float eins = 1.f;
	
	float textLeng = renderCtx->getLineLength(font, text, eins, false);

	float* reee = new float[4]; // Absolute Screen coordinates
	reee[0] = 0;    // startX
	reee[1] = textLeng + 2;  //   endX
	reee[2] = 0;  // startY
	reee[3] = 12;  //   endY

	float* col = new float[8];
	col[0] = 0.f;
	col[1] = 0.f;
	col[2] = 0.f;
	col[3] = 1;

	renderCtx->fillRectangle(reee, col, 0.2f); // alpha

	col[0] = 0.3f;
	col[1] = 1;
	col[2] = 0.3f;
	col[3] = 1;

	float* pos = new float[4];
	pos[0] = 1;
	pos[1] = 50;
	pos[2] = 1;
	pos[3] = 12;

	uintptr_t oof = 0;
	memset(&oof, 0xFF, 4);
	
	renderCtx->drawText(font, pos, text, col, 1, 0, &eins, &oof);
	renderCtx->flushText(0);

	return retval;
}

char* __fastcall Hooks::I8n_get(void* f, char* str)
{
	static auto oGet = g_Hooks.renderTextHook->GetOriginal<I8n_get_t>();

	//static char* shit = "yeet";

	//BigCantWork yote;
	//strcpy_s(yote.boiii, 16, shit);
	//yote.length1 = 4;
	//yote.length2 = 4;
	//yote.length3 = 4;

	//if (strcmp(str, "menu.play") == 0)
		//return &yote;
	return oGet(f, str);
}

TextHolder * __fastcall Hooks::Options_getVersionString(void * ya, __int64 idk)
{
	static auto oGetVer = g_Hooks.Options_getVersionStringHook->GetOriginal<Options_getVersionString_t>();
	TextHolder* version = oGetVer(ya, idk);
	version->inlineText[0] = 'E';
	return version;
}

float * Hooks::Dimension_getFogColor(__int64 a1, float * color, float brightness)
{
	static auto oGetFogColor = g_Hooks.Dimension_getFogColorHook->GetOriginal<Dimension_getFogColor_t>();

	static float rcolors[4];

	//float* retval = oGetFogColor(a1, color, brightness);

	if (rcolors[3] < 1) {
		rcolors[0] = 1;
		rcolors[1] = 0.2f;
		rcolors[2] = 0.2f;
		rcolors[3] = 1;
	}

	Utils::ColorConvertRGBtoHSV(rcolors[0], rcolors[1], rcolors[2], rcolors[0], rcolors[1], rcolors[2]); // perfect code, dont question this

	rcolors[0] += 0.001f;
	if (rcolors[0] >= 1)
		rcolors[0] = 0;

	Utils::ColorConvertHSVtoRGB(rcolors[0], rcolors[1], rcolors[2], rcolors[0], rcolors[1], rcolors[2]);

	return rcolors;
}

void Hooks::GameMode_destroyBlock(void * _this, C_BlockPos * pos, uint8_t face)
{
	static auto oDestroyBlock = g_Hooks.GameMode_destroyBlockHook->GetOriginal<GameMode_destroyBlock_t>();

	oDestroyBlock(_this, pos, face);
}
