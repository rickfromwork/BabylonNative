#include "GraphicsImpl.h"

#include <cassert>
#include <limits>

namespace Babylon
{
    Graphics::Graphics()
        : m_impl{std::make_unique<Impl>()}
    {
    }

    Graphics::~Graphics() = default;

    template<>
    void Graphics::UpdateWindow<void*>(void* windowPtr)
    {
        m_impl->SetNativeWindow(windowPtr, nullptr);
    }

    template<>
    void Graphics::UpdateWindow<void*, void*>(void* windowPtr, void* windowTypePtr)
    {
        m_impl->SetNativeWindow(windowPtr, windowTypePtr);
    }

    template<>
    std::unique_ptr<Graphics> Graphics::CreateGraphics<void*, size_t, size_t>(void* nativeWindowPtr, size_t width, size_t height)
    {
        std::unique_ptr<Graphics> graphics{new Graphics()};
        graphics->UpdateWindow<void*>(nativeWindowPtr);
        graphics->UpdateSize(width, height);
        return graphics;
    }

    template<>
    std::unique_ptr<Graphics> Graphics::CreateGraphics<void*, void*, size_t, size_t>(void* nativeWindowPtr, void* nativeWindowTypePtr, size_t width, size_t height)
    {
        std::unique_ptr<Graphics> graphics{new Graphics()};
        graphics->UpdateWindow<void*, void*>(nativeWindowPtr, nativeWindowTypePtr);
        graphics->UpdateSize(width, height);
        return graphics;
    }

    void Graphics::UpdateSize(size_t width, size_t height)
    {
        m_impl->Resize(width, height);
    }

    void Graphics::AddToJavaScript(Napi::Env env)
    {
        JsRuntime::NativeObject::GetFromJavaScript(env)
            .Set(JS_GRAPHICS_NAME, Napi::External<Graphics>::New(env, this));

        m_impl->AddToJavaScript(env);
    }

    Graphics& Graphics::GetFromJavaScript(Napi::Env env)
    {
        return *JsRuntime::NativeObject::GetFromJavaScript(env)
            .Get(JS_GRAPHICS_NAME)
            .As<Napi::External<Graphics>>()
            .Data();
    }

    void Graphics::EnableRendering()
    {
        m_impl->EnableRendering();
    }

    void Graphics::DisableRendering()
    {
        m_impl->DisableRendering();
    }

    void Graphics::StartRenderingCurrentFrame()
    {
        m_impl->StartRenderingCurrentFrame();
    }

    void Graphics::FinishRenderingCurrentFrame()
    {
        m_impl->FinishRenderingCurrentFrame();
    }

    Graphics::CallbackHandle Graphics::RegisterOnBeginFrame(std::function<void()> callback)
    {
        return m_impl->RegisterOnBeginFrame(callback);
    }

    void Graphics::UnregisterOnBeginFrame(Graphics::CallbackHandle callbackHandle)
    {
        m_impl->UnregisterOnBeginFrame(callbackHandle);
    }

    Graphics::CallbackHandle Graphics::RegisterOnEndFrame(std::function<void()> callback)
    {
        return m_impl->RegisterOnEndFrame(callback);
    }

    void Graphics::UnregisterOnEndFrame(Graphics::CallbackHandle callbackHandle)
    {
        m_impl->UnregisterOnEndFrame(callbackHandle);
    }

    void Graphics::SetDiagnosticOutput(std::function<void(const char* output)> outputFunction)
    {
        m_impl->SetDiagnosticOutput(std::move(outputFunction));
    }

    void Graphics::SetHardwareScalingLevel(float level)
    {
        m_impl->SetHardwareScalingLevel(level);
    }

    float Graphics::GetHardwareScalingLevel()
    {
        return m_impl->GetHardwareScalingLevel();
    }
}
