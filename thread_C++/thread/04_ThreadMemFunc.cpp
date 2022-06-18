#include <thread>
#include <iostream>

using namespace std;

class Request
{
public:
	explicit Request(int id) : mId(id) { }

	void process() const
	{
		cout << "Processing request " << mId << endl;
	}

private:
	int mId;
};

int main()
{
	Request req(100);
	thread t{ &Request::process, &req };

	t.join();

	return 0;
}
