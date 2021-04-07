#pragma once

#include "sciter\sciter-x.h"
#include "sciter\sciter-x-host-callback.h"
#include "sciter\sciter-x-request.hpp"
#include "sciter\sciter-x-dom.hpp"
#include "sciter\sciter-x-behavior.h"

#include "UnigineEngine.h"
#include "UnigineGui.h"
#include "UnigineWidgets.h"
#include "UnigineEditor.h"
#include "UnigineTextures.h"
#include "UnigineApp.h"
#include "UnigineMaterials.h"

extern HINSTANCE ghInstance;

class SciterImpl 
{
public:
    static void init();
    static void createHUDWidgetSprite();
    static void Render();

    static void Destroyed();

    static UINT SC_CALLBACK SciterCallback(LPSCITER_CALLBACK_NOTIFICATION pns, LPVOID callbackParam);
    static UINT DoLoadData(LPSCN_LOAD_DATA pnmld);
    static UINT DoAttachBehavior(LPSCN_ATTACH_BEHAVIOR lpab);

    static void CreateTexture(Unigine::WidgetSpritePtr sprite, void* pixels, uint32_t width, uint32_t height, uint32_t stride);

    static void DrawHUD();
    static void HandleMouse();
};