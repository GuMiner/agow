#pragma once

// Represents a generic object that can have a callback.
template<typename T>
class ICallback
{
public:
    virtual ~ICallback()
    {
    }

    virtual void Callback(T callingObject) = 0;
};

template<typename T>
class TypedCallback
{
    T type;
    ICallback<T>* callback;

public:
    TypedCallback(T type, ICallback<T>* callback = nullptr)
        : type(type), callback(callback)
    {
    }

    T GetType() const
    {
        return type;
    }

    void CallCallback(T callingObject) const
    {
        if (callback != nullptr)
        {
            callback->Callback(callingObject);
        }
    }
};