#ifndef __IDatabase_hpp__
#define __IDatabase_hpp__
#include <string>

class  IDatabase
{
public:

	virtual ~IDatabase() = default;
	virtual bool connect(const std::string& connection_string) = 0;
	virtual void disconnect() = 0;
};

#endif // !__IDatabase_hpp__
