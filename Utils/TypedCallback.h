#pragma once

// Represents a generic object that can have a callback.
template<typename T>
class ICallback
{
public:
    virtual ~ICallback()
    {
    }

    virtual void Callback(T callingObject, void* callbackSpecificData) = 0;
};

// Represents an instance of a callback that has a type, a callback, and can take in (and optionally delete) callback specific data.
template<typename T>
class TypedCallback
{
    T type;
    ICallback<T>* callback;

    bool deleteCSDAfterCallback;
    void* callbackSpecificData;

public:
    TypedCallback(T type, ICallback<T>* callback = nullptr, void* callbackSpecificData = nullptr, bool deleteCSDAfterCallback = false)
        : type(type), callback(callback), callbackSpecificData(callbackSpecificData), deleteCSDAfterCallback(deleteCSDAfterCallback)
    {
    }

    T GetType() const
    {
        return type;
    }

    void CallCallback()
    {
        if (callback != nullptr)
        {
            callback->Callback(type, callbackSpecificData);
        }

        if (callbackSpecificData != nullptr && deleteCSDAfterCallback)
        {
            delete callbackSpecificData;
            callbackSpecificData = nullptr;
        }
    }

    void CallCallback(T callingObject)
    {
        if (callback != nullptr)
        {
            callback->Callback(callingObject, callbackSpecificData);
        }

        if (callbackSpecificData != nullptr && deleteCSDAfterCallback)
        {
            delete callbackSpecificData;
            callbackSpecificData = nullptr;
        }
    }
};