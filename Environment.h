#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <string>
#include <stdexcept>

class Environment {
private:
    std::map<std::string, double> variables;
public:
    void set(const std::string& name, double value) {
        variables[name] = value;
    }
    double get(const std::string& name) const {
        auto it = variables.find(name);
        if (it == variables.end()) {
            throw std::runtime_error("Variable no definida: " + name);
        }
        return it->second;
    }
    bool exists(const std::string& name) const {
        return variables.find(name) != variables.end();
    }
};

#endif
