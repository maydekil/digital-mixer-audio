#include "NativeAudioPlayer.hpp"

#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>
#include <string>

extern char** environ;

namespace soundpad {
namespace {

std::string gLastError;
pid_t gAfplayPid = 0;
volatile sig_atomic_t gStopRequested = 0;

void stopAfplay(int) {
  gStopRequested = 1;
  if (gAfplayPid > 0) kill(gAfplayPid, SIGTERM);
}

bool runAfplay(const std::filesystem::path& path) {
  const std::string pathText = path.string();
  const char* player = "/usr/bin/afplay";
  char* const args[] = {const_cast<char*>(player), const_cast<char*>(pathText.c_str()), nullptr};

  pid_t pid = 0;
  const int spawnResult = posix_spawn(&pid, player, nullptr, nullptr, args, environ);
  if (spawnResult != 0) {
    gLastError = "failed to start /usr/bin/afplay";
    return false;
  }

  gAfplayPid = pid;
  int status = 0;
  while (waitpid(pid, &status, 0) < 0) {
    if (errno == EINTR) continue;
    gAfplayPid = 0;
    gLastError = "failed waiting for /usr/bin/afplay";
    return false;
  }
  gAfplayPid = 0;

  if (gStopRequested || (WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM)) return true;

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    gLastError = "/usr/bin/afplay failed with exit code " + std::to_string(WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return false;
  }

  return true;
}

}  // namespace

bool playFileBlocking(const std::filesystem::path& path) {
  gLastError.clear();
  gStopRequested = 0;
  if (!std::filesystem::exists(path)) {
    gLastError = "sound pad asset is missing: " + path.string();
    return false;
  }

  signal(SIGTERM, stopAfplay);
  signal(SIGINT, stopAfplay);
  return runAfplay(path);
}

const char* lastPlaybackError() {
  return gLastError.c_str();
}

}  // namespace soundpad
