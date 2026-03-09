#include <boost/dll/shared_library.hpp>
#include <exception>
#include <iostream>
#include <string>

int main() {
  try {
    std::cout << "Enter library name (hw1_06_05_v1 or hw1_06_05_v2): ";

    std::string library_name;
    if (!(std::cin >> library_name)) {
      std::cerr << "Input error\n";
      return 1;
    }

    boost::dll::shared_library library(
        library_name, boost::dll::load_mode::append_decorations);

    auto const function = library.get<std::string()>("test");

    std::cout << function() << '\n';
    return 0;
  } catch (std::exception const& exception) {
    std::cerr << "Error: " << exception.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Unknown error\n";
    return 1;
  }
}
