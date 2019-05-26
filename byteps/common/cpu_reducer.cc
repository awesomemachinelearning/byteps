// Copyright 2019 ByteDance Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

#include "cpu_reducer.h"
#include "global.h"
#include "logging.h"

namespace byteps {
namespace common {

CpuReducer::CpuReducer(std::shared_ptr<BytePSComm> comm) {
    std::vector<int> peers;
    auto pcie_size = BytePSGlobal::GetPcieSwitchSize();
    for (int i = BytePSGlobal::GetLocalRank() % pcie_size;
         i < BytePSGlobal::GetLocalSize();
         i += pcie_size) {
        peers.push_back(i);
    }
    _comm = std::make_shared<BytePSCommSocket>(comm, std::string("cpu"), peers);
    _num_threads = BYTEPS_CPU_REDUCER_THREADS;
    return;
}

bool CpuReducer::isRoot() {
    return (_comm->getRoot() == BytePSGlobal::GetLocalRank());
}

int CpuReducer::sum(void* dst, void* src, size_t len, DataType dtype) {
    switch (dtype) {
        case BYTEPS_FLOAT32:
            return _sum_float32(dst, src, len);
        case BYTEPS_FLOAT64:
            return _sum_float64(dst, src, len);
        case BYTEPS_FLOAT16:
            return _sum_float16(dst, src, len);
        case BYTEPS_UINT8:
            return _sum_unit8(dst, src, len);
        case BYTEPS_INT32:
            return _sum_int32(dst, src, len);
        case BYTEPS_INT8:
            return _sum_int8(dst, src, len);
        case BYTEPS_INT64:
            return _sum_int64(dst, src, len);
        default:
            BPS_CHECK(0) << "Unsupported data type: " << dtype;
    }
    return 0;
}

int CpuReducer::_sum_float32(void* dst, void* src, size_t len) {
    auto d = (float*)dst;
    auto s = (float*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len / 4; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

int CpuReducer::_sum_float64(void* dst, void* src, size_t len) {
    auto d = (double*)dst;
    auto s = (double*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len / 8; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

// TODO
int CpuReducer::_sum_float16(void* dst, void* src, size_t len) {
    auto d = (float*)dst;
    auto s = (float*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len / 4; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

int CpuReducer::_sum_unit8(void* dst, void* src, size_t len) {
    auto d = (unsigned char*)dst;
    auto s = (unsigned char*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

int CpuReducer::_sum_int32(void* dst, void* src, size_t len) {
    auto d = (int*)dst;
    auto s = (int*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len / 4; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

int CpuReducer::_sum_int8(void* dst, void* src, size_t len) {
    auto d = (signed char*)dst;
    auto s = (signed char*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

int CpuReducer::_sum_int64(void* dst, void* src, size_t len) {
    auto d = (long long*)dst;
    auto s = (long long*)src;
#pragma omp parallel for simd num_threads(_num_threads)
    for (size_t i = 0; i < len / 8; ++i) {
        d[i] = d[i] + s[i];
    }
    return 0;
}

} // namespace common
} // namespace byteps