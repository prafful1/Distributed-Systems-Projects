#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

struct Message {
    int sender_id;
    int receiver_id;
    int timestamp;
};

class Process {
public:
    Process(int id, std::queue<Message>& message_queue, std::mutex& mtx, std::condition_variable& cv)
        : id(id), logical_clock(0), message_queue(message_queue), mtx(mtx), cv(cv) {}

    void send_message(int receiver_id) {
        std::unique_lock<std::mutex> lock(mtx);
        increment_clock();
        Message msg = {id, receiver_id, logical_clock};
        message_queue.push(msg);
        cv.notify_all();
        std::cout << "Process " << id << " sent message to Process " << receiver_id << " with timestamp " << logical_clock << std::endl;
    }

    void receive_message() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !message_queue.empty(); });

        while (!message_queue.empty()) {
            Message msg = message_queue.front();
            if (msg.receiver_id == id) {
                message_queue.pop();
                logical_clock = std::max(logical_clock, msg.timestamp) + 1;
                std::cout << "Process " << id << " received message from Process " << msg.sender_id << " with timestamp " << msg.timestamp << ". Updated clock: " << logical_clock << std::endl;
            } else {
                break;
            }
        }
    }

    void increment_clock() {
        logical_clock++;
    }

private:
    int id;
    int logical_clock;
    std::queue<Message>& message_queue;
    std::mutex& mtx;
    std::condition_variable& cv;
};

void process_function(Process& process, std::vector<Process>& processes) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        process.receive_message();
    }
}

int main() {
    const int num_processes = 3;
    std::vector<Process> processes;
    std::vector<std::thread> threads;
    std::queue<Message> message_queue;
    std::mutex mtx;
    std::condition_variable cv;

    for (int i = 0; i < num_processes; ++i) {
        processes.emplace_back(i, message_queue, mtx, cv);
    }

    for (int i = 0; i < num_processes; ++i) {
        threads.emplace_back(process_function, std::ref(processes[i]), std::ref(processes));
    }

    // Simulate sending messages
    std::this_thread::sleep_for(std::chrono::seconds(1));
    processes[0].send_message(1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    processes[1].send_message(2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    processes[2].send_message(0);

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}