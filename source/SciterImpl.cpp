#include "SciterImpl.h"

#include "sciter\facade-resources.cpp"
#include "sciter\sciter-x-lite-keycodes.h"

#include "sciter\sciter-x.h"
#include "sciter\sciter-x-host-callback.h"
#include "sciter\sciter-x-request.hpp"
#include "sciter\sciter-x-dom.hpp"
#include "sciter\sciter-x-behavior.h"

#include "sciter\sciter-x-window.hpp"

#include <UnigineMeshDynamic.h>
#include <UnigineRender.h>
#include <UnigineWidgets.h>
#include "UnigineApp.h"

using namespace Unigine;

WidgetSpritePtr hud;
GuiPtr gui;

static BlobPtr blob;
static TexturePtr texture;

#define SciterInstanceId ((void *)1)

bool bInitialized = false;
bool sciter_needs_drawing = false;

const int SpecialKeyToInt(unsigned int key)
{
    int ConvertedKey = 0;

    switch (key)
    {
    case App::KEY_ESC:
        ConvertedKey = KB_ESCAPE;
        break;
    case App::KEY_TAB:
        ConvertedKey = KB_TAB;
        break;
    case App::KEY_BACKSPACE:
        ConvertedKey = KB_BACKSPACE;
        break;
    case App::KEY_RETURN:
        ConvertedKey = KB_RETURN;
        break;
    case App::KEY_DELETE:
        ConvertedKey = KB_DELETE;
        break;
    case App::KEY_INSERT:
        ConvertedKey = KB_INSERT;
        break;
    case App::KEY_HOME:
        ConvertedKey = KB_HOME;
        break;
    case App::KEY_END:
        ConvertedKey = KB_END;
        break;
    case App::KEY_PGUP:
        ConvertedKey = KB_PAGE_UP;
        break;
    case App::KEY_PGDOWN:
        ConvertedKey = KB_PAGE_DOWN;
        break;
    case App::KEY_LEFT:
        ConvertedKey = KB_LEFT;
        break;
    case App::KEY_RIGHT:
        ConvertedKey = KB_RIGHT;
        break;
    case App::KEY_UP:
        ConvertedKey = KB_UP;
        break;
    case App::KEY_DOWN:
        ConvertedKey = KB_DOWN;
        break;
    case App::KEY_SHIFT:
        ConvertedKey = KB_SHIFT;
        break;
    case App::KEY_CTRL:
        ConvertedKey = KB_CONTROL;
        break;
    case App::KEY_CMD:
        ConvertedKey = KB_LEFT_SUPER;
        break;
    case App::KEY_SCROLL:
        ConvertedKey = KB_SCROLL;
        break;
    case App::KEY_CAPS:
        ConvertedKey = KB_CAPS_LOCK;
        break;
    case App::KEY_NUM:
        ConvertedKey = KB_NUMLOCK;
        break;
    case App::KEY_F1:
        ConvertedKey = KB_F1;
        break;
    case App::KEY_F2:
        ConvertedKey = KB_F2;
        break;
    case App::KEY_F3:
        ConvertedKey = KB_F3;
        break;
    case App::KEY_F4:
        ConvertedKey = KB_F4;
        break;
    case App::KEY_F5:
        ConvertedKey = KB_F5;
        break;
    case App::KEY_F6:
        ConvertedKey = KB_F6;
        break;
    case App::KEY_F7:
        ConvertedKey = KB_F7;
        break;
    case App::KEY_F8:
        ConvertedKey = KB_F8;
        break;
    case App::KEY_F9:
        ConvertedKey = KB_F9;
        break;
    case App::KEY_F10:
        ConvertedKey = KB_F10;
        break;
    case App::KEY_F11:
        ConvertedKey = KB_F11;
        break;
    case App::KEY_F12:
        ConvertedKey = KB_F12;
        break;
    case App::NUM_KEYS:
        ConvertedKey = KB_NUMPAD0;
        break;
    }

    return ConvertedKey;
}

static int on_button_pressed(int button)
{
    unsigned mb = 0;
    LPARAM lParam;
    MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_DOWN;

    switch (button) {
        case App::BUTTON_LEFT:
            mb |= MOUSE_BUTTONS::MAIN_MOUSE_BUTTON;
            break;
        case App::BUTTON_RIGHT:
            mb |= MOUSE_BUTTONS::PROP_MOUSE_BUTTON;
            break;
        case App::BUTTON_MIDDLE:
            mb |= MOUSE_BUTTONS::MIDDLE_MOUSE_BUTTON;
        break;
    }

    POINT pos = { int(App::getMouseX()),int(App::getMouseY()) };

    auto ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_MOUSE(evt, MOUSE_BUTTONS(mb), KEYBOARD_STATES(0), pos));
    if (!ok)
        ok = 0;

    return 0;
}

static int on_button_released(int button)
{
    unsigned mb = 0;
    MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_UP;
    POINT pos = { int(App::getMouseX()),int(App::getMouseY()) };

    switch (button) {
    case App::BUTTON_LEFT:
        mb |= MOUSE_BUTTONS::MAIN_MOUSE_BUTTON;
        break;
    case App::BUTTON_RIGHT:
        mb |= MOUSE_BUTTONS::PROP_MOUSE_BUTTON;
        break;
    case App::BUTTON_MIDDLE:
        mb |= MOUSE_BUTTONS::MIDDLE_MOUSE_BUTTON;
        break;
    }

    auto ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_MOUSE(evt, MOUSE_BUTTONS(mb), KEYBOARD_STATES(0), pos));
    if (!ok)
        ok = 0;

    return 0;
}

static int on_key_pressed(unsigned int key)
{
    unsigned mb = 0;
    KEY_EVENTS evt = KEY_EVENTS::KEY_DOWN;

    auto ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_KEY(evt, SpecialKeyToInt(key), KEYBOARD_STATES(0)));
    if (!ok)
        ok = 0;
    return 0;
}

static int on_key_released(unsigned int key)
{
    unsigned mb = 0;
    KEY_EVENTS evt = KEY_EVENTS::KEY_UP;

    auto ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_KEY(evt, SpecialKeyToInt(key), KEYBOARD_STATES(0)));
    if (!ok)
        ok = 0;

    return 0;
}

static int on_unicode_key_pressed(unsigned int key)
{
    if (key < App::KEY_ESC || key >= App::NUM_KEYS)
    {
        char ch = static_cast<char>(key);
        unsigned mb = 0;
        KEY_EVENTS evt = KEY_EVENTS::KEY_CHAR;

        auto ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_KEY(evt, ch, KEYBOARD_STATES(0)));
        if (!ok)
            ok = 0;

    }
    return 0;
}

void SciterImpl::init()
{
    App::setKeyPressFunc(on_key_pressed);
    App::setKeyReleaseFunc(on_key_released);
    App::setButtonPressFunc(on_button_pressed);
    App::setButtonReleaseFunc(on_button_released);
    App::setKeyPressUnicodeFunc(on_unicode_key_pressed);

    gui = Gui::get();
    blob = Blob::create();
    texture = Texture::create();
    
    createHUDWidgetSprite();

    auto version = SAPI()->version;
    Log::message("%d \n",version);

    Log::message("Trying to initialize Sciter");

    SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES,
        ALLOW_FILE_IO |
        ALLOW_SOCKET_IO |
        ALLOW_EVAL |
        ALLOW_SYSINFO);

    SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, true);

    bool ok;

    ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_CREATE(GFX_LAYER_SKIA, TRUE));

    if (ok)
        Log::message("Sciter Engine Initialized");

    SciterSetCallback(SciterInstanceId, SciterCallback, NULL);

    ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_RESOLUTION(UINT(100)));
    if (!ok)
        Log::message("Couldn't set resolution");

    ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_SIZE(App::getWidth(),App::getHeight()));
    if (!ok)
        Log::message("Couldn't set size");
    
    sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

    ok = SciterLoadFile(SciterInstanceId, L"facade\\tests\\animations\\demo.htm"); // does not load animations
    //ok = SciterLoadFile(SciterInstanceId, L"C:\\facade\\tests\\animations\\demo.htm"); // loads anuimations

    if (ok)
        Log::message("File loaded!");

    OleInitialize(NULL); // for shell interaction: drag-n-drop, etc.

    bInitialized = true;

    SciterProcX(SciterInstanceId, SCITER_X_MSG_FOCUS(true));
}

void SciterImpl::Render() 
{
    SciterProcX(SciterInstanceId, SCITER_X_MSG_HEARTBIT(App::getIFps()));

    if (!bInitialized)
        return;

    auto on_bitmap = [](LPCBYTE rgba, INT x, INT y, UINT width, UINT height, LPVOID param)
    {
        auto* self = reinterpret_cast<SciterImpl*>(param);

        CreateTexture(hud, (void*)rgba, width, height, width * height * 4);
    };

    SCITER_X_MSG_PAINT paint;
    paint.targetType = SPT_RECEIVER;
    paint.target.receiver.param = NULL;
    paint.target.receiver.callback = on_bitmap;
    
    if (sciter_needs_drawing) {
        sciter_needs_drawing = false;

        auto ok = SciterProcX(SciterInstanceId, paint);

        if (!ok)
            Log::message("Couldn't render");
    }
}

void BGRA_To_RGBA(unsigned char* input, int pixel_width, int pixel_height, unsigned char* output)
{
    int offset = 0;

    for (int y = 0; y < pixel_height; y++) {
        for (int x = 0; x < pixel_width; x++) {

            auto temp = output[offset];
            output[offset] = input[offset + 2];
            output[offset + 1] = input[offset + 1];
            output[offset + 2] = temp;
            output[offset + 3] = input[offset + 3];

            offset += 4;
        }
    }
}

void SciterImpl::CreateTexture(Unigine::WidgetSpritePtr hud, void* pixels, uint32_t width, uint32_t height, uint32_t stride)
{
    if (!pixels)
        return;

    texture->create2D(width, height, Texture::FORMAT_RGBA8, Texture::DEFAULT_FLAGS);

    auto ConvertedPixels = (unsigned char *)pixels;

    //BGRA_To_RGBA(ConvertedPixels, width, height, ConvertedPixels); //performance impact

    blob->setData(ConvertedPixels, stride);

    texture->setBlob(blob);
    blob->setData(nullptr, 0);

    hud->setRender(texture);
}

void SciterImpl::createHUDWidgetSprite()
{
    hud = WidgetSprite::create(gui);
    hud->setPosition(0, 0);
    hud->setWidth(App::getWidth());
    hud->setHeight(App::getHeight());
    hud->setLayerBlendFunc(0, Gui::BLEND_ONE, Gui::BLEND_ONE_MINUS_SRC_ALPHA);

    gui->addChild(hud, Gui::ALIGN_OVERLAP);
}

UINT on_invalidate_rect(LPSCN_INVALIDATE_RECT pnm) 
{
    sciter_needs_drawing = true;  
    return 0;
}

UINT SC_CALLBACK SciterImpl::SciterCallback(LPSCITER_CALLBACK_NOTIFICATION pns, LPVOID callbackParam)
{
    switch (pns->code)
    {
    case SC_LOAD_DATA:          return DoLoadData((LPSCN_LOAD_DATA)pns);
    case SC_ATTACH_BEHAVIOR:    return DoAttachBehavior((LPSCN_ATTACH_BEHAVIOR)pns);
    case SC_INVALIDATE_RECT: return on_invalidate_rect((LPSCN_INVALIDATE_RECT)pns);
    }
    return 0;
}

void SciterImpl::DrawHUD()
{
    HandleMouse();
    Render();
}

void SciterImpl::HandleMouse()
{
    MOUSE_EVENTS  me = MOUSE_MOVE;
    POINT pos = { int(App::getMouseX()),int(App::getMouseY()) };

    auto ok = SciterProcX(SciterInstanceId, SCITER_X_MSG_MOUSE(me, MOUSE_BUTTONS(0), KEYBOARD_STATES(0), pos));
}

UINT SciterImpl::DoAttachBehavior(LPSCN_ATTACH_BEHAVIOR lpab)
{
    sciter::event_handler* pb = sciter::behavior_factory::create(lpab->behaviorName, lpab->element);
    if (pb)
    {
        lpab->elementTag = pb;
        lpab->elementProc = sciter::event_handler::element_proc;
        return TRUE;
    }
    return FALSE;
}

UINT SciterImpl::DoLoadData(LPSCN_LOAD_DATA pnmld)
{
    LPCBYTE pb = 0; UINT cb = 0;
    aux::wchars wu = aux::chars_of(pnmld->uri);
    if (wu.like(WSTR("res:*")))
    {
        // then by calling possibly overloaded load_resource_data method
        if (sciter::load_resource_data(GetModuleHandle(NULL), wu.start + 4, pb, cb))
            ::SciterDataReady(pnmld->hwnd, pnmld->uri, pb, cb);
    }
    else if (wu.like(WSTR("this://app/*"))) {
        // try to get them from archive (if any, you need to call sciter::archive::open() first)
        aux::bytes adata = sciter::archive::instance().get(wu.start + 11);
        if (adata.length)
            ::SciterDataReady(pnmld->hwnd, pnmld->uri, adata.start, adata.length);
    }
    return LOAD_OK;
}

void SciterImpl::Destroyed()
{
    SciterProcX(SciterInstanceId, SCITER_X_MSG_DESTROY());
    PostQuitMessage(0);
}
