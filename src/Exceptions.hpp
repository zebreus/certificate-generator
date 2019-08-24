#include <string>
#include <stdexcept>

class InvalidConfigurationError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class InvalidTemplateError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class UnableToWriteFileError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};
