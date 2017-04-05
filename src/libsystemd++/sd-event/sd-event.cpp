// Copyright 2017-2017 Karl Kraus. See LICENSE for legal info.

#include <systemd++/sd-event.h>

namespace sd
{

event::event()
{
	int r;

	r = sd_event_default(&_event);
	if (r < 0)
	{
		_event = sd_event_unref(_event);
		throw error(r);
	}
}
event::event(sd_event* event) :
	_event(event)
{
	sd_event_ref(_event);
}
event::event(const event& other) :
	_event(other._event)
{
	sd_event_ref(_event);
}
event::event(event&& other) :
	_event(std::exchange(other._event, nullptr))
{
}
event::~event()
{
	_event = sd_event_unref(_event);
}

event& event::operator=(const event& other)
{
	sd_event_unref(_event);

	_event = other._event;
	sd_event_ref(_event);
}

event& event::operator=(event&& other)
{
	sd_event_unref(_event);
	_event = std::exchange(other._event, nullptr);
}

void event::add_signal(
	int signal
)
{
	int r;

	r = sd_event_add_signal(_event, nullptr, signal, nullptr, nullptr);
	if (r < 0) throw error(r);
}

void event::prepare()
{
	int r;

	r = sd_event_prepare(_event);
	if (r < 0) throw error(r);
}
void event::wait(uint64_t usec)
{
	int r;

	r = sd_event_wait(_event, usec);
	if (r < 0) throw error(r);
}
void event::dispatch()
{
	int r;

	r = sd_event_dispatch(_event);
	if (r < 0) throw error(r);
}
void event::run(uint64_t usec)
{
	int r;

	r = sd_event_run(_event, usec);
	if (r < 0) throw error(r);
}
void event::loop()
{
	int r;

	r = sd_event_loop(_event);
	if (r < 0) throw error(r);
}
void event::exit(int code)
{
	int r;

	r = sd_event_exit(_event, code);
	if (r < 0) throw error(r);
}

uint64_t event::now(clockid_t clock)
{
	int r;

	uint64_t usec;
	r = sd_event_now(_event, clock, &usec);
	if (r < 0) throw error(r);

	return usec;
}

int event::get_fd()
{
	int r;

	r = sd_event_get_fd(_event);
	if (r < 0) throw error(r);

	return r;
}
int event::get_state()
{
	int r;

	r = sd_event_get_state(_event);
	if (r < 0) throw error(r);

	return r;
}
pid_t event::get_tid()
{
	int r;

	pid_t pid;
	r = sd_event_get_tid(_event, &pid);
	if (r < 0) throw error(r);

	return pid;
}
int event::get_exit_code()
{
	int r, code;

	r = sd_event_get_exit_code(_event, &code);
	if (r < 0) throw error(r);

	return code;
}
bool event::set_watchdog(bool b)
{
	int r;

	r = sd_event_set_watchdog(_event, b);
	if (r < 0) throw error(r);

	return r;
}
bool event::get_watchdog()
{
	int r;

	r = sd_event_get_watchdog(_event);
	if (r < 0) throw error(r);

	return r;
}
uint64_t event::get_iteration()
{
	int r;

	uint64_t iteration;
	r = sd_event_get_iteration(_event, &iteration);
	if (r < 0) throw error(r);

	return r;
}

namespace event_source
{

_base::_base() :
	_event_source(nullptr)
{
}
_base::~_base()
{
	_event_source = sd_event_source_unref(_event_source);
}

event _base::get_event()
{
	return sd_event_source_get_event(_event_source);
}
void* _base::get_userdata()
{
	return sd_event_source_get_userdata(_event_source);
}
void* _base::set_userdata(void *userdata)
{
	return sd_event_source_set_userdata(_event_source, userdata);
}

void _base::set_description(const std::string& description)
{
	int r;

	r = sd_event_source_set_description(_event_source, description.data());
	if (r < 0) throw error(r);
}
std::string _base::get_description()
{
	int r;

	const char* desc;
	r = sd_event_source_get_description(_event_source, &desc);
	if (r < 0) throw error(r);

	return desc;
}
void _base::set_prepare(sd_event_handler_t callback)
{
	int r;

	r = sd_event_source_set_prepare(_event_source, callback);
	if (r < 0) throw error(r);
}
bool _base::get_pending()
{
	int r;

	r = sd_event_source_get_pending(_event_source);
	if (r < 0) throw error(r);

	return r;
}
int64_t _base::get_priority()
{
	int r;

	int64_t priority;
	r = sd_event_source_get_priority(_event_source, &priority);
	if (r < 0) throw error(r);

	return priority;
}
void _base::set_priority(int64_t priority)
{
	int r;

	r = sd_event_source_set_priority(_event_source, priority);
	if (r < 0) throw error(r);
}
int _base::get_enabled()
{
	int r;

	int b;
	r = sd_event_source_get_enabled(_event_source, &b);
	if (r < 0) throw error(r);

	return b;
}
void _base::set_enabled(int enabled)
{
	int r;

	r = sd_event_source_set_enabled(_event_source, enabled);
	if (r < 0) throw error(r);
}

int io::get_io_fd()
{
	int r;

	r = sd_event_source_get_io_fd(_event_source);
	if (r < 0) throw error(r);

	return r;
}
void io::set_io_fd(int fd)
{
	int r;

	r = sd_event_source_set_io_fd(_event_source, fd);
	if (r < 0) throw error(r);
}
uint32_t io::get_io_events()
{
	int r;

	uint32_t events;
	r = sd_event_source_get_io_events(_event_source, &events);
	if (r < 0) throw error(r);

	return events;
}
void io::set_io_events(uint32_t events)
{
	int r;

	r = sd_event_source_set_io_events(_event_source, events);
	if (r < 0) throw error(r);
}
uint32_t io::get_io_revents()
{
	int r;

	uint32_t revents = 0;
	r = sd_event_source_get_io_revents(_event_source, &revents);
	if (r < 0 && r != -ENODATA) throw error(r);

	return revents;
}

uint64_t time::get_time()
{
	int r;

	uint64_t usec;
	r = sd_event_source_get_time(_event_source, &usec);
	if (r < 0) throw error(r);

	return usec;
}
void time::set_time(uint64_t usec)
{
	int r;

	r = sd_event_source_set_time(_event_source, usec);
	if (r < 0) throw error(r);
}
uint64_t time::get_time_accuracy()
{
	int r;

	uint64_t usec;
	r = sd_event_source_get_time_accuracy(_event_source, &usec);
	if (r < 0) throw error(r);

	return usec;
}
void time::set_time_accuracy(uint64_t usec)
{
	int r;

	r = sd_event_source_set_time_accuracy(_event_source, usec);
	if (r < 0) throw error(r);
}
clockid_t time::get_time_clock()
{
	int r;

	clockid_t clock;
	r = sd_event_source_get_time_clock(_event_source, &clock);
	if (r < 0) throw error(r);

	return r;
}

int signal::get_signal()
{
	int r;

	r = sd_event_source_get_signal(_event_source);
	if (r < 0) throw error(r);

	return r;
}

pid_t child::get_child_pid()
{
	int r;

	pid_t pid;
	r = sd_event_source_get_child_pid(_event_source, &pid);
	if (r < 0) throw error(r);

	return pid;
}

}; // namespace event_source

}; // namespace sd
