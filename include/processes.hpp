#ifndef DCANALYSIS_IMPL_PROCESSES_HPP
#define DCANALYSIS_IMPL_PROCESSES_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

namespace dcanalysis_impl {
    
    template<typename T>
    void downsample(
        const T* in_data,
        size_t in_size,
        T* out_data,
        size_t out_size,
        int ratio,
        bool interleaved
        );

    size_t downsample_size(size_t in_size, int ratio, bool interleaved);

    void fshift(
        const real_t* i_data,
        size_t i_size,
        const real_t* q_data,
        size_t q_size,
        real_t* out_data,
        size_t out_size,
        real_t fs,
        real_t _fshift
        );
    
    template<typename T>
    void fshift(
        const T* i_data,
        size_t i_size,
        const T* q_data,
        size_t q_size,
        T* out_data,
        size_t out_size,
        int n,
        real_t fs,
        real_t _fshift,
        CodeFormat format
        );
    
    size_t fshift_size(size_t i_size, size_t q_size);
    
    template<typename T>
    void normalize(
        const T* in_data,
        size_t in_size,
        real_t* out_data,
        size_t out_size,
        int n,
        CodeFormat format
        );
    
    void polyval(
        const real_t* in_data,
        size_t in_size,
        real_t* out_data,
        size_t out_size,
        const real_t* c_data,
        size_t c_size
        );
    
    template<typename T>
    void quantize(
        const real_t* in_data,
        size_t in_size,
        T* out_data,
        size_t out_size,
        real_t fsr,
        int n,
        real_t noise,
        CodeFormat format
        );

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_PROCESSES_HPP