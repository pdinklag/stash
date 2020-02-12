#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include <tlx/cmdline_parser.hpp>

#include <stash/pred/binsearch.hpp>
#include <stash/pred/binsearch_cache.hpp>
#include <stash/pred/index.hpp>
#include <stash/pred/index_compact.hpp>
#include <stash/pred/rank.hpp>
#include <stash/pred/sample.hpp>
#include <stash/pred/tree3.hpp>

#include <stash/io/load_file.hpp>
#include <stash/util/malloc_callback.hpp>
#include <stash/util/time.hpp>
#include <stash/util/uint40.hpp>

using namespace stash;

using value_t = uint40_t;
using binsearch       = pred::binsearch<std::vector<value_t>, value_t>;
using binsearch_cache = pred::binsearch_cache<std::vector<value_t>, value_t>;
using rank            = pred::rank<std::vector<value_t>, value_t>;

template<size_t k>
using sample = pred::sample<std::vector<value_t>, value_t, k>;

template<size_t k>
using index = pred::index<std::vector<value_t>, value_t, k>;

template<size_t k>
using index_compact = pred::index_compact<std::vector<value_t>, value_t, k>;

template<size_t l1, size_t l2, size_t l3>
using tree3 = pred::tree3<std::vector<value_t>, value_t, l1, l2, l3>;

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

std::string test_types[] = { "predecessor", "successor" };

struct test_result {
    uint8_t  type;
    uint64_t t_construct;
    size_t   m_ds;
    uint64_t t_queries;
    uint64_t sum;
};

template<typename pred_t>
test_result test_predecessor(
    const std::vector<value_t>& array,
    const std::vector<value_t>& queries) {

    // construct
    auto t0 = time();
    auto m0 = mem;
    pred_t q(array);
    uint64_t t_construct = time() - t0;
    size_t   m_ds = mem - m0;

    // do queries
    const auto min = array[0];
    uint64_t t_queries;
    uint64_t sum = 0;
    {
        auto t0 = time();
        for(value_t x : queries) {
            auto r = q.predecessor(x);

            if(x >= min) {
                assert(r.exists);
                assert(x >= array[r.pos]);
                sum += array[r.pos];
            } else {
                assert(!r.exists);
            }
        }
        t_queries = time() - t0;
    }

    return test_result { 0, t_construct, m_ds, t_queries, sum };
}

template<typename pred_t>
test_result test_successor(
    const std::vector<value_t>& array,
    const std::vector<value_t>& queries) {

    // construct
    auto t0 = time();
    auto m0 = mem;
    pred_t q(array);
    uint64_t t_construct = time() - t0;
    size_t   m_ds = mem - m0;

    // do queries
    const auto max = array[array.size()-1ULL];
    uint64_t t_queries;
    uint64_t sum = 0;
    {
        auto t0 = time();
        for(value_t x : queries) {
            auto r = q.successor(x);
            if(x <= max) {
                assert(r.exists);
                assert(x <= array[r.pos]);
                sum += array[r.pos];
            } else {
                assert(!r.exists);
            }
        }
        t_queries = time() - t0;
    }

    return test_result { 1, t_construct, m_ds, t_queries, sum };
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string input_filename;
    cp.add_param_string("file", input_filename, "The input file.");

    size_t num_queries = 10'000'000ULL;
    cp.add_bytes('q', "queries", num_queries, "The number of queries to perform.");

    size_t universe = 0;
    cp.add_bytes('u', "universe", universe, "The universe to draw query numbers from. Default is maximum value + 1.");

    bool add_max = false;
    cp.add_flag('m', "max", add_max, "Append the maximum possible value to the input sequence.");

    bool lines = false;
    cp.add_flag("lines", lines, "Input file contains string representations of the input sequence, one per line.");

    bool no_pred = false;
    cp.add_bool("no-pred", no_pred, "Don't do predecessor benchmark.");

    bool no_succ = false;
    cp.add_bool("no-succ", no_succ, "Don't do successor benchmark.");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    // load input
    std::cout << "# loading input ..." << std::endl;

    std::vector<value_t> array;
    if(lines) {
        array = io::load_file_lines_as_vector<value_t>(input_filename);
    } else {
        array = io::load_file_as_vector<uint40_t, value_t>(input_filename);
    }

    if(!universe) {
        universe = (size_t)array[array.size() - 1] + 1;
    }

    if(add_max) {
        array.push_back(value_t(UINT64_MAX));
    }

    // generate queries
    std::cout << "# generating queries ..." << std::endl;
    auto queries = generate_queries(num_queries, universe);

    // lambda to print a test result
    auto print_result = [&](const std::string& name, test_result&& r){
        std::cout << "RESULT algo="<< name
            << " queries=" << queries.size()
            << " type=" << test_types[r.type]
            << " universe=" << universe
            << " keys=" << array.size()
            << " t_construct=" << r.t_construct
            << " t_queries=" << r.t_queries
            << " m_ds=" << r.m_ds
            << " sum=" << r.sum << std::endl;
    };

    // run tests
    if(!no_pred) {
    std::cout << "# predecessor ..." << std::endl;
    print_result("bs", test_predecessor<binsearch>(array, queries));
    print_result("bs*", test_predecessor<binsearch_cache>(array, queries));
    print_result("rank", test_predecessor<rank>(array, queries));
    print_result("sample<64>", test_predecessor<sample<64>>(array, queries));
    print_result("sample<128>", test_predecessor<sample<128>>(array, queries));
    print_result("sample<256>", test_predecessor<sample<256>>(array, queries));
    print_result("sample<512>", test_predecessor<sample<512>>(array, queries));
    print_result("sample<1024>", test_predecessor<sample<1024>>(array, queries));
    print_result("idx<4>", test_predecessor<index<4>>(array, queries));
    print_result("idx<5>", test_predecessor<index<5>>(array, queries));
    print_result("idx<6>", test_predecessor<index<6>>(array, queries));
    print_result("idx<7>", test_predecessor<index<7>>(array, queries));
    print_result("idx<8>", test_predecessor<index<8>>(array, queries));
    print_result("idx<9>", test_predecessor<index<9>>(array, queries));
    print_result("idx<10>", test_predecessor<index<10>>(array, queries));
    print_result("idx<11>", test_predecessor<index<11>>(array, queries));
    print_result("idx<12>", test_predecessor<index<12>>(array, queries));
    print_result("cidx<4>", test_predecessor<index_compact<4>>(array, queries));
    print_result("cidx<5>", test_predecessor<index_compact<5>>(array, queries));
    print_result("cidx<6>", test_predecessor<index_compact<6>>(array, queries));
    print_result("cidx<7>", test_predecessor<index_compact<7>>(array, queries));
    print_result("cidx<8>", test_predecessor<index_compact<8>>(array, queries));
    print_result("cidx<9>", test_predecessor<index_compact<9>>(array, queries));
    print_result("cidx<10>", test_predecessor<index_compact<10>>(array, queries));
    print_result("cidx<11>", test_predecessor<index_compact<11>>(array, queries));
    print_result("cidx<12>", test_predecessor<index_compact<12>>(array, queries));
    }

    if(!no_succ) {
    std::cout << "# successor ..." << std::endl;
    print_result("bs", test_successor<binsearch>(array, queries));
    print_result("bs*", test_successor<binsearch_cache>(array, queries));
    print_result("rank", test_successor<rank>(array, queries));
    print_result("sample<64>", test_successor<sample<64>>(array, queries));
    print_result("sample<128>", test_successor<sample<128>>(array, queries));
    print_result("sample<256>", test_successor<sample<256>>(array, queries));
    print_result("sample<512>", test_successor<sample<512>>(array, queries));
    print_result("sample<1024>", test_successor<sample<1024>>(array, queries));
    print_result("idx<4>", test_successor<index<4>>(array, queries));
    print_result("idx<5>", test_successor<index<5>>(array, queries));
    print_result("idx<6>", test_successor<index<6>>(array, queries));
    print_result("idx<7>", test_successor<index<7>>(array, queries));
    print_result("idx<8>", test_successor<index<8>>(array, queries));
    print_result("idx<9>", test_successor<index<9>>(array, queries));
    print_result("idx<10>", test_successor<index<10>>(array, queries));
    print_result("idx<11>", test_successor<index<11>>(array, queries));
    print_result("idx<12>", test_successor<index<12>>(array, queries));
    print_result("cidx<4>", test_successor<index_compact<4>>(array, queries));
    print_result("cidx<5>", test_successor<index_compact<5>>(array, queries));
    print_result("cidx<6>", test_successor<index_compact<6>>(array, queries));
    print_result("cidx<7>", test_successor<index_compact<7>>(array, queries));
    print_result("cidx<8>", test_successor<index_compact<8>>(array, queries));
    print_result("cidx<9>", test_successor<index_compact<9>>(array, queries));
    print_result("cidx<10>", test_successor<index_compact<10>>(array, queries));
    print_result("cidx<11>", test_successor<index_compact<11>>(array, queries));
    print_result("cidx<12>", test_successor<index_compact<12>>(array, queries));
    }
}
