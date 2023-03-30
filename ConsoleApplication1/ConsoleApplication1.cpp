#include <NIDAQmx.h>
#include <iostream>
#include <thread>
#include <stdlib.h>
#include <vector>

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

// forward declaration of Subject class
class Subject;

// Observer class
class Observer {
public:
    virtual ~Observer() {}
    virtual void update(Subject* subject) = 0;
};

// Subject class
class Subject {
public:
    void attach(Observer* observer) {
        observers_.push_back(observer);
    }

    void detach(Observer* observer) {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            observers_.erase(it);
        }
    }

    void notify() {
        for (auto observer : observers_) {
            observer->update(this);
        }
    }

    void set_state() {
        notify();
    }

    std::string get_state() const {
        return state_;
    }

private:
    std::vector<Observer*> observers_;
    std::string state_;
};

// Concrete Observer class
class ConcreteObserver : public Observer {
public:
    ConcreteObserver(TaskHandle handler, int delay, int duration) : handler_(handler), delay_(delay), duration_(duration) {}
    void update(Subject* subject) override {
        std::thread t(writeOutput, this->handler_, this->delay_, this->duration_);
        t.join();
    }
private:
    TaskHandle handler_;
    int delay_;
    int duration_;
};

void writer()
{
    TaskHandle taskHandle = NULL;
    float64 voltage = 0.0;

    // Create the task
    DAQmxCreateTask("Writer", &taskHandle);

    // Create the analog output voltage channel
    DAQmxCreateAOVoltageChan(taskHandle, "Dev1/ao0", "", -5.0, 5.0, DAQmx_Val_Volts, "");

    // Start the task
    DAQmxStartTask(taskHandle);

    while (true)
    {
        // Set the voltage to 4.0V
        // Write the voltage to the AO port
        DAQmxWriteAnalogScalarF64(taskHandle, true, 5.0, 4.0, NULL);


        const int timer_duration_ms = 100;
        // Get the current time
        auto start_time = std::chrono::high_resolution_clock::now();

        // Loop until the timer duration is reached
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < timer_duration_ms) {
            continue;
        }
        // Set the voltage back to 0.0V
        // Write the voltage to the AO port
        DAQmxWriteAnalogScalarF64(taskHandle, true, 5.0, 0.0, NULL);

        start_time = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < timer_duration_ms) {
            continue;
        }
    }
}

void reader()
{
    const int size = 5;
    int counter = 0;
    TaskHandle taskHandle = NULL;
    TaskHandle output = NULL;
    float64 data[size];
    int32 read;
    bool hasChanged = false;
    float64 curr = 0;

    // Create a new task
    DAQmxCreateTask("Reader", &taskHandle);
    DAQmxCreateTask("ouput", &output);

    // Add an analog input channel to the task
    DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai11", "", DAQmx_Val_Cfg_Default, -5.0, 5.0, DAQmx_Val_Volts, NULL);
    DAQmxCreateAOVoltageChan(output, "Dev1/ao1", "", -5.0, 5.0, DAQmx_Val_Volts, "");

    // Start the task
    DAQmxStartTask(taskHandle);
    DAQmxStartTask(output);

    ConcreteObserver* obs = new ConcreteObserver(output, 10 ,45);
    Subject sub;
    sub.attach(obs);

    // Read a single voltage value from the channel
    auto start_time = std::chrono::high_resolution_clock::now();
    while (true) {
        DAQmxReadAnalogF64(taskHandle, size, 5.0, DAQmx_Val_GroupByScanNumber, data, size, &read, NULL);
        counter += size;
        if (!hasChanged && data[0]>3.5) {
            sub.notify();
            hasChanged = true;
        }
        else if (hasChanged && data[0] < 3.5) {
            hasChanged = false;
        }
    }

    // Stop and clear the task
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);
}

int main()
{
    std::thread t1(writer);
    std::thread t2(reader);

    t1.join();
    t2.join();
    return 0;
}