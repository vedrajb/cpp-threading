// thread.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <list>
#include <string>
#include <mutex>
#include <sstream>
#include <Windows.h>
#include <future>

constexpr int MAX = 10000;

std::list< std::string > g_data;
std::mutex g_mtx;

void Download1(std::string arg1)
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::cout << oss.str() << std::endl;

    for (int i = 0; i < MAX; ++i)
    {
        std::lock_guard< std::mutex > lock(g_mtx);
        g_data.push_back(oss.str());
    }
}

void Download2(std::string arg1)
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::cout << oss.str() << std::endl;

    for (int i = 0; i < MAX; ++i)
    {
        std::lock_guard< std::mutex > lock(g_mtx);
        g_data.push_back(oss.str());

    }
}

void threads()
{
    std::string temp;

    std::thread th1(Download1, std::cref(temp));
    SetThreadDescription(th1.native_handle(), TEXT("th1"));
    std::thread th2(Download2, std::cref(temp));
    SetThreadDescription(th2.native_handle(), TEXT("th2"));

    th1.join();
    th2.join();

    /*const int START = MAX - 1;
    const int END = (MAX + 100);

    auto it = g_data.begin();
    int count = START;
    while (count < END)
    {
    ++it;
    ++count;
    }

    count = START;
    while (count < END)
    {
        std::cout << *(it++) << ",";
        count ++;
    }*/
}

int printProgress(int count)
{
    using namespace std::chrono_literals;
    int sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += i;
        std::cout << static_cast< char >(196) << " ";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(500ms);
    }
    std::cout << std::endl;
    return sum;
}

void taskBasedAsync()
{
    using namespace std::chrono_literals;
    std::future<int> task = std::async(std::launch::async, printProgress, 10);
    std::this_thread::sleep_for(2s);
    std::cout << "[taskBasedAsync] Continue execution...\n";
    std::cout << "..." << std::boolalpha << task.valid() << std::endl;
    if (task.valid())
    {
        auto ret = task.get();
        std::cout << "[taskBasedAsync] Finished execution: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
}

void taskBasedDeferred()
{
    using namespace std::chrono_literals;
    std::future<int> task = std::async(std::launch::deferred, printProgress, 10);
    std::this_thread::sleep_for(2s);
    std::cout << "[taskBasedDeferred] Continue execution...\n";
    std::cout << "..." << std::boolalpha << task.valid() << std::endl;
    if (task.valid())
    {
        auto ret = task.get();
        std::cout << "[taskBasedDeferred] Finished execution: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
}

void taskBasedAsyncWait()
{
    using namespace std::chrono_literals;
    std::future<int> task = std::async(std::launch::async, printProgress, 10);
    //std::this_thread::sleep_for(2s);
    std::cout << "[taskBasedAsyncWait] Continue execution...\n";
    std::cout << "[taskBasedAsyncWait] Valid: " << std::boolalpha << task.valid() << std::endl;
    task.wait();
    std::cout << "[taskBasedAsyncWait] Valid: " << std::boolalpha << task.valid() << std::endl;
    if (task.valid())
    {
        auto ret = task.get();
        std::cout << "[taskBasedAsyncWait] Finished execution: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
}

void taskBasedAsyncWaitFor()
{
    using namespace std::chrono_literals;
    std::future<int> task = std::async(std::launch::async, printProgress, 10);
    //std::this_thread::sleep_for(2s);
    std::cout << "[taskBasedAsyncWaitFor] Continue execution...\n";
    if (task.valid())
    {
        auto state = std::future_status::timeout;
        while (std::future_status::timeout == state)
        {
            std::cout << "[taskBasedAsyncWaitFor] waiting...\n";
            state = task.wait_for(1s);
        }
        std::cout << "[taskBasedAsyncWaitFor] Task completed\n";
        auto ret = task.get();
        std::cout << "[taskBasedAsyncWaitFor] Output: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
}

void taskBasedAsyncWaitUntil()
{
    using namespace std::chrono_literals;
    std::future<int> task = std::async(std::launch::async, printProgress, 10);
    std::cout << "[taskBasedAsyncWaitUntil] Continue execution...\n";
    if (task.valid())
    {
        auto state = std::future_status::timeout;
        while (std::future_status::timeout == state)
        {
            auto timepoint = std::chrono::system_clock::now() + 1s;
            std::cout << "[taskBasedAsyncWaitUntil] waiting till " << timepoint.time_since_epoch().count() << std::endl;
            state = task.wait_until(timepoint);
        }
        std::cout << "[taskBasedAsyncWaitUntil] Task completed\n";
        auto ret = task.get();
        std::cout << "[taskBasedAsyncWaitUntil] Output: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
}

int opPromise(std::promise< int >& data)
{
    std::cout << "[opPromise] Waiting for data...\n";
    auto f = data.get_future();
    int count = f.get();
    std::cout << "[opPromise] Received count = " << count << std::endl;
    int sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += i;
        std::cout << static_cast< char >(196) << " ";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << std::endl;
    return sum;
}

void promiseBasedTask()
{
    using namespace std::chrono_literals;
    std::promise< int > data;
    auto task = std::async(std::launch::async, opPromise, std::ref(data));
    std::this_thread::sleep_for(2s);
    std::cout << "[promiseBasedTask] Set count = 10\n";
    data.set_value(10);
    std::cout << "[promiseBasedTask] Continue execution...\n";
    if (task.valid())
    {
        auto state = std::future_status::timeout;
        while (std::future_status::timeout == state)
        {
            std::cout << "[promiseBasedTask] waiting...\n";
            state = task.wait_for(1s);
        }
        std::cout << "[promiseBasedTask] Task completed\n";
        auto ret = task.get();
        std::cout << "[promiseBasedTask] Output: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
}

int opPromiseException(std::promise< int >& data)
{
    std::cout << "[opPromiseException] Waiting for data...\n";
    auto f = data.get_future();
    try
    {
        int sum = 0;
        auto count = f.get();
        std::cout << "[opPromiseException] Received count = " << count << std::endl;
        for (int i = 0; i < count; i++)
        {
            sum += i;
            std::cout << static_cast< char >(196) << " ";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << std::endl;
        return sum;
    }
    catch (const std::exception& ex)
    {
        std::cout << "[opPromiseException] Exception in promise: " << ex.what() << std::endl;
    }
}

void promiseException()
{
    using namespace std::chrono_literals;
    std::promise< int > data;
    auto task = std::async(std::launch::async, opPromiseException, std::ref(data));
    std::this_thread::sleep_for(2s);
    std::cout << "[promiseException] Setting data\n";
    try 
    {
        throw std::runtime_error{"Data not available!"};
        auto state = std::future_status::timeout;
        while (std::future_status::timeout == state)
        {
            std::cout << "[promiseException] waiting...\n";
            state = task.wait_for(1s);
        }
        std::cout << "[promiseException] Task completed\n";
        auto ret = task.get();
        std::cout << "[promiseException] Output: " << ret << ", " << std::boolalpha << task.valid() << std::endl;
    }
    catch (const std::exception& ex)
    {
        data.set_exception(std::make_exception_ptr(ex));
        //task.wait();
    }
}

int main()
{
    std::cout << "[main] Cores: " << std::thread::hardware_concurrency() << std::endl;

    std::cout << "\n*** [main] threads() ***\n" << std::endl;
    threads();
    std::cout << "\n*** [main] printProgress(10) ***\n" << std::endl;
    printProgress(10);
    std::cout << "\n*** [main] taskBasedAsync() ***\n" << std::endl;
    taskBasedAsync();
    std::cout << "\n*** [main] taskBasedDeferred() ***\n" << std::endl;
    taskBasedDeferred();
    std::cout << "\n*** [main] taskBasedAsyncWait() ***\n" << std::endl;
    taskBasedAsyncWait();
    std::cout << "\n*** [main] taskBasedAsyncWaitFor() ***\n" << std::endl;
    taskBasedAsyncWaitFor();
    std::cout << "\n*** [main] taskBasedAsyncWaitUntil() ***\n" << std::endl;
    taskBasedAsyncWaitUntil();
    std::cout << "\n*** [main] promiseBasedTask() ***\n" << std::endl;
    promiseBasedTask();
    std::cout << "\n*** [main] promiseException() ***\n" << std::endl;
    promiseException();

    return 0;
}

















