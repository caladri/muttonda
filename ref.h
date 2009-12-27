#ifndef	REF_H
#define	REF_H

/*
 * XXX
 * Allow for conversion from Ref<T> to Ref<const T> through some good
 * mechanism, so that we can mark const-typed references better.
 */
template<typename T>
class Ref {
	class RefObj {
		T *ptr_;
		unsigned count_;
	public:
		RefObj(T *ptr)
		: ptr_(ptr),
		  count_(1)
		{ }

	private:
		~RefObj()
		{
			delete ptr_;
			ptr_ = NULL;
		}

	public:
		void hold(void)
		{
			count_++;
		}

		void drop(void)
		{
			if (count_-- == 1)
				delete this;
		}

		T *get(void) const
		{
			return (ptr_);
		}
	};

	RefObj *obj_;
public:
	Ref(void)
	: obj_(NULL)
	{ }

	Ref(T *ptr)
	: obj_(new RefObj(ptr))
	{ }

	Ref(const Ref& ref)
	: obj_(ref.obj_)
	{
		if (obj_ != NULL)
			obj_->hold();
	}

	~Ref()
	{
		if (obj_ != NULL) {
			obj_->drop();
			obj_ = NULL;
		}
	}

	const Ref& operator= (const Ref& ref)
	{
		if (obj_ != NULL) {
			obj_->drop();
			obj_ = NULL;
		}

		if (ref.obj_ != NULL) {
			obj_ = ref.obj_;
			obj_->hold();
		}
		return (*this);
	}

	T *operator-> (void) const
	{
		return (obj_->get());
	}

	const T& operator* (void) const
	{
		T *ptr = obj_->get();
		return (*ptr);
	}

	bool null(void) const
	{
		return (obj_ == NULL);
	}
};

#endif /* !REF_H */
