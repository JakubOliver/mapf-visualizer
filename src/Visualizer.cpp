//
// Created by jakub on 10/7/26.
//

#include "Visualizer.h"

#include <fstream>
#include <iostream>
#include <regex>

#include "../include/ofApp.hpp"
#include "ofMain.h"

const std::regex r_pos =
    std::regex(R"(\((\d+),(\d+),?([XY]{1}_[A-Z]{4,5})?\),)");

static Config parseConfig(const std::string& line, Graph& graph) {
  Config config;
  std::smatch match;
  auto iter = line.cbegin();

  while (iter < line.cend()) {
    auto search_end = std::min(iter + 128, line.cend());

    if (!std::regex_search(iter, search_end, match, r_pos)) break;

    auto x = std::stoi(match[1].str());
    auto y = std::stoi(match[2].str());
    Orientation orientation = Orientation::NONE;

    if (match[3].matched) {
      orientation = Orientation::from_string(match[3].str());
    }

    config.push_back(
        Pose(graph.U[graph.width * y + x], orientation));
    iter += match.position() + match.length();
  }

  return config;
}

void Visualizer::run(int argc, char *argv[]) {
  // simple arguments check
  if (argc < 3 || !std::ifstream(argv[1]) || !std::ifstream(argv[2])) {
    std::cout << "Please check the arguments, e.g.,\n"
              << "> mapf-visualizer assets/random-32-32-20.map "
                 "assets/demo_random-32-32-20.txt"
              << std::endl;

    return;
  }

  // load graph
  Graph G(argv[1]);

  // load plan
  auto solution_file = std::ifstream(argv[2]);
  Solution solution;
  Config goals;
  std::string line;
  while (getline(solution_file, line)) {
    if (line.starts_with("goals=")) {
      goals = parseConfig(line, G);
      continue;
    }

    // so we only process real solution lines
    if (line.find(":(") == std::string::npos) continue;

    solution.push_back(parseConfig(line, G));
  }
  solution_file.close();

  if (goals.empty()) goals = solution.back();

  // visualize
  ofSetupOpenGL(100, 100, OF_WINDOW);
  ofRunApp(new ofApp(&G, &solution, goals,
                     (argc > 3 && std::string(argv[3]) == "--capture-only")));
}
