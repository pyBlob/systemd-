// Copyright 2017-2017 Karl Kraus. See LICENSE for legal info.

#pragma once

#include <systemd/sd-event.h>
#include <string>
#include <functional>

#include <systemd++/sd-error.h>

namespace sd
{

class event
{
public:
	sd_event *_event = nullptr;

	event();
	event(sd_event* event);
	event(const event& other);
	event(event&& other);
	~event();

	event& operator=(const event&);
	event& operator=(event&&);

	void add_signal(int signal);

	void prepare();
	void wait(uint64_t usec);
	void dispatch();
	void run(uint64_t usec);
	void loop();
	void exit(int code);

	uint64_t now(clockid_t clock);

	int get_fd();
	int get_state();
	pid_t get_tid();
	int get_exit_code();
	bool set_watchdog(bool b);
	bool get_watchdog();
	uint64_t get_iteration();
};

namespace event_source
{

class _base
{
protected:
	_base();

	sd_event_source *_event_source = nullptr;

public:
	virtual ~_base();

	_base(const _base&) = delete;
	_base(_base&&) = delete;

	_base& operator=(const _base&) = delete;
	_base& operator=(_base&&) = delete;

	event get_event();
	void* get_userdata();
	void* set_userdata(void *userdata);

	void set_description(const std::string& description);
	std::string get_description();
	void set_prepare(sd_event_handler_t callback);
	bool get_pending();
	int64_t get_priority();
	void set_priority(int64_t priority);
	int get_enabled();
	void set_enabled(int enabled);
};

template <typename... Args>
struct _handler
{

	using CallbackFn =
		int(*)(sd_event_source*, Args..., void*);
	using Callback =
		std::function<void(Args...)>;

	static int call(sd_event_source* _es, Args... args, void* userdata)
	{
		try { (*(Callback*) userdata)(args...); }
		catch (const error& e) { return e.code; }
		return 0;
	}

	template <typename... InitArgs>
	struct _add
	{

		using InitFn =
			int(*)(sd_event*, sd_event_source**, InitArgs..., CallbackFn, void*);

		template<InitFn initFn>
		class _wrapper :
			public _base
		{
		private:
			Callback cb;

		protected:
			_wrapper(Callback _cb, InitArgs... initArgs) :
				cb(_cb)
			{
				event e;
				int r = initFn(e._event,
					&_event_source,
					initArgs...,
					call,
					&cb
				);
				if (r < 0) throw error(r);
			}

			template<typename Callee>
			_wrapper(Callee* __obj, InitArgs... initArgs) :
				_wrapper(
					[__obj](Args... args){ (*__obj)(args...); },
					initArgs...
				)
			{
			}

			template<typename Callee, typename Callable>
			_wrapper(Callee* __obj, Callable __f, InitArgs... initArgs) :
				_wrapper(
					[__obj, __f](Args... args){ (__obj->*__f)(args...); },
					initArgs...
				)
			{
			}
		
		public:
			virtual ~_wrapper() = default;
		};

		template <InitFn initFn>
		class event_source :
			public _wrapper<initFn>
		{
		public:
			using _wrapper<initFn>::_wrapper;
		};

	};

};

using __io = _handler<int, uint32_t>::_add<int, uint32_t>;
using io = __io::event_source<sd_event_add_io>;

template<>
template<>
template<>
class __io::event_source<sd_event_add_io> :
	public __io::_wrapper<sd_event_add_io>
{
public:
	using __io::_wrapper<sd_event_add_io>::_wrapper;

	int get_io_fd();
	void set_io_fd(int fd);
	uint32_t get_io_events();
	void set_io_events(uint32_t events);
	uint32_t get_io_revents();
};

using __time = _handler<uint64_t>::_add<clockid_t, uint64_t, uint64_t>;
using time = __time::event_source<sd_event_add_time>;

template<>
template<>
template<>
class __time::event_source<sd_event_add_time> :
	public __time::_wrapper<sd_event_add_time>
{
public:
	event_source(Callback cb, clockid_t clock, uint64_t usec, uint64_t accuracy = 0) :
		_wrapper(cb, clock, usec, accuracy)
	{
	}

	event_source(Callback cb, clockid_t clock = CLOCK_MONOTONIC) :
		event_source(cb, clock, event().now(clock))
	{
	}

	/*
	template<typename Callee, typename... InitArgs>
	event_source(Callee* __obj, InitArgs... initArgs) :
		event_source(
			[__obj](Args... args){ (*__obj)(args...); },
			initArgs...
		)
	{
	}
	*/

	template<typename Callee, typename... InitArgs>
	event_source(Callee* __obj, InitArgs... initArgs) :
		event_source(
			[__obj](uint64_t usec){ (*__obj)(usec); },
			initArgs...
		)
	{
	}

	template<typename Callee, typename Callable, typename... InitArgs, typename... Args>
	event_source(Callee* __obj, Callable __f, InitArgs... initArgs) :
		event_source(
			[__obj, __f](uint64_t usec){ (__obj->*__f)(usec); },
			initArgs...
		)
	{
	}

	uint64_t get_time();
	void set_time(uint64_t usec);
	uint64_t get_time_accuracy();
	void set_time_accuracy(uint64_t usec);
	clockid_t get_time_clock();
};

using __signal = _handler<const signalfd_siginfo*>::_add<int>;
using signal = __signal::event_source<sd_event_add_signal>;

template<>
template<>
template<>
class __signal::event_source<sd_event_add_signal> :
	public __signal::_wrapper<sd_event_add_signal>
{
public:
	using __signal::_wrapper<sd_event_add_signal>::_wrapper;

	int get_signal();
};

using __child = _handler<const siginfo_t*>::_add<pid_t, int>;
using child = __child::event_source<sd_event_add_child>;

template<>
template<>
template<>
class __child::event_source<sd_event_add_child> :
	public __child::_wrapper<sd_event_add_child>
{
public:
	using __child::_wrapper<sd_event_add_child>::_wrapper;

	pid_t get_child_pid();
};

using __static = _handler<>::_add<>;
using defer = __static::event_source<sd_event_add_defer>;
using post = __static::event_source<sd_event_add_post>;
using exit = __static::event_source<sd_event_add_exit>;

}; // namespace event_source

}; // namespace sd
