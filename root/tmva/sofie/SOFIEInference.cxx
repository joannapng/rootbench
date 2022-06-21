// Author: Federico Sossai (fsossai), 2021

#include <benchmark/benchmark.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <vector>
#include <memory>
#include <functional>
#include <random>

#include "Linear_event.hxx"
#include "Linear_16.hxx"
#include "Linear_32.hxx"
#include "Linear_64.hxx"
#include "Generator_B1.hxx"
#include "Generator_B64.hxx"
#include "Conv_d100_L1_B1.hxx"
#include "Conv_d100_L14_B1.hxx"
#include "Conv_d100_L14_B32.hxx"
#include "Conv3d_d32_L4_B1.hxx"
#include "RNN_d10_L20_h8_B1.hxx"
#include "GRU_d10_L20_h8_B1.hxx"
#include "LSTM_d10_L20_h8_B1.hxx"
#include "higgs_model_dense.hxx"
#include "DDB_B1.hxx"   // CMS onnx model

#include "DenseLayer100_B1.hxx"
#include "DenseLayer1000_B1.hxx"
#include "DenseLayer10000_B1.hxx"
#include "DenseLayer50000_B1.hxx"
#include "DenseLayer100_B64.hxx"
#include "DenseLayer1000_B64.hxx"
#include "DenseLayer10000_B64.hxx"
#include "DenseLayer50000_B64.hxx"
//#include "DenseLayer50000_B64_vdt.hxx"

#include "resnet18v1.hxx"
#include "TMath.h"


using namespace std;
bool verbose = false;
template <class S>
void BM_SOFIE_Inference(benchmark::State &state)
{
   size_t inputSize = state.range(0);  // input size (without batch size)
   size_t bsize = (state.range(1) > 0) ? state.range(1) : 0;
   size_t nevts = 64;
   size_t nrep = nevts / bsize;

   vector<float> input(inputSize*nevts);

   static std::uniform_real_distribution<float> distribution(-1, 1);
   static std::default_random_engine generator;
   std::generate(input.begin(), input.end(), []() { return distribution(generator); });

   float *input_ptr = input.data();
   S s("");

   double totDuration = 0;
   int ntimes = 0;
   for (auto _ : state) {
      auto t1 = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < nevts; i += bsize)
         auto y = s.infer(input.data()+ inputSize*i);

      auto t2 = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
      totDuration += duration / 1.E3;  // in milliseconds
      ntimes++;
   }

   state.counters["time/evt(ms)"] = totDuration / double(ntimes * nevts);
   // input[0] = -999;
   // s.inf
   // std::cout << "number of times " << s.itime << std::endl;
   // int n = s.itime - 1;
   // for (size_t i = 0; i < 5; ++i) {
   //    double mean = TMath::Mean(n, resTimes[i].data());
   //    double rms = TMath::RMS(n, resfTimes[i].data());
   //    std::cout << "elapsed time for " << i << " : " << mean << " +/- " << rms / sqrt(n) << std::endl;
   //  }
    //if (verbose) std::cout << "output : " << output.size() << " : " << output.front() << " ......" << output.back() << std::endl;
}

// inference for model with 3 inputs
template <class S>
void BM_SOFIE_Inference_3(benchmark::State &state)
{
   size_t bsize = state.range(0);  // batch size
   size_t inputSize1 = state.range(1);  // input 1 size
   size_t inputSize2 = state.range(2);  // input 2 size
   size_t inputSize3 = state.range(3);

   size_t nevts = 64;
   size_t nrep = nevts / bsize;

   size_t eventSize = inputSize1 + inputSize2+inputSize3;

   vector<float> input1(inputSize1*nevts);
   vector<float> input2(inputSize2*nevts);
   vector<float> input3(inputSize3*nevts);

   static std::uniform_real_distribution<float> distribution(-1, 1);
   static std::default_random_engine generator;
   std::generate(input1.begin(), input1.end(), []() { return distribution(generator); });
   std::generate(input2.begin(), input2.end(), []() { return distribution(generator); });
   std::generate(input3.begin(), input3.end(), []() { return distribution(generator); });

   S s("");

   std::cout << "initi done - do benchmark \n";

   double totDuration = 0;
   int ntimes = 0;
   for (auto _ : state) {
      auto t1 = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < nevts; i += bsize) {
         float * p1 = input1.data()+ inputSize1*i;
         float * p2 = input2.data()+ inputSize2*i;
         float * p3 = input3.data()+ inputSize3*i;
         auto y = s.infer(p1,p2,p3);
      }
      auto t2 = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
      totDuration += duration / 1.E3;  // in milliseconds
      ntimes++;
   }

   state.counters["time/evt(ms)"] = totDuration / double(ntimes * nevts);
}

// CMS benchmark (3 inputs)
//BENCHMARK_TEMPLATE(BM_SOFIE_Inference_3, TMVA_SOFIE_DDB_B1::Session)->Name("DDB_B1")->Args({1, 1*27, 60*8, 5*2})->Unit(benchmark::kMillisecond);

// single layer BM

BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer100_B1::Session)->Name("DenseLayer100_B1")->Args({100, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer1000_B1::Session)->Name("DenseLayer1000_B1")->Args({100, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer10000_B1::Session)->Name("DenseLayer10000_B1")->Args({100, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer50000_B1::Session)->Name("DenseLayer50000_B1")->Args({100, 1})->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer100_B64::Session)->Name("DenseLayer100_B64")->Args({100, 64})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer1000_B64::Session)->Name("DenseLayer1000_B64")->Args({100, 64})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer10000_B64::Session)->Name("DenseLayer10000_B64")->Args({100, 64})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_DenseLayer50000_B64::Session)->Name("DenseLayer50000_B64")->Args({100, 64})->Unit(benchmark::kMillisecond);

//Gemm benchmarks
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Linear_16::Session)->Name("Linear_16")->Args({100, 16})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Linear_32::Session)->Name("Linear_32")->Args({100, 32})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Linear_64::Session)->Name("Linear_64")->Args({100, 64})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Linear_event::Session)->Name("Linear_event")->Args({100, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Generator_B1::Session)->Name("Generator_B1")->Args({14, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Generator_B64::Session)->Name("Generator_B64")->Args({14, 64})->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_higgs_model_dense::Session)->Name("higgs_model_dense")->Args({7, 1})->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Conv_d100_L14_B1::Session)->Name( "Conv_d100_L14_B1")->Args({100*100, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Conv_d100_L14_B32::Session)->Name("Conv_d100_L14_B32")->Args({100*100, 32})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Conv_d100_L1_B1::Session)->Name( "Conv_d100_L1_B1")->Args({100*100, 1})->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_Conv3d_d32_L4_B1::Session)->Name( "Conv3d_d32_L4_B1")->Args({32*32*32, 1})->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_resnet18v1::Session)->Name("resnet18v1")->Args({3 * 224 * 224, 1})->Unit(benchmark::kMillisecond);

//Recurrent benchmark
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_RNN_d10_L20_h8_B1::Session)->Name("RNN_d10_L20_h8_B1")->Args({3 * 5, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_GRU_d10_L20_h8_B1::Session)->Name("GRU_d10_L20_h8_B1")->Args({3 * 5, 1})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SOFIE_Inference, TMVA_SOFIE_LSTM_d10_L20_h8_B1::Session)->Name("LSTM_d10_L20_h8_B1")->Args({1 * 1, 1})->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();
