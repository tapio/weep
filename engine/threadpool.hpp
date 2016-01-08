#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>


class thread_pool {
public:
	typedef std::function<void()> task_t;

	thread_pool(unsigned num_threads = std::thread::hardware_concurrency()) {
		resize(num_threads);
	}

	~thread_pool() {
		stop();
	}

	void resize(unsigned num_threads) {
		stop();
		m_stop = false;
		for (unsigned i = 0; i < num_threads; ++i) {
			m_threads.emplace_back([this] {
				while (true) {
					task_t task;
					{
						std::unique_lock<std::mutex> lock(m_mutex);
						m_condition.wait(lock, [this]{ return m_stop || !m_tasks.empty(); });
						if (m_stop && m_tasks.empty())
							return;
						task = std::move(m_tasks.front());
						m_tasks.pop();
					}
					task();
					--m_count;
				}
			});
		}
	}

	void stop() {
		sync();
		m_stop = true;
		m_condition.notify_all();
		for (std::thread& t: m_threads)
			if (t.joinable())
				t.join();
		m_threads.clear();
	}

	template<class F>
	void enqueue(F task) {
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_tasks.emplace(task);
			++m_count;
		}
		m_condition.notify_one();
	}

	void sync() const {
		while (m_count)
			std::this_thread::yield();
	}

	unsigned size() const {
		return m_threads.size();
	}

private:
	std::vector<std::thread> m_threads;
	std::queue<task_t> m_tasks;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	std::atomic_int m_count = { 0 };
	std::atomic_bool m_stop = { false };
};
