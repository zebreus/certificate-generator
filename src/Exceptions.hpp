#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <string>
#include <stdexcept>


//When there are syntax errors in the configuration file
class InvalidConfigurationError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

//When there are syntax errors in the template file
class InvalidTemplateError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

//When a file cant be opened
class FileAccessError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

//When a fork failed
class ForkFailedError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

//When latex execution failed
class LatexExecutionError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

//When the xelatex command is not found
class LatexMissingError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

#endif
