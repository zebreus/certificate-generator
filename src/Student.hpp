#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "Certificate.hpp"

using json = nlohmann::json;
using namespace std;

/**
 * @class Student
 *
 * @brief A student represents a student
 *
 * A student represents a student
 */
class Student{

private:
	json properties;

public:
	/** @brief Constructor that creates a Student
    * @param [in] properties is a json containing the properties of the Student
    * @return A pointer to the created Student
    *
    */
	Student(json properties);
	
    /** @brief Returns the properties of this Student
    * @return A json containing the properties of this Student
    */
	json getProperties() const;
};

#endif
