#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include <tlx/cmdline_parser.hpp>

#include <pred/binary_search.hpp>
#include <pred/binary_search_cache.hpp>
#include <pred/idx_binary_search.hpp>
#include <pred/idx_binary_search_civ.hpp>
#include <pred/idx_binary_search_sparse.hpp>
#include <pred/rank_select.hpp>
#include <pred/two_level_binary_search.hpp>

#include <io/load_file.hpp>
#include <util/malloc_callback.hpp>
#include <util/time.hpp>
#include <util/uint_types.hpp>

using value_t = uint40_t;
using binary_search       = pred::binary_search<std::vector<value_t>, value_t>;
using binary_search_cache = pred::binary_search_cache<std::vector<value_t>, value_t>;
using rank_select         = pred::rank_select<std::vector<value_t>, value_t>;

template<size_t k>
using two_level_binary_search = pred::two_level_binary_search<std::vector<value_t>, value_t, k>;

template<size_t k>
using idx_binary_search = pred::idx_binary_search<std::vector<value_t>, value_t, k>;

template<size_t k>
using idx_binary_search_civ = pred::idx_binary_search_civ<std::vector<value_t>, value_t, k>;

template<size_t k>
using idx_binary_search_sparse = pred::idx_binary_search_sparse<std::vector<value_t>, value_t, k>;

size_t mem = 0;

namespace malloc_callback {

void on_alloc(size_t num) {
    mem += num;
}

void on_free(size_t num) {
    mem -= num;
}

}

std::vector<value_t> generate_queries(size_t num, size_t universe, size_t seed = 147ULL) {
    std::vector<value_t> queries;
    queries.reserve(num);

    // seed
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<uint64_t> dist(0, universe);

    // generate
    for(size_t i = 0; i < num; i++) {
        queries.push_back(value_t(dist(gen)));
    }

    return queries;
}

struct test_result {
    uint64_t t_construct;
    size_t   m_ds;
    uint64_t t_queries;
    uint64_t  sum;
};

template<typename pred_t>
test_result test(
    const std::vector<value_t>& array,
    const std::vector<value_t>& queries) {

    // construct
    auto t0 = time();
    auto m0 = mem;
    pred_t q(array);
    uint64_t t_construct = time() - t0;
    size_t   m_ds = mem - m0;

    // do queries
    uint64_t t_queries;
    uint64_t sum = 0;
    {
        auto t0 = time();
        for(value_t x : queries) {
            auto pred_x = q.predecessor(x).value;
            assert(x >= pred_x);
            sum += pred_x;
        }
        t_queries = time() - t0;
    }

    return test_result { t_construct, m_ds, t_queries, sum };
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string input_filename;
    cp.add_param_string("file", input_filename, "The input file.");

    size_t num_queries = 10'000'000ULL;
    cp.add_bytes('q', "queries", num_queries, "The number of queries to perform.");

    size_t universe = std::numeric_limits<value_t>::max();
    cp.add_bytes('u', "universe", universe, "The universe to draw query numbers from.");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    // load input
    std::cout << "# loading input ..." << std::endl;
    auto array = load_file_as_vector<value_t>(input_filename);

    // generate queries
    std::cout << "# generating queries ..." << std::endl;
    auto queries = generate_queries(num_queries, universe);

    // lambda to print a test result
    auto print_result = [&](const std::string& name, test_result&& r){
        std::cout << "RESULT algo="<< name
            << " queries=" << queries.size()
            << " universe=" << universe
            << " keys=" << array.size()
            << " t_construct=" << r.t_construct
            << " t_queries=" << r.t_queries
            << " m_ds=" << r.m_ds
            << " sum=" << r.sum << std::endl;
    };

    // run tests
    std::cout << "# running tests ..." << std::endl;
    print_result("idx<4>", test<idx_binary_search<4>>(array, queries));
    print_result("idx<5>", test<idx_binary_search<5>>(array, queries));
    print_result("idx<6>", test<idx_binary_search<6>>(array, queries));
    print_result("idx<7>", test<idx_binary_search<7>>(array, queries));
    print_result("idx<8>", test<idx_binary_search<8>>(array, queries));
    print_result("idx<9>", test<idx_binary_search<9>>(array, queries));
    print_result("idx<10>", test<idx_binary_search<10>>(array, queries));
    print_result("idx<11>", test<idx_binary_search<11>>(array, queries));
    print_result("idx<12>", test<idx_binary_search<12>>(array, queries));
    print_result("idx<13>", test<idx_binary_search<13>>(array, queries));
    print_result("idx<14>", test<idx_binary_search<14>>(array, queries));
    print_result("idx<15>", test<idx_binary_search<15>>(array, queries));
    print_result("idx<16>", test<idx_binary_search<16>>(array, queries));
    print_result("cidx<4>", test<idx_binary_search_civ<4>>(array, queries));
    print_result("cidx<5>", test<idx_binary_search_civ<5>>(array, queries));
    print_result("cidx<6>", test<idx_binary_search_civ<6>>(array, queries));
    print_result("cidx<7>", test<idx_binary_search_civ<7>>(array, queries));
    print_result("cidx<8>", test<idx_binary_search_civ<8>>(array, queries));
    print_result("cidx<9>", test<idx_binary_search_civ<9>>(array, queries));
    print_result("cidx<10>", test<idx_binary_search_civ<10>>(array, queries));
    print_result("cidx<11>", test<idx_binary_search_civ<11>>(array, queries));
    print_result("cidx<12>", test<idx_binary_search_civ<12>>(array, queries));
    print_result("cidx<13>", test<idx_binary_search_civ<13>>(array, queries));
    print_result("cidx<14>", test<idx_binary_search_civ<14>>(array, queries));
    print_result("cidx<15>", test<idx_binary_search_civ<15>>(array, queries));
    print_result("cidx<16>", test<idx_binary_search_civ<16>>(array, queries));
    print_result("sidx<4>", test<idx_binary_search_sparse<4>>(array, queries));
    print_result("sidx<5>", test<idx_binary_search_sparse<5>>(array, queries));
    print_result("sidx<6>", test<idx_binary_search_sparse<6>>(array, queries));
    print_result("sidx<7>", test<idx_binary_search_sparse<7>>(array, queries));
    print_result("sidx<8>", test<idx_binary_search_sparse<8>>(array, queries));
    print_result("sidx<9>", test<idx_binary_search_sparse<9>>(array, queries));
    print_result("sidx<10>", test<idx_binary_search_sparse<10>>(array, queries));
    print_result("sidx<11>", test<idx_binary_search_sparse<11>>(array, queries));
    print_result("sidx<12>", test<idx_binary_search_sparse<12>>(array, queries));
    print_result("sidx<13>", test<idx_binary_search_sparse<13>>(array, queries));
    print_result("sidx<14>", test<idx_binary_search_sparse<14>>(array, queries));
    print_result("sidx<15>", test<idx_binary_search_sparse<15>>(array, queries));
    print_result("sidx<16>", test<idx_binary_search_sparse<16>>(array, queries));
    print_result("sample<64>", test<two_level_binary_search<64>>(array, queries));
    print_result("sample<128>", test<two_level_binary_search<128>>(array, queries));
    print_result("sample<256>", test<two_level_binary_search<256>>(array, queries));
    print_result("sample<512>", test<two_level_binary_search<512>>(array, queries));
    print_result("sample<1024>", test<two_level_binary_search<1024>>(array, queries));
    print_result("sample<2048>", test<two_level_binary_search<2048>>(array, queries));
    print_result("sample<4096>", test<two_level_binary_search<4096>>(array, queries));
    print_result("sample<8192>", test<two_level_binary_search<8192>>(array, queries));
    print_result("bs*", test<binary_search_cache>(array, queries));
    print_result("bs", test<binary_search>(array, queries));
    print_result("rs", test<rank_select>(array, queries));
}
