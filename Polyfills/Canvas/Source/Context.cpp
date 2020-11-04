#include <bx/math.h>
#include <map>
#include <algorithm>
#include <assert.h>
#include "nanovg/nanovg.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "Canvas.h"
#include "Context.h"
#include "MeasureText.h"

namespace Babylon::Polyfills::Internal
{
    Napi::Value Context::CreateInstance(Napi::Env env, Canvas* canvas, uint32_t viewId)
    {
        Napi::HandleScope scope{ env };

        Napi::Function func = ParentT::DefineClass(
            env,
            JS_CONSTRUCTOR_NAME,
            {
                ParentT::InstanceMethod("fillRect", &Context::FillRect),
                ParentT::InstanceMethod("measureText", &Context::MeasureText),
                ParentT::InstanceMethod("fillText", &Context::FillText),
                ParentT::InstanceMethod("fill", &Context::Fill),
                ParentT::InstanceMethod("save", &Context::Save),
                ParentT::InstanceMethod("restore", &Context::Restore),
                ParentT::InstanceMethod("clearRect", &Context::ClearRect),
                ParentT::InstanceMethod("translate", &Context::Translate),
                ParentT::InstanceMethod("rotate", &Context::Rotate),
                ParentT::InstanceMethod("scale", &Context::Scale),
                ParentT::InstanceMethod("beginPath", &Context::BeginPath),
                ParentT::InstanceMethod("closePath", &Context::ClosePath),
                ParentT::InstanceMethod("rect", &Context::Rect),
                ParentT::InstanceMethod("clip", &Context::Clip),
                ParentT::InstanceMethod("strokeRect", &Context::StrokeRect),
                ParentT::InstanceMethod("stroke", &Context::Stroke),
                ParentT::InstanceMethod("moveTo", &Context::MoveTo),
                ParentT::InstanceMethod("lineTo", &Context::LineTo),
                ParentT::InstanceMethod("quadraticCurveTo", &Context::QuadraticCurveTo),
                InstanceAccessor("fillStyle", &Context::GetFillStyle, &Context::SetFillStyle),
                InstanceAccessor("strokeStyle", &Context::GetStrokeStyle, &Context::SetStrokeStyle),
                InstanceAccessor("lineWidth", &Context::GetLineWidth, &Context::SetLineWidth),
            });
        return func.New({ Napi::External<Canvas>::New(env, canvas), Napi::Value::From(env, viewId)});
    }

    Context::Context(const Napi::CallbackInfo& info)
        : ParentT{ info }
        , m_canvas{ info[0].As<Napi::External<Canvas>>().Data() }
        , m_viewId{ static_cast<bgfx::ViewId>(info[1].As<Napi::Number>().Uint32Value()) }
        , m_nvg{ nvgCreate(1, m_viewId) }
    {
        for (auto& font : Canvas::fontsInfos)
        {
            m_fonts[font.first] = nvgCreateFontMem(m_nvg, font.first.c_str(), font.second.data(), font.second.size(), 0);
        }
        registeredContexts.push_back(this);
    }

    Context::~Context()
    {
        nvgDelete(m_nvg);

        auto iter = std::find(registeredContexts.begin(), registeredContexts.end(), this);
        if (iter != registeredContexts.end())
        {
            registeredContexts.erase(iter);
        }
    }

    void Context::BeginContextsFrame()
    {
        for (auto& context : registeredContexts)
        {
            context->BeginFrame();
        }
    }
    void Context::EndContextsFrame()
    {
        for (auto& context : registeredContexts)
        {
            context->EndFrame();
        }
    }

    NVGcolor StringToColor(const std::string& colorString)
    {
        std::string str = colorString;
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });

        static const std::map<std::string, uint32_t> webColors = {
            {"aliceblue", 0xf0f8ff},
            {"antiquewhite", 0xfaebd7},
            {"aqua", 0x00ffff},
            {"aquamarine", 0x7fffd4},
            {"azure", 0xf0ffff},
            {"beige", 0xf5f5dc},
            {"bisque", 0xffe4c4},
            {"black", 0x000000},
            {"blanchedalmond", 0xffebcd},
            {"blue", 0x0000ff},
            {"blueviolet", 0x8a2be2},
            {"brown", 0xa52a2a},
            {"burlywood", 0xdeb887},
            {"cadetblue", 0x5f9ea0},
            {"chartreuse", 0x7fff00},
            {"chocolate", 0xd2691e},
            {"coral", 0xff7f50},
            {"cornflowerblue", 0x6495ed},
            {"cornsilk", 0xfff8dc},
            {"crimson", 0xdc143c},
            {"cyan", 0x00ffff},
            {"darkblue", 0x00008b},
            {"darkcyan", 0x008b8b},
            {"darkgoldenrod", 0xb8860b},
            {"darkgray", 0xa9a9a9},
            {"darkgrey", 0xa9a9a9},
            {"darkgreen", 0x006400},
            {"darkkhaki", 0xbdb76b},
            {"darkmagenta", 0x8b008b},
            {"darkolivegreen", 0x556b2f},
            {"darkorange", 0xff8c00},
            {"darkorchid", 0x9932cc},
            {"darkred", 0x8b0000},
            {"darksalmon", 0xe9967a},
            {"darkseagreen", 0x8fbc8f},
            {"darkslateblue", 0x483d8b},
            {"darkslategray", 0x2f4f4f},
            {"darkslategrey", 0x2f4f4f},
            {"darkturquoise", 0x00ced1},
            {"darkviolet", 0x9400d3},
            {"deeppink", 0xff1493},
            {"deepskyblue", 0x00bfff},
            {"dimgray", 0x696969},
            {"dimgrey", 0x696969},
            {"dodgerblue", 0x1e90ff},
            {"firebrick", 0xb22222},
            {"floralwhite", 0xfffaf0},
            {"forestgreen", 0x228b22},
            {"fuchsia", 0xff00ff},
            {"gainsboro", 0xdcdcdc},
            {"ghostwhite", 0xf8f8ff},
            {"gold", 0xffd700},
            {"goldenrod", 0xdaa520},
            {"gray", 0x808080},
            {"grey", 0x808080},
            {"green", 0x008000},
            {"greenyellow", 0xadff2f},
            {"honeydew", 0xf0fff0},
            {"hotpink", 0xff69b4},
            {"indianred", 0xcd5c5c},
            {"indigo", 0x4b0082},
            {"ivory", 0xfffff0},
            {"khaki", 0xf0e68c},
            {"lavender", 0xe6e6fa},
            {"lavenderblush", 0xfff0f5},
            {"lawngreen", 0x7cfc00},
            {"lemonchiffon", 0xfffacd},
            {"lightblue", 0xadd8e6},
            {"lightcoral", 0xf08080},
            {"lightcyan", 0xe0ffff},
            {"lightgoldenrodyellow", 0xfafad2},
            {"lightgray", 0xd3d3d3},
            {"lightgrey", 0xd3d3d3},
            {"lightgreen", 0x90ee90},
            {"lightpink", 0xffb6c1},
            {"lightsalmon", 0xffa07a},
            {"lightseagreen", 0x20b2aa},
            {"lightskyblue", 0x87cefa},
            {"lightslategray", 0x778899},
            {"lightslategrey", 0x778899},
            {"lightsteelblue", 0xb0c4de},
            {"lightyellow", 0xffffe0},
            {"lime", 0x00ff00},
            {"limegreen", 0x32cd32},
            {"linen", 0xfaf0e6},
            {"magenta", 0xff00ff},
            {"maroon", 0x800000},
            {"mediumaquamarine", 0x66cdaa},
            {"mediumblue", 0x0000cd},
            {"mediumorchid", 0xba55d3},
            {"mediumpurple", 0x9370db},
            {"mediumseagreen", 0x3cb371},
            {"mediumslateblue", 0x7b68ee},
            {"mediumspringgreen", 0x00fa9a},
            {"mediumturquoise", 0x48d1cc},
            {"mediumvioletred", 0xc71585},
            {"midnightblue", 0x191970},
            {"mintcream", 0xf5fffa},
            {"mistyrose", 0xffe4e1},
            {"moccasin", 0xffe4b5},
            {"navajowhite", 0xffdead},
            {"navy", 0x000080},
            {"oldlace", 0xfdf5e6},
            {"olive", 0x808000},
            {"olivedrab", 0x6b8e23},
            {"orange", 0xffa500},
            {"orangered", 0xff4500},
            {"orchid", 0xda70d6},
            {"palegoldenrod", 0xeee8aa},
            {"palegreen", 0x98fb98},
            {"paleturquoise", 0xafeeee},
            {"palevioletred", 0xdb7093},
            {"papayawhip", 0xffefd5},
            {"peachpuff", 0xffdab9},
            {"peru", 0xcd853f},
            {"pink", 0xffc0cb},
            {"plum", 0xdda0dd},
            {"powderblue", 0xb0e0e6},
            {"purple", 0x800080},
            {"red", 0xff0000},
            {"rosybrown", 0xbc8f8f},
            {"royalblue", 0x4169e1},
            {"saddlebrown", 0x8b4513},
            {"salmon", 0xfa8072},
            {"sandybrown", 0xf4a460},
            {"seagreen", 0x2e8b57},
            {"seashell", 0xfff5ee},
            {"sienna", 0xa0522d},
            {"silver", 0xc0c0c0},
            {"skyblue", 0x87ceeb},
            {"slateblue", 0x6a5acd},
            {"slategray", 0x708090},
            {"slategrey", 0x708090},
            {"snow", 0xfffafa},
            {"springgreen", 0x00ff7f},
            {"steelblue", 0x4682b4},
            {"tan", 0xd2b48c},
            {"teal", 0x008080},
            {"thistle", 0xd8bfd8},
            {"tomato", 0xff6347},
            {"turquoise", 0x40e0d0},
            {"violet", 0xee82ee},
            {"wheat", 0xf5deb3},
            {"white", 0xffffff},
            {"whitesmoke", 0xf5f5f5},
            {"yellow", 0xffff00},
            {"yellowgreen", 0x9acd32}};
        
        if (str[0] == '#' && str.length() == 4)
        {
            unsigned int components[4];
            int count = sscanf(str.c_str(), "#%1x%1x%1x", &components[0], &components[1], &components[2]);
            for (int i = count; count < 4; count++)
            {
                components[i] += components[i] << 4;
            }
            for (int i = count; count < 4; count++)
            {
                components[i] = 255;
            }
            return nvgRGBA(components[0], components[1], components[2], components[3]);
        }
        else if (str[0] == '#' && str.length() == 4)
        {
            unsigned int components[4];
            int count = sscanf(str.c_str(), "#%02x%02x%02x%02x", &components[0], &components[1], &components[2], &components[3]);
            for (int i = count; count < 4; count++)
            {
                components[i] = 255;
            }
            return nvgRGBA(components[0], components[1], components[2], components[3]);
        }
        else
        {
            auto iter = webColors.find(str);
            if (iter != webColors.end())
            {
                uint32_t color = iter->second;
                return nvgRGBA((color>>16), (color>>8)&0xFF, (color&0xFF), 0xFF);
            }
            else
            {
                assert(0);
            }
        }
        return nvgRGBA(255, 0, 255, 255);
    }

    void Context::FillRect(const Napi::CallbackInfo& info)
    {
        auto left = info[0].As<Napi::Number>().FloatValue();
        auto top = info[1].As<Napi::Number>().FloatValue();
        auto width = info[2].As<Napi::Number>().FloatValue();
        auto height = info[3].As<Napi::Number>().FloatValue();
        //nvgRect(m_nvg, left, top, width, height);

        NVGpaint paint = nvgLinearGradient(m_nvg, 0, 5, 0, 10, nvgRGBA(0, 160, 192, 255), nvgRGBA(0, 160, 192, 255));
        nvgBeginPath(m_nvg);
        nvgRect(m_nvg, left, top, width, height);

        nvgFillPaint(m_nvg, paint);
        nvgFill(m_nvg);
    }

    Napi::Value Context::GetFillStyle(const Napi::CallbackInfo&)
    {
        return Napi::Value::From(Env(), 0);
    }

    void Context::SetFillStyle(const Napi::CallbackInfo&, const Napi::Value& value)
    {
        const auto color = StringToColor(value.As<Napi::String>().Utf8Value());
        nvgFillColor(m_nvg, color);
    }

    Napi::Value Context::GetStrokeStyle(const Napi::CallbackInfo&)
    {
        return Napi::Value::From(Env(), 0);
    }

    void Context::SetStrokeStyle(const Napi::CallbackInfo&, const Napi::Value& value)
    {
        auto color = StringToColor(value.As<Napi::String>().Utf8Value());
        nvgStrokeColor(m_nvg, color);
    }

    Napi::Value Context::GetLineWidth(const Napi::CallbackInfo& )
    {
        return Napi::Value::From(Env(), 0);
    }

    void Context::SetLineWidth(const Napi::CallbackInfo&, const Napi::Value& value)
    {
        const auto width = value.As<Napi::Number>().FloatValue();
        nvgStrokeWidth(m_nvg, width);
    }

    void Context::Fill(const Napi::CallbackInfo&)
    {
        nvgFill(m_nvg);
    }

    void Context::Save(const Napi::CallbackInfo&)
    {
        nvgSave(m_nvg);
    }

    void Context::Restore(const Napi::CallbackInfo&)
    {
        nvgRestore(m_nvg);
    }

    void Context::ClearRect(const Napi::CallbackInfo&)
    {
    }

    void Context::Translate(const Napi::CallbackInfo& info)
    {
        const auto x = info[0].As<Napi::Number>().FloatValue();
        const auto y = info[1].As<Napi::Number>().FloatValue();
        nvgTranslate(m_nvg, x, y);
    }

    void Context::Rotate(const Napi::CallbackInfo& info)
    {
        const auto angle = info[0].As<Napi::Number>().FloatValue();
        nvgRotate(m_nvg, nvgDegToRad(angle));
    }

    void Context::Scale(const Napi::CallbackInfo& info)
    {
        const auto x = info[0].As<Napi::Number>().FloatValue();
        const auto y = info[1].As<Napi::Number>().FloatValue();
        nvgScale(m_nvg, x, y);
    }

    void Context::BeginPath(const Napi::CallbackInfo&)
    {
        nvgBeginPath(m_nvg);
    }

    void Context::ClosePath(const Napi::CallbackInfo&)
    {
        nvgClosePath(m_nvg);
    }

    void Context::Rect(const Napi::CallbackInfo& info)
    {
        const auto left = info[0].As<Napi::Number>().FloatValue();
        const auto top = info[1].As<Napi::Number>().FloatValue();
        const auto width = info[2].As<Napi::Number>().FloatValue();
        const auto height = info[3].As<Napi::Number>().FloatValue();
        nvgRect(m_nvg, left, top, width, height);
    }

    void Context::Clip(const Napi::CallbackInfo&)
    {
    }

    void Context::StrokeRect(const Napi::CallbackInfo& info)
    {
        const auto left = info[0].As<Napi::Number>().FloatValue();
        const auto top = info[1].As<Napi::Number>().FloatValue();
        const auto width = info[2].As<Napi::Number>().FloatValue();
        const auto height = info[3].As<Napi::Number>().FloatValue();
        nvgRect(m_nvg, left, top, width, height);
    }

    void Context::Stroke(const Napi::CallbackInfo&)
    {
    }

    void Context::MoveTo(const Napi::CallbackInfo& info)
    {
        const auto x = info[0].As<Napi::Number>().FloatValue();
        const auto y = info[1].As<Napi::Number>().FloatValue();
        nvgMoveTo(m_nvg, x, y);
    }

    void Context::LineTo(const Napi::CallbackInfo& info)
    {
        const auto x = info[0].As<Napi::Number>().FloatValue();
        const auto y = info[1].As<Napi::Number>().FloatValue();
        nvgLineTo(m_nvg, x, y);
    }

    void Context::QuadraticCurveTo(const Napi::CallbackInfo& info)
    {
        const auto cx = info[0].As<Napi::Number>().FloatValue();
        const auto cy = info[1].As<Napi::Number>().FloatValue();
        const auto x = info[2].As<Napi::Number>().FloatValue();
        const auto y = info[3].As<Napi::Number>().FloatValue();
        nvgBezierTo(m_nvg, cx, cy, cx, cy, x, y);
    }

    Napi::Value Context::MeasureText(const Napi::CallbackInfo& info)
    {
        const std::string text = info[0].As<Napi::String>().Utf8Value();
        return MeasureText::CreateInstance(info.Env(), this, text);
    }

    void Context::FillText(const Napi::CallbackInfo& info)
    {
        const std::string text = info[0].As<Napi::String>().Utf8Value();
        auto x = info[1].As<Napi::Number>().FloatValue();
        auto y = info[2].As<Napi::Number>().FloatValue();

        if (!m_fonts.empty())
        {
            nvgFontFaceId(m_nvg, m_fonts.begin()->second);
            nvgText(m_nvg, x, y, text.c_str(), nullptr);
        }
    }

    void Context::BeginFrame()
    {
        const auto width = m_canvas->GetWidth();
        const auto height = m_canvas->GetHeight();

        bgfx::setViewFrameBuffer(m_viewId, m_canvas->GetFrameBufferHandle());
        bgfx::setViewClear(m_viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0, 1.0f, 0);
        bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

        bgfx::discard();
        
        nvgBeginFrame(m_nvg, float(width), float(height), 1.0f);

        const bgfx::Caps* caps = bgfx::getCaps();
        bool flipY = bgfx::getCaps()->originBottomLeft;
        if (!flipY)
        {
            nvgScale(m_nvg, 1.f, -1.f);
            nvgTranslate(m_nvg, 0.f, -float(height));
        }
    }

    void Context::EndFrame()
    {
        bgfx::discard();
        nvgEndFrame(m_nvg);
    }
}
