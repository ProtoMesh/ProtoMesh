#include <Logger/Logger.hpp>
#include "../os-specifics/unix/unix.hpp"

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
    Logger::setTimeProvider(make_shared<UnixRelativeTimeProvider>());
    Logger::setLogLevel(Debug);

    /// Print the version
    Logger(Custom, "    ProtoMesh α") << "v0.0.1-6afc6d" << endl;

    Logger(Info) << "Creating UnixNetworkProvider" << endl;
    NETWORK_PROVIDER_T net = make_shared<UnixNetworkProvider>();
    std::vector<NETWORK_T> networks = net->getAvailableNetworks();

//    while (!interrupted) {};

    cout << endl;

    Logger(Error) << "Received interrupt. Goodbye cruel world!" << endl;
}
