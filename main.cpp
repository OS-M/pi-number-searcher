#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <iomanip>
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

void FindEntries(const char* pi,
                 size_t from,
                 size_t to,
                 const std::string& string,
                 std::vector<size_t>& results,
                 std::mutex& mutex,
                 size_t thread_number = 0) {
  {
    TimeMeasurer T("Thread #" + std::to_string(thread_number));
    for (size_t i = from; i < to; i++) {
      if (i + string.size() <= to
          && std::string(pi + i, pi + i + string.size()) == string) {
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
  std::string filename;
  std::cout << "Enter filename (or \"0\" for pi.txt)\n";
  std::cin >> filename;
  if (filename == "0") {
    filename = "pi.txt";
  }
  std::ifstream input_file("../" + filename);
  if (!input_file.is_open()) {
    std::cout << "Invalid filename or unreachable file";
    return 1;
  }
  std::cout << std::fixed << std::setprecision(2);

  int max_size = 1e9;
  char* pi = new char[max_size + 1];
  {
    TimeMeasurer T("Loading");
    int i = 0;
    while (!input_file.eof() && i < max_size) {
      if (i % (max_size / 25) == 0) {
        std::cout << "Loading... " << 100 * (double) i / max_size << "%"
                  << '\n';
      }
      pi[i++] = input_file.get();
    }
    max_size = i;
    std::cout << "Loaded!\n";
  }

  while (true) {
    std::cout << "====\nCommands:\nFind (integer: number to find) "
                 "(integer: number of threads)\n"
                 "Substr (integer: position) (integer: count)\n====\n";

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
        TimeMeasurer T("Search");
        auto blocks = GenerateBlocks(0, max_size, needed.size(), thread_number);
        std::cout << "Running on " << blocks.size() << " threads\n";
        for (int i = 0; i < thread_number; i++) {
          threads[i] = std::make_unique<std::thread>(FindEntries,
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
      std::cout << "How many entries show (they are not sorted)?\n";
      int number;
      std::cin >> number;

      {
        TimeMeasurer time_measurer("Sorting");
        std::sort(indexes.begin(), indexes.end());
      }

      std::cout << "Found entries from positions (1-indexation):\n";
      for (int i = 0; i < std::min(static_cast<int>(indexes.size()), number);
           i++) {
        std::cout << indexes[i] + 1 << '\n';
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
