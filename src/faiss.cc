#include <napi.h>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>
#include <faiss/impl/FaissException.h>
#include <faiss/impl/io.h>
#include <faiss/index_factory.h>
#include <faiss/MetricType.h>
#include <faiss/impl/IDSelector.h>

using namespace Napi;
using idx_t = faiss::idx_t;

template <class T, typename Y>
class IndexBase : public Napi::ObjectWrap<T>
{
public:
  IndexBase(const Napi::CallbackInfo &info) : Napi::ObjectWrap<T>(info)
  {
    Napi::Env env = info.Env();

    if (info.Length() > 0 && info[0].IsNumber())
    {
      auto n = info[0].As<Napi::Number>().Uint32Value();
      index_ = std::unique_ptr<Y>(new Y(n));
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

    Napi::Object instance = T::constructor->New({});
    T *index = Napi::ObjectWrap<T>::Unwrap(instance);
    std::string fname = info[0].As<Napi::String>().Utf8Value();

    try
    {
      index->index_ = std::unique_ptr<faiss::Index>(dynamic_cast<faiss::Index *>(faiss::read_index(fname.c_str())));
    }
    catch (const faiss::FaissException &ex)
    {
      Napi::Error::New(env, ex.what()).ThrowAsJavaScriptException();
    }

    return instance;
  }

  static Napi::Value fromBuffer(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 argument, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsBuffer())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be a buffer.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    Napi::Object instance = T::constructor->New({});
    T *index = Napi::ObjectWrap<T>::Unwrap(instance);

    auto buffer = Napi::Buffer<uint8_t>::Copy(env, info[0].As<Napi::Buffer<uint8_t>>().Data(), info[0].As<Napi::Buffer<uint8_t>>().Length());
    faiss::VectorIOReader *reader = new faiss::VectorIOReader();
    reader->data = std::vector<uint8_t>(buffer.Data(), buffer.Data() + buffer.Length());

    try
    {
      index->index_ = std::unique_ptr<faiss::Index>(dynamic_cast<faiss::Index *>(faiss::read_index(reader)));
    }
    catch (const faiss::FaissException &ex)
    {
      Napi::Error::New(env, ex.what()).ThrowAsJavaScriptException();
    }

    return instance;
  }

  static Napi::Value fromFactory(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() < 2)
    {
      Napi::Error::New(env, "Expected 2 or 3 arguments, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsNumber())
    {
      Napi::TypeError::New(env, "Invalid the first argument type, must be a number.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[1].IsString())
    {
      Napi::TypeError::New(env, "Invalid the second argument type, must be a string.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    auto metric = faiss::MetricType::METRIC_L2;
    if (info[2].IsNumber())
    {
      metric = static_cast<faiss::MetricType>(info[2].As<Napi::Number>().Uint32Value());
    }

    Napi::Object instance = T::constructor->New({});
    T *index = Napi::ObjectWrap<T>::Unwrap(instance);

    const uint32_t d = info[0].As<Napi::Number>().Uint32Value();
    std::string description = info[1].As<Napi::String>().Utf8Value();

    try
    {
      index->index_ = std::unique_ptr<faiss::Index>(dynamic_cast<faiss::Index *>(faiss::index_factory(d, description.c_str(), metric)));
    }
    catch (const faiss::FaissException &ex)
    {
      Napi::Error::New(env, ex.what()).ThrowAsJavaScriptException();
    }

    return instance;
  }

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

  Napi::Value train(const Napi::CallbackInfo &info)
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

    index_->train(dv.quot, xb);

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

  Napi::Value mergeFrom(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
      Napi::Error::New(env, "Expected 1 argument, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[0].IsObject())
    {
      Napi::TypeError::New(env, "Invalid argument type, must be an object.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    Napi::Object otherIndex = info[0].As<Napi::Object>();
    T *otherIndexInstance = Napi::ObjectWrap<T>::Unwrap(otherIndex);

    if (otherIndexInstance->index_->d != index_->d)
    {
      Napi::Error::New(env, "The merging index must have the same dimension.").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    try
    {
      index_->merge_from(*(otherIndexInstance->index_));
    }
    catch (const faiss::FaissException &ex)
    {
      Napi::Error::New(env, ex.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }

    return env.Undefined();
  }

  Napi::Value removeIds(const Napi::CallbackInfo &info)
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

    idx_t *xb = new idx_t[length];
    for (size_t i = 0; i < length; i++)
    {
      Napi::Value val = arr[i];
      if (!val.IsNumber())
      {
        Napi::Error::New(env, "Expected a Number as array item. (at: " + std::to_string(i) + ")")
            .ThrowAsJavaScriptException();
        return env.Undefined();
      }
      xb[i] = val.As<Napi::Number>().Int64Value();
    }

    size_t num = index_->remove_ids(faiss::IDSelectorArray{length, xb});

    delete[] xb;
    return Napi::Number::New(info.Env(), num);
  }

  Napi::Value toBuffer(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (info.Length() != 0)
    {
      Napi::Error::New(env, "Expected 0 arguments, but got " + std::to_string(info.Length()) + ".")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    faiss::VectorIOWriter *writer = new faiss::VectorIOWriter();

    faiss::write_index(index_.get(), writer);

    // return buffer from IOWriter
    return Napi::Buffer<uint8_t>::Copy(env, writer->data.data(), writer->data.size());
  }

protected:
  std::unique_ptr<faiss::Index> index_;
  inline static Napi::FunctionReference *constructor;
};

// faiss::Index is abstract so IndexFlatL2 is used as fallback
class Index : public IndexBase<Index, faiss::IndexFlatL2>
{
public:
  using IndexBase::IndexBase;

  static constexpr const char *CLASS_NAME = "Index";

  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    // clang-format off
    auto func = DefineClass(env, CLASS_NAME, {
      InstanceMethod("ntotal", &Index::ntotal),
      InstanceMethod("getDimension", &Index::getDimension),
      InstanceMethod("isTrained", &Index::isTrained),
      InstanceMethod("add", &Index::add),
      InstanceMethod("train", &Index::train),
      InstanceMethod("search", &Index::search),
      InstanceMethod("write", &Index::write),
      InstanceMethod("mergeFrom", &Index::mergeFrom),
      InstanceMethod("removeIds", &Index::removeIds),
      InstanceMethod("toBuffer", &Index::toBuffer),
      StaticMethod("read", &Index::read),
      StaticMethod("fromBuffer", &Index::fromBuffer),
      StaticMethod("fromFactory", &Index::fromFactory),
    });
    // clang-format on

    constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set(CLASS_NAME, func);
    return exports;
  }
};

class IndexFlatL2 : public IndexBase<IndexFlatL2, faiss::IndexFlatL2>
{
public:
  using IndexBase::IndexBase;

  static constexpr const char *CLASS_NAME = "IndexFlatL2";

  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    // clang-format off
    auto func = DefineClass(env, CLASS_NAME, {
      InstanceMethod("ntotal", &IndexFlatL2::ntotal),
      InstanceMethod("getDimension", &IndexFlatL2::getDimension),
      InstanceMethod("isTrained", &IndexFlatL2::isTrained),
      InstanceMethod("add", &IndexFlatL2::add),
      InstanceMethod("train", &IndexFlatL2::train),
      InstanceMethod("search", &IndexFlatL2::search),
      InstanceMethod("write", &IndexFlatL2::write),
      InstanceMethod("mergeFrom", &IndexFlatL2::mergeFrom),
      InstanceMethod("removeIds", &IndexFlatL2::removeIds),
      InstanceMethod("toBuffer", &IndexFlatL2::toBuffer),
      StaticMethod("read", &IndexFlatL2::read),
      StaticMethod("fromBuffer", &IndexFlatL2::fromBuffer),
    });
    // clang-format on

    constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set(CLASS_NAME, func);
    return exports;
  }
};

class IndexFlatIP : public IndexBase<IndexFlatIP, faiss::IndexFlatIP>
{
public:
  using IndexBase::IndexBase;

  static constexpr const char *CLASS_NAME = "IndexFlatIP";

  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    // clang-format off
    auto func = DefineClass(env, CLASS_NAME, {
      InstanceMethod("ntotal", &IndexFlatIP::ntotal),
      InstanceMethod("getDimension", &IndexFlatIP::getDimension),
      InstanceMethod("isTrained", &IndexFlatIP::isTrained),
      InstanceMethod("add", &IndexFlatIP::add),
      InstanceMethod("train", &IndexFlatIP::train),
      InstanceMethod("search", &IndexFlatIP::search),
      InstanceMethod("write", &IndexFlatIP::write),
      InstanceMethod("mergeFrom", &IndexFlatIP::mergeFrom),
      InstanceMethod("removeIds", &IndexFlatIP::removeIds),
      InstanceMethod("toBuffer", &IndexFlatIP::toBuffer),
      StaticMethod("read", &IndexFlatIP::read),
      StaticMethod("fromBuffer", &IndexFlatIP::fromBuffer),
    });
    // clang-format on

    constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set(CLASS_NAME, func);
    return exports;
  }
};

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  Index::Init(env, exports);
  IndexFlatL2::Init(env, exports);
  IndexFlatIP::Init(env, exports);

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
