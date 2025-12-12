#define _USE_MATH_DEFINES
#include <cmath>
#include <napi.h>

Napi::Value AddWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();

    return Napi::Number::New(env, a + b);
}

Napi::Value SubtractWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();

    return Napi::Number::New(env, a - b);
}

Napi::Value MultiplyWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();
    
    return Napi::Number::New(env, a * b);
}

Napi::Value DivideWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();
    
    return Napi::Number::New(env, a / b);
}

Napi::Value PowerWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();
    
    return Napi::Number::New(env, std::pow(a, b));
}

Napi::Value SqrtWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected a number").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    
    return Napi::Number::New(env, std::sqrt(a));
}

Napi::Value HypotWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();
    
    return Napi::Number::New(env, std::hypot(a, b));
}

Napi::Value ToBase64Wrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string str = info[0].As<Napi::String>();
    std::string out;

    static const std::string b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int val = 0, valb = -6;
    for (unsigned char c : str) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');

    return Napi::String::New(env, out);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("add", Napi::Function::New(env, AddWrapped));
    exports.Set("subtract", Napi::Function::New(env, SubtractWrapped));
    exports.Set("multiply", Napi::Function::New(env, MultiplyWrapped));
    exports.Set("divide", Napi::Function::New(env, DivideWrapped));
    exports.Set("power", Napi::Function::New(env, PowerWrapped));
    exports.Set("sqrt", Napi::Function::New(env, SqrtWrapped));
    exports.Set("hypot", Napi::Function::New(env, HypotWrapped));
    exports.Set("PI", Napi::Number::New(env, M_PI));
    exports.Set("E", Napi::Number::New(env, M_E));
    exports.Set("toBase64", Napi::Function::New(env, ToBase64Wrapped));
    
    return exports;
}

NODE_API_MODULE(addon, Init);