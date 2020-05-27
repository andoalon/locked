#include "locked/locked.hh"

#include <queue>
#include <cassert>

struct Foo
{

};

struct FooManager
{
	FooManager() = default;
	FooManager(const FooManager&) = delete;
	FooManager(FooManager&&) = delete;
	FooManager& operator=(const FooManager&) = delete;
	FooManager& operator=(FooManager&&) = delete;

	void insert_foo(Foo foo)
	{
		auto the_foos = foos.lock_mutable<std::lock_guard>();
		the_foos->push(std::move(foo));
	}

	LockedPtr<const std::queue<Foo>, std::shared_lock<std::shared_mutex>> read_foos() const
	{
		return foos.lock_const();
	}

	LockedPtr<std::queue<Foo>, std::lock_guard<std::shared_mutex>> read_write_foos()
	{
		return foos.lock_mutable();
	}

	[[nodiscard]] std::queue<Foo> copy_foos() const
	{
		return *foos.lock_const();
	}

private:
	Locked<std::queue<Foo>, std::shared_mutex> foos;
};


int main()
{
	FooManager manager;
	assert(manager.read_foos()->empty());

	manager.insert_foo({});
	assert(manager.read_foos()->size() == 1);

	[[maybe_unused]] const auto copy = manager.copy_foos();
	manager.read_write_foos()->emplace();

	assert(copy.size() == 1);
	assert(manager.read_foos()->size() == 2);

	return 0;
}
