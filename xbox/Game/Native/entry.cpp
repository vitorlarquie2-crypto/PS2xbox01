#include "host.h"
#include "ps2_runtime.h"
#include "ps2_host_backend.h"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace
{
    std::mutex lifecycleMutex, stateMutex;
    std::thread worker;
    PS2Runtime *active = nullptr;
    std::atomic<int> status{0};
    std::string failure;
}

int ps2x_status(char *error, uint32_t capacity)
{
    std::lock_guard lock(stateMutex);
    if (error && capacity)
    {
        const size_t count = std::min(failure.size(), static_cast<size_t>(capacity - 1));
        std::memcpy(error, failure.data(), count);
        error[count] = '\0';
    }
    return status.load();
}
namespace
{
void stopLocked()
{
    {
        std::lock_guard lock(stateMutex);
        ps2x::host::close();
        if (active) active->requestStop();
    }
    if (worker.joinable()) worker.join();
    status.store(0);
}
}
void ps2x_stop()
{
    std::lock_guard lock(lifecycleMutex);
    stopLocked();
}
namespace
{
    struct Shutdown { ~Shutdown() { ps2x_stop(); } } shutdown;
}
int ps2x_start(const char *elfPath)
{
    std::lock_guard lifecycleLock(lifecycleMutex);
    if (!elfPath || !*elfPath || status.load() == 1) return 0;
    stopLocked();
    const std::string path(elfPath);
    ps2x::host::reset();
    {
        std::lock_guard lock(stateMutex);
        failure.clear();
        status.store(1);
    }
    try
    {
        worker = std::thread([path]
        {
            std::unique_ptr<PS2Runtime> runtime;
            try
            {
                runtime = std::make_unique<PS2Runtime>();
                if (!runtime->initialize("PS2Xbox")) throw std::runtime_error("Falha ao inicializar o runtime.");
                if (!runtime->loadELF(path)) throw std::runtime_error("Falha ao carregar o ELF do jogo.");
                if (!runtime->hasFunction(runtime->cpu().pc)) throw std::runtime_error("Este pacote não contém código recompilado para o ELF selecionado.");
                {
                    std::lock_guard lock(stateMutex);
                    active = runtime.get();
                }
                if (!WindowShouldClose()) runtime->run();
                {
                    std::lock_guard lock(stateMutex);
                    active = nullptr;
                }
                runtime.reset();
                status.store(2);
            }
            catch (const std::exception &e)
            {
                std::lock_guard lock(stateMutex);
                active = nullptr; failure = e.what(); status.store(-1);
            }
            catch (...)
            {
                std::lock_guard lock(stateMutex);
                active = nullptr; failure = "Falha inesperada no runtime."; status.store(-1);
            }
        });
    }
    catch (const std::exception &e)
    {
        std::lock_guard lock(stateMutex);
        failure = e.what(); status.store(-1);
        return 0;
    }
    return 1;
}
