

#include "filesystem"
namespace fs = std::filesystem;

void uploadFile(const fs::path &filename, std::error_code &ec) {
  if (!fs::is_regular_file(filename)) {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
    return;
  }
}

int main(int argc, char **argv) {

}