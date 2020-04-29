#ifndef BOUNDED_BUFFER_HPP
#define BOUNDED_BUFFER_HPP

#include <condition_variable>
#include <chrono>
#include <mutex>
#include <queue>

/*
 * A bounded buffer for producer/consumer applications. Has multiple
 * pushing/popping interfaces and also tracks the number of "dropped packets"
 * (i.e. number of elements which were unsuccesfully pushed into a full buffer).
 */
template <typename T>
class BoundedBuffer
{
public:
    explicit BoundedBuffer(std::size_t cap_) : cap(cap_) {}

    BoundedBuffer(std::size_t cap_,
                  std::chrono::milliseconds timeout_) :
        cap(cap_),
        timeout(timeout_)
    {}

    bool empty() const;
    std::size_t size() const;
    std::size_t capacity() const;
    std::size_t dropped_elements() const;

    T front() const;
    T back() const;

    /*
     * Attempts to execute action immediately. Push fails and returns false if
     * the buffer is full. Pop fails and returns a nullptr if the buffer is
     * empty, so its return value should be checked against nullptr before
     * dereferencing.
     */
    bool try_push(const T&);
    std::shared_ptr<T> try_pop();

    /*
     * Waits indefinitely.
     */
    void push_wait(const T&);
    std::shared_ptr<T> pop_wait();

    /*
     * Waits for a given amount of time before failing. The timeout value is
     * specified in the constructor, not as an argument to these functions.
     */
    bool push_wait_for(const T&);
    std::shared_ptr<T> pop_wait_for();
private:
    std::queue<T> q;

    mutable std::mutex m;
    std::condition_variable q_has_element;
    std::condition_variable q_has_space;

    // Capacity, since std::queue doesn't contain a capacity value.
    std::size_t cap;
    std::chrono::milliseconds timeout = std::chrono::milliseconds::zero();

    // Number of "dropped packets", the number of elements that have been
    // unsuccessfully pushed into the buffer.
    std::size_t dropped{};
};

template <typename T>
bool BoundedBuffer<T>::empty() const
{
    std::lock_guard<std::mutex> g(m);
    return q.empty();
}

template <typename T>
std::size_t BoundedBuffer<T>::size() const
{
    std::lock_guard<std::mutex> g(m);
    return q.size();
}

template <typename T>
std::size_t BoundedBuffer<T>::capacity() const
{
    std::lock_guard<std::mutex> g(m);
    return cap;
}

template <typename T>
std::size_t BoundedBuffer<T>::dropped_elements() const
{
    std::lock_guard<std::mutex> g(m);
    return dropped;
}

template <typename T>
T BoundedBuffer<T>::front() const
{
    std::lock_guard<std::mutex> g(m);
    return q.front();
}

template <typename T>
T BoundedBuffer<T>::back() const
{
    std::lock_guard<std::mutex> g(m);
    return q.back();
}

template <typename T>
bool BoundedBuffer<T>::try_push(const T& e)
{
    std::lock_guard<std::mutex> lk(m);
    if (q.size() != cap)
    {
        q.push(e);
        q_has_element.notify_one();
        return true;
    }
    else
    {
        dropped++;
        return false;
    }
}

template <typename T>
std::shared_ptr<T> BoundedBuffer<T>::try_pop()
{
    std::lock_guard<std::mutex> lk(m);
    if (q.empty())
    {
        return nullptr;
    }
    auto rv = std::make_shared<T>(q.front());
    q.pop();
    q_has_space.notify_one();
    return rv;
}

template <typename T>
void BoundedBuffer<T>::push_wait(const T& e)
{
    std::unique_lock<std::mutex> lk(m);
    q_has_space.wait(lk, [this]{ return q.size() != cap; });
    q.push(e);

    q_has_element.notify_one();
}

template <typename T>
std::shared_ptr<T> BoundedBuffer<T>::pop_wait()
{
    std::unique_lock<std::mutex> lk(m);
    q_has_element.wait(lk, [this]{ return !q.empty(); });
    auto rv = std::make_shared<T>(q.front());
    q.pop();

    q_has_space.notify_one();
    return rv;
}

template <typename T>
bool BoundedBuffer<T>::push_wait_for(const T& e)
{
    bool success;
    std::unique_lock<std::mutex> lk(m);
    if (q_has_space.wait_for(lk,
                             timeout,
                             [this]{ return q.size() != cap; }))
    {
        q.push(e);
        success = true;
    }
    else
    {
        dropped++;
        success = false;
    }

    q_has_element.notify_one();
    return success;
}

template <typename T>
std::shared_ptr<T> BoundedBuffer<T>::pop_wait_for()
{
    std::shared_ptr<T> rv;
    std::unique_lock<std::mutex> lk(m);
    if (q_has_element.wait_for(lk, timeout, [this]{ return !q.empty(); }))
    {
        rv = std::make_shared<T>(q.front());
        q.pop();
    }
    else
    {
        rv = nullptr;
    }

    q_has_space.notify_one();
    return rv;
}

#endif /* BOUNDED_BUFFER_HPP */
