#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>
#include <bitset>
#include <memory>
#include <chrono>
#include <cassert>

struct TimeMeasurer {
  explicit TimeMeasurer(std::string description_ = "") {
    start = std::chrono::high_resolution_clock::now();
    description = std::move(description_);
  }
  ~TimeMeasurer() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = now - start;
    std::cout << "Duration of \"" << description << "\" is ";
    std::cout << std::setprecision(8) << duration.count() << "s\n";
  }
  std::string description;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

void find_entries(const char* pi,
                  size_t from,
                  size_t to,
                  const std::string& needed,
                  std::vector<size_t>& results,
                  std::mutex& mutex,
                  size_t thread_number = 0) {
  {
    TimeMeasurer T("Thread #" + std::to_string(thread_number));
    for (size_t i = from; i < to; i++) {
      if (i + needed.size() <= to
          && std::string(pi + i, pi + i + needed.size()) == needed) {
        std::lock_guard<std::mutex> lg(mutex);
        results.push_back(i);
      }
    }
    mutex.lock();
  }
  mutex.unlock();
}

std::vector<std::pair<size_t, size_t>> GenerateBlocks(size_t from,
                                                      size_t to,
                                                      size_t word_size,
                                                      size_t block_count) {
  TimeMeasurer T("Generating blocks");
  assert(from < to);
  assert(word_size > 0);
  std::vector<std::pair<size_t, size_t>> answer;
  answer.reserve(block_count);
  size_t recommended_block_size = (to - from) / block_count;
  size_t last_index{0};
  for (int i = 0; i < block_count - 1; i++) {
    answer.emplace_back(last_index,
                        last_index + recommended_block_size + word_size - 1);
    last_index += recommended_block_size;
  }
  answer.emplace_back(last_index, to);
  for (size_t i = 0; i < answer.size(); i++) {
    std::cout << "Block #" << i << ": ";
    std::cout << "[" << answer[i].first + 1 << ", " << answer[i].second + 1
              << ")\n";
  }
  return answer;
}

int main() {
  char* pi = new char[(int) 1e9 + 100];
  std::ifstream input_file("../pi.txt");
  std::cout << std::fixed << std::setprecision(2);

  int pi_size = 1e9;
  {
    TimeMeasurer T("Loading");
    int i = 0;
    while (!input_file.eof() && i < pi_size) {
      if (i % (pi_size / 25) == 0) {
        std::cout << "Loading... " << 100 * (double) i / pi_size << "%" << '\n';
      }
      pi[i++] = input_file.get();
    }
    pi_size = i;
    std::cout << "Loaded!\n";
  }

  while (1) {
    std::string request;
    std::cin >> request;
    if (request == "Find" || request == "find") {
      std::string needed;
      int thread_number;
      std::cin >> needed >> thread_number;

      std::vector<size_t> indexes;
      std::mutex indexes_mutex;
      std::vector<std::unique_ptr<std::thread>> threads(thread_number);

      {
        TimeMeasurer T("Processing");
        auto blocks = GenerateBlocks(0, pi_size, needed.size(), thread_number);
        std::cout << "Running on " << blocks.size() << " threads\n";
        for (int i = 0; i < thread_number; i++) {
          threads[i] = std::make_unique<std::thread>(find_entries,
                                                     std::ref(pi),
                                                     blocks[i].first,
                                                     blocks[i].second,
                                                     needed,
                                                     std::ref(indexes),
                                                     std::ref(indexes_mutex),
                                                     i);
        }
        for (auto& thread : threads) {
          thread->join();
        }
      }
      std::cout << "--Found " << indexes.size() << " entries--\n";
      std::cout << "Would you like to see results? ";
      char c;
      std::cin >> c;
      if (c == 'y' || c == 'Y') {
        for (auto elem : indexes) {
          std::cout << elem + 1 << '\n';
        }
      }
    } else {
      int pos;
      int cnt;
      std::cin >> pos >> cnt;
      pos--;
      for (int i = pos; i < pos + cnt; i++) {
        std::cout << pi[i];
      }
      std::cout << '\n';
      break;
    }
  }
  return 0;
}
