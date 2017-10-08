#include <Logger/Logger.hpp>
#include "../os-specifics/linux/linux.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted++;
}

int main() {
    signal(SIGINT, onInterrupt);

    /// Get the terminal width
    struct winsize size {};
    ioctl(STDOUT_FILENO,TIOCGWINSZ, &size);

    /// Logger setup
    Logger::setOutputStream(&std::cout);
    Logger::setOutputWidth(size.ws_col ? size.ws_col : 100);
    Logger::setTimeProvider(make_shared<LinuxRelativeTimeProvider>());
    Logger::setLogLevel(Info);

    /// Print the version
    Logger(Custom, "    HoMesh α") << "v0.0.1-6afc6d" << endl;

//    while (!interrupted) {};

    Logger(Error) << "Received interrupt. Goodbye cruel world!" << endl;
}
