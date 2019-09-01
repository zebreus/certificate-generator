#include "Student.hpp"

Student::Student(json properties)
	: properties(properties)
{
}

json Student::getProperties() const
{
	return properties;
}
