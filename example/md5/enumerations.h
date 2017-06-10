//
// Created by Albert on 6/10/17.
//

#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H
#include <vector>
#include <map>
#include <iostream>
#include <experimental/any>
#include <memory>
#include <unordered_set>
#include <stdexcept>

namespace Enumerations {
    class EnumerationBase {
    public:
        virtual ~EnumerationBase() {
        }

        EnumerationBase(const EnumerationBase &pb) = default;

        EnumerationBase() = default;

        virtual std::experimental::fundamentals_v1::any current() = 0;

        virtual void next() = 0;  // Go to next value
        virtual void reset() = 0; // Reset current value to start value
        virtual bool hasNext() = 0; // Check we have next value
    };

    using EnumerationObjects = std::vector<std::pair<std::string, std::unique_ptr<EnumerationBase> >>;

    template<class T>
    class Enumeration : public EnumerationBase {
    public:
        Enumeration(const Enumeration &pb) = default;

        Enumeration() = default;

        virtual ~Enumeration() = default;

        virtual std::experimental::fundamentals_v1::any current() override {
            return current_val;
        }

        virtual void next() override {
            if (!this->hasNext()) {
                throw std::out_of_range("Current enumeration doesn't has next values");
            }
            current_val += step;
        }
        virtual void reset() override {
            this->current_val = this->start;
        }
        virtual bool hasNext() override {
            return current_val + step <= end;
        }
        Enumeration(T start, T end, T step) : start(start), current_val(start), end(end), step(step) {}
    private:
        T start;
        T end;
        T step;
        T current_val;
    };


    class EnumerationMap {
        using Container = std::map<std::string, std::experimental::fundamentals_v1::any>;
    private:
        Container container;
    public:
        template<typename Type>
        Type get(std::string name) {
            return std::experimental::fundamentals_v1::any_cast<Type>(this->container[name]);
        }

        void insert(std::string name, std::experimental::fundamentals_v1::any val) {
            this->container[name] = val;
        }

        bool empty() {
            return this->container.empty();
        }
    };

    class Enumerator {
    public:
        template<typename Type>
        Enumerator &addEnumeration(std::string name, Enumeration<Type> enumeration) {
            this->enumeration.push_back({name, std::make_unique<Enumeration<Type> >(enumeration)});
            return *this;
        }

        template<typename Callback>
        void enumerate(Callback callback) {
            this->enumerate(0, this->enumeration, callback);
        }


    private:
        EnumerationObjects enumeration;
        template<typename Callback>
        void enumerate(int index_enumeration, EnumerationObjects &enumerations, Callback callback) {
            if (index_enumeration == enumerations.size()) {
                EnumerationMap enumerationMap;
                for (auto &p: enumerations) {
                    enumerationMap.insert(p.first, p.second->current());
                }
                if (!enumerationMap.empty())
                    callback(enumerationMap);
                return;
            }
            while (enumerations[index_enumeration].second->hasNext()) {
                enumerate(index_enumeration + 1, enumerations, callback);
                enumerations[index_enumeration].second->next();
            }
            enumerate(index_enumeration + 1, enumerations, callback);

            enumerations[index_enumeration].second->reset();
        }
    };
}


#endif //TRAINING_ENUMERATIONS_H
