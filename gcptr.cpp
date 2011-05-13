#include "gcptr.h"

#define	NOMINMAX
#include <windows.h>

#pragma warning(disable : 4786)
#include <set>
#include <map>
#include <stdexcept>
#include <limits>

namespace
{
	typedef std::set<gc_detail::pointer_base*> ptr_set;
	typedef std::map<void*, gc_detail::node_t*> node_map;

	struct data_t
	{
		data_t() : threshold(1024), allocated(0), collecting(false), current_mark(false)
			{ InitializeCriticalSection(&cs); }
		~data_t() { gc_collect(); DeleteCriticalSection(&cs); }

		ptr_set pointers;
		node_map nodes;
		size_t threshold;
		size_t allocated;
		bool collecting;
		bool current_mark;
		CRITICAL_SECTION cs;
	};

	data_t& data() { static data_t instance; return instance; }

	struct data_lock
	{
		data_lock() { EnterCriticalSection(&data().cs); }
		~data_lock() { LeaveCriticalSection(&data().cs); }
	};

	void mark(gc_detail::pointer_base* ptr)
	{
		// Mark the node associated with the pointer and then recursively
		// mark all data().pointers contained by the object pointed to.
		gc_detail::node_t* node = ptr->node;
		if (node && node->mark != data().current_mark)
		{
			node->mark = data().current_mark;
			for (ptr_set::iterator i = data().pointers.begin(); i != data().pointers.end(); ++i)
			{
				gc_detail::pointer_base* next = *i;
				if (node->contains(next))
					mark(next);
			}
		}
	}
};

namespace gc_detail
{
	node_t::node_t(void* obj, int n) : object(obj), size(n), destroy(0), mark(data().current_mark)
	{
	}

	node_t::~node_t()
	{
		if (object && destroy)
			destroy(object);
	}

	bool node_t::contains(void* obj)
	{
		char* begin = static_cast<char*>(object);
		char* end = begin + size;
		return begin <= obj && obj < end;
	}

	pointer_base::pointer_base(node_t* node) : node(node)
	{
		data_lock lock;
		data().pointers.insert(this);
	}

	pointer_base::~pointer_base()
	{
		data_lock lock;
		data().pointers.erase(this);
	}

	node_t* get_node(void* obj, void (*destroy)(void*))
	{
		data_lock lock;

		if (!obj)
			return 0;

		node_map::iterator i = data().nodes.find(obj);
		if (i == data().nodes.end())
			throw std::invalid_argument("Object was not created with new(gc)");

		node_t* n = i->second;
		if (n->destroy == 0)
			n->destroy = destroy;

		return n;
	}
};

void* operator new(size_t size, const gc_detail::gc_t&)
{
	data_lock lock;

	bool collected = false;

	if (data().threshold != std::numeric_limits<size_t>::max())
	{
		// Determine if we've exceeded the threshold and so should collect.
		data().allocated += size;
		if (data().allocated > data().threshold)
		{
			gc_collect();
			collected = true;
		}
	}

	// Attempt the first allocation.  The standard requires new to throw
	// on failure but user code may change this behavior and VC++ by default
	// only returns 0.  We'll catch exceptions and if we've already collected
	// re-throw the exception.
	void* obj = 0;
	try { obj = ::operator new(size); } catch(...) { if (collected) throw; }

	// If we've failed to allocate but new didn't throw an exception and we've
	// not collected yet we'll collect and then re-try calling new.  If new
	// throws at this point we'll ignore it and let the caller handle it.
	if (obj == 0 && !collected)
	{
		gc_collect();
		obj = ::operator new(size);
	}

	// If we actually allocated memory with new then we need to add it to
	// the node map.
	if (obj != 0)
		data().nodes.insert(node_map::value_type(obj, new gc_detail::node_t(obj, size)));

	return obj;
}

void operator delete(void* obj, const gc_detail::gc_t&)
{
	data_lock lock;

	node_map::iterator i = data().nodes.find(obj);
	if (i != data().nodes.end())
		i->second->object = 0;
	::operator delete(obj);
}

void gc_collect()
{
	data_lock lock;

	// During the sweep phase we'll be deleting objects that could cause
	// a recursive call to 'collect' which would cause invalid results.  So
	// we prevent recursion here.
	if (data().collecting)
		return;

	data().collecting = true;

	// Toggle the 'current_mark' so that we can start over.
	data().current_mark = !data().current_mark;

	{	// Mark phase
		// Loop through all of the pointers looking for 'root' pointers.  A 'root'
		// pointer is a pointer that's not contained within the object pointed
		// to by any other pointer.  When a 'root' pointer is found it is
		// marked, and all the pointers referenced through the 'root' pointer
		// are also marked.
		for (ptr_set::iterator i = data().pointers.begin(); i != data().pointers.end(); ++i)
		{
			gc_detail::pointer_base* ptr = *i;
			if (!ptr->node || ptr->node->mark == data().current_mark)
				continue;	// Don't need to check pointers that are marked.

			bool root = true;
			for (node_map::iterator j = data().nodes.begin(); j != data().nodes.end(); ++j)
			{
				gc_detail::node_t* node = j->second;
				if (node->contains(ptr))
				{
					root = false;
					break;	// If any other pointer contains this pointer we're not a root.
				}
			}

			if (root)
				mark(ptr);
		}
	}

	{	// Sweep phase
		// Step through all of the nodes and delete any that are not marked.
		for (node_map::iterator i = data().nodes.begin(); i != data().nodes.end(); /*nothing*/)
		{
			gc_detail::node_t* node = i->second;
			if (node->mark != data().current_mark)
			{
				delete node;
				i = data().nodes.erase(i);
			}
			else
				++i;
		}
	}

	data().collecting = false;
	data().allocated = 0;
}

void gc_set_threshold(size_t bytes)
{
	data_lock lock;
	data().threshold = bytes;
}
