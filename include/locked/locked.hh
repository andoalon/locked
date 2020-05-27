#include <mutex>
#include <shared_mutex>
#include <type_traits>

namespace detail
{
	template <typename T, typename AlwaysVoid>
	constexpr bool is_shared_mutex_impl = false;

	template <typename T>
	constexpr bool is_shared_mutex_impl<T, std::void_t<decltype(std::declval<T>().lock_shared())>> = true;
} // namespace detail

template <typename T>
constexpr bool is_shared_mutex = ::detail::is_shared_mutex_impl<T, void>;

static_assert(is_shared_mutex<std::shared_mutex>);
static_assert(is_shared_mutex<std::shared_timed_mutex>);
static_assert(!is_shared_mutex<std::mutex>);
static_assert(!is_shared_mutex<std::timed_mutex>);

template <typename T>
constexpr bool is_shared_lock = false;

template <typename Mutex>
constexpr bool is_shared_lock<std::shared_lock<Mutex>> = true;

static_assert(is_shared_lock<std::shared_lock<std::mutex>>);
static_assert(is_shared_lock<std::shared_lock<std::shared_mutex>>);
static_assert(!is_shared_lock<std::lock_guard<std::mutex>>);
static_assert(!is_shared_lock<std::scoped_lock<std::mutex>>);
static_assert(!is_shared_lock<std::unique_lock<std::mutex>>);


template <typename T, typename Mutex>
struct Locked;

template <typename T, typename Lock>
struct [[nodiscard]] LockedPtr
{
public:
	template <typename Mutex, typename = std::enable_if_t<std::is_constructible_v<Lock, Mutex&>>>
	explicit LockedPtr(Locked<T, Mutex>& aLocked) noexcept(std::is_nothrow_constructible_v<Lock, Mutex&>)
		: lock(aLocked.mutex)
		, locked_object(std::addressof(aLocked.object))
	{}

	template <typename Mutex, typename = std::enable_if_t<std::is_constructible_v<Lock, Mutex&>>>
	explicit LockedPtr(const Locked<std::remove_const_t<T>, Mutex>& aLocked) noexcept(std::is_nothrow_constructible_v<Lock, Mutex&>)
		: lock(aLocked.mutex)
		, locked_object(std::addressof(aLocked.object))
	{}

	LockedPtr(const LockedPtr &) = delete;
	LockedPtr & operator=(const LockedPtr &) = delete;

	LockedPtr(LockedPtr &&) = default;
	LockedPtr& operator=(LockedPtr&&) = default;

	[[nodiscard]] T& operator*() const noexcept { return *locked_object; }
	[[nodiscard]] T* operator->() const noexcept { return locked_object; }

private:
	Lock lock;
	T* locked_object;
};

template <typename T, typename Mutex = std::mutex>
struct Locked
{
public:
	template <typename U, typename Lock>
	friend struct LockedPtr;

	static_assert(!std::is_const_v<T>, "You shouldn't need a lock if your object is const");

	template <typename ... Args, typename = decltype(T(std::forward<Args>(std::declval<Args>())...))>
	explicit Locked(Args&& ... aArgs) noexcept(noexcept(T(std::forward<Args>(std::declval<Args>())...)))
		: object(std::forward<Args>(aArgs)...)
	{}

	Locked(const Locked&) = delete;
	Locked(Locked&&) = delete;
	Locked& operator=(const Locked&) = delete;
	Locked& operator=(Locked&&) = delete;

	using DefaultMutableLock = std::lock_guard<Mutex>;
	using DefaultConstLock = std::conditional_t<is_shared_mutex<Mutex>,
		std::shared_lock<Mutex>,
		DefaultMutableLock>;

	template <typename LockType>
	static constexpr bool is_valid_lock = std::is_constructible_v<LockType, Mutex&>;

	template <typename LockType = DefaultMutableLock, typename = std::enable_if_t<is_valid_lock<LockType>>>
	LockedPtr<T, LockType> lock_mutable() noexcept(noexcept(LockedPtr<T, LockType>(*this)))
	{
		static_assert(!is_shared_lock<LockType>,
			"A non-shared lock must be used to lock a shared mutex for writing");

		return LockedPtr<T, LockType>(*this);
	}

	template <template <typename MutexType, typename ...> typename LockTemplate, typename = std::enable_if_t<is_valid_lock<LockTemplate<Mutex>>>>
	LockedPtr<T, LockTemplate<Mutex>> lock_mutable() noexcept(noexcept(lock_mutable<LockTemplate<Mutex>>()))
	{
		return lock_mutable<LockTemplate<Mutex>>();
	}

	template <typename LockType = DefaultConstLock, typename = std::enable_if_t<is_valid_lock<LockType>>>
	LockedPtr<const T, LockType> lock_const() const noexcept(noexcept(LockedPtr<const T, LockType>(*this)))
	{
		static_assert(!is_shared_mutex<Mutex> || is_shared_lock<LockType>,
			"A shared lock should be used to lock a shared mutex for reading, in order to allow other concurrent readers");

		return LockedPtr<const T, LockType>(*this);
	}

	template <template <typename MutexType, typename ...> typename LockTemplate>
	LockedPtr<const T, LockTemplate<Mutex>> lock_const() const noexcept(noexcept(lock_const<LockTemplate<Mutex>>()))
	{
		return lock_const<LockTemplate<Mutex>>();
	}

private:
	T object;
	mutable Mutex mutex;
};
