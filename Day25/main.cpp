#include "aoc/helpers.h"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <random>
#include <queue>

#pragma GCC diagnostic push                             // save the actual diag context
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // disable maybe warnings

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>

#pragma GCC diagnostic pop                              // restore the actual diag context

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(jqt: rhn xhk nvd
rsh: frs pzl lsr
xhk: hfx
cmg: qnr nvd lhk bvb
rhn: xhk bvb hfx
bvb: xhk hfx
pzl: lsr hfx nvd
qnr: nvd
ntq: jqt hfx bvb xhk
nvd: lhk
lsr: lhk
rzs: qnr cmg lsr rsh
frs: qnr lhk lsr)");
  constexpr int SR_Part1 = 54;
  constexpr int SR_Part2 = 0;

  using NodeMap = std::unordered_map<std::string, size_t>;

  const auto AddNode = [](NodeMap& map, size_t& nodes, const std::string& id) {
    auto it = map.emplace(id, 0);
    if (it.second) {
      it.first->second = nodes++;
      DEBUG_PRINT("Added node " << id << " at index " << it.first->second);
    }
    return it.first->second;
  };

  using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;

  const auto LoadInput = [](auto f) {
    Graph r;
    NodeMap nodemap;
    size_t nodes = 0;
    std::string_view line;
    while (aoc::getline(f, line)) {
      const auto parts = aoc::split(line, ": ");
      std::string id{ parts[0].data(),  parts[0].size() };
      const auto idx = AddNode(nodemap, nodes, id);

      for (size_t i = 1; i < parts.size(); i++) {
        id = std::string{ parts[i].data(), parts[i].size() };
        const auto cidx = AddNode(nodemap, nodes, id);
        boost::add_edge(cidx, idx, r);
      }
    }
    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Graph r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int64_t part1 = 0;

  {
    aoc::AutoTimer t1{"Part 1"};
    auto parity_map = boost::make_one_bit_color_map(boost::num_vertices(r), boost::get(boost::vertex_index, r));
    auto weights = boost::make_constant_property<Graph::edge_descriptor>(1.0);
    boost::stoer_wagner_min_cut(r, weights, boost::parity_map(parity_map));

    std::vector<std::pair<size_t,size_t>> edges_to_cut;
    for(const auto& edge : boost::make_iterator_range(boost::edges(r))) {
      auto src = boost::source(edge, r);
      auto dst = boost::target(edge, r);
      if (boost::get(parity_map, src) != boost::get(parity_map, dst)){
        edges_to_cut.push_back({src, dst});
      }
    }

    for(auto& edge : edges_to_cut){
      boost::remove_edge(edge.first, edge.second, r);
    }

    std::vector<int> component(boost::num_vertices(r));
    boost::connected_components(r, component.data());

    part1 = std::count(component.begin(), component.end(), 0) * std::count(component.begin(), component.end(), 1);
  }

  aoc::print_result(1, part1);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
  }

  return 0;
}
