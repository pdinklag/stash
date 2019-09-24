#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <io/BitIStream.hpp>
#include <util/MallocCallback.hpp>
#include <util/FileSize.hpp>
#include <util/Time.hpp>

size_t malloc_cur = 0;
size_t malloc_max = 0;

void malloc_reset() {
    malloc_cur = 0;
    malloc_max = 0;
}

void malloc_reset_max() {
    malloc_max = malloc_cur;
}

void malloc_callback::on_alloc(size_t size) {
    malloc_cur += size;
    malloc_max = std::max(malloc_max, malloc_cur);
}

void malloc_callback::on_free(size_t size) {
    malloc_cur -= size;
}

namespace bvcc {
    namespace dinklage {
        #include <bvcc/dinklage/bit_vector.hpp>
        #include <bvcc/dinklage/bit_vector_rank.hpp>
        #include <bvcc/dinklage/bit_vector_select.hpp>
    }
    namespace naive {
        #include <bvcc/naive/bit_vector.hpp>
        #include <bvcc/naive/bit_vector_rank.hpp>
        #include <bvcc/naive/bit_vector_select.hpp>
    }
}

inline bool xassert(const bool b, const std::string& name, const std::string& reason) {
    if(!b) {
        std::cout << "RESULT name=" << name
            << " status=FAIL" << " fail=" << reason << std::endl;
    }
    return b;
}

template<typename bv_t, typename rank_t, typename select_t>
bool bench(
    const std::string& name,
    const size_t n,
    std::vector<size_t>& pos) {

    malloc_reset();
    uint64_t t0;

    // construct bit vector
    t0 = time();
    bv_t bv(n);
    for(size_t i : pos) {
        bv.bitset(i, 1);
    }

    const auto m_construct_bv = malloc_max;
    const auto m_bv = malloc_cur;
    const auto t_construct_bv = time() - t0;

    // verify bit vector
    {
        size_t p = 0;
        for(size_t i = 0; i < bv.size(); i++) {
            if(bv.bitread(i)) {
                if(!xassert(i == pos[p], name, "bv_bits")) return false;
                ++p;
            } else {
            }
        }
        if(!xassert(p == pos.size(), name, "bv_size")) return false;
    }

    // construct rank
    malloc_reset();
    t0 = time();
    rank_t r(bv);

    const auto t_construct_rank = time() - t0;
    const auto m_construct_rank = malloc_max;
    const auto m_rank = malloc_cur;

    // verify rank
    {
        if(!xassert(r.rank1(n-1) == pos.size(), name, "rank1_full")) return false;
        if(!xassert(r.rank0(n-1) == n - pos.size(), name, "rank0_full")) return false;

        for(size_t i = 0; i < pos.size(); i++) {
            const size_t p = pos[i];
            if(!xassert(r.rank1(p) == i+1, name, "rank1")) return false;
            if(!xassert(r.rank0(p) == p-i, name, "rank0")) return false;

            if(i > 0 && pos[i-1] < p-1) {
                if(!xassert(r.rank1(p-1) == i, name, "rank1")) return false;
                if(!xassert(r.rank0(p-1) == p-i, name, "rank0")) return false;
            }
        }
    }

    // rank queries
    uint64_t rsum = 0;
    t0 = time();

    for(size_t i = 0; i < n; i++) {
        rsum += r.rank1(i);
    }

    const auto t_rank = time() - t0;

    // construct select
    malloc_reset();
    t0 = time();
    select_t s(bv);

    const auto t_construct_select = time() - t0;
    const auto m_construct_select = malloc_max;
    const auto m_select = malloc_cur;

    // verify select
    {
        for(size_t i = 0; i < pos.size(); i++) {
            const size_t p = pos[i];
            const size_t s1 = s.select1(i+1);
            if(!xassert(s1 == p, name, "select1")) return false;
        }
    }

    // select queries
    uint64_t ssum = 0;
    t0 = time();

    for(size_t i = 0; i < pos.size(); i++) {
        ssum += s.select1(i+1);
    }

    const auto t_select = time() - t0;

    // print result
    std::cout << "RESULT name=" << name
        << " status=OK"
        << " n=" << n
        << " t_construct_bv=" << t_construct_bv
        << " t_construct_rank=" << t_construct_rank
        << " t_rank=" << t_rank
        << " t_construct_select=" << t_construct_select
        << " t_select=" << t_select
        << " m_construct_bv=" << m_construct_bv
        << " m_bv=" << m_bv
        << " m_construct_rank=" << m_construct_rank
        << " m_rank=" << m_rank
        << " m_construct_select=" << m_construct_select
        << " m_select=" << m_select
        << " rsum=" << rsum
        << " ssum=" << ssum
        << std::endl;
    return true;
}

int main(int argc, char** argv) {
    // params
    if(argc < 2) {
        std::cerr << "usage: " << argv[0] << " <file>" << std::endl;
        return -1;
    }
    
    std::string bv_filename(argv[1]);

    // read input
    std::cout << "Preparing ..." << std::endl;
    std::vector<size_t> pos;
    
    // read bits
    size_t n;
    {
        size_t i = 0;
        std::ifstream ins(bv_filename);
        BitIStream in(ins);
        while(!in.eof()) {
            if(in.read_bit()) {
                pos.push_back(i);
            }
            ++i;
        }
        n = i;
    }

    std::cout << "Input: n=" << n << ", " << pos.size() << " bits set" << std::endl;

    // run benchmarks
    bench<
        bvcc::dinklage::bit_vector,
        bvcc::dinklage::bit_vector_rank,
        bvcc::dinklage::bit_vector_select>("dinklage", n, pos);
    bench<
        bvcc::naive::bit_vector,
        bvcc::naive::bit_vector_rank,
        bvcc::naive::bit_vector_select>("naive", n, pos);
}
