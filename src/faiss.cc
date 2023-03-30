#include <napi.h>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <faiss/IndexFlat.h>

using namespace Napi;
using idx_t = faiss::Index::idx_t;

class IndexFlatL2 : public Napi::ObjectWrap<IndexFlatL2>
{
public:
  idx_t dim_;
  std::unique_ptr<faiss::IndexFlatL2> index_;
  IndexFlatL2(const Napi::CallbackInfo &info) : Napi::ObjectWrap<IndexFlatL2>(info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 arguments, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return;
    }
    if (!info[0].IsNumber())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be a number.").ThrowAsJavaScriptException();
      return;
    }

    dim_ = info[0].As<Napi::Number>().Uint32Value();
    index_ = std::unique_ptr<faiss::IndexFlatL2>(new faiss::IndexFlatL2(dim_));
  }

  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    // clang-format off
    Napi::Function func = DefineClass(env, "IndexFlatL2", {
      InstanceMethod("ntotal", &IndexFlatL2::ntotal),
      InstanceMethod("getDimension", &IndexFlatL2::getDimension),
      InstanceMethod("isTrained", &IndexFlatL2::isTrained),
      InstanceMethod("add", &IndexFlatL2::add),
      InstanceMethod("search", &IndexFlatL2::search)
    });
    // clang-format on

    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("IndexFlatL2", func);
    return exports;
  }

private:
  Napi::Value isTrained(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, index_->is_trained);
  }

  Napi::Value add(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 arguments, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }
    if (!info[0].IsArray())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be a Array.").ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }

    Napi::Array arr = info[0].As<Napi::Array>();
    size_t length = arr.Length();
    auto dv = std::div(length, dim_);
    if (dv.rem != 0)
    {
      Napi::Error::New(env, "Invalid the given array length.")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }

    float *xb = new float[length];
    for (size_t i = 0; i < length; i++)
    {
      Napi::Value val = arr[i];
      if (!val.IsNumber())
      {
        Napi::Error::New(info.Env(), "Expected a Number as Array Item")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
      }
      xb[i] = val.As<Napi::Number>().FloatValue();
    }

    index_->add(dv.quot, xb);

    delete[] xb;
    return info.Env().Undefined();
  }

  Napi::Value search(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 2)
    {
      Napi::Error::New(env, "Expected 2 arguments, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }
    if (!info[0].IsArray())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be an Array.").ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }
    if (!info[1].IsNumber())
    {
      Napi::TypeError::New(env, "Invalid the second argument type, must be a number.").ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }

    const uint32_t k = info[1].As<Napi::Number>().Uint32Value();
    if (k > index_->ntotal)
    {
      Napi::Error::New(env, "Invalid the number of k (cannot be given a value greater than `ntotal`: " +
                                std::to_string(index_->ntotal) + ").")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Array arr = info[0].As<Napi::Array>();
    size_t length = arr.Length();
    auto dv = std::div(length, dim_);
    if (dv.rem != 0)
    {
      Napi::Error::New(env, "Invalid the given array length.")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }

    float *xq = new float[length];
    for (size_t i = 0; i < length; i++)
    {
      Napi::Value val = arr[i];
      if (!val.IsNumber())
      {
        Napi::Error::New(info.Env(), "Expected a Number as Array Item")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
      }
      xq[i] = val.As<Napi::Number>().FloatValue();
    }

    auto nq = dv.quot;
    idx_t *I = new idx_t[k * nq];
    float *D = new float[k * nq];

    index_->search(nq, xq, k, D, I);

    Napi::Array arr_distances = Napi::Array::New(env, k * nq);
    Napi::Array arr_labels = Napi::Array::New(env, k * nq);
    for (size_t i = 0; i < k * nq; i++)
    {
      idx_t label = I[i];
      float distance = D[i];
      arr_distances[i] = Napi::Number::New(env, distance);
      arr_labels[i] = Napi::Number::New(env, label);
    }
    delete[] I;
    delete[] D;

    Napi::Object results = Napi::Object::New(env);
    results.Set("distances", arr_distances);
    results.Set("labels", arr_labels);
    return results;
  }

  Napi::Value ntotal(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, index_->ntotal);
  }

  Napi::Value getDimension(const Napi::CallbackInfo &info)
  {
    return Napi::Number::New(info.Env(), dim_);
  }
};

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  IndexFlatL2::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)