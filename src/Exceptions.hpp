#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

//Superclass for all generator errors
class GeneratorError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

//When there are syntax errors in the configuration file
class InvalidConfigurationError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

//When there are syntax errors in the template file
class InvalidTemplateError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

//When a file cant be opened
class FileAccessError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

//When a fork failed
class ForkFailedError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

//When latex execution failed
class LatexExecutionError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

//When the xelatex command is not found
class LatexMissingError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

//When the internal configuration is tried to be set a second time
class ConfigurationError : public GeneratorError {
	using GeneratorError::GeneratorError;
};

#endif
