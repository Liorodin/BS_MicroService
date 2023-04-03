#include "Utilities.hpp"

void writeOutput(TaskHandle taskHandle, int delay, int duration) {
    // Get the current time
    auto start_time = std::chrono::high_resolution_clock::now();
    // Loop until the timer duration is reached
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < delay) {
        continue;
    }
    DAQmxWriteAnalogScalarF64(taskHandle, true, 5.0, 4.0, NULL);

    // Get the current time
    start_time = std::chrono::high_resolution_clock::now();
    // Loop until the timer duration is reached
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < duration) {
        continue;
    }
    // Set the voltage back to 0.0V
    // Write the voltage to the AO port
    DAQmxWriteAnalogScalarF64(taskHandle, true, 5.0, 0.0, NULL);
}

Subject::Subject() {}

void Subject::attach(Observer* observer){
    observers_.push_back(observer);
}

void Subject::detach(Observer* observer) {
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void Subject::notify() {
    for (auto observer : observers_) {
        observer->update(this);
    }
}
    
void Subject::set_state() {
    notify();
}

void ConcreteObserver::update(Subject* subject) {
    std::thread t(writeOutput, this->handler_, this->delay_, this->duration_);
    t.join();
}