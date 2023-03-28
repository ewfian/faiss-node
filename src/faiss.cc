#include <napi.h>
#include "faiss/faiss/IndexFlat.h"

using namespace Napi;

Napi::String Method(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  int d = 64;      // dimension
  int nb = 100000; // database size
  int nq = 10000;  // nb of queries
  float *xb = new float[d * nb];
  float *xq = new float[d * nq];
  for (int i = 0; i < nb; i++)
  {
    for (int j = 0; j < d; j++)
    {
      xb[d * i + j] = drand48();
    }
    xb[d * i] += i / 1000.;
  }
  for (int i = 0; i < nq; i++)
  {
    for (int j = 0; j < d; j++)
    {
      xq[d * i + j] = drand48();
    }
    xq[d * i] += i / 1000.;
  }
  faiss::IndexFlatL2 index(d); // call constructor
  printf("is_trained = %s\n", index.is_trained ? "true" : "false");
  index.add(nb, xb); // add vectors to the index
  printf("ntotal = %lld\n", index.ntotal);
  return Napi::String::New(env, "Hello, faiss!");
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "HelloWorld"),
              Napi::Function::New(env, Method));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)