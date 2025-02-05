#pragma once

template <typename T>
class ConcurrentSet
{
public:
    void insert(const T& value)
    {
        lock_guard<mutex> lock(_mtx);
        _data.insert(value);
    }

    void erase(const T& value)
    {
        lock_guard<mutex> lock(_mtx);
        _data.erase(value);
    }

    bool contains(const T& value) const
    {
        lock_guard<mutex> lock(_mtx);
        return _data.find(value) != _data.end();
    }

    void apply(const function<void(T&)> fnc)
    {
        lock_guard<mutex> lock(_mtx);
        for (auto& val : _data)
        {
            fnc(val);
        }
    }

private:
    set<T> _data;
    mutable mutex _mtx;
};
