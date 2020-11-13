#pragma once
#include <algorithm>
#include "../shared_ptr/SharedPtr.hpp"

template <typename T>
class WeakPtr {
friend class SharedPtr;
public:
    constexpr WeakPtr() noexcept = default;
    WeakPtr(const WeakPtr<T>& otherPtr) noexcept;
    WeakPtr(const SharedPtr<T>& otherPtr) noexcept;
    WeakPtr(WeakPtr&& otherPtr) noexcept;
    ~WeakPtr();

    WeakPtr& operator=(const WeakPtr& otherPtr) noexcept;
    WeakPtr& operator=(const SharedPtr<T>& otherPtr) noexcept;
    WeakPtr& operator=(WeakPtr&& otherPtr) noexcept;

    size_t useCount() const noexcept;
    bool expired() const noexcept;
    SharedPtr<T> lock() const noexcept;
    void reset() noexcept;

private:
    void removeControlBlock();
    void decrementWeakCounter();
    void incrementWeakCounter();
    
    T* rawPtr_;
    ControlBlock<T>* controlBlock_;
};

template <typename T>
void WeakPtr<T>::removeControlBlock() {
    if (controlBlock_->sharedRefsCounter_ == 0 && controlBlock_->weakRefsCounter_ == 0) {
        delete controlBlock_;
    }
}

template <typename T>
void WeakPtr<T>::decrementWeakCounter() {
    controlBlock_->weakRefsCounter_--;
}

template <typename T>
void WeakPtr<T>::incrementWeakCounter() {
    controlBlock_->weakRefsCounter_++;
}

template <typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& otherPtr) noexcept
    : rawPtr_(otherPtr.get()), controlBlock_(otherPtr.controlBlock_) {
    incrementWeakCounter();
}

template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr<T>& otherPtr) noexcept
    : rawPtr_(otherPtr.rawPtr_), controlBlock_(otherPtr.controlBlock_) {
    incrementWeakCounter();
}

template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr<T>&& otherPtr) noexcept
    : rawPtr_(otherPtr.rawPtr_), controlBlock_(otherPtr.controlBlock_) {
    otherPtr.rawPtr_ = nullptr;
    otherPtr.controlBlock_ = nullptr;
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
    if (controlBlock_) {
        decrementWeakCounter();
        removeControlBlock();
    }
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& otherPtr) noexcept {
    if (this != &otherPtr) {
        rawPtr_ = otherPtr.rawPtr_;
        controlBlock_ = otherPtr.controlBlock_;
        incrementWeakCounter();
    }
    return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<T>& otherPtr) noexcept {
    if (controlBlock_) {
        rawPtr_ = otherPtr.rawPtr_;
        controlBlock_ = otherPtr.controlBlock_;
        incrementWeakCounter();
    }
    return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& otherPtr) noexcept {
    if (this != otherPtr) {
        rawPtr_ = otherPtr.rawPtr_;
        controlBlock_ = otherPtr.controlBlock_;
        otherPtr.rawPtr_ = nullptr;
        otherPtr.controlBlock_ = nullptr;
    }
    return *this;
}

template <typename T>
size_t WeakPtr<T>::useCount() const noexcept {
    return controlBlock_->sharedRefsCounter_;
}

template <typename T>
bool WeakPtr<T>::expired() const noexcept {
    return useCount() == 0;
}

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() const noexcept {
    if (expired()) {
        return SharedPtr<T>();
    }
    return SharedPtr<T>(*this);
}

template <typename T>
void WeakPtr<T>::reset() noexcept {
    rawPtr_ = nullptr;
    if (controlBlock_) {
        decrementWeakCounter();
        removeControlBlock();
    } else {
        controlBlock_ = nullptr;
    }
}
