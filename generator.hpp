////////////////////////////////////////////////////////////////
// Reference implementation of std::generator proposal P2168R0.
//
// Incorporates a suggested change to require use of std::elements_of(range)
// when yielding elements of a child range from a generator.
//
// This now also supports yielding an arbitrary range/view as long
// as the elements of that range are convertible to the current
// generator's reference type.

#if __has_include(<coroutine>)
#include <coroutine>
#else
#include <experimental/coroutine>
namespace std {
using std::experimental::coroutine_handle;
using std::experimental::noop_coroutine;
using std::experimental::suspend_always;
using std::experimental::suspend_never;
} // namespace std
#endif

#include <exception>
#include <iterator>
#include <new>
#include <type_traits>
#include <utility>
#include <ranges>


template <typename T>
class __manual_lifetime {
  public:
    __manual_lifetime() noexcept {
    }
    ~__manual_lifetime() {
    }

    template <typename... Args>
    T &construct(Args &&...args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        return *::new (static_cast<void *>(std::addressof(value_)))
            T((Args &&) args...);
    }

    void destruct() noexcept(std::is_nothrow_destructible_v<T>) {
        value_.~T();
    }

    T &get() &noexcept {
        return value_;
    }
    T &&get() &&noexcept {
        return (T &&) value_;
    }
    const T &get() const &noexcept {
        return value_;
    }
    const T &&get() const &&noexcept {
        return (const T &&)value_;
    }

  private:
    union {
        std::remove_const_t<T> value_;
    };
};

template <typename T>
class __manual_lifetime<T &> {
  public:
    __manual_lifetime() noexcept : value_(nullptr) {
    }
    ~__manual_lifetime() {
    }

    T &construct(T &value) noexcept {
        value_ = std::addressof(value);
        return value;
    }

    template <typename Func>
    T &construct_from(Func &&func) noexcept(std::is_nothrow_invocable_v<Func>) {
        static_assert(std::is_same_v<std::invoke_result_t<Func>, T &>);
        value_ = std::addressof(((Func &&) func)());
        return get();
    }

    void destruct() noexcept {
    }

    T &get() const noexcept {
        return *value_;
    }

  private:
    T *value_;
};

template <typename T>
class __manual_lifetime<T &&> {
  public:
    __manual_lifetime() noexcept : value_(nullptr) {
    }
    ~__manual_lifetime() {
    }

    T &&construct(T &&value) noexcept {
        value_ = std::addressof(value);
        return (T &&) value;
    }

    template <typename Func>
    T &&construct_from(Func &&func) noexcept(std::is_nothrow_invocable_v<Func>) {
        static_assert(std::is_same_v<std::invoke_result_t<Func>, T &&>);
        value_ = std::addressof(((Func &&) func)());
        return get();
    }

    void destruct() noexcept {
    }

    T &&get() const noexcept {
        return (T &&) * value_;
    }

  private:
    T *value_;
};

// template<typename T>
// struct elements_of;

template <typename R>
struct elements_of {
    R &&__range; // \expos

    explicit constexpr elements_of(R &&r) noexcept : __range((R &&) r) {
    }
    constexpr elements_of(elements_of &&) noexcept = default;

    constexpr elements_of(const elements_of &) = delete;
    constexpr elements_of &operator=(const elements_of &) = delete;
    constexpr elements_of &operator=(elements_of &&) = delete;

    constexpr operator R &&() &&noexcept {
        return std::forward<R>(__range);
    }
};

template <typename R>
elements_of(R &&) -> elements_of<R>;

template <typename Alloc>
static constexpr bool allocator_needs_to_be_stored =
    !std::allocator_traits<Alloc>::is_always_equal::value ||
    !std::is_default_constructible_v<Alloc>;

// Round s up to next multiple of a.
constexpr size_t aligned_allocation_size(size_t s, size_t a) {
    return (s + a - 1) & ~(a - 1);
}

template<typename Alloc = std::allocator<std::byte>>
class promise_base_type;

template<typename Alloc>
class promise_base_type {
    using char_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;

    static constexpr std::size_t offset_of_allocator(std::size_t frameSize) {
        return aligned_allocation_size(frameSize, alignof(char_allocator));
    }

    static constexpr std::size_t padded_frame_size(std::size_t frameSize) {
        return offset_of_allocator(frameSize) + sizeof(char_allocator);
    }

    static char_allocator& get_allocator(void* frame, std::size_t frameSize) {
        return *reinterpret_cast<char_allocator*>(
            static_cast<char*>(frame) + offset_of_allocator(frameSize));
    }

public:
    template<typename... Args>
    static void* operator new(std::size_t frameSize, std::allocator_arg_t, Alloc& alloc, Args&...) {
        char_allocator localAlloc(alloc);
        void* frame = alloc.allocate(padded_frame_size(frameSize));

        // Store allocator at end of the coroutine frame.
        // Assuming the allocator's move constructor is non-throwing (a requirement for allocators)
        ::new (static_cast<void*>(std::addressof(get_allocator(frame, frameSize)))) char_allocator(std::move(localAlloc));

        return frame;
    }

    template<typename This, typename... Args>
    static void* operator new(std::size_t frameSize, This&, std::allocator_arg_t, Alloc& alloc, Args&...) {
        return promise_base_type::operator new(frameSize, std::allocator_arg, alloc);
    }

    static void operator delete(void* ptr, std::size_t frameSize) {
        char_allocator& alloc = get_allocator(ptr, frameSize);
        char_allocator localAlloc(std::move(alloc));
        alloc.~char_allocator();
        localAlloc.deallocate(static_cast<char*>(ptr), padded_frame_size(frameSize));
    }
};

template<typename Alloc>
    requires (!allocator_needs_to_be_stored<Alloc>)
class promise_base_type<Alloc> {
    using char_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<char>;
public:
    static void* operator new(std::size_t size) {
        char_allocator alloc;
        return alloc.allocate(size);
    }

    static void operator delete(void* ptr, std::size_t size) {
        char_allocator alloc;
        alloc.deallocate(static_cast<char*>(ptr), size);
    }
};

namespace recursive {

template <typename Ref, typename Value = std::remove_cvref_t<Ref>, typename Alloc = std::allocator<std::byte>>
class generator {
  public:
    class promise_type : public promise_base_type<Alloc> {
      public:
        promise_type() noexcept
            : rootOrLeaf_(
                  std::coroutine_handle<promise_type>::from_promise(*this))
        {
        }

        generator get_return_object() noexcept {
            return generator{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        void unhandled_exception() {
            if (exception_ == nullptr)
                throw;
            *exception_ = std::current_exception();
        }

        void return_void() noexcept {
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }
        // Transfers control back to the parent of a nested coroutine
        struct final_awaiter {
            bool await_ready() noexcept {
                return false;
            }
            std::coroutine_handle<>
            await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                auto &promise = h.promise();
                std::coroutine_handle<promise_type> parent = promise.parent_;
                if (parent) {
                    auto &root = promise.rootOrLeaf_.promise();
                    root.rootOrLeaf_ = parent;
                    return parent;
                }
                return std::noop_coroutine();
            }
            void await_resume() noexcept {
            }
        };

        final_awaiter final_suspend() noexcept {
            return {};
        }

        std::suspend_always yield_value(Ref &&x) noexcept(
            std::is_nothrow_move_constructible_v<Ref>) {
            auto &root = rootOrLeaf_.promise();
            root.value_.construct((Ref &&) x);
            return {};
        }

        template <typename T>
        requires(!std::is_reference_v<Ref>) &&
            std::is_convertible_v<T, Ref> std::suspend_always yield_value(
                T &&x) noexcept(std::is_nothrow_constructible_v<Ref, T>) {
            auto &root = rootOrLeaf_.promise();
            root.value_.construct((T &&) x);
            return {};
        }

        struct yield_sequence_awaiter {
            using promise_type = generator::promise_type;

            generator gen_;
            std::exception_ptr exception_;

            yield_sequence_awaiter(generator &&g) noexcept
                // Taking ownership of the generator ensures frame are destroyed
                // in the reverse order of their creation
                : gen_(std::move(g)) {
            }

            bool await_ready() noexcept {
                return !gen_.coro_;
            }

            // set the parent, root and exceptions pointer and
            // resume the nested coroutine
            std::coroutine_handle<>
            await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                auto &current = h.promise();
                auto &nested = gen_.coro_.promise();
                auto &root = current.rootOrLeaf_.promise();

                nested.rootOrLeaf_ = current.rootOrLeaf_;
                root.rootOrLeaf_ = gen_.coro_;
                nested.parent_ = h;

                nested.exception_ = &exception_;

                // Immediately resume the nested coroutine (nested generator)
                return gen_.coro_;
            }

            void await_resume() {
                if (exception_) {
                    std::rethrow_exception(std::move(exception_));
                }
            }
        };

        yield_sequence_awaiter yield_value(elements_of<generator> g) noexcept {
            return yield_sequence_awaiter{(generator &&) std::move(g)};
        }

        // Adapt any std::elements_of() range that
        template <std::ranges::range R>
        // requires std::convertible_to<std::ranges::range_reference_t<R>, Ref>
        yield_sequence_awaiter yield_value(elements_of<R> r) {
           R &&range = std::move(r);
           for (auto &&v : range)
                co_yield v;
        }

        void resume() {
            rootOrLeaf_.resume();
        }

        // Disable use of co_await within this coroutine.
        void await_transform() = delete;

      private:
        friend generator;
        std::coroutine_handle<promise_type> rootOrLeaf_;
        std::coroutine_handle<promise_type> parent_;
        std::exception_ptr *exception_ = nullptr;
        __manual_lifetime<Ref> value_;
    };

    generator() noexcept = default;

    generator(generator &&other) noexcept
        : coro_(std::exchange(other.coro_, {})),
          started_(std::exchange(other.started_, false)) {
    }

    ~generator() noexcept {
        if (coro_) {
            if (started_ && !coro_.done()) {
                coro_.promise().value_.destruct();
            }
            coro_.destroy();
        }
    }

    generator &operator=(generator g) noexcept {
        swap(g);
        return *this;
    }

    void swap(generator &other) noexcept {
        std::swap(coro_, other.coro_);
        std::swap(started_, other.started_);
    }

    struct sentinel {};

    class iterator {
        using coroutine_handle = std::coroutine_handle<promise_type>;

      public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Value;
        using reference = Ref;
        using pointer = std::add_pointer_t<Ref>;

        iterator() noexcept = default;
        iterator(const iterator &) = delete;

        iterator(iterator &&o) noexcept : coro_(std::exchange(o.coro_, {})) {
        }

        iterator &operator=(iterator &&o) {
            std::swap(coro_, o.coro_);
            return *this;
        }

        ~iterator() {
        }

        friend bool operator==(const iterator &it, sentinel) noexcept {
            return !it.coro_ || it.coro_.done();
        }

        iterator &operator++() {
            coro_.promise().value_.destruct();
            coro_.promise().resume();
            return *this;
        }
        void operator++(int) {
            (void)operator++();
        }

        reference operator*() const noexcept {
            return static_cast<reference>(coro_.promise().value_.get());
        }

        pointer
        operator->() const noexcept requires std::is_reference_v<reference> {
            return std::addressof(operator*());
        }

      private : friend generator;
        explicit iterator(coroutine_handle coro) noexcept : coro_(coro) {
        }

        coroutine_handle coro_;
    };

    iterator begin() {
        if (coro_) {
            started_ = true;
            coro_.resume();
        }
        return iterator{coro_};
    }

    sentinel end() noexcept {
        return {};
    }

  private:
    explicit generator(std::coroutine_handle<promise_type> coro) noexcept
        : coro_(coro) {
    }

    std::coroutine_handle<promise_type> coro_;
    bool started_ = false;
};

}

#if __has_include(<ranges>)
template <typename T, typename U>
constexpr inline bool std::ranges::enable_view<recursive::generator<T, U>> = true;
#endif


namespace simple {

template <typename Ref, typename Value = std::remove_cvref_t<Ref>, typename Alloc = std::allocator<std::byte>>
class generator {
  public:
    class promise_type : public promise_base_type<Alloc> {
      public:
        promise_type() noexcept
        {
        }

        generator get_return_object() noexcept {
            return generator{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        void unhandled_exception() {
            if (exception_ == nullptr)
                throw;
            *exception_ = std::current_exception();
        }

        void return_void() noexcept {
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }
        std::suspend_always yield_value(Ref &&x) noexcept(
            std::is_nothrow_move_constructible_v<Ref>) {
            value_.construct((Ref &&) x);
            return {};
        }

        template <typename T>
        requires(!std::is_reference_v<Ref>) && std::is_convertible_v<T, Ref> std::suspend_always yield_value(
                T &&x) noexcept(std::is_nothrow_constructible_v<Ref, T>) {
            value_.construct((T &&) x);
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void resume() {
            std::coroutine_handle<promise_type>::from_promise(*this).resume();
        }

        // Disable use of co_await within this coroutine.
        void await_transform() = delete;

      private:
        friend generator;
        std::exception_ptr *exception_ = nullptr;
        __manual_lifetime<Ref> value_;
    };

    generator() noexcept = default;

    generator(generator &&other) noexcept
        : coro_(std::exchange(other.coro_, {})),
          started_(std::exchange(other.started_, false)) {
    }

    ~generator() noexcept {
        if (coro_) {
            if (started_ && !coro_.done()) {
                coro_.promise().value_.destruct();
            }
            coro_.destroy();
        }
    }

    generator &operator=(generator g) noexcept {
        swap(g);
        return *this;
    }

    void swap(generator &other) noexcept {
        std::swap(coro_, other.coro_);
        std::swap(started_, other.started_);
    }

    struct sentinel {};

    class iterator {
        using coroutine_handle = std::coroutine_handle<promise_type>;

      public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Value;
        using reference = Ref;
        using pointer = std::add_pointer_t<Ref>;

        iterator() noexcept = default;
        iterator(const iterator &) = delete;

        iterator(iterator &&o) noexcept : coro_(std::exchange(o.coro_, {})) {
        }

        iterator &operator=(iterator &&o) {
            std::swap(coro_, o.coro_);
            return *this;
        }

        ~iterator() {
        }

        friend bool operator==(const iterator &it, sentinel) noexcept {
            return !it.coro_ || it.coro_.done();
        }

        iterator &operator++() {
            coro_.promise().value_.destruct();
            coro_.promise().resume();
            return *this;
        }
        void operator++(int) {
            (void)operator++();
        }

        reference operator*() const noexcept {
            return static_cast<reference>(coro_.promise().value_.get());
        }

        pointer
        operator->() const noexcept requires std::is_reference_v<reference> {
            return std::addressof(operator*());
        }

      private : friend generator;
        explicit iterator(coroutine_handle coro) noexcept : coro_(coro) {
        }

        coroutine_handle coro_;
    };

    iterator begin() {
        if (coro_) {
            started_ = true;
            coro_.resume();
        }
        return iterator{coro_};
    }

    sentinel end() noexcept {
        return {};
    }

  private:
    explicit generator(std::coroutine_handle<promise_type> coro) noexcept
        : coro_(coro) {
    }

    std::coroutine_handle<promise_type> coro_;
    bool started_ = false;
};

}

#if __has_include(<ranges>)
template <typename T, typename U>
constexpr inline bool std::ranges::enable_view<simple::generator<T, U>> = true;
#endif