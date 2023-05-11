#include <napi.h>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>

using namespace Napi;
using idx_t = faiss::idx_t;

template <class T, typename Index>
class IndexFlat : public Napi::ObjectWrap<T>
{
public:
  IndexFlat(const Napi::CallbackInfo &info) : Napi::ObjectWrap<T>(info)
  {
    Napi::Env env = info.Env();
    if (info[0].IsExternal())
    {
      const std::string fname = *info[0].As<Napi::External<std::string>>().Data();
      index_ = std::unique_ptr<Index>(dynamic_cast<Index *>(faiss::read_index(fname.c_str())));
    }
    else
    {
      if (!info.IsConstructCall())
      {
        Napi::Error::New(env, "Class constructors cannot be invoked without 'new'")
            .ThrowAsJavaScriptException();
        return;
      }

      if (info.Length() != 1)
      {
        Napi::Error::New(env, "Expected 1 argument, but got " + std::to_string(info.Length()) + ".")
            .ThrowAsJavaScriptException();
        return;
      }
      if (!info[0].IsNumber())
      {
        Napi::TypeError::New(env, "Invalid the first argument type, must be a number.").ThrowAsJavaScriptException();
        return;
      }

      auto n = info[0].As<Napi::Number>().Uint32Value();

      index_ = std::unique_ptr<Index>(new Index(n));
    }
  }

  static Napi::Value read(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 argument, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsString())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be a string.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    return constructor->New({Napi::External<std::string>::New(env, new std::string(info[0].As<Napi::String>()))});
  }

protected:
  std::unique_ptr<Index> index_;
  inline static Napi::FunctionReference *constructor = new Napi::FunctionReference();
  Napi::Value isTrained(const Napi::CallbackInfo &info)
  {
    return Napi::Boolean::New(info.Env(), index_->is_trained);
  }

  Napi::Value add(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 argument, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsArray())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be an Array.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    Napi::Array arr = info[0].As<Napi::Array>();
    size_t length = arr.Length();
    auto dv = std::div(length, index_->d);
    if (dv.rem != 0)
    {
      Napi::Error::New(env, "Invalid the given array length.")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    float *xb = new float[length];
    for (size_t i = 0; i < length; i++)
    {
      Napi::Value val = arr[i];
      if (!val.IsNumber())
      {
        Napi::Error::New(env, "Expected a Number as array item. (at: " + std::to_string(i) + ")")
            .ThrowAsJavaScriptException();
        return env.Undefined();
      }
      xb[i] = val.As<Napi::Number>().FloatValue();
    }

    index_->add(dv.quot, xb);

    delete[] xb;
    return env.Undefined();
  }

  Napi::Value search(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 2)
    {
      Napi::Error::New(env, "Expected 2 arguments, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsArray())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be an Array.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[1].IsNumber())
    {
      Napi::TypeError::New(env, "Invalid the second argument type, must be a Number.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    const uint32_t k = info[1].As<Napi::Number>().Uint32Value();
    if (k > index_->ntotal)
    {
      Napi::Error::New(env, "Invalid the number of k (cannot be given a value greater than `ntotal`: " +
                                std::to_string(index_->ntotal) + ").")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    Napi::Array arr = info[0].As<Napi::Array>();
    size_t length = arr.Length();
    auto dv = std::div(length, index_->d);
    if (dv.rem != 0)
    {
      Napi::Error::New(env, "Invalid the given array length.")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    float *xq = new float[length];
    for (size_t i = 0; i < length; i++)
    {
      Napi::Value val = arr[i];
      if (!val.IsNumber())
      {
        Napi::Error::New(env, "Expected a Number as array item. (at: " + std::to_string(i) + ")")
            .ThrowAsJavaScriptException();
        return env.Undefined();
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
    return Napi::Number::New(info.Env(), index_->ntotal);
  }

  Napi::Value getDimension(const Napi::CallbackInfo &info)
  {
    return Napi::Number::New(info.Env(), index_->d);
  }

  Napi::Value write(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 argument, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsString())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be a string.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    const std::string fname = info[0].As<Napi::String>().Utf8Value();

    faiss::write_index(index_.get(), fname.c_str());

    return env.Undefined();
  }
};

class IndexFlatL2 : public IndexFlat<IndexFlatL2, faiss::IndexFlatL2>
{
public:
  using IndexFlat::IndexFlat;
  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    // clang-format off
    Napi::Function func = DefineClass(env, "IndexFlatL2", {
      InstanceMethod("ntotal", &IndexFlatL2::ntotal),
      InstanceMethod("getDimension", &IndexFlatL2::getDimension),
      InstanceMethod("isTrained", &IndexFlatL2::isTrained),
      InstanceMethod("add", &IndexFlatL2::add),
      InstanceMethod("search", &IndexFlatL2::search),
      InstanceMethod("write", &IndexFlatL2::write),
      StaticMethod("read", &IndexFlatL2::read),
    });
    // clang-format on

    *constructor = Napi::Persistent(func);

    exports.Set("IndexFlatL2", func);
    return exports;
  }
};

class IndexFlatIP : public IndexFlat<IndexFlatIP, faiss::IndexFlatIP>
{
public:
  using IndexFlat::IndexFlat;
  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    // clang-format off
    Napi::Function func = DefineClass(env, "IndexFlatIP", {
      InstanceMethod("ntotal", &IndexFlatIP::ntotal),
      InstanceMethod("getDimension", &IndexFlatIP::getDimension),
      InstanceMethod("isTrained", &IndexFlatIP::isTrained),
      InstanceMethod("add", &IndexFlatIP::add),
      InstanceMethod("search", &IndexFlatIP::search),
      InstanceMethod("write", &IndexFlatIP::write),
      StaticMethod("read", &IndexFlatIP::read),
    });
    // clang-format on

    *constructor = Napi::Persistent(func);

    exports.Set("IndexFlatIP", func);
    return exports;
  }
};

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  IndexFlatL2::Init(env, exports);
  IndexFlatIP::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)