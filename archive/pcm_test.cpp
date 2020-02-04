#include <unistd.h>

#include <pcm/cpucounters.h>

#include <stash/io/load_file.hpp>
#include <stash/util/time.hpp>

#include <stash/rapl/reader.hpp>

using namespace stash;

int main(int argc, char** argv) {
    // cmd
    if(argc < 2) {
        std::cerr << "usage: " << argv[0] << " <file>" << std::endl;
	return -1;
    }

    // init
    const auto num_rapl_packages = rapl::reader::num_packages();

    PCM* m = PCM::getInstance();

    const auto cpu_model = m->getCPUModel();
    if (!(m->hasPCICFGUncore()))
    {
        std::cerr << "unsupported processor model: " << cpu_model << std::endl;
        return -3;
    }

    rapl::reader** r = new rapl::reader*[num_rapl_packages];
    rapl::energy* e0 = new rapl::energy[num_rapl_packages];
    rapl::energy* e1 = new rapl::energy[num_rapl_packages];
    for(uint32_t i = 0; i < num_rapl_packages; i++) {
        r[i] = new rapl::reader(i);
    }

    // before
    ServerUncorePowerState* state0 = new ServerUncorePowerState[m->getNumSockets()];
    for(int i = 0; i < m->getNumSockets(); ++i) {
        state0[i] = m->getServerUncorePowerState(i);
    }
    for(size_t i = 0; i < num_rapl_packages; i++) {
        e0[i] = r[i]->read();
    }
    auto t0 = time();

    // do stuff
    {
        uint64_t sum = 0;
        auto v = io::load_file_lines_as_vector<uint64_t>(argv[1]);
        for(auto x : v) {
            sum += x;
	}
        std::cout << "sum=" << sum << std::endl;
    }

    // after
    auto t1 = time();
    ServerUncorePowerState* state1 = new ServerUncorePowerState[m->getNumSockets()];
    for(int i = 0; i < m->getNumSockets(); ++i) {
        state1[i] = m->getServerUncorePowerState(i);
    }
    for(size_t i = 0; i < num_rapl_packages; i++) {
        e1[i] = r[i]->read();
	delete r[i];
    }

    // log
    std::cout << "time=" << (t1-t0) << std::endl;
    for(int i = 0; i < m->getNumSockets(); ++i) {
        const auto cpu = getConsumedJoules(state0[i], state1[i]);
        const auto ram = getDRAMConsumedJoules(state0[i], state1[i]);
        std::cout << "socket " << i << ": cpu_joules=" << cpu << ", ram_joules=" << ram << std::endl;
    }

    std::cout << "RAPL:" << std::endl;
    for(size_t i = 0; i < num_rapl_packages; i++) {
        auto de = e1[i] - e0[i];
	std::cout << "package " << i << ": " << de << std::endl;
    }

    // clean up
    delete[] r;
    delete[] e0;
    delete[] e1;
    delete[] state0;
    delete[] state1;
    return 0;
}

