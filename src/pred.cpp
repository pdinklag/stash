#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include <tlx/cmdline_parser.hpp>

#include <pred/BinSearch.hpp>
#include <pred/CacheBinSearch.hpp>
#include <pred/RankSelect.hpp>

#include <io/LoadFile.hpp>
#include <util/MallocCallback.hpp>
#include <util/Time.hpp>
#include <util/UintTypes.hpp>

using value_t = uint40_t;
using BinSearch      = pred::BinSearch<std::vector<value_t>, value_t>;
using CacheBinSearch = pred::CacheBinSearch<std::vector<value_t>, value_t>;
using RankSelect     = pred::RankSelect<std::vector<value_t>, value_t>;

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
    std::uniform_int_distribution<> dist(0, universe);

    // generate
    for(size_t i = 0; i < num; i++) {
        queries.push_back(dist(gen));
    }

    return queries;
}

struct TestResult {
    uint64_t t_construct;
    size_t   m_ds;
    uint64_t t_queries;
    value_t  sum;
};

template<typename pred_t>
TestResult test(
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
    value_t sum = 0;
    {
        auto t0 = time();
        for(auto x : queries) {
            sum += q.predecessor(x).value;
        }
        t_queries = time() - t0;
    }

    return TestResult { t_construct, m_ds, t_queries, sum };
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string input_filename;
    cp.add_param_string("file", input_filename, "The input file.");
    
    size_t num_queries = 10'000'000ULL;
    cp.add_size_t('q', "queries", num_queries, "The number of queries to perform.");

    size_t universe = std::numeric_limits<value_t>::max();
    cp.add_bytes('u', "universe", universe, "The universe to draw query numbers from.");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    // load input
    std::cout << "# loading input ..." << std::endl;
    auto array = load_file_as_vector<value_t>(input_filename);

    // lambda to print a test result
    auto print_result = [&](const std::string& name, TestResult&& r){
        std::cout << "RESULT algo="<< name
            << " queries=" << num_queries
            << " universe=" << universe
            << " keys=" << array.size()
            << " t_construct=" << r.t_construct
            << " t_queries=" << r.t_queries
            << " m_ds=" << r.m_ds
            << " sum=" << r.sum << std::endl;
    };

    // generate queries
    std::cout << "# generating queries ..." << std::endl;
    auto queries = generate_queries(num_queries, universe);

    // run tests
    std::cout << "# running tests ..." << std::endl;
    print_result("binary_search",  test<BinSearch>(array, queries));
    print_result("binary_search*", test<CacheBinSearch>(array, queries));
    print_result("rank_select",    test<RankSelect>(array, queries));
}
