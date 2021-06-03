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
    std::cout << "Duration of \"" << description << "\" was ";
    std::cout << std::setprecision(8) << duration.count() << "s\n";
  }
  std::string description;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

void FindEntries(char const* text,
                 size_t length,
                 const std::string& string,
                 std::vector<size_t>& results,
                 std::mutex& mutex,
                 size_t thread_number = 0) {
  {
    TimeMeasurer T("Thread #" + std::to_string(thread_number));
    std::vector<int> prefix_function(length);
    std::string full_text = string + "#" + std::string(text, text + length);
    for (int i = 1; i < prefix_function.size(); i++) {
      int j = prefix_function[i - 1];
      while (j > 0 && full_text[i] != full_text[j]) {
        j = prefix_function[j - 1];
      }
      if (full_text[i] == full_text[j]) {
        j++;
      }
      prefix_function[i] = j;
    }
    std::vector<size_t> res;
    for (int i = 0; i < prefix_function.size(); i++) {
      if (prefix_function[i] == string.size()) {
        res.push_back(i - 2 * string.size());
      }
    }
    mutex.lock();
    results.insert(results.end(), res.begin(), res.end());
    mutex.unlock();
  }
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
  size_t target_block_size = (to - from) / block_count;
  size_t last_index{0};
  for (int i = 0; i < block_count - 1; i++) {
    answer.emplace_back(
        last_index,
        std::min(to, last_index + target_block_size + word_size - 1));
    last_index += target_block_size;
  }
  answer.emplace_back(last_index, to);
  for (size_t i = 0; i < answer.size(); i++) {
    std::cout << "Block #" << i << ": ";
    std::cout << "[" << answer[i].first + 1 << ", " << answer[i].second + 1
              << ")\n";
  }
  return answer;
}

std::string GetGreenText(const std::string& string) {
  return "\033[1;32m" + string + "\033[0m";
}

std::string GetRedText(const std::string& string) {
  return "\033[31m" + string + "\033[0m";
}

std::string GetBlueText(const std::string& string) {
  return "\033[34m" + string + "\033[0m";
}

int main() {
  std::string filename;
  std::cout << GetGreenText("Enter filename (or \"0\" for pi.txt)\n");
  std::cin >> filename;
  if (filename == "0") {
    filename = "pi.txt";
  }
  std::ifstream input_file("../" + filename);
  if (!input_file.is_open()) {
    std::cout << GetRedText("Invalid filename or unreachable file " + filename);
    return 1;
  }
  std::cout << std::fixed << std::setprecision(2);

  int size = 1e9;
  char* text = new char[size + 1];
  {
    TimeMeasurer T("Loading");
    int i = 0;
    while (!input_file.eof() && i < size) {
      if (i % (size / 25) == 0) {
        std::cout
            << "Loading... " << 100 * (double) i / size << "%" << '\n';
      }
      text[i++] = input_file.get();
    }
    size = i;
    std::cout << "Loaded!\n";
  }

  while (true) {
    std::cout << GetGreenText(
        "====\nCommands:\nFind (integer: number to find) "
        "(integer: number of threads)\n"
        "Substr (integer: position) (integer: count)\n====\n");

    std::string request;
    std::cin >> request;
    if (request == "Find" || request == "find") {
      std::string string;
      int thread_number;
      std::cin >> string >> thread_number;

      std::vector<size_t> indexes;
      std::mutex indexes_mutex;
      std::vector<std::unique_ptr<std::thread>> threads(thread_number);

      {
        TimeMeasurer T("Search");
        auto blocks = GenerateBlocks(0, size, string.size(), thread_number);
        std::cout << "Running on " << blocks.size() << " threads\n";
        for (int i = 0; i < thread_number; i++) {
          threads[i] = std::make_unique<std::thread>(FindEntries,
                                                     text + blocks[i].first,
                                                     blocks[i].second
                                                         - blocks[i].first,
                                                     string,
                                                     std::ref(indexes),
                                                     std::ref(indexes_mutex),
                                                     i);
        }
        for (auto& thread : threads) {
          thread->join();
        }
      }
      std::cout << GetGreenText(
          "--Found " + std::to_string(indexes.size()) + " entries--\n");
      std::cout
          << GetGreenText("How many entries show?\n");
      int number;
      std::cin >> number;

      {
        TimeMeasurer time_measurer("Sorting");
        std::sort(indexes.begin(), indexes.end());
      }

      std::cout
          << GetBlueText("Found entries from positions (1-indexation):\n");
      for (int i = 0; i < std::min(static_cast<int>(indexes.size()), number);
           i++) {
        std::cout << GetBlueText(std::to_string(indexes[i] + 1)) << '\n';
      }
    } else {
      int pos;
      int cnt;
      std::cin >> pos >> cnt;
      pos--;
      for (int i = pos; i < pos + cnt; i++) {
        std::cout << text[i];
      }
      std::cout << '\n';
      break;
    }
  }
  return 0;
}
