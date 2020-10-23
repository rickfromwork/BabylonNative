#include "LibNativeBridge.h"

#import <Babylon/AppRuntime.h>
#import <Babylon/Graphics.h>
#import <Babylon/ScriptLoader.h>
#import <Babylon/Plugins/NativeEngine.h>
#import <Babylon/Plugins/NativeWindow.h>
#import <Babylon/Plugins/NativeXr.h>
#import <Babylon/Polyfills/Window.h>
#import <Babylon/Polyfills/XMLHttpRequest.h>
#import <Babylon/Polyfills/Canvas.h>
#import <Shared/InputManager.h>

std::unique_ptr<Babylon::Graphics> graphics{};
std::unique_ptr<Babylon::AppRuntime> runtime{};
std::unique_ptr<InputManager<Babylon::AppRuntime>::InputBuffer> inputBuffer{};

@implementation LibNativeBridge

- (instancetype)init
{
    self = [super init];
    return self;
}

- (void)dealloc
{
}

- (void)init:(void*)view width:(int)inWidth height:(int)inHeight
{
    inputBuffer.reset();
    runtime.reset();
    graphics.reset();

    float width = inWidth;
    float height = inHeight;
    void* windowPtr = view;
    
    graphics = Babylon::Graphics::InitializeFromWindow(windowPtr, width, height);
    runtime = std::make_unique<Babylon::AppRuntime>();
    inputBuffer = std::make_unique<InputManager<Babylon::AppRuntime>::InputBuffer>(*runtime);

    runtime->Dispatch([windowPtr, width, height](Napi::Env env)
    {
        Babylon::Polyfills::Window::Initialize(env);
        Babylon::Polyfills::XMLHttpRequest::Initialize(env);
        Babylon::Polyfills::Canvas::Initialize(env);

        Babylon::Plugins::NativeWindow::Initialize(env, windowPtr, width, height);

        graphics->AddToJavaScript(env);
        Babylon::Plugins::NativeEngine::Initialize(env);

        // Initialize NativeXr plugin.
        Babylon::Plugins::NativeXr::Initialize(env);

        InputManager<Babylon::AppRuntime>::Initialize(env, *inputBuffer);
    });

    Babylon::ScriptLoader loader{ *runtime };
    loader.Eval("document = {}", "");
    loader.LoadScript("app:///ammo.js");
    loader.LoadScript("app:///recast.js");
    loader.LoadScript("app:///babylon.max.js");
    loader.LoadScript("app:///babylon.glTF2FileLoader.js");
    loader.LoadScript("app:///babylonjs.materials.js");
    loader.LoadScript("app:///experience.js");
}

- (void)resize:(int)inWidth height:(int)inHeight
{
    if (graphics)
    {
        graphics->UpdateSize(static_cast<size_t>(inWidth), static_cast<size_t>(inHeight));
    }
    
    if (runtime) 
    {
        runtime->Dispatch([inWidth, inHeight](Napi::Env env)
        {
            Babylon::Plugins::NativeWindow::UpdateSize(env, static_cast<size_t>(inWidth), static_cast<size_t>(inHeight));
        });
    }
}

- (void)setInputs:(int)x y:(int)y tap:(bool)tap
{
    if (inputBuffer)
    {
        inputBuffer->SetPointerPosition(x, y);
        inputBuffer->SetPointerDown(tap);
    }
}

@end
