#if !defined(_GCPTR_H_68F9569A_2697_45DD_B71D_E1F77D95E40E_INCLUDED_)
#define _GCPTR_H_68F9569A_2697_45DD_B71D_E1F77D95E40E_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

namespace gc_detail
{
	struct gc_t
	{
	};

	struct node_t;

	struct pointer_base
	{
		pointer_base(node_t* h);

		node_t* node;

	protected:
		~pointer_base();
	};

	struct node_t
	{
		node_t(void* obj, int n);
		~node_t();
		bool contains(void* obj);

		void* object;
		int size;
		bool mark;
		void (*destroy)(void*);
	};

	template <typename T>
	struct destructor
	{
		static void destroy(void* obj) { delete static_cast<T*>(obj); }
	};

	node_t* get_node(void* obj, void (*destroy)(void*));

	template <typename T>
	node_t* get_node(T* obj)
	{
		return get_node(obj, destructor<T>::destroy);
	}
};	// namespace gc_detail

namespace
{
	gc_detail::gc_t gc;
};

void* operator new(size_t size, const gc_detail::gc_t&);
void operator delete(void* p, const gc_detail::gc_t&);

void gc_collect();
void gc_set_threshold(size_t bytes);

template <typename T>
class gc_ptr : public gc_detail::pointer_base
{
public:
	gc_ptr() : gc_detail::pointer_base(0) { }
	explicit gc_ptr(T* p) : gc_detail::pointer_base(gc_detail::get_node(p)) { }
	template <typename U>
		explicit gc_ptr(const gc_ptr<U>& other) : gc_detail::pointer_base(other.node) { }

	gc_ptr<T>& operator=(T* p) { node = gc_detail::get_handle(p); return *this; }
	template <typename U>
		gc_ptr<T>& operator=(const gc_ptr<U>& other) { node = other.node; return *this; }

	T* get() const { return node ? static_cast<T*>(node->object) : 0; }
	T& operator*() const { return *get(); }
	T* operator->() const { return get(); }
};

#endif // !defined(_GCPTR_H_68F9569A_2697_45DD_B71D_E1F77D95E40E_INCLUDED_)
