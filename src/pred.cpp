#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include <tlx/cmdline_parser.hpp>

#include <pred/BinSearch.hpp>
#include <util/Time.hpp>

using value_t = uint64_t;
using BinSearchU32 = pred::BinSearch<std::vector<uint64_t>, uint64_t>;

template<typename pred_t>
void test(
    const std::string& name,
    const std::vector<uint64_t>& array,
    const unsigned int num_queries,
    const unsigned int universe,
    const unsigned int seed = 147U) {

    // construct
    auto t0 = time();
    pred_t q(array);
    uint64_t t_construct = time() - t0;

    // seed
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<> dist(0, universe);

    // do queries
    uint64_t t_queries;
    uint64_t sum = 0;
    {
        auto t0 = time();
        for(uint64_t i = 0; i < num_queries; i++) {
            sum += q.predecessor(dist(gen)).value;
        }
        t_queries = time() - t0;
    }
    
    std::cout << "RESULT algo=" << name << " queries=" << num_queries << " universe=" << universe << " t_construct=" << t_construct << " t_queries=" << t_queries << " sum=" << sum << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    //std::string input_filename;
    //cp.add_param_string("file", input_filename, "The input file.");

    unsigned int universe = UINT32_MAX;
    cp.add_uint('u', "universe", universe, "The universe size.");

    unsigned int num_queries = 10'000'000ULL;
    cp.add_uint('q', "queries", num_queries, "The number of queries to perform.");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    std::vector<uint64_t> array = { 2, 4, 5, 7, 11, 12, 15, 18, 19, 22, 24, 28, 31, 35, 37, 42 };
    test<BinSearchU32>("binary_search", array, num_queries, universe);
}

